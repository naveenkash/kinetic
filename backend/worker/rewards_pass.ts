// The worker's rewards pass (backend-architecture 12.3): every stored match report is turned
// into Ledger, Rope Mark, career and summary rows in one transaction, then marked processed
// and each player is told on their user channel. A report whose facts are malformed or over
// the ruleset's ceilings is quarantined.
//
// Failures back off (RETRY_BASE_S, doubling with every failure, at most RETRY_MAX_S). Only a
// failure the report itself causes (a data or constraint error) counts as an attempt; after
// MAX_PROCESS_ATTEMPTS it is quarantined as processing_failed. Anything else (the database
// down, a timeout, a bug) stops the pass without counting, so an outage cannot quarantine the
// backlog. CIRCUIT_BREAKER counted failures in a row also stop the pass: that many different
// reports failing at once is more likely a bad deploy than bad reports. It slows a systemic
// failure down but does not stop it; alert on reports with a last_error.
//
// The ruleset is resolved once per pass from the required `ruleset` config key, and must
// equal the copy `deno task content` loaded for clients, so the Ledger always reconciles with
// the rates players see. If the key is missing or unknown, or the copies differ, the pass does
// nothing.
import type postgres from "npm:postgres@3.4.5";
import type { WorkerContext } from "./loop.ts";
import { freeScrapPlaces } from "./claims.ts";
import { applyClaimProgress, claimFor, type ClaimOutcome } from "./claims_progress.ts";
import { type ManifestPaid, payManifest } from "./manifest_step.ts";
import { rateMatch, type RatingChange } from "./ratings_step.ts";
import { parseReport } from "./report_parse.ts";
import {
  type CareerTotals,
  earners,
  factProblems,
  type PlayerRewards,
  type PlayerState,
  type Report,
  rewardPlayer,
  type Ruleset,
  rulesetFor,
  ZERO_CAREER,
} from "./rewards.ts";

export const MAX_PROCESS_ATTEMPTS = 5;
// Failures of any kind before a report is quarantined anyway: a report that keeps hitting the
// same bug must not wait forever (nor hold a Claim's tally until its grace runs out).
export const MAX_FAILURES = 20;
const REPORTS_PER_PASS = 20;
const RETRY_BASE_S = 30;
const RETRY_MAX_S = 3600;
const CIRCUIT_BREAKER = 3;
// SQLSTATE classes a report's own content can cause: data exceptions and constraint violations.
const REPORT_ERROR_CLASSES = ["22", "23"];

type Tx = postgres.TransactionSql;
type Json = postgres.JSONValue;

const message = (e: unknown) => (e instanceof Error ? e.message : String(e));

async function quarantine(tx: Tx, matchId: string, reason: string): Promise<void> {
  await tx`
    update game.match_reports set status = 'quarantined', quarantine_reason = ${reason}
     where match_id = ${matchId}`;
}

export async function rulesetInForce(sql: postgres.Sql | Tx): Promise<Ruleset> {
  const [row] = await sql<{ value: postgres.JSONValue }[]>`select value from game.config where key = 'ruleset'`;
  if (!row || typeof row.value !== "number" || !Number.isInteger(row.value)) {
    throw new Error(`the ruleset config key must be a version number, not ${JSON.stringify(row?.value)}`);
  }
  const rules = rulesetFor(row.value);
  const [copy] = await sql<{ same: boolean }[]>`
    select rules = ${sql.json(rules as unknown as postgres.JSONValue)} as same
      from game.rulesets where version = ${rules.version}`;
  if (!copy?.same) {
    throw new Error(
      `ruleset ${rules.version} is not the copy clients read: run deno task content, or add a new version`,
    );
  }
  return rules;
}

interface LedgerRow {
  account_id: string;
  tonnage_total: string;
  grade: number;
  weight_balance: string | null;
  cleared: boolean;
  serial: boolean;
}

interface Progress {
  marks: { account_id: string; rig: string; count: number }[];
  // Sums as text: numeric totals may be past what a float8 cast holds.
  career: (Record<keyof CareerTotals, string> & { account_id: string })[];
  entries: { account_id: string; entry_id: string }[];
}

// Locks the earners' Ledger rows, in account order so two passes cannot deadlock.
async function lockLedgers(tx: Tx, accounts: string[]): Promise<LedgerRow[]> {
  await tx`insert into game.ledgers ${tx(accounts.map((account_id) => ({ account_id })), "account_id")}
           on conflict do nothing`;
  return await tx<LedgerRow[]>`
    select account_id, tonnage_total, grade, weight_balance, cleared_at is not null as cleared,
           cutter_serial is not null as serial
      from game.ledgers where account_id in ${tx(accounts)} order by account_id for update`;
}

