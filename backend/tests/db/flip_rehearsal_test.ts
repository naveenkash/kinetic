// The season flip from the console against the local database: the checklist holds the flip
// until the next season is set up and the build ships the variants; the preview matches the
// tally; the rehearsal runs the Claim through frozen, tallied, reviewed and resolved, and the
// Yard's next variant lands in config.
import { assert, assertEquals } from "jsr:@std/assert@1.0.13";
import postgres from "npm:postgres@3.4.5";
import { FACTIONS } from "../../worker/claims.ts";
import { claimsPass } from "../../worker/claims_pass.ts";
import type { WorkerContext } from "../../worker/loop.ts";
import { processReport, rulesetInForce } from "../../worker/rewards_pass.ts";
import { claimChecklist, claimPreview, rehearseFlip } from "../../tools/season.ts";

const DB_URL = Deno.env.get("DATABASE_URL") ?? "postgresql://postgres:postgres@127.0.0.1:55422/postgres";
const tag = `${Date.now() % 1_000_000}`;

function context(sql: postgres.Sql): WorkerContext {
  return {
    sql,
    functionsUrl: "",
    log: () => {},
    ...({ fleet: null, keyring: null } as unknown as Pick<WorkerContext, "fleet" | "keyring">),
  };
}

async function makeAccount(sql: postgres.Sql, name: string): Promise<string> {
  const id = crypto.randomUUID();
  const meta = { kinetic_provider: "dev", kinetic_provider_user_id: name };
  await sql`insert into auth.users (id, email, aud, role, raw_app_meta_data)
            values (${id}, ${name + "@users.kinetic.invalid"}, 'authenticated', 'authenticated', ${sql.json(meta)})`;
  await sql`select api.srv_ensure_account(${id}, 'dev', ${name}, ${name})`;
  return id;
}

async function makeReport(sql: postgres.Sql, winner: string, loser: string) {
  const match = crypto.randomUUID();
  await sql`insert into game.matches (match_id, playlist_id, arena, region, build, fleet, state)
            values (${match}, 'scrap_3v3_casual', 'YRD', 'local', '0.1.0', 'local', 'completed')`;
  await sql`insert into game.match_players (match_id, slot, team, account_id, harness) values
            (${match}, 1, 1, ${winner}, 'Rigger'), (${match}, 2, 2, ${loser}, 'Breaker')`;
  const facts = { eliminations: 1, counters: {} };
  const payload = {
    match_id: match,
    winner_team: 1,
    forfeit_team: null,
    no_contest: false,
    players: [
      { account_id: winner, team: 1, is_bot: false, abandoned: false, facts },
      { account_id: loser, team: 2, is_bot: false, abandoned: false, facts },
    ],
  };
  await sql`insert into game.match_reports (match_id, payload) values (${match}, ${sql.json(payload)})`;
  return match;
}

