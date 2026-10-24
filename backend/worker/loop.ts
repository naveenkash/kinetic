// One worker tick: matchmaking, allocation, match_found pushes, timeouts, match rewards,
// deallocation and cleanup. backend-architecture sections 10.2 to 10.5 and 12.3. Runs only on
// the leader instance.
import type postgres from "npm:postgres@3.4.5";
import { formMatches, type Member, type Playlist, type Proposal, type Ticket } from "./matchmaker.ts";
import type { Allocation, Fleet } from "./fleet/types.ts";
import { type Keyring, signJoinTicket, signServerCredential } from "../supabase/functions/_shared/tokens.ts";
import { claimsPass } from "./claims_pass.ts";
import { purchasesPass } from "./purchases_pass.ts";
import { accountsPass, type ObjectStore } from "./accounts_pass.ts";
import type { SteamStore } from "../supabase/functions/_shared/steam_store.ts";
import { rewardsPass } from "./rewards_pass.ts";

export const LEADER_LOCK_KEY = 4257100001;
export const MAX_ALLOCATION_ATTEMPTS = 3;
export const BOOT_TIMEOUT_S = 90;
export const NOBODY_JOINED_S = 60;
export const HEARTBEAT_LOST_S = 60;
const SERVER_CREDENTIAL_TTL_S = 4 * 3600;
// Longer than the longest rate-limit window (a day), so cleanup never resets a live window.
const RATE_LIMIT_RETENTION_S = 25 * 3600;

export interface WorkerContext {
  sql: postgres.Sql;
  fleet: Fleet;
  keyring: Keyring;
  functionsUrl: string;
  // Steam microtransactions; null where no publisher key is configured.
  steam?: SteamStore | null;
  // Storage for exports and retention; the account pass does nothing without it.
  storage?: ObjectStore | null;
  // False once this instance may have lost the leader lock; passes stop at the next check.
  isLeader?: () => boolean;
  now?: () => Date;
  random?: () => number;
  log?: (...args: unknown[]) => void;
}

const log = (ctx: WorkerContext, ...args: unknown[]) => (ctx.log ?? console.log)("[worker]", ...args);
const message = (e: unknown) => (e instanceof Error ? e.message : String(e));

type Sql = postgres.Sql | postgres.TransactionSql;
type Json = postgres.JSONValue;

function broadcast(sql: Sql, topic: string, event: string, payload: Record<string, unknown>) {
  return sql`select private.broadcast(${topic}, ${event}, ${sql.json(payload as Json)})`;
}

// Matchmaking ----------------------------------------------------------------------

type ActivePlaylist = Playlist & { arenas: string[] };

function groupByBuild(tickets: Ticket[]): Ticket[][] {
  const builds = [...new Set(tickets.map((t) => t.build))];
  return builds.map((build) => tickets.filter((t) => t.build === build));
}

// Writes one match from a proposal inside a savepoint, so a bad proposal cannot undo the
// rest of the pass. Returns the match id, or null if the proposal was skipped.
async function createMatch(
  ctx: WorkerContext,
  tx: postgres.TransactionSql,
  playlist: ActivePlaylist,
  proposal: Proposal,
  tickets: Ticket[],
): Promise<string | null> {
  const matchId = crypto.randomUUID();
  const random = ctx.random ?? Math.random;
  const arena = playlist.arenas[Math.floor(random() * playlist.arenas.length)];
  const source = tickets.filter((t) => proposal.ticket_ids.includes(t.ticket_id));
  const players = proposal.slots.map((s) => ({ match_id: matchId, ...s }));
  try {
    await tx.savepoint(async (sp) => {
      await sp`
        insert into game.matches (match_id, playlist_id, arena, region, build, fleet, source_tickets)
        values (${matchId}, ${playlist.playlist_id}, ${arena}, ${proposal.region}, ${proposal.build},
                ${ctx.fleet.name}, ${sp.json(source as unknown as Json)})`;
      await sp`insert into game.match_players ${
        sp(players, "match_id", "slot", "team", "account_id", "is_bot", "harness", "face")
      }`;
      await sp`delete from game.mm_tickets where ticket_id in ${sp(proposal.ticket_ids)}`;
      for (const t of source) {
        await broadcast(sp, `crew:${t.crew_id}`, "queue_state", { queued: false, match_id: matchId });
      }
    });
  } catch (e) {
    log(ctx, `proposal skipped (${proposal.ticket_ids.join(",")}): ${message(e)}`);
    return null;
  }
  log(ctx, `match ${matchId} formed in ${proposal.region} with ${proposal.slots.length} slots`);
  return matchId;
}

