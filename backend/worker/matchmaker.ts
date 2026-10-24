// Pure matching logic: tickets in, match proposals out. No I/O, so it is unit-tested directly.
// Rules are canonical in docs/systems/game-modes-and-match-flow.md sections 6 and 9 and
// docs/tech/networking.md section 9; backend-architecture section 10.2 describes the loop.

export const PING_CAP_MS = 150;
export const STACK_MIX_AFTER_S = 60;
export const POOL_WIDEN_ADJACENT_S = 90;
export const POOL_WIDEN_ALL_S = 180;
export const BOT_HARNESS = "Rigger";

const CASUAL_POOLS = ["apprentice", "hand", "journeyman"];

export interface Member {
  account_id: string;
  harness: string;
  face: string | null;
  grade: number;
}

export interface Ticket {
  ticket_id: string;
  crew_id: string;
  playlist_id: string;
  grade_pool: string;
  build: string;
  members: Member[];
  pings_ms: Record<string, number>;
  created_at: Date;
}

export interface Playlist {
  playlist_id: string;
  team_size: number;
  teams: number;
  regions: string[];
  bot_fill_pools: string[];
  bot_fill_after_s: number;
}

export interface Slot {
  slot: number;
  team: number;
  account_id: string | null;
  is_bot: boolean;
  harness: string;
  face: string | null;
}

export interface Proposal {
  playlist_id: string;
  build: string;
  region: string;
  ticket_ids: string[];
  slots: Slot[];
}

interface Team {
  tickets: Ticket[];
  stack: boolean;
}

const waitedS = (t: Ticket, now: Date) => (now.getTime() - t.created_at.getTime()) / 1000;
const size = (tickets: Ticket[]) => tickets.reduce((n, t) => n + t.members.length, 0);
const oldest = (tickets: Ticket[]) => Math.min(...tickets.map((t) => t.created_at.getTime()));

function eligibleRegions(t: Ticket, playlist: Playlist): Set<string> {
  const reachable = Object.entries(t.pings_ms).filter(([region, ping]) =>
    typeof ping === "number" && ping >= 0 && ping <= PING_CAP_MS && playlist.regions.includes(region)
  );
  return new Set(reachable.map(([region]) => region));
}

// How many casual pools away a ticket may match, after waiting (game-modes 6.1).
function reach(t: Ticket, now: Date): number {
  if (t.grade_pool === "rated") return 0;
  const w = waitedS(t, now);
  if (w >= POOL_WIDEN_ALL_S) return CASUAL_POOLS.length;
  if (w >= POOL_WIDEN_ADJACENT_S) return 1;
  return 0;
}

export function poolsCompatible(a: Ticket, b: Ticket, now: Date): boolean {
  if (a.grade_pool === b.grade_pool) return true;
  if (a.grade_pool === "rated" || b.grade_pool === "rated") return false;
  const ia = CASUAL_POOLS.indexOf(a.grade_pool);
  const ib = CASUAL_POOLS.indexOf(b.grade_pool);
  if (ia < 0 || ib < 0) return false;
  return Math.abs(ia - ib) <= Math.max(reach(a, now), reach(b, now));
}

function groupCompatible(tickets: Ticket[], now: Date): boolean {
  return tickets.every((a, i) => tickets.slice(i + 1).every((b) => poolsCompatible(a, b, now)));
}

function sharedRegions(tickets: Ticket[], playlist: Playlist): string[] {
  if (tickets.length === 0) return [];
  const [first, ...rest] = tickets.map((t) => eligibleRegions(t, playlist));
  return [...first].filter((region) => rest.every((regions) => regions.has(region))).sort();
}

// The shared region that minimizes the highest ping among all players (networking 9).
function bestRegion(tickets: Ticket[], regions: string[]): { region: string; worstPing: number } | null {
  const scored = regions.map((region) => ({ region, worstPing: Math.max(...tickets.map((t) => t.pings_ms[region])) }));
  return scored.reduce<{ region: string; worstPing: number } | null>(
    (best, r) => (best === null || r.worstPing < best.worstPing ? r : best),
    null,
  );
}

function fits(tickets: Ticket[], playlist: Playlist, now: Date): boolean {
  return groupCompatible(tickets, now) && sharedRegions(tickets, playlist).length > 0;
}