Deno.test("a season flip rehearsed from the console", async (t) => {
  const sql = postgres(DB_URL, { max: 2, onnotice: () => {} });
  const ctx = context(sql);
  const claim = `fr-${tag}`;
  const next = `fr-next-${tag}`;
  const staff = `flip-${tag}`;
  const accounts: string[] = [];
  const matches: string[] = [];
  const saved = await sql<{ key: string; value: unknown }[]>`
    select key, value from game.config where key in ('variants_shipped', 'variant:YRD')`;
  try {
    const [busy] = await sql`select claim_id from game.claims where state = 'active'`;
    assert(!busy, `another Claim is active locally (${busy?.claim_id}); archive it to run this test`);
    await sql`insert into admin.staff (staff_id) values (${staff}), ('test-cleanup') on conflict do nothing`;
    await sql`delete from game.config where key in ('variants_shipped', 'variant:YRD')`;
    const rules = await rulesetInForce(sql);
    const a = await makeAccount(sql, `fr_a_${tag}`);
    const b = await makeAccount(sql, `fr_b_${tag}`);
    accounts.push(a, b);
    await sql`insert into game.claims (claim_id, name, arena, state, starts_at, ends_at)
              values (${claim}, 'Rehearsal Claim', 'YRD', 'active', now() - interval '1 day', now() + interval '6 days')`;
    await sql`insert into game.allegiances (account_id, claim_id, faction) values (${a}, ${claim}, 'htr')`;
    await sql`insert into game.allegiance_history (account_id, claim_id, faction, declared_at)
              values (${a}, ${claim}, 'htr', now() - interval '1 hour')`;
    for (let i = 0; i < 2; i++) {
      const m = await makeReport(sql, a, b);
      matches.push(m);
      assertEquals(await processReport(ctx, m, rules), "processed");
    }
    // One more stored but not processed yet: the rehearsal's rewards pass takes it.
    matches.push(await makeReport(sql, a, b));
    await claimsPass(ctx); // Refreshes the active Claim's standing

    await t.step("the checklist holds the flip until the next season and the build are ready", async () => {
      const first = await claimChecklist(sql, claim);
      assert(!first.ready);
      const failed = first.checks.filter((c) => !c.ok).map((c) => c.check);
      assert(failed.includes("next_claim_scheduled") && failed.includes("variants_shipped"));
      assert(failed.includes("reports_processed") && !failed.includes("worker_running"));

      await sql`insert into game.claims (claim_id, name, arena, starts_at, ends_at)
                values (${next}, 'Next Claim', 'HGT', now() + interval '6 days 1 hour', now() + interval '20 days')`;
      const second = await claimChecklist(sql, claim);
      const pending = second.checks.filter((c) => !c.ok).map((c) => c.check);
      assert(pending.includes("next_claim_contracts") && pending.includes("next_claim_crates"));

      for (const f of FACTIONS) {
        await sql`insert into game.contracts (contract_id, claim_id, faction, week, counter_key, target, title, reward)
                  values (${`${next}-${f}`}, ${next}, ${f}, 1, 'eliminations', 3, 'Three scraps.', ${sql.json({})})`;
      }
      await sql`insert into game.manifest_crates (claim_id, track, tier, tonnage_t, reward) values
                (${next}, 'free', 1, 100, ${sql.json({})}), (${next}, 'sponsored', 1, 100, ${sql.json({})})`;
      await sql`insert into game.config (key, value) values ('variants_shipped', ${sql.json(["haulers_doubled"])})`;
      const third = await claimChecklist(sql, claim);
      assert(third.ready, JSON.stringify(third.checks.filter((c) => !c.ok)));
      // Only the warning is left: one report is still waiting for the rewards pass.
      assertEquals(third.checks.filter((c) => !c.ok).map((c) => c.check), ["reports_processed"]);
    });

    await t.step("the preview names the holder and the Yard's next variant, read-only", async () => {
      const preview = await claimPreview(sql, claim);
      assertEquals([preview.holder, preview.variant, preview.state], ["htr", "haulers_doubled", "active"]);
      assertEquals(preview.anomalies.unprocessed_reports, [matches[2]]);
      const [still] = await sql`select state from game.claims where claim_id = ${claim}`;
      assertEquals(still.state, "active");
    });

    await t.step("the rehearsal runs the flip through to resolved", async () => {
      const result = await rehearseFlip(ctx, claim, { staff });
      assert(result.ok, JSON.stringify(result.steps.filter((s) => !s.ok)));
      assertEquals(
        result.steps.map((s) => s.step),
        ["checklist", "preview", "frozen", "tallied", "tally_against_preview", "reviewed", "resolved", "next_claim"],
      );
      const [done] = await sql`select state, outcome, reviewed_by from game.claims where claim_id = ${claim}`;
      assertEquals([done.state, done.outcome.holder, done.outcome.variant], ["resolved", "htr", "haulers_doubled"]);
      const [flag] = await sql`select value from game.config where key = 'variant:YRD'`;
      assertEquals(flag.value, "haulers_doubled");
      // The third match was processed before the tally: three wins at 3 P each.
      const [htr] = await sql`select points_total from game.claim_standing
                              where claim_id = ${claim} and faction = 'htr'`;
      assertEquals(Number(htr.points_total), 9);
      const audit = await sql`select action from admin.audit where staff_id = ${staff} and target = ${
        "claims:" + claim
      }`;
      assert(audit.length >= 2); // Reviewed and resolved, under the rehearsing staff member
    });

    await t.step("a Claim already resolved is not rehearsed again", async () => {
      const again = await rehearseFlip(ctx, claim, { staff });
      assertEquals([again.ok, again.steps.length, again.steps[0].step], [false, 1, "checklist"]);
    });
  } finally {
    await sql.begin(async (tx) => {
      await tx`select set_config('kinetic.staff', 'test-cleanup', true)`;
      await tx`alter table game.claim_standing_events disable trigger standing_events_append_only`;
      await tx`delete from game.claim_standing_events where claim_id in (${claim}, ${next})`;
      await tx`alter table game.claim_standing_events enable trigger standing_events_append_only`;
      for (const c of [claim, next]) {
        await tx`delete from game.allegiance_history where claim_id = ${c}`;
        await tx`delete from game.manifest_paid where claim_id = ${c}`;
        await tx`delete from game.manifest_progress where claim_id = ${c}`;
        await tx`delete from game.manifest_crates where claim_id = ${c}`;
        await tx`delete from game.claim_standing where claim_id = ${c}`;
        await tx`delete from game.contract_progress where contract_id like ${c + "%"}`;
        await tx`delete from game.contracts where claim_id = ${c}`;
        await tx`delete from game.allegiances where claim_id = ${c}`;
        await tx`delete from game.claims where claim_id = ${c}`;
      }
      if (matches.length) {
        await tx`delete from game.match_reports where match_id in ${tx(matches)}`;
        await tx`delete from game.matches where match_id in ${tx(matches)}`;
      }
      if (accounts.length) await tx`delete from auth.users where id in ${tx(accounts)}`;
      await tx`delete from game.config where key in ('variants_shipped', 'variant:YRD')`;
      for (const row of saved) {
        await tx`insert into game.config (key, value) values (${row.key}, ${tx.json(row.value as postgres.JSONValue)})`;
      }
    });
    await sql.end();
  }
});

