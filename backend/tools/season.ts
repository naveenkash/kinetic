// Season-flip tooling (backend-architecture 14 and 26, Phase 4): what staff look at before a
// Claim ends, and a rehearsal of the whole flip on staging.
//
// - claimPreview: the tally the Claim would get now (standings, holder, the arena's next
//   variant, the reviewer's anomalies), read-only, from the same code the worker tallies with.
// - claimChecklist: the readiness checks before the freeze. Blocking checks are the ones a
//   flip must not start without; the rest are warnings for the reviewer.
// - rehearseFlip: on a non-production database, runs one Claim through the worker's steps and
//   a reviewer's (frozen, tallied, reviewed, resolved) and reports each step. It pulls the
//   Claim's end forward to now, so it changes data: run it on staging, restored from a recent
//   production copy (runbooks.md), never on production. `deno task flip-rehearsal`, which
//   first checks the target proves what it is (rehearsalTargetProblem).
//
// Choices the docs leave open (PROPOSED):
// - The next Claim must start within 24 hours of this one's end.
// - The next Claim needs at least one week-1 Contract per faction and a crate on each track.
// - `variants_shipped` in config lists every arena variant the live build can draw; the
//   release process writes it with min_build. Every variant this arena can take must be in it.
// - The worker counts as running if an active Claim's standing refreshed within two hours.
import type postgres from "npm:postgres@3.4.5";
import { arenaVariant, FACTIONS, holder, standings } from "../worker/claims.ts";
import { anomalies, claimsPass, factionTotals, tallyClaim } from "../worker/claims_pass.ts";
import type { WorkerContext } from "../worker/loop.ts";
import { rewardsPass } from "../worker/rewards_pass.ts";

type Sql = postgres.Sql | postgres.TransactionSql;

export const NEXT_CLAIM_GAP_S = 24 * 3600;
export const WORKER_STALE_S = 2 * 3600;

interface ClaimRow {
  claim_id: string;
  arena: string;
  state: string;
  starts_at: Date;
  ends_at: Date;
  outcome: Record<string, unknown> | null;
}

async function claimRow(sql: Sql, claimId: string): Promise<ClaimRow | undefined> {
  const [row] = await sql<ClaimRow[]>`
    select claim_id, arena, state, starts_at, ends_at, outcome from game.claims where claim_id = ${claimId}`;
  return row;
}

// Every variant the arena can take next season, whoever holds it, and the base for a tie.
export function possibleVariants(arena: string): string[] {
  return [...new Set(["base", ...FACTIONS.map((f) => arenaVariant(arena, f))])].sort();
}

export async function claimPreview(sql: Sql, claimId: string) {
  const claim = await claimRow(sql, claimId);
  if (!claim) throw new Error("unknown_claim");
  const final = standings(await factionTotals(sql, claimId));
  const winner = holder(final);
  return {
    claim_id: claimId,
    state: claim.state,
    holder: winner,
    variant: winner ? arenaVariant(claim.arena, winner) : null,
    standings: final.map((s) => ({
      faction: s.faction,
      standing: Math.round(s.standing * 10) / 10,
      points: s.points,
      active_players: s.active_players,
      rated_wins: s.rated_wins,
    })),
    anomalies: await anomalies(sql, claimId),
  };
}

export interface Check {
  check: string;
  ok: boolean;
  blocking: boolean;
  detail: string;
}