function botSlot(slot: number, team: number): Slot {
  return { slot, team, account_id: null, is_bot: true, harness: BOT_HARNESS, face: null };
}

function teamSlots(teams: Ticket[][], playlist: Playlist): Slot[] {
  return teams.flatMap((tickets, i) => {
    const team = i + 1;
    const members = tickets.flatMap((t) => t.members);
    return Array.from({ length: playlist.team_size }, (_, k): Slot => {
      const slot = i * playlist.team_size + k + 1;
      const m = members[k];
      return m
        ? { slot, team, account_id: m.account_id, is_bot: false, harness: m.harness, face: m.face }
        : botSlot(slot, team);
    });
  });
}

function propose(teams: Ticket[][], playlist: Playlist, slots: Slot[]): Proposal {
  const all = teams.flat();
  const region = bestRegion(all, sharedRegions(all, playlist))!.region;
  return {
    playlist_id: playlist.playlist_id,
    build: all[0].build,
    region,
    ticket_ids: all.map((t) => t.ticket_id),
    slots,
  };
}

const idsOf = (tickets: Ticket[]) => tickets.map((t) => t.ticket_id);

// 1. Teams: a full crew is a stack; partial crews combine (game-modes 6.2).
function assembleTeams(sorted: Ticket[], playlist: Playlist, now: Date): Team[] {
  const stacks: Team[] = sorted
    .filter((t) => t.members.length === playlist.team_size)
    .map((t) => ({ tickets: [t], stack: true }));
  const partial = sorted.filter((t) => t.members.length < playlist.team_size);

  let used = new Set<string>();
  let mixed: Team[] = [];
  for (const seed of partial) {
    if (used.has(seed.ticket_id)) continue;
    const others = partial.filter((c) => c !== seed && !used.has(c.ticket_id));
    const combo = completeTeam([seed], others, playlist, now);
    if (!combo) continue;
    used = new Set([...used, ...idsOf(combo)]);
    mixed = [...mixed, { tickets: combo, stack: false }];
  }
  return [...stacks, ...mixed];
}

// Oldest-first depth-first search for tickets that fill the team exactly.
function completeTeam(current: Ticket[], candidates: Ticket[], playlist: Playlist, now: Date): Ticket[] | null {
  const have = size(current);
  if (have === playlist.team_size) return current;
  for (const [i, c] of candidates.entries()) {
    if (have + c.members.length > playlist.team_size) continue;
    const next = [...current, c];
    if (!fits(next, playlist, now)) continue;
    const done = completeTeam(next, candidates.slice(i + 1), playlist, now);
    if (done) return done;
  }
  return null;
}

// The opponent with the lowest worst ping. A stack meets a mixed team only after the stack
// has waited 60 s (game-modes 6.2).
function bestOpponent(team: Team, candidates: Team[], playlist: Playlist, now: Date): Team | null {
  let best: { team: Team; worstPing: number } | null = null;
  for (const u of candidates) {
    const all = [...team.tickets, ...u.tickets];
    if (!groupCompatible(all, now)) continue;
    const region = bestRegion(all, sharedRegions(all, playlist));
    if (!region) continue;
    const stack = team.stack ? team : u;
    const mixesStackAndTeam = team.stack !== u.stack;
    if (mixesStackAndTeam && (now.getTime() - oldest(stack.tickets)) / 1000 < STACK_MIX_AFTER_S) continue;
    if (best === null || region.worstPing < best.worstPing) best = { team: u, worstPing: region.worstPing };
  }
  return best?.team ?? null;
}

// 2. Pair teams, oldest first.
function pairTeams(teams: Team[], playlist: Playlist, now: Date): Proposal[] {
  const order = [...teams].sort((a, b) => oldest(a.tickets) - oldest(b.tickets));
  let used = new Set<Team>();
  let proposals: Proposal[] = [];
  for (const t of order) {
    if (used.has(t)) continue;
    const opponent = bestOpponent(t, order.filter((u) => u !== t && !used.has(u)), playlist, now);
    if (!opponent) continue;
    used = new Set([...used, t, opponent]);
    const teamTickets = [t.tickets, opponent.tickets];
    proposals = [...proposals, propose(teamTickets, playlist, teamSlots(teamTickets, playlist))];
  }
  return proposals;
}

