// The rewards pass against the local database (supabase start): one report in, Ledger, Rope
// Mark, career, entry and summary rows out, in one transaction; quarantine and retry paths.
// Needs only the database, not the edge functions or the worker.
import { assert, assertEquals, assertRejects } from "jsr:@std/assert@1.0.13";
import postgres from "npm:postgres@3.4.5";
import type { WorkerContext } from "../../worker/loop.ts";
import { MAX_FAILURES, MAX_PROCESS_ATTEMPTS, processReport, rulesetInForce } from "../../worker/rewards_pass.ts";

const DB_URL = Deno.env.get("DATABASE_URL") ?? "postgresql://postgres:postgres@127.0.0.1:55422/postgres";

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

function facts(overrides: Record<string, unknown> = {}) {
  return {
    mass_distance_kg_m: 3_120_000,
    eliminations: 3,
    assists: 1,
    spotter_credits: 2,
    ringout_heights_m: [38],
    rope_marks: [{ rig: "Gallows" }],
    counters: { rated_tons_moved: 0 },
    floor_seconds: 100,
    airborne_seconds: 20,
    hoisted_seconds: 5,
    ...overrides,
  };
}

// Player a (team 1, a Rigger on the roster) wins against b (team 2, who abandons) and a bot.
async function makeReport(sql: postgres.Sql, a: string, b: string, aFacts = facts(), aId = a): Promise<string> {
  const match = crypto.randomUUID();
  await sql`insert into game.matches (match_id, playlist_id, arena, region, build, fleet, state)
            values (${match}, 'scrap_3v3_casual', 'YRD', 'local', '0.1.0', 'local', 'completed')`;
  await sql`insert into game.match_players (match_id, slot, team, account_id, is_bot, harness) values
            (${match}, 1, 1, ${a}, false, 'Rigger'), (${match}, 2, 2, ${b}, false, 'Breaker'),
            (${match}, 3, 2, null, true, 'Rigger')`;
  const payload = {
    match_id: match,
    winner_team: 1,
    no_contest: false,
    players: [
      // The report's harness is ignored: career stats go under the roster's.
      { account_id: aId, team: 1, harness: "Climber", is_bot: false, abandoned: false, facts: aFacts },
      { account_id: b, team: 2, harness: "Breaker", is_bot: false, abandoned: true, facts: facts() },
      { account_id: null, team: 2, is_bot: true, abandoned: false },
    ],
  };
  await sql`insert into game.match_reports (match_id, payload) values (${match}, ${sql.json(payload)})`;
  return match;
}

