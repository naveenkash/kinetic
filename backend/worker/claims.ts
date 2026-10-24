// Claim seasons (backend-architecture 14; game-modes 7; bible/04): Standing points per match,
// the weekly cap, Contract progress, faction standing S_f and the per-arena outcome of a Claim.
// Pure: rewards_pass.ts and claims_pass.ts read the state, call these and write the result.
//
// Rulings the docs leave open (PROPOSED):
// - Free Scrap: the reported winner is first; the rest place by eliminations, tied players
//   sharing a place (the report has no times-scrapped fact for game-modes 5's tie-break). A
//   player with no scraps is never top 3 unless they won, so an idle lobby cannot farm it.
// - A no-contest counts like a forfeit: 0 P, and the match does not count.
// - The weekly cap covers every kind of P (match, Contract and evidence).
// - A Claim's weeks run from its starts_at: days 0 to 6 are week 1.

export const FACTIONS = ["guild", "htr", "ascendancy", "hangers", "fray"] as const;
export type Faction = typeof FACTIONS[number];

export const WEEKLY_CAP_P = 60;
export const CONTRACT_P = 2;
export const EVIDENCE_P = 5;
const SMALL_FACTION_SHARE = 0.6;
const FREE_SCRAP_TOP = 3;
const WEEK_MS = 7 * 24 * 3600 * 1000;

const POINTS = {
  casualWin: 3,
  casualLoss: 1,
  ratedWin: 4.5,
  ratedLoss: 1.5,
  freeScrapTop: 2,
  freeScrapOther: 1,
};

// A Contract's or crate's reward, read defensively: the database checks rewards as they are
// written, so anything else here is skipped rather than failing a match.
export function rewardLists(reward: unknown): { items: string[]; entries: string[] } {
  const r = typeof reward === "object" && reward !== null ? reward as Record<string, unknown> : {};
  const strings = (v: unknown) => Array.isArray(v) ? v.filter((x): x is string => typeof x === "string") : [];
  return { items: strings(r.items), entries: strings(r.entries) };
}

export interface MatchResult {
  mode: string;
  rated: boolean;
  win: boolean;
  // Left, forfeited, or a no-contest: the match does not count.
  voided: boolean;
  // Free Scrap only: 1-based place.
  place?: number;
}

export function matchPoints(r: MatchResult): number {
  if (r.voided) return 0;
  if (r.mode === "free_scrap") {
    return (r.place ?? Infinity) <= FREE_SCRAP_TOP ? POINTS.freeScrapTop : POINTS.freeScrapOther;
  }
  if (r.rated) return r.win ? POINTS.ratedWin : POINTS.ratedLoss;
  return r.win ? POINTS.casualWin : POINTS.casualLoss;
}

// Competition ranking: equal scores share the place of the first of them.
export function places(scores: number[]): number[] {
  return scores.map((s) => 1 + scores.filter((other) => other > s).length);
}

export function freeScrapPlaces(
  players: { team: number; eliminations: number }[],
  winnerTeam: number | null,
): number[] {
  const isWinner = (p: { team: number }) => winnerTeam !== null && p.team === winnerTeam;
  const offset = players.some(isWinner) ? 1 : 0;
  const rest = players.filter((p) => !isWinner(p)).map((p) => p.eliminations);
  return players.map((p) => {
    if (isWinner(p)) return 1;
    if (p.eliminations === 0) return Infinity;
    return offset + 1 + rest.filter((other) => other > p.eliminations).length;
  });
}

// The share of `points` that still fits under the weekly cap.
export function capped(points: number, earnedThisWeek: number, cap = WEEKLY_CAP_P): number {
  return Math.max(0, Math.min(points, cap - earnedThisWeek));
}

export function claimWeek(startsAt: Date, at: Date): number {
  return Math.floor((at.getTime() - startsAt.getTime()) / WEEK_MS) + 1;
}

export interface Contract {
  contract_id: string;
  counter_key: string;
  target: number;
}

export interface ContractUpdate {
  contract_id: string;
  progress: number;
  completed: boolean;
}

// Adds this match's counters to the player's open Contracts. A Contract already completed
// stays as it is; one that reaches its target now is completed.
export function advanceContracts(
  contracts: Contract[],
  progress: Map<string, { progress: number; completed: boolean }>,
  counters: Record<string, number>,
): ContractUpdate[] {
  return contracts.flatMap((c) => {
    const before = progress.get(c.contract_id) ?? { progress: 0, completed: false };
    const added = counters[c.counter_key] ?? 0;
    if (before.completed || added <= 0) return [];
    const now = before.progress + added;
    return [{ contract_id: c.contract_id, progress: now, completed: now >= c.target }];
  });
}

export interface FactionTotals {
  faction: Faction;
  points: number;
  active_players: number;
  rated_wins: number;
}

export interface FactionStanding extends FactionTotals {
  standing: number;
}

// S_f = (sum of P) / max(A_f, 0.6 x mean(A)) over all five factions (game-modes 7). A faction
// with no players stands at 0.
export function standings(totals: FactionTotals[]): FactionStanding[] {
  const all = FACTIONS.map((f) =>
    totals.find((t) => t.faction === f) ?? { faction: f, points: 0, active_players: 0, rated_wins: 0 }
  );
  const meanActive = all.reduce((sum, t) => sum + t.active_players, 0) / FACTIONS.length;
  return all.map((t) => {
    const divisor = Math.max(t.active_players, SMALL_FACTION_SHARE * meanActive);
    return { ...t, standing: divisor > 0 ? t.points / divisor : 0 };
  });
}

// The faction holding the Claim: highest S_f, ties to more Rated wins. Null if nobody played,
// or if the tie survives the tie-break (the reviewer decides).
export function holder(s: FactionStanding[]): Faction | null {
  const ranked = [...s].sort((a, b) => b.standing - a.standing || b.rated_wins - a.rated_wins);
  const [first, second] = ranked;
  if (!first || first.standing <= 0) return null;
  if (second && second.standing === first.standing && second.rated_wins === first.rated_wins) return null;
  return first.faction;
}

// Arena codes (PROPOSED): YRD the Yard, HGT Hangtown, BLS Ballast, ANC the Anchor.
// The variant each arena takes next season when a faction holds its Claim (bible/04).
export function arenaVariant(arena: string, winner: Faction): string {
  switch (arena) {
    case "HGT":
      return winner === "htr" ? "district_removed" : "base";
    case "BLS":
      if (winner === "htr") return "crown_and_spine";
      return winner === "guild" ? "stabilized" : "base";
    case "YRD":
      return winner === "htr" ? "haulers_doubled" : "base";
    case "ANC":
      return `colors_${winner}`;
    default:
      return "base";
  }
}