async function matchPlaylist(ctx: WorkerContext, playlist: ActivePlaylist, now: Date): Promise<string[]> {
  return await ctx.sql.begin(async (tx) => {
    const rows = await tx<Ticket[]>`
      select ticket_id, crew_id, playlist_id, grade_pool, build, members, pings_ms, created_at
      from game.mm_tickets
      where playlist_id = ${playlist.playlist_id}
      order by created_at
      for update skip locked`;
    let created: string[] = [];
    for (const tickets of groupByBuild(rows)) {
      for (const proposal of formMatches(tickets, playlist, now)) {
        const matchId = await createMatch(ctx, tx, playlist, proposal, tickets);
        if (matchId) created = [...created, matchId];
      }
    }
    return created;
  });
}

export async function matchmakingPass(ctx: WorkerContext): Promise<string[]> {
  const now = ctx.now?.() ?? new Date();
  const playlists = await ctx.sql<ActivePlaylist[]>`
    select playlist_id, team_size, teams, regions, bot_fill_pools, bot_fill_after_s, arenas
    from game.playlists where enabled`;
  let created: string[] = [];
  for (const playlist of playlists) {
    created = [...created, ...await matchPlaylist(ctx, playlist, now)];
  }
  return created;
}

// Allocation -----------------------------------------------------------------------

// Asks the fleet for a server carrying this attempt's credential.
async function requestServer(
  ctx: WorkerContext,
  matchId: string,
  allocationId: string,
  target: { region: string; build: string },
): Promise<Allocation> {
  const credential = await signServerCredential(
    ctx.keyring,
    { match_id: matchId, allocation_id: allocationId },
    SERVER_CREDENTIAL_TTL_S,
  );
  return await ctx.fleet.allocate({
    matchId,
    region: target.region,
    build: target.build,
    env: {
      MATCH_ID: matchId,
      SERVER_CREDENTIAL: credential,
      FUNCTIONS_URL: ctx.functionsUrl,
      JOIN_TICKET_PUBLIC_KEYS: JSON.stringify(ctx.keyring.publicJwks),
    },
  });
}

async function allocateOnce(ctx: WorkerContext, matchId: string): Promise<void> {
  // A fresh allocation id per attempt, so a server from an earlier attempt loses access.
  const allocationId = crypto.randomUUID();
  // Heartbeats from an earlier attempt's server do not count for this one.
  const [claim] = await ctx.sql<{ region: string; build: string; allocation_attempts: number }[]>`
    update game.matches
       set allocation_id = ${allocationId}, allocation_attempts = allocation_attempts + 1,
           last_heartbeat_at = null
     where match_id = ${matchId} and state = 'allocating' and address is null
    returning region, build, allocation_attempts`;
  if (!claim) return;

  let alloc: Allocation;
  try {
    alloc = await requestServer(ctx, matchId, allocationId, claim);
  } catch (e) {
    log(ctx, `allocation attempt ${claim.allocation_attempts} failed for ${matchId}: ${message(e)}`);
    if (claim.allocation_attempts >= MAX_ALLOCATION_ATTEMPTS) await abortMatch(ctx, matchId, "allocation_failed");
    return;
  }

  try {
    // A server that already heartbeated (a fleet that waits for readiness) is ready now.
    const [recorded] = await ctx.sql`
      update game.matches
         set address = ${`${alloc.host}:${alloc.port}`}, fleet_ref = ${alloc.fleetRef},
             state = case when last_heartbeat_at is not null then 'waiting_for_players' else state end,
             ready_at = case when last_heartbeat_at is not null then now() else ready_at end
       where match_id = ${matchId} and allocation_id = ${allocationId}
         and state = 'allocating' and address is null
      returning match_id`;
    if (recorded) return;
    log(ctx, `match ${matchId} changed during allocation; releasing ${alloc.fleetRef}`);
  } catch (e) {
    log(ctx, `could not record allocation for ${matchId}: ${message(e)}`);
  }
  // Not recorded: release the server so it does not run unseen.
  await ctx.fleet.deallocate(alloc.fleetRef).catch((e) => log(ctx, `deallocate failed: ${message(e)}`));
}

