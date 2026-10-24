// Match facts to rewards: Tonnage, the Ledger (Grade, the Weight, Clear, the Cutter ceremony,
// entries), Rope Marks and career totals. backend-architecture section 12.3 steps 1 to 3 and 8.
// Pure: the database pass in rewards_pass.ts loads the state, calls this and writes the result.
//
// Who earns (PROPOSED where the docs are silent):
// - Humans with an account. Bots earn nothing; the server already moves a bot's Spotter credit
//   to the human killer it set up (game-modes 9), so it arrives here as that human's fact.
// - A player who abandoned the match earns nothing from it.
// - A no-contest still pays the Tonnage its facts earned, but nobody wins it.
import { RULESETS } from "../ruleset/mod.ts";

export interface Ruleset {
  version: number;
  tonnage: {
    per_tonne_metre: number;
    per_scrap: number;
    per_spotter_credit: number;
    per_assist: number;
    per_ringout_metre: number;
  };
  grade_thresholds_t: number[];
  weight_start_t: number;
  // Tonnage credit (the shop's earned currency, 13.3) paid per tonne of match Tonnage.
  tonnage_credit_per_t: number;
  rope_mark_tiers: { tape: number; band: number; mark: number };
  rigs: string[];
  fact_caps: Record<FactCap, number>;
  // Per-match ceilings for the Contract counters; a counter not listed uses `default`.
  counter_caps: Record<string, number> & { default: number };
  entries: EntryRule[];
  // Reaching Band on a rig writes a Ledger entry (bible/04): this prefix plus the rig's slug.
  band_entry_prefix: string;
}

export type FactCap =
  | "mass_distance_kg_m"
  | "eliminations"
  | "assists"
  | "spotter_credits"
  | "ringouts"
  | "ringout_height_m"
  | "rope_marks"
  | "seconds";

export type CareerStat = keyof CareerTotals;

export interface EntryRule {
  entry_id: string;
  when: { career: CareerStat; at_least: number } | { grade_at_least: number } | { cleared: true };
}

export type RopeTier = "none" | "tape" | "band" | "mark";

export interface ReportFacts {
  mass_distance_kg_m: number;
  eliminations: number;
  assists: number;
  spotter_credits: number;
  ringout_heights_m: number[];
  rope_marks: { rig: string }[];
  counters: Record<string, number>;
  floor_seconds: number;
  airborne_seconds: number;
  hoisted_seconds: number;
}

export interface ReportPlayer {
  account_id: string | null;
  team: number;
  is_bot: boolean;
  abandoned: boolean;
  facts: ReportFacts;
}

export interface Report {
  match_id: string;
  winner_team: number | null;
  forfeit_team: number | null;
  // A Rated short crew forfeiting after 60 s (game-modes 8): no Rating penalty.
  forfeit_penalty_free: boolean;
  no_contest: boolean;
  players: ReportPlayer[];
}

export interface CareerTotals {
  matches: number;
  wins: number;
  scraps: number;
  assists: number;
  spotter_credits: number;
  ringouts: number;
  mass_distance_kg_m: number;
  rated_tons_moved: number;
  floor_seconds: number;
  airborne_seconds: number;
  hoisted_seconds: number;
}

export interface LedgerState {
  tonnage_total: number;
  grade: number;
  // Null until the first rewarded match: the full lease (ruleset weight_start_t).
  weight_balance: number | null;
  cleared: boolean;
  has_cutter_serial: boolean;
}

export interface PlayerState {
  ledger: LedgerState;
  rope_marks: Record<string, number>;
  career: CareerTotals;
  entries: string[];
}

export interface TonnageLine {
  key: "mass_moved" | "scraps" | "spotter_credits" | "ringouts" | "assists";
  count: number;
  rate: number;
  tonnage: number;
}

export interface RopeMarkChange {
  rig: string;
  count: number;
  tier_before: RopeTier;
  tier: RopeTier;
}

export interface PlayerRewards {
  account_id: string;
  harness: string;
  win: boolean;
  tonnage: number;
  credit: number;
  breakdown: TonnageLine[];
  ledger: LedgerState & { weight_balance: number };
  grade_before: number;
  weight_before: number;
  newly_cleared: boolean;
  cutter_ceremony: boolean;
  rope_marks: RopeMarkChange[];
  career_delta: CareerTotals;
  unlocks: string[];
}

