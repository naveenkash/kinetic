// Reads a stored report payload into the shape rewards.ts expects. srv_store_report checked
// the roster; this checks the facts. Anything malformed quarantines the report.
import type { Report, ReportFacts, ReportPlayer } from "./rewards.ts";

type Obj = Record<string, unknown>;

const UUID = /^[0-9a-f]{8}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{12}$/i;
// Counters the core counts in whole numbers; the rest (tonnes) may be fractional.
const FRACTIONAL_COUNTERS = ["rated_tons_moved"];

const isObj = (v: unknown): v is Obj => typeof v === "object" && v !== null && !Array.isArray(v);
const isAmount = (v: unknown): v is number => typeof v === "number" && Number.isFinite(v) && v >= 0;
const isCount = (v: unknown): v is number => isAmount(v) && Number.isInteger(v);

class Malformed extends Error {}

function read(o: Obj, key: string, valid: (v: unknown) => v is number, name = key): number {
  const v = o[key] ?? 0;
  if (!valid(v)) throw new Malformed(name);
  return v;
}

function amounts(v: unknown, key: string): number[] {
  if (v === undefined || v === null) return [];
  if (!Array.isArray(v) || !v.every(isAmount)) throw new Malformed(key);
  return v;
}

function parseCounters(v: unknown): Record<string, number> {
  if (v === undefined || v === null) return {};
  if (!isObj(v)) throw new Malformed("counters");
  return Object.fromEntries(
    Object.keys(v).map((k) => {
      const valid = FRACTIONAL_COUNTERS.includes(k) ? isAmount : isCount;
      return [k, read(v, k, valid, `counters.${k}`)];
    }),
  );
}

function parseFacts(v: unknown): ReportFacts {
  if (!isObj(v)) throw new Malformed("facts");
  const marks = v.rope_marks ?? [];
  if (!Array.isArray(marks) || !marks.every((m) => isObj(m) && typeof m.rig === "string")) {
    throw new Malformed("rope_marks");
  }
  return {
    mass_distance_kg_m: read(v, "mass_distance_kg_m", isAmount),
    eliminations: read(v, "eliminations", isCount),
    assists: read(v, "assists", isCount),
    spotter_credits: read(v, "spotter_credits", isCount),
    ringout_heights_m: amounts(v.ringout_heights_m, "ringout_heights_m"),
    rope_marks: marks.map((m) => ({ rig: (m as Obj).rig as string })),
    counters: parseCounters(v.counters),
    floor_seconds: read(v, "floor_seconds", isAmount),
    airborne_seconds: read(v, "airborne_seconds", isAmount),
    hoisted_seconds: read(v, "hoisted_seconds", isAmount),
  };
}

function parsePlayer(v: unknown): ReportPlayer {
  if (!isObj(v)) throw new Malformed("player");
  const account = v.account_id ?? null;
  if (account !== null && (typeof account !== "string" || !UUID.test(account))) throw new Malformed("account_id");
  if (typeof v.team !== "number") throw new Malformed("team");
  const is_bot = v.is_bot === true;
  return {
    // Canonical lowercase, as the database returns ids.
    account_id: account === null ? null : account.toLowerCase(),
    team: v.team,
    is_bot,
    abandoned: v.abandoned === true,
    // A bot's facts are not rewarded; a server need not send them.
    facts: is_bot && v.facts === undefined ? parseFacts({}) : parseFacts(v.facts),
  };
}

export function parseReport(payload: unknown): { report: Report } | { problem: string } {
  try {
    if (!isObj(payload) || !Array.isArray(payload.players)) throw new Malformed("players");
    const winner = payload.winner_team ?? null;
    if (winner !== null && typeof winner !== "number") throw new Malformed("winner_team");
    const forfeit = payload.forfeit_team ?? null;
    if (forfeit !== null && typeof forfeit !== "number") throw new Malformed("forfeit_team");
    return {
      report: {
        match_id: String(payload.match_id),
        winner_team: winner,
        forfeit_team: forfeit,
        forfeit_penalty_free: payload.forfeit_penalty_free === true,
        no_contest: payload.no_contest === true,
        players: payload.players.map(parsePlayer),
      },
    };
  } catch (e) {
    if (e instanceof Malformed) return { problem: `malformed ${e.message}` };
    throw e;
  }
}