export async function allocationPass(ctx: WorkerContext): Promise<void> {
  const pending = await ctx.sql<{ match_id: string }[]>`
    select match_id from game.matches
    where state = 'allocating' and address is null
    order by created_at`;
  for (const m of pending) {
    if (ctx.isLeader && !ctx.isLeader()) return;
    await allocateOnce(ctx, m.match_id);
  }
}

// match_found ----------------------------------------------------------------------

// The claim and every push commit together (realtime.send is transactional), so either all
// players of a match are told or none are and the next tick retries.
async function notifyMatch(ctx: WorkerContext, matchId: string): Promise<void> {
  await ctx.sql.begin(async (tx) => {
    const [m] = await tx<{ address: string }[]>`
      update game.matches set notified_at = now()
       where match_id = ${matchId} and state = 'waiting_for_players' and notified_at is null
      returning address`;
    if (!m) return;
    const players = await tx<{ account_id: string; team: number; slot: number }[]>`
      select account_id, team, slot from game.match_players
      where match_id = ${matchId} and not is_bot and account_id is not null`;
    const tickets = await Promise.all(
      players.map((p) => signJoinTicket(ctx.keyring, { ...p, match_id: matchId })),
    );
    for (const [i, p] of players.entries()) {
      await broadcast(tx, `user:${p.account_id}`, "match_found", {
        match_id: matchId,
        address: m.address,
        team: p.team,
        slot: p.slot,
        join_ticket: tickets[i],
      });
    }
    log(ctx, `match ${matchId} ready at ${m.address}; ${players.length} players notified`);
  });
}

export async function notifyPass(ctx: WorkerContext): Promise<void> {
  const ready = await ctx.sql<{ match_id: string }[]>`
    select match_id from game.matches
    where state = 'waiting_for_players' and notified_at is null and address is not null`;
  for (const { match_id: matchId } of ready) {
    await notifyMatch(ctx, matchId).catch((e) => log(ctx, `notify failed for ${matchId}: ${message(e)}`));
  }
}

// Timeouts and aborts ----------------------------------------------------------------

// Aborts a match. Crews that were never told about it go back to the queue in their
// original position, but only if the crew is exactly as it was and every member is free;
// otherwise the crew is told it is no longer queued. Notified players get `match_aborted`.
async function tellPlayersAborted(tx: postgres.TransactionSql, matchId: string, reason: string): Promise<void> {
  const players = await tx<{ account_id: string }[]>`
    select account_id from game.match_players
    where match_id = ${matchId} and not is_bot and account_id is not null`;
  for (const p of players) {
    await broadcast(tx, `user:${p.account_id}`, "match_aborted", { match_id: matchId, reason });
  }
}

const sameAccounts = (a: { account_id: string }[], b: { account_id: string }[]) =>
  a.map((x) => x.account_id).sort().join(",") === b.map((x) => x.account_id).sort().join(",");

// Puts a crew's ticket back in its original queue position, but only if the crew is exactly
// as it was and every member is free; either way the crew is told where it stands.
async function requeueCrew(tx: postgres.TransactionSql, t: Ticket, reason: string): Promise<void> {
  // Lock the crew so a member cannot leave between this read and the requeue.
  await tx`select 1 from game.crews where crew_id = ${t.crew_id} for update`;
  const current = await tx<(Member & { busy: boolean })[]>`
    select m.account_id, m.harness, m.face, l.grade,
           private.active_match_id(m.account_id) is not null as busy
    from game.crew_members m
    join game.ledgers l using (account_id)
    where m.crew_id = ${t.crew_id}
    order by m.joined_at`;
  if (current.length === 0) return;

  const canRequeue = sameAccounts(t.members, current) && current.every((x) => !x.busy);
  const members = current.map(({ account_id, harness, face, grade }) => ({ account_id, harness, face, grade }));
  const requeued = canRequeue &&
    (await tx`
      insert into game.mm_tickets
        (ticket_id, crew_id, playlist_id, grade_pool, build, members, pings_ms, created_at)
      values (${t.ticket_id}, ${t.crew_id}, ${t.playlist_id}, ${t.grade_pool}, ${t.build},
              ${tx.json(members as unknown as Json)}, ${tx.json(t.pings_ms as unknown as Json)}, ${t.created_at})
      on conflict do nothing
      returning ticket_id`).length > 0;
  await broadcast(
    tx,
    `crew:${t.crew_id}`,
    "queue_state",
    requeued ? { queued: true, requeued: true } : { queued: false, reason },
  );
}