export const ZERO_CAREER: CareerTotals = {
  matches: 0,
  wins: 0,
  scraps: 0,
  assists: 0,
  spotter_credits: 0,
  ringouts: 0,
  mass_distance_kg_m: 0,
  rated_tons_moved: 0,
  floor_seconds: 0,
  airborne_seconds: 0,
  hoisted_seconds: 0,
};

const KG_PER_TONNE = 1000;

export function rulesetFor(version: number | null | undefined): Ruleset {
  const latest = Math.max(...RULESETS.map((r) => r.version));
  const wanted = version ?? latest;
  const found = RULESETS.find((r) => r.version === wanted);
  if (!found) throw new Error(`unknown ruleset ${wanted}`);
  return found;
}

// Humans who earn from this match.
export function earners(report: Report): (ReportPlayer & { account_id: string })[] {
  return report.players.filter((p): p is ReportPlayer & { account_id: string } =>
    !p.is_bot && !p.abandoned && p.account_id !== null
  );
}

export function tonnageLines(facts: ReportFacts, rules: Ruleset): TonnageLine[] {
  const t = rules.tonnage;
  const line = (key: TonnageLine["key"], count: number, rate: number): TonnageLine => ({
    key,
    count,
    rate,
    tonnage: Math.round(count * rate),
  });
  const ringoutMetres = facts.ringout_heights_m.reduce((sum, h) => sum + h, 0);
  return [
    line("mass_moved", facts.mass_distance_kg_m / KG_PER_TONNE, t.per_tonne_metre),
    line("scraps", facts.eliminations, t.per_scrap),
    line("spotter_credits", facts.spotter_credits, t.per_spotter_credit),
    line("ringouts", ringoutMetres, t.per_ringout_metre),
    line("assists", facts.assists, t.per_assist),
  ];
}

export function gradeFor(tonnageTotal: number, rules: Ruleset): number {
  const reached = rules.grade_thresholds_t.filter((threshold) => tonnageTotal >= threshold).length;
  return Math.max(1, reached);
}

export function ropeTier(count: number, rules: Ruleset): RopeTier {
  const tiers = rules.rope_mark_tiers;
  if (count >= tiers.mark) return "mark";
  if (count >= tiers.band) return "band";
  if (count >= tiers.tape) return "tape";
  return "none";
}

function careerDelta(player: ReportPlayer, win: boolean): CareerTotals {
  const f = player.facts;
  return {
    matches: 1,
    wins: win ? 1 : 0,
    scraps: f.eliminations,
    assists: f.assists,
    spotter_credits: f.spotter_credits,
    ringouts: f.ringout_heights_m.length,
    mass_distance_kg_m: f.mass_distance_kg_m,
    rated_tons_moved: f.counters.rated_tons_moved ?? 0,
    floor_seconds: f.floor_seconds,
    airborne_seconds: f.airborne_seconds,
    hoisted_seconds: f.hoisted_seconds,
  };
}

function addCareer(a: CareerTotals, b: CareerTotals): CareerTotals {
  const keys = Object.keys(a) as CareerStat[];
  return Object.fromEntries(keys.map((k) => [k, a[k] + b[k]])) as unknown as CareerTotals;
}

function ropeMarkChanges(player: ReportPlayer, before: Record<string, number>, rules: Ruleset): RopeMarkChange[] {
  const landed = new Map<string, number>();
  for (const mark of player.facts.rope_marks) {
    if (rules.rigs.includes(mark.rig)) landed.set(mark.rig, (landed.get(mark.rig) ?? 0) + 1);
  }
  return [...landed.entries()].sort(([a], [b]) => a.localeCompare(b)).map(([rig, added]) => {
    const previous = before[rig] ?? 0;
    return {
      rig,
      count: previous + added,
      tier_before: ropeTier(previous, rules),
      tier: ropeTier(previous + added, rules),
    };
  });
}

function entryMet(rule: EntryRule, career: CareerTotals, ledger: LedgerState): boolean {
  const when = rule.when;
  if ("career" in when) return career[when.career] >= when.at_least;
  if ("grade_at_least" in when) return ledger.grade >= when.grade_at_least;
  return ledger.cleared;
}