export async function claimChecklist(sql: Sql, claimId: string): Promise<{ ready: boolean; checks: Check[] }> {
  const checks: Check[] = [];
  const add = (check: string, ok: boolean, blocking: boolean, detail: string) =>
    checks.push({ check, ok, blocking, detail });
  const claim = await claimRow(sql, claimId);
  if (!claim) {
    add("claim_exists", false, true, `no Claim ${claimId}`);
    return { ready: false, checks };
  }
  add("claim_exists", true, true, `${claim.claim_id} (${claim.arena}), ${claim.state}`);
  add(
    "claim_state",
    ["active", "frozen", "tallied"].includes(claim.state),
    true,
    `the flip runs from active, frozen or tallied; this Claim is ${claim.state}`,
  );

  const [next] = await sql<{ claim_id: string; starts_at: Date }[]>`
    select claim_id, starts_at from game.claims
     where state = 'scheduled' and starts_at >= ${claim.ends_at} order by starts_at limit 1`;
  const gap = next ? (next.starts_at.getTime() - claim.ends_at.getTime()) / 1000 : Infinity;
  // Past the freeze the gap no longer matters (and a rehearsal moved the end): one scheduled is enough.
  const settled = claim.state === "tallied" || claim.state === "reviewed";
  add(
    "next_claim_scheduled",
    settled ? !!next : gap <= NEXT_CLAIM_GAP_S,
    true,
    next
      ? `${next.claim_id} starts ${Math.round(gap / 3600)} h after this one ends`
      : "no Claim is scheduled after this one",
  );
  if (next) {
    const covered = await sql<{ faction: string }[]>`
      select distinct faction from game.contracts where claim_id = ${next.claim_id} and week = 1`;
    const missing = FACTIONS.filter((f) => !covered.some((c) => c.faction === f));
    add(
      "next_claim_contracts",
      missing.length === 0,
      true,
      missing.length ? `no week-1 Contract for ${missing.join(", ")}` : "week-1 Contracts for every faction",
    );
    const tracks = await sql<{ track: string }[]>`
      select distinct track from game.manifest_crates where claim_id = ${next.claim_id}`;
    const absent = ["free", "sponsored"].filter((t) => !tracks.some((r) => r.track === t));
    add(
      "next_claim_crates",
      absent.length === 0,
      true,
      absent.length ? `no ${absent.join(" or ")} crates` : "crates on both tracks",
    );
  }

  const [shipped] = await sql<{ value: unknown }[]>`select value from game.config where key = 'variants_shipped'`;
  const list = Array.isArray(shipped?.value) ? (shipped.value as unknown[]).filter((v) => typeof v === "string") : [];
  const needed = possibleVariants(claim.arena).filter((v) => v !== "base");
  const unshipped = needed.filter((v) => !list.includes(v));
  add(
    "variants_shipped",
    unshipped.length === 0,
    true,
    unshipped.length
      ? `the live build is not recorded as shipping ${unshipped.join(", ")} (config variants_shipped)`
      : `the live build ships ${needed.join(", ") || "every variant"}`,
  );

  const [backlog] = await sql<{ stored: number; quarantined: number }[]>`
    select (count(*) filter (where r.status = 'stored'))::int as stored,
           (count(*) filter (where r.status = 'quarantined'))::int as quarantined
      from game.match_reports r
     where r.received_at >= ${claim.starts_at} and r.received_at < ${claim.ends_at}`;
  add("reports_processed", backlog.stored === 0, false, `${backlog.stored} reports from this Claim not processed yet`);
  add("reports_quarantined", backlog.quarantined === 0, false, `${backlog.quarantined} quarantined reports to review`);

  if (claim.state === "active") {
    const [fresh] = await sql<{ at: Date | null }[]>`
      select min(refreshed_at) as at from game.claim_standing where claim_id = ${claimId}`;
    const age = fresh?.at ? (Date.now() - fresh.at.getTime()) / 1000 : Infinity;
    add(
      "worker_running",
      age <= WORKER_STALE_S,
      true,
      Number.isFinite(age) ? `standing refreshed ${Math.round(age / 60)} min ago` : "standing never refreshed",
    );
  }
  return { ready: checks.every((c) => c.ok || !c.blocking), checks };
}

export interface RehearsalStep {
  step: string;
  ok: boolean;
  detail: unknown;
}

export interface RehearsalOptions {
  staff: string;
  // A tie the tally cannot break needs the reviewer's call.
  holder?: string;
  variant?: string;
  ignoreChecks?: boolean;
  // Tally without waiting for every report (they are listed as unprocessed).
  forceTally?: boolean;
  maxRewardPasses?: number;
}

const LOCAL_HOSTS = new Set(["localhost", "127.0.0.1", "[::1]", "::1"]);

// The Supabase project a connection string reaches: db.<ref>.supabase.co, or the pooler's
// postgres.<ref> user; otherwise the host itself.
export function projectRef(databaseUrl: string): string | null {
  let url: URL;
  try {
    url = new URL(databaseUrl);
  } catch {
    return null;
  }
  const direct = /^db\.([a-z0-9]+)\.supabase\.co$/.exec(url.hostname);
  if (direct) return direct[1];
  const pooled = /^postgres\.([a-z0-9]+)$/.exec(decodeURIComponent(url.username));
  if (pooled) return pooled[1];
  return url.hostname;
}

// The target must prove it is what the operator named, by facts the connection string cannot
// fake: `local` is a database on this machine; `staging` is a project listed in
// KINETIC_STAGING_DB_REFS (comma-separated project refs or hosts), which only staging's own
// configuration sets. Nothing else is rehearsed, whatever the shell's environment says.
export function rehearsalTargetProblem(target: string, databaseUrl: string, stagingRefs: string | undefined) {
  const ref = projectRef(databaseUrl);
  if (!ref) return "DATABASE_URL does not parse";
  if (target === "local") {
    return LOCAL_HOSTS.has(new URL(databaseUrl).hostname) ? null : `--target local needs a database on this machine`;
  }
  if (target !== "staging") return "--target must be local or staging";
  const allowed = (stagingRefs ?? "").split(",").map((r) => r.trim()).filter(Boolean);
  if (allowed.length === 0) return "KINETIC_STAGING_DB_REFS is not set: list staging's project ref there";
  return allowed.includes(ref) ? null : `project ${ref} is not listed in KINETIC_STAGING_DB_REFS`;
}

// Refuses anything that looks like production. The caller also passes the target's name.
export function refuseProduction(env: { KINETIC_ENV?: string; DATABASE_URL?: string; KINETIC_PROD_DB_HOST?: string }) {
  if (env.KINETIC_ENV === "production") return "KINETIC_ENV is production";
  const prodHost = env.KINETIC_PROD_DB_HOST;
  if (prodHost && env.DATABASE_URL) {
    try {
      if (new URL(env.DATABASE_URL).hostname === prodHost) return "DATABASE_URL is the production database";
    } catch {
      return "DATABASE_URL does not parse";
    }
  }
  return null;
}