async function readProgress(tx: Tx, accounts: string[]): Promise<Progress> {
  const marks = await tx<Progress["marks"]>`
    select account_id, rig, count from game.rope_marks where account_id in ${tx(accounts)}`;
  const career = await tx<Progress["career"]>`
    select account_id, sum(matches)::text as matches, sum(wins)::text as wins, sum(scraps)::text as scraps,
           sum(assists)::text as assists, sum(spotter_credits)::text as spotter_credits,
           sum(ringouts)::text as ringouts, sum(mass_distance_kg_m)::text as mass_distance_kg_m,
           sum(rated_tons_moved)::text as rated_tons_moved, sum(floor_seconds)::text as floor_seconds,
           sum(airborne_seconds)::text as airborne_seconds, sum(hoisted_seconds)::text as hoisted_seconds
      from game.career_stats where account_id in ${tx(accounts)} group by account_id`;
  const entries = await tx<Progress["entries"]>`
    select account_id, entry_id from game.ledger_entries where account_id in ${tx(accounts)}`;
  return { marks, career, entries };
}

function toState(l: LedgerRow, progress: Progress): PlayerState {
  const mine = <T extends { account_id: string }>(rows: T[]) => rows.filter((r) => r.account_id === l.account_id);
  const [totals] = mine(progress.career);
  const keys = Object.keys(ZERO_CAREER) as (keyof CareerTotals)[];
  const career = Object.fromEntries(keys.map((k) => [k, totals ? Number(totals[k]) : 0])) as unknown as CareerTotals;
  return {
    ledger: {
      tonnage_total: Number(l.tonnage_total),
      grade: l.grade,
      weight_balance: l.weight_balance === null ? null : Number(l.weight_balance),
      cleared: l.cleared,
      has_cutter_serial: l.serial,
    },
    rope_marks: Object.fromEntries(mine(progress.marks).map((m) => [m.rig, m.count])),
    career,
    entries: mine(progress.entries).map((e) => e.entry_id),
  };
}

async function loadStates(tx: Tx, accounts: string[]): Promise<Map<string, PlayerState>> {
  const ledgers = await lockLedgers(tx, accounts);
  const progress = await readProgress(tx, accounts);
  return new Map(ledgers.map((l) => [l.account_id, toState(l, progress)]));
}

async function writeLedger(tx: Tx, r: PlayerRewards): Promise<void> {
  await tx`
    update game.ledgers
       set tonnage_total = ${r.ledger.tonnage_total}, grade = ${r.ledger.grade},
           weight_balance = ${r.ledger.weight_balance},
           cleared_at = case when ${r.newly_cleared} then now() else cleared_at end,
           cutter_serial = case when ${r.cutter_ceremony}
                                then (select 'CT-' || lpad(n::text, greatest(6, length(n::text)), '0')
                                        from (select nextval('game.cutter_serial_seq') as n) s)
                                else cutter_serial end,
           cutter_issued_at = case when ${r.cutter_ceremony} then now() else cutter_issued_at end,
           updated_at = now()
     where account_id = ${r.account_id}`;
}

async function writeProgress(tx: Tx, matchId: string, r: PlayerRewards): Promise<void> {
  const d = r.career_delta;
  await tx`
    insert into game.career_stats (account_id, harness, matches, wins, scraps, assists, spotter_credits, ringouts,
                                   mass_distance_kg_m, rated_tons_moved, floor_seconds, airborne_seconds,
                                   hoisted_seconds)
    values (${r.account_id}, ${r.harness}, ${d.matches}, ${d.wins}, ${d.scraps}, ${d.assists}, ${d.spotter_credits},
            ${d.ringouts}, ${d.mass_distance_kg_m}, ${d.rated_tons_moved}, ${d.floor_seconds}, ${d.airborne_seconds},
            ${d.hoisted_seconds})
    on conflict (account_id, harness) do update set
      matches = career_stats.matches + excluded.matches, wins = career_stats.wins + excluded.wins,
      scraps = career_stats.scraps + excluded.scraps, assists = career_stats.assists + excluded.assists,
      spotter_credits = career_stats.spotter_credits + excluded.spotter_credits,
      ringouts = career_stats.ringouts + excluded.ringouts,
      mass_distance_kg_m = career_stats.mass_distance_kg_m + excluded.mass_distance_kg_m,
      rated_tons_moved = career_stats.rated_tons_moved + excluded.rated_tons_moved,
      floor_seconds = career_stats.floor_seconds + excluded.floor_seconds,
      airborne_seconds = career_stats.airborne_seconds + excluded.airborne_seconds,
      hoisted_seconds = career_stats.hoisted_seconds + excluded.hoisted_seconds`;
  for (const m of r.rope_marks) {
    await tx`
      insert into game.rope_marks (account_id, rig, count, tier) values (${r.account_id}, ${m.rig}, ${m.count}, ${m.tier})
      on conflict (account_id, rig) do update set count = excluded.count, tier = excluded.tier, updated_at = now()`;
  }
  if (r.unlocks.length > 0) {
    await tx`
      insert into game.ledger_entries ${
      tx(r.unlocks.map((entry_id) => ({ account_id: r.account_id, entry_id, match_id: matchId })))
    } on conflict do nothing`;
  }
}