function bandEntries(changes: RopeMarkChange[], rules: Ruleset): string[] {
  const reachedBand = (t: RopeTier) => t === "band" || t === "mark";
  return changes.filter((c) => reachedBand(c.tier) && !reachedBand(c.tier_before))
    .map((c) => rules.band_entry_prefix + c.rig.toLowerCase().replaceAll(" ", "-"));
}

// The harness comes from the match roster, not the report.
export function rewardPlayer(
  report: Report,
  player: ReportPlayer & { account_id: string },
  harness: string,
  state: PlayerState,
  rules: Ruleset,
): PlayerRewards {
  const win = !report.no_contest && report.winner_team !== null && player.team === report.winner_team;
  const breakdown = tonnageLines(player.facts, rules);
  const tonnage = breakdown.reduce((sum, l) => sum + l.tonnage, 0);
  const before = state.ledger;
  const weightBefore = before.weight_balance ?? rules.weight_start_t;
  const tonnageTotal = before.tonnage_total + tonnage;
  const ledger = {
    tonnage_total: tonnageTotal,
    grade: Math.max(before.grade, gradeFor(tonnageTotal, rules)),
    weight_balance: Math.max(0, weightBefore - tonnage),
    cleared: before.cleared || weightBefore - tonnage <= 0,
    has_cutter_serial: before.has_cutter_serial,
  };
  const career_delta = careerDelta(player, win);
  const career = addCareer(state.career, career_delta);
  const rope_marks = ropeMarkChanges(player, state.rope_marks, rules);
  const unlocks = [
    ...rules.entries.filter((e) => entryMet(e, career, ledger)).map((e) => e.entry_id),
    ...bandEntries(rope_marks, rules),
  ].filter((id) => !state.entries.includes(id));
  const cutter_ceremony = !before.has_cutter_serial && ledger.grade >= 2;
  return {
    account_id: player.account_id,
    harness,
    win,
    tonnage,
    credit: Math.floor(tonnage * rules.tonnage_credit_per_t),
    breakdown,
    ledger: { ...ledger, has_cutter_serial: before.has_cutter_serial || cutter_ceremony },
    grade_before: before.grade,
    weight_before: weightBefore,
    newly_cleared: ledger.cleared && !before.cleared,
    cutter_ceremony,
    rope_marks,
    career_delta,
    unlocks,
  };
}

// Facts over the ruleset's per-match ceilings (backend-architecture 12.2): the report is
// quarantined rather than rewarded. Returns one problem string per violation.
function counterChecks(counters: Record<string, number>, rules: Ruleset): [string, number, number][] {
  const caps = rules.counter_caps;
  return Object.entries(counters).map((
    [key, value],
  ) => [`counters.${key}`, value, Object.hasOwn(caps, key) ? caps[key] : caps.default]);
}

function highest(values: number[]): number {
  let top = 0;
  for (const v of values) top = Math.max(top, v);
  return top;
}

export function factProblems(report: Report, rules: Ruleset): string[] {
  const caps = rules.fact_caps;
  const problems: string[] = [];
  for (const [i, p] of report.players.entries()) {
    const f = p.facts;
    const checks: [string, number, number][] = [
      ...counterChecks(f.counters, rules),
      ["mass_distance_kg_m", f.mass_distance_kg_m, caps.mass_distance_kg_m],
      ["eliminations", f.eliminations, caps.eliminations],
      ["assists", f.assists, caps.assists],
      ["spotter_credits", f.spotter_credits, caps.spotter_credits],
      ["ringouts", f.ringout_heights_m.length, caps.ringouts],
      ["ringout_height_m", highest(f.ringout_heights_m), caps.ringout_height_m],
      ["rope_marks", f.rope_marks.length, caps.rope_marks],
      ["floor_seconds", f.floor_seconds, caps.seconds],
      ["airborne_seconds", f.airborne_seconds, caps.seconds],
      ["hoisted_seconds", f.hoisted_seconds, caps.seconds],
    ];
    for (const [name, value, cap] of checks) {
      if (value > cap) problems.push(`player ${i}: ${name} ${value} over ${cap}`);
    }
  }
  return problems;
}