Deno.test("the rewards pass", async (t) => {
  const sql = postgres(DB_URL, { max: 2, onnotice: () => {} });
  const ctx = context(sql);
  const rules = await rulesetInForce(sql);
  const accounts: string[] = [];
  const matches: string[] = [];
  try {
    const a = await makeAccount(sql, `rw_a_${Date.now() % 100000}`);
    const b = await makeAccount(sql, `rw_b_${Date.now() % 100000}`);
    accounts.push(a, b);

    await t.step("a report pays the earner once, and only the earner", async () => {
      const match = await makeReport(sql, a, b);
      matches.push(match);
      assertEquals(await processReport(ctx, match, rules), "processed");
      assertEquals(await processReport(ctx, match, rules), null);
      const [ledger] = await sql`select tonnage_total, grade, weight_balance from game.ledgers where account_id = ${a}`;
      assertEquals([Number(ledger.tonnage_total), ledger.grade, Number(ledger.weight_balance)], [
        2720,
        2,
        45000 - 2720,
      ]);
      const [serial] = await sql`select cutter_serial from game.ledgers where account_id = ${a}`;
      assert(/^CT-\d{6}$/.test(serial.cutter_serial));
      const [loser] = await sql`select tonnage_total, weight_balance from game.ledgers where account_id = ${b}`;
      assertEquals([Number(loser.tonnage_total), loser.weight_balance], [0, null]);
      const stats = await sql`select harness, matches, wins, scraps, ringouts from game.career_stats
                              where account_id = ${a}`;
      assertEquals(stats.map((s) => [s.harness, s.matches, s.wins, s.scraps, s.ringouts]), [["Rigger", 1, 1, 3, 1]]);
      const marks = await sql`select rig, count, tier from game.rope_marks where account_id = ${a}`;
      assertEquals(marks.map((m) => [m.rig, m.count, m.tier]), [["Gallows", 1, "tape"]]);
      const entries = await sql`select entry_id from game.ledger_entries where account_id = ${a} order by entry_id`;
      assertEquals(entries.map((e) => e.entry_id), [
        "cutter-ceremony",
        "first-ringout",
        "first-scrap",
        "first-spotter-credit",
      ]);
      const summaries = await sql`select account_id, tonnage, grade_before, grade_after from game.match_summaries
                                  where match_id = ${match}`;
      assertEquals(summaries.map((s) => [s.account_id, s.tonnage, s.grade_before, s.grade_after]), [[a, 2720, 1, 2]]);
      const [report] = await sql`select status, ruleset from game.match_reports where match_id = ${match}`;
      assertEquals([report.status, report.ruleset], ["processed", 1]);
      const [wallet] =
        await sql`select balance from game.wallets where account_id = ${a} and currency = 'tonnage_credit'`;
      assertEquals(Number(wallet.balance), 272);
      const txns = await sql`select delta, reason, ref from game.wallet_txns where account_id = ${a}`;
      assertEquals(txns.map((t) => [Number(t.delta), t.reason, t.ref]), [[272, "match", match]]);
    });

    await t.step("a second match adds up, the serial is kept, and an uppercase id still pays", async () => {
      const [before] = await sql`select cutter_serial from game.ledgers where account_id = ${a}`;
      const marks = facts({ rope_marks: [{ rig: "Gallows" }, { rig: "Kiss" }] });
      const match = await makeReport(sql, a, b, marks, a.toUpperCase());
      matches.push(match);
      assertEquals(await processReport(ctx, match, rules), "processed");
      const [ledger] = await sql`select tonnage_total, cutter_serial from game.ledgers where account_id = ${a}`;
      assertEquals([Number(ledger.tonnage_total), ledger.cutter_serial], [5440, before.cutter_serial]);
      const rows = await sql`select rig, count from game.rope_marks where account_id = ${a} order by rig`;
      assertEquals(rows.map((m) => [m.rig, m.count]), [["Gallows", 2], ["Kiss", 1]]);
    });

    await t.step("facts over the ceilings quarantine the report and pay nothing", async () => {
      const match = await makeReport(sql, a, b, facts({ eliminations: 999 }));
      matches.push(match);
      assertEquals(await processReport(ctx, match, rules), "quarantined");
      const [report] = await sql`select status, quarantine_reason from game.match_reports where match_id = ${match}`;
      assertEquals(report.status, "quarantined");
      assert(report.quarantine_reason.startsWith("fact_ceiling"));
      const [ledger] = await sql`select tonnage_total from game.ledgers where account_id = ${a}`;
      assertEquals(Number(ledger.tonnage_total), 5440);
    });

    await t.step("a report that fails on its own data backs off, then is quarantined, half-written never", async () => {
      const match = await makeReport(sql, a, b);
      matches.push(match);
      // A ruleset that pays more Tonnage than a summary row holds: a data error from this report.
      const overpaying = { ...rules, tonnage: { ...rules.tonnage, per_scrap: 1e12 } };
      for (let i = 0; i < MAX_PROCESS_ATTEMPTS; i++) {
        assertEquals(await processReport(ctx, match, overpaying), "failed");
      }
      const [report] = await sql`select status, quarantine_reason, process_attempts, next_attempt_at > now() as later
                                 from game.match_reports where match_id = ${match}`;
      assertEquals([report.status, report.quarantine_reason, report.process_attempts, report.later], [
        "quarantined",
        "processing_failed",
        MAX_PROCESS_ATTEMPTS,
        true,
      ]);
      assertEquals((await sql`select 1 from game.match_summaries where match_id = ${match}`).length, 0);
      const [ledger] = await sql`select tonnage_total from game.ledgers where account_id = ${a}`;
      assertEquals(Number(ledger.tonnage_total), 5440);
      assertEquals((await sql`select api.srv_requeue_report(${match}) as r`)[0].r.status, "stored");
      assertEquals(await processReport(ctx, match, rules), "processed");
    });

    await t.step("a failure the report did not cause backs off without counting", async () => {
      const match = await makeReport(sql, a, b);
      matches.push(match);
      // A bug, not the report: a ruleset missing its rig list throws a TypeError mid-transaction.
      const broken = { ...rules, rigs: null as unknown as string[] };
      assertEquals(await processReport(ctx, match, broken), "error");
      assertEquals(await processReport(ctx, match, broken), "error");
      const [report] =
        await sql`select status, process_attempts, failures, next_attempt_at > now() + interval '45 s' as doubled
                                 from game.match_reports where match_id = ${match}`;
      assertEquals([report.status, report.process_attempts, report.failures, report.doubled], ["stored", 0, 2, true]);
      assertEquals((await sql`select 1 from game.match_summaries where match_id = ${match}`).length, 0);
      for (let i = 2; i < MAX_FAILURES; i++) await processReport(ctx, match, broken);
      const [stuck] = await sql`select status, quarantine_reason from game.match_reports where match_id = ${match}`;
      assertEquals([stuck.status, stuck.quarantine_reason], ["quarantined", "processing_failed"]);
    });

    await t.step("an unreachable database records nothing and is not counted", async () => {
      const match = await makeReport(sql, a, b);
      matches.push(match);
      const down = postgres("postgresql://postgres:postgres@127.0.0.1:1/postgres", { max: 1, connect_timeout: 1 });
      try {
        assertEquals(await processReport(context(down), match, rules), "error");
      } finally {
        await down.end({ timeout: 1 });
      }
      const [report] = await sql`select status, process_attempts from game.match_reports where match_id = ${match}`;
      assertEquals([report.status, report.process_attempts], ["stored", 0]);
    });

    await t.step("the worker refuses a ruleset that differs from the clients' copy", async () => {
      const rollback = new Error("rollback");
      await assertRejects(
        () =>
          sql.begin(async (tx) => {
            await tx`update game.rulesets set rules = rules || '{"tonnage_credit_per_t": 1}' where version = ${rules.version}`;
            await assertRejects(() => rulesetInForce(tx), Error, "not the copy clients read");
            throw rollback;
          }),
        Error,
        "rollback",
      );
    });
  } finally {
    if (matches.length) {
      await sql`delete from game.match_reports where match_id in ${sql(matches)}`;
      await sql`delete from game.matches where match_id in ${sql(matches)}`;
    }
    if (accounts.length) await sql`delete from auth.users where id in ${sql(accounts)}`;
    await sql.end();
  }
});