export async function rehearseFlip(ctx: WorkerContext, claimId: string, opts: RehearsalOptions) {
  const sql = ctx.sql;
  const steps: RehearsalStep[] = [];
  const record = (step: string, ok: boolean, detail: unknown) => {
    steps.push({ step, ok, detail });
    return ok;
  };
  const done = () => ({ claim_id: claimId, ok: steps.every((s) => s.ok), steps });

  const checklist = await claimChecklist(sql, claimId);
  if (!record("checklist", checklist.ready || !!opts.ignoreChecks, checklist)) return done();
  const before = await claimPreview(sql, claimId);
  record("preview", true, before);

  // Frozen: pull the end forward to now and let the worker's pass freeze it.
  await sql`update game.claims set ends_at = now() where claim_id = ${claimId} and state = 'active' and ends_at > now()`;
  await claimsPass(ctx);
  let claim = await claimRow(sql, claimId);
  if (!record("frozen", claim?.state === "frozen" || claim?.state === "tallied", { state: claim?.state })) {
    return done();
  }

  // Tallied: once every report stored during the Claim is processed.
  const passes = opts.maxRewardPasses ?? 20;
  for (let i = 0; i < passes && claim?.state === "frozen"; i++) {
    await rewardsPass(ctx);
    await claimsPass(ctx);
    claim = await claimRow(sql, claimId);
  }
  if (claim?.state === "frozen" && opts.forceTally) {
    await tallyClaim(sql, { claim_id: claimId, arena: claim.arena });
    claim = await claimRow(sql, claimId);
  }
  if (!record("tallied", claim?.state === "tallied", { state: claim?.state, outcome: claim?.outcome })) return done();
  const outcome = claim!.outcome ?? {};
  // Reports processed after the preview can change the holder legitimately: noted, not failed.
  record("tally_against_preview", true, {
    changed: outcome.holder !== before.holder || outcome.variant !== before.variant,
    preview: { holder: before.holder, variant: before.variant },
    tally: { holder: outcome.holder, variant: outcome.variant },
  });

  // Reviewed: a tie needs the reviewer's call.
  const needsCall = outcome.holder === null || outcome.holder === undefined;
  if (needsCall && !opts.holder) {
    record("reviewed", false, "the tally has no holder: rerun with --holder and --variant to rehearse the call");
    return done();
  }
  await sql.begin(async (tx) => {
    await tx`select set_config('kinetic.staff', ${opts.staff}, true)`;
    await tx`select api.srv_claim_review(${claimId}, ${opts.staff}, ${opts.holder ?? null}, ${opts.variant ?? null})`;
  });
  claim = await claimRow(sql, claimId);
  if (!record("reviewed", claim?.state === "reviewed", { state: claim?.state, holder: claim?.outcome?.holder })) {
    return done();
  }

  // Resolved: the arena's variant is in config for the next season.
  await sql.begin(async (tx) => {
    await tx`select set_config('kinetic.staff', ${opts.staff}, true)`;
    await tx`select api.srv_claim_resolve(${claimId})`;
  });
  claim = await claimRow(sql, claimId);
  const variant = claim?.outcome?.variant as string | undefined;
  const [flag] = await sql<{ value: unknown }[]>`select value from game.config where key = ${
    "variant:" + claim?.arena
  }`;
  record("resolved", claim?.state === "resolved" && (!variant || flag?.value === variant), {
    state: claim?.state,
    variant: variant ?? null,
    config: flag?.value ?? null,
  });

  // The next Claim: the worker activates it once it starts.
  await claimsPass(ctx);
  const [active] = await sql<{ claim_id: string }[]>`select claim_id from game.claims where state = 'active'`;
  const [next] = await sql<{ claim_id: string; starts_at: Date }[]>`
    select claim_id, starts_at from game.claims where state = 'scheduled' order by starts_at limit 1`;
  // On staging the next Claim usually starts days later; the worker activates it then.
  record("next_claim", !!active || !!next, {
    active: active?.claim_id ?? null,
    next_scheduled: next ? { claim_id: next.claim_id, starts_at: next.starts_at.toISOString() } : null,
  });
  return done();
}

// A readable report for the rehearsal's log.
export function rehearsalText(result: { claim_id: string; ok: boolean; steps: RehearsalStep[] }): string {
  const lines = [`Flip rehearsal for ${result.claim_id}: ${result.ok ? "PASSED" : "FAILED"}`];
  for (const s of result.steps) {
    lines.push(`  ${s.ok ? "ok  " : "FAIL"} ${s.step}`);
    if (s.step === "checklist") {
      for (const c of (s.detail as { checks: Check[] }).checks) {
        lines.push(`        ${c.ok ? "ok  " : c.blocking ? "FAIL" : "warn"} ${c.check}: ${c.detail}`);
      }
    } else if (!s.ok || ["tally_against_preview", "resolved", "next_claim"].includes(s.step)) {
      lines.push(`        ${JSON.stringify(s.detail)}`);
    }
  }
  return lines.join("\n");
}