// Places a ticket on the emptier side that keeps the match valid, or returns null.
function placeOnSide(sides: Ticket[][], c: Ticket, playlist: Playlist, now: Date): Ticket[][] | null {
  const order = size(sides[1]) <= size(sides[0]) ? [1, 0] : [0, 1];
  for (const s of order) {
    if (size(sides[s]) + c.members.length > playlist.team_size) continue;
    const trial = sides.map((x, i) => (i === s ? [...x, c] : x));
    if (fits(trial.flat(), playlist, now)) return trial;
  }
  return null;
}

// 3. Bots at match start, only in the playlist's bot pools after the wait, and never a
//    team without a human (game-modes 9: a crew of three bots cannot exist). Each ticket in
//    turn seeds a match; a seed that cannot find an opponent is skipped, not a stop.
function botFill(leftovers: Ticket[], playlist: Playlist, now: Date): Proposal[] {
  const pool = leftovers.filter((t) =>
    playlist.bot_fill_pools.includes(t.grade_pool) && waitedS(t, now) >= playlist.bot_fill_after_s
  );
  let used = new Set<string>();
  let proposals: Proposal[] = [];
  for (const seed of pool) {
    if (used.has(seed.ticket_id)) continue;
    let sides: Ticket[][] = [[seed], []];
    for (const c of pool) {
      if (c === seed || used.has(c.ticket_id)) continue;
      sides = placeOnSide(sides, c, playlist, now) ?? sides;
    }
    if (sides[1].length === 0) continue;
    proposals = [...proposals, propose(sides, playlist, teamSlots(sides, playlist))];
    used = new Set([...used, ...idsOf(sides.flat())]);
  }
  return proposals;
}

// Free Scrap and other every-player-for-themselves formats: one team per player. Each
// ticket in turn seeds a group; a seed that cannot complete one is skipped.
function formFreeForAll(sorted: Ticket[], playlist: Playlist, now: Date): Proposal[] {
  let used = new Set<string>();
  let proposals: Proposal[] = [];
  for (const seed of sorted) {
    if (used.has(seed.ticket_id) || seed.members.length > playlist.teams) continue;
    let group: Ticket[] = [seed];
    for (const t of sorted) {
      if (size(group) === playlist.teams) break;
      if (t === seed || used.has(t.ticket_id)) continue;
      if (size(group) + t.members.length > playlist.teams) continue;
      if (fits([...group, t], playlist, now)) group = [...group, t];
    }
    if (size(group) !== playlist.teams || !fits(group, playlist, now)) continue;
    const slots = group.flatMap((t) => t.members).map((m, i): Slot => ({
      slot: i + 1,
      team: i + 1,
      account_id: m.account_id,
      is_bot: false,
      harness: m.harness,
      face: m.face,
    }));
    proposals = [...proposals, propose([group], playlist, slots)];
    used = new Set([...used, ...idsOf(group)]);
  }
  return proposals;
}

// A player in two tickets would land in two matches. The database prevents it; this keeps
// the oldest ticket if it ever happens anyway.
function dropOverlapping(sorted: Ticket[]): Ticket[] {
  return sorted.reduce<{ kept: Ticket[]; seen: Set<string> }>(({ kept, seen }, t) => {
    const ids = t.members.map((m) => m.account_id);
    return ids.some((id) => seen.has(id)) ? { kept, seen } : { kept: [...kept, t], seen: new Set([...seen, ...ids]) };
  }, { kept: [], seen: new Set() }).kept;
}

export function formMatches(tickets: Ticket[], playlist: Playlist, now: Date): Proposal[] {
  const sorted = dropOverlapping(
    [...tickets]
      .filter((t) => t.playlist_id === playlist.playlist_id && t.members.length > 0)
      .sort((a, b) => a.created_at.getTime() - b.created_at.getTime() || a.ticket_id.localeCompare(b.ticket_id)),
  );
  const builds = new Set(sorted.map((t) => t.build));
  if (builds.size > 1) throw new Error("formMatches needs tickets from one build");

  if (playlist.team_size === 1 && playlist.teams > 2) {
    return formFreeForAll(sorted, playlist, now);
  }
  const paired = pairTeams(assembleTeams(sorted, playlist, now), playlist, now);
  if (playlist.teams !== 2) return paired;
  const matched = new Set(paired.flatMap((p) => p.ticket_ids));
  const leftovers = sorted.filter((t) => !matched.has(t.ticket_id));
  return [...paired, ...botFill(leftovers, playlist, now)];
}