// Idempotent per match: the key is unique per account, so a match never pays credit twice.
async function writeCredit(tx: Tx, matchId: string, r: PlayerRewards): Promise<void> {
  if (r.credit <= 0) return;
  const [txn] = await tx`
    insert into game.wallet_txns (account_id, currency, delta, reason, ref, idempotency_key)
    values (${r.account_id}, 'tonnage_credit', ${r.credit}, 'match', ${matchId}, ${"match:" + matchId})
    on conflict (account_id, idempotency_key) do nothing
    returning txn_id`;
  if (!txn) return;
  await tx`
    insert into game.wallets (account_id, currency, balance) values (${r.account_id}, 'tonnage_credit', ${r.credit})
    on conflict (account_id, currency) do update set balance = wallets.balance + excluded.balance`;
}

async function writeSummary(
  tx: Tx,
  matchId: string,
  ruleset: number,
  r: PlayerRewards,
  claim: ClaimOutcome | null,
  rating: RatingChange | undefined,
  manifest: ManifestPaid | null,
): Promise<void> {
  const standing = claim ? { faction: claim.faction, points: claim.points } : null;
  await tx`
    insert into game.match_summaries (match_id, account_id, ruleset, win, tonnage, breakdown, grade_before,
                                      grade_after, weight_before, weight_after, rope_marks, unlocks, credit,
                                      contracts, standing, rating, manifest)
    values (${matchId}, ${r.account_id}, ${ruleset}, ${r.win}, ${r.tonnage}, ${tx.json(r.breakdown as unknown as Json)},
            ${r.grade_before}, ${r.ledger.grade}, ${r.weight_before}, ${r.ledger.weight_balance},
            ${tx.json(r.rope_marks as unknown as Json)}, ${tx.json(r.unlocks)}, ${r.credit},
            ${tx.json((claim?.contracts ?? []) as unknown as Json)}, ${standing ? tx.json(standing) : null},
            ${rating ? tx.json(rating as unknown as Json) : null},
            ${manifest ? tx.json(manifest as unknown as Json) : null})`;
  await tx`select private.broadcast(${"user:" + r.account_id}, 'match_processed',
                                    ${tx.json({ match_id: matchId, tonnage: r.tonnage, grade: r.ledger.grade })})`;
}

// Earners whose account still exists, with their harness from the roster. The accounts are
// key-share locked first, in account order: an account deletion then waits for this pass
// instead of deadlocking with it. An account deleted before now earns nothing.
async function liveEarners(tx: Tx, matchId: string, report: Report) {
  const humans = earners(report);
  if (humans.length === 0) return [];
  const rows = await tx<{ account_id: string; harness: string }[]>`
    select a.account_id, mp.harness
      from game.accounts a
      join game.match_players mp on mp.match_id = ${matchId} and mp.account_id = a.account_id
     where a.account_id in ${tx(humans.map((h) => h.account_id))}
     order by a.account_id
       for key share of a`;
  const harness = new Map(rows.map((r) => [r.account_id, r.harness]));
  return humans.flatMap((h) => harness.has(h.account_id) ? [{ player: h, harness: harness.get(h.account_id)! }] : []);
}

interface StoredReport {
  payload: unknown;
  received_at: Date;
  mode: string;
  rated: boolean;
}

// Every match's result for Claim Standing (game-modes 7): leavers never reach here; a forfeit
// or a no-contest voids the match for everyone in it (PROPOSED for the winners of a forfeit:
// the match does not count, as for the crew that forfeited).
function claimResult(stored: StoredReport, report: Report, win: boolean, place: number) {
  const voided = report.no_contest || report.forfeit_team !== null;
  return {
    mode: stored.mode,
    rated: stored.rated,
    win,
    voided,
    place: stored.mode === "free_scrap" ? place : undefined,
  };
}