// Aborts a match. Crews that were never told about it go back to the queue (requeueCrew);
// players who were told get `match_aborted`.
export async function abortMatch(ctx: WorkerContext, matchId: string, reason: string): Promise<void> {
  await ctx.sql.begin(async (tx) => {
    const [m] = await tx<{ source_tickets: Ticket[]; notified_at: Date | null }[]>`
      update game.matches
         set state = 'aborted', abort_reason = ${reason}, ended_at = now()
       where match_id = ${matchId} and state in ('allocating', 'waiting_for_players', 'in_progress')
      returning source_tickets, notified_at`;
    if (!m) return;
    if (m.notified_at !== null) {
      await tellPlayersAborted(tx, matchId, reason);
      return;
    }
    for (const t of m.source_tickets) await requeueCrew(tx, t, reason);
  });
  log(ctx, `match ${matchId} aborted: ${reason}`);
}

export async function timeoutPass(ctx: WorkerContext): Promise<void> {
  const bootTimeouts = await ctx.sql<{ match_id: string }[]>`
    select match_id from game.matches
    where state = 'allocating' and created_at < now() - make_interval(secs => ${BOOT_TIMEOUT_S})`;
  for (const m of bootTimeouts) await abortMatch(ctx, m.match_id, "server_boot_timeout");

  const nobodyJoined = await ctx.sql<{ match_id: string }[]>`
    select m.match_id from game.matches m
    where m.state = 'waiting_for_players'
      and m.ready_at < now() - make_interval(secs => ${NOBODY_JOINED_S})
      and not exists (select 1 from game.match_players mp
                      where mp.match_id = m.match_id and mp.joined_at is not null)`;
  for (const m of nobodyJoined) await abortMatch(ctx, m.match_id, "nobody_joined");

  const lost = await ctx.sql<{ match_id: string }[]>`
    select match_id from game.matches
    where state in ('waiting_for_players', 'in_progress')
      and coalesce(last_heartbeat_at, ready_at, created_at) < now() - make_interval(secs => ${HEARTBEAT_LOST_S})`;
  for (const m of lost) await abortMatch(ctx, m.match_id, "heartbeat_lost");
}

// Marked deallocated only after the fleet confirms, so a failed call is retried next tick.
export async function deallocatePass(ctx: WorkerContext): Promise<void> {
  const done = await ctx.sql<{ match_id: string; fleet_ref: string }[]>`
    select match_id, fleet_ref from game.matches
    where state in ('completed', 'aborted') and fleet_ref is not null and deallocated_at is null`;
  for (const m of done) {
    try {
      await ctx.fleet.deallocate(m.fleet_ref);
      await ctx.sql`update game.matches set deallocated_at = now() where match_id = ${m.match_id}`;
    } catch (e) {
      log(ctx, `deallocate failed for ${m.match_id}: ${message(e)}`);
    }
  }
}

export async function cleanupPass(ctx: WorkerContext): Promise<void> {
  await Promise.all([
    ctx.sql`delete from game.crew_invites where expires_at < now()`,
    ctx.sql`
      update game.presence set online = false
       where online and updated_at < now() - private.presence_timeout()`,
    ctx.sql`
      delete from private.rate_limits
       where window_start < now() - make_interval(secs => ${RATE_LIMIT_RETENTION_S})`,
  ]);
}

export async function tick(ctx: WorkerContext): Promise<void> {
  const passes: [string, (c: WorkerContext) => Promise<unknown>][] = [
    ["matchmaking", matchmakingPass],
    ["allocation", allocationPass],
    ["notify", notifyPass],
    ["timeouts", timeoutPass],
    ["rewards", rewardsPass],
    ["claims", claimsPass],
    ["purchases", purchasesPass],
    ["accounts", accountsPass],
    ["deallocate", deallocatePass],
    ["cleanup", cleanupPass],
  ];
  for (const [name, pass] of passes) {
    if (ctx.isLeader && !ctx.isLeader()) {
      log(ctx, "leadership lost; stopping this tick");
      return;
    }
    try {
      await pass(ctx);
    } catch (e) {
      log(ctx, `${name} pass failed:`, e instanceof Error ? e.stack ?? e.message : e);
    }
  }
}