Deno.test("a tie needs the reviewer's call, and a rehearsal resumes from tallied", async () => {
  const sql = postgres(DB_URL, { max: 2, onnotice: () => {} });
  const ctx = context(sql);
  const claim = `ft-${tag}`;
  const staff = `flip-${tag}`;
  const saved = await sql<
    { key: string; value: unknown }[]
  >`select key, value from game.config where key = 'variant:BLS'`;
  try {
    const [busy] = await sql`select claim_id from game.claims where state = 'active'`;
    assert(!busy, `another Claim is active locally (${busy?.claim_id})`);
    await sql`insert into admin.staff (staff_id) values (${staff}), ('test-cleanup') on conflict do nothing`;
    // Nobody played: no holder.
    await sql`insert into game.claims (claim_id, name, arena, state, starts_at, ends_at)
              values (${claim}, 'Empty Claim', 'BLS', 'active', now() - interval '1 day', now() + interval '6 days')`;
    const first = await rehearseFlip(ctx, claim, { staff, ignoreChecks: true });
    assertEquals(first.ok, false);
    assertEquals(first.steps.at(-1)?.step, "reviewed");
    const [tallied] = await sql`select state, outcome from game.claims where claim_id = ${claim}`;
    assertEquals([tallied.state, tallied.outcome.holder], ["tallied", null]);

    const second = await rehearseFlip(ctx, claim, {
      staff,
      ignoreChecks: true,
      holder: "guild",
      variant: "stabilized",
    });
    // Every step up to resolved passes; the next-Claim step depends on what else is scheduled here.
    assert(second.steps.filter((x) => x.step !== "next_claim").every((x) => x.ok), JSON.stringify(second.steps));
    assertEquals(second.steps.map((x) => x.step).slice(2, 4), ["frozen", "tallied"]);
    const [done] = await sql`select state, outcome, reviewed_by from game.claims where claim_id = ${claim}`;
    assertEquals([done.state, done.outcome.holder, done.outcome.variant, done.reviewed_by], [
      "resolved",
      "guild",
      "stabilized",
      staff,
    ]);
  } finally {
    await sql.begin(async (tx) => {
      await tx`select set_config('kinetic.staff', 'test-cleanup', true)`;
      await tx`delete from game.claim_standing where claim_id = ${claim}`;
      await tx`delete from game.claims where claim_id = ${claim}`;
      await tx`delete from game.config where key = 'variant:BLS'`;
      for (const row of saved) {
        await tx`insert into game.config (key, value) values (${row.key}, ${tx.json(row.value as postgres.JSONValue)})`;
      }
    });
    await sql.end();
  }
});