async function rewardReport(tx: Tx, matchId: string, stored: StoredReport, rules: Ruleset): Promise<string> {
  const payload = stored.payload;
  const parsed = parseReport(payload);
  if ("problem" in parsed) {
    await quarantine(tx, matchId, parsed.problem);
    return "quarantined";
  }
  const problems = factProblems(parsed.report, rules);
  if (problems.length > 0) {
    await quarantine(tx, matchId, `fact_ceiling: ${problems.join("; ")}`);
    return "quarantined";
  }
  const players = await liveEarners(tx, matchId, parsed.report);
  const accounts = players.map((p) => p.player.account_id);
  const states = accounts.length > 0 ? await loadStates(tx, accounts) : new Map<string, PlayerState>();
  const claim = await claimFor(tx, stored.received_at);
  // Rated playlists only; leavers are rated too, so this covers the whole roster.
  const ratings = stored.rated ? await rateMatch(tx, stored.mode, parsed.report) : new Map<string, RatingChange>();
  const ranks = freeScrapPlaces(
    parsed.report.players.map((p) => ({ team: p.team, eliminations: p.facts.eliminations })),
    parsed.report.winner_team,
  );
  for (const { player, harness } of players) {
    const rewards = rewardPlayer(parsed.report, player, harness, states.get(player.account_id)!, rules);
    await writeLedger(tx, rewards);
    await writeProgress(tx, matchId, rewards);
    await writeCredit(tx, matchId, rewards);
    const place = ranks[parsed.report.players.indexOf(player)];
    const result = claimResult(stored, parsed.report, rewards.win, place);
    const manifest = claim ? await payManifest(tx, claim.claim_id, player.account_id, rewards.tonnage) : null;
    const progress = claim
      ? await applyClaimProgress(tx, claim, player.account_id, matchId, result, player.facts.counters)
      : null;
    await writeSummary(tx, matchId, rules.version, rewards, progress, ratings.get(player.account_id), manifest);
  }
  await tx`
    update game.match_reports set status = 'processed', processed_at = now(), ruleset = ${rules.version}
     where match_id = ${matchId}`;
  return "processed";
}

function causedByReport(error: unknown): boolean {
  const code = (error as { code?: unknown }).code;
  return typeof code === "string" && REPORT_ERROR_CLASSES.includes(code.slice(0, 2));
}

async function recordFailure(ctx: WorkerContext, matchId: string, error: unknown, counted: boolean): Promise<void> {
  const add = counted ? 1 : 0;
  await ctx.sql`
    update game.match_reports
       set process_attempts = process_attempts + ${add}, failures = failures + 1, last_error = ${message(error)},
           next_attempt_at = now() + make_interval(secs => least(${RETRY_BASE_S} * power(2, least(failures, 20)),
                                                                  ${RETRY_MAX_S})),
           status = case when process_attempts + ${add} >= ${MAX_PROCESS_ATTEMPTS} or failures + 1 >= ${MAX_FAILURES}
                         then 'quarantined' else status end,
           quarantine_reason = case when process_attempts + ${add} >= ${MAX_PROCESS_ATTEMPTS}
                                         or failures + 1 >= ${MAX_FAILURES}
                                    then 'processing_failed' else quarantine_reason end
     where match_id = ${matchId} and status = 'stored'`;
}

// Processes one report under the given ruleset. Returns its new status, "failed" (counted
// against the report), "error" (not counted: the pass should stop), or null if another pass
// holds it or it is not stored.
export async function processReport(ctx: WorkerContext, matchId: string, rules: Ruleset): Promise<string | null> {
  try {
    return await ctx.sql.begin(async (tx) => {
      const [row] = await tx<StoredReport[]>`
        select r.payload, r.received_at, p.mode, p.rated
          from game.match_reports r
          join game.matches m using (match_id)
          join game.playlists p using (playlist_id)
         where r.match_id = ${matchId} and r.status = 'stored'
           for update of r skip locked`;
      return row ? await rewardReport(tx, matchId, row, rules) : null;
    });
  } catch (e) {
    const counted = causedByReport(e);
    (ctx.log ?? console.log)("[worker]", `rewards failed for ${matchId}: ${message(e)}`);
    await recordFailure(ctx, matchId, e, counted).catch((f) =>
      (ctx.log ?? console.log)("[worker]", `could not record the failure for ${matchId}: ${message(f)}`)
    );
    return counted ? "failed" : "error";
  }
}

export async function rewardsPass(ctx: WorkerContext): Promise<void> {
  const rules = await rulesetInForce(ctx.sql);
  const due = await ctx.sql<{ match_id: string }[]>`
    select match_id from game.match_reports
     where status = 'stored' and (next_attempt_at is null or next_attempt_at <= now())
     order by received_at limit ${REPORTS_PER_PASS}`;
  let failedInARow = 0;
  for (const r of due) {
    if (ctx.isLeader && !ctx.isLeader()) return;
    const status = await processReport(ctx, r.match_id, rules);
    if (status === "error") return;
    failedInARow = status === "failed" ? failedInARow + 1 : 0;
    if (failedInARow >= CIRCUIT_BREAKER) {
      (ctx.log ?? console.log)("[worker]", `rewards: ${failedInARow} reports failed in a row; pausing the pass`);
      return;
    }
  }
}
