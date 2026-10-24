// Claims through the rewards pass against the local database (supabase start, deno task
// content): Standing per match under the weekly cap, Contracts and their rewards, a voided
// forfeit, then the freeze and the tally.
import { assert, assertEquals } from "jsr:@std/assert@1.0.13";
import postgres from "npm:postgres@3.4.5";
import { claimsPass } from "../../worker/claims_pass.ts";
import type { WorkerContext } from "../../worker/loop.ts";
import { processReport, rulesetInForce } from "../../worker/rewards_pass.ts";

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

// A casual Scrap that `a` (team 1) wins against `b`, with a snap survived.
async function makeReport(sql: postgres.Sql, a: string, b: string, extra: Record<string, unknown> = {}) {
  const match = crypto.randomUUID();
  await sql`insert into game.matches (match_id, playlist_id, arena, region, build, fleet, state)
            values (${match}, 'scrap_3v3_casual', 'YRD', 'local', '0.1.0', 'local', 'completed')`;
  await sql`insert into game.match_players (match_id, slot, team, account_id, harness) values
            (${match}, 1, 1, ${a}, 'Rigger'), (${match}, 2, 2, ${b}, 'Breaker')`;
  const facts = { eliminations: 1, counters: { snaps_survived: 1 } };
  const payload = {
    match_id: match,
    winner_team: 1,
    forfeit_team: null,
    no_contest: false,
    players: [
      { account_id: a, team: 1, is_bot: false, abandoned: false, facts },
      { account_id: b, team: 2, is_bot: false, abandoned: false, facts },
    ],
    ...extra,
  };
  await sql`insert into game.match_reports (match_id, payload) values (${match}, ${sql.json(payload)})`;
  return match;
}

Deno.test("Claims through the rewards pass", async (t) => {
  const sql = postgres(DB_URL, { max: 2, onnotice: () => {} });
  const ctx = context(sql);
  const claim = `w-${tag}`;
  const accounts: string[] = [];
  const matches: string[] = [];
  try {
    const [busy] = await sql`select claim_id from game.claims where state = 'active'`;
    assert(!busy, `another Claim is active locally (${busy?.claim_id}); archive it to run this test`);
    const rules = await rulesetInForce(sql);
    const a = await makeAccount(sql, `cl_a_${tag}`);
    const b = await makeAccount(sql, `cl_b_${tag}`);
    accounts.push(a, b);
    await sql`insert into game.claims (claim_id, name, arena, state, starts_at, ends_at)
              values (${claim}, 'Test Claim', 'YRD', 'active', now() - interval '1 day', now() + interval '6 days')`;
    await sql`insert into game.contracts (contract_id, claim_id, faction, week, counter_key, target, title, reward)
              values (${claim + "-snap"}, ${claim}, 'guild', 1, 'snaps_survived', 2, 'Snap a line. Survive it.',
                      ${sql.json({ items: ["tape.guild_ochre_black"], entries: ["guild-entry-41"] })})`;
    await sql`insert into game.manifest_crates (claim_id, track, tier, tonnage_t, reward) values
              (${claim}, 'free', 1, 100, ${sql.json({ items: ["livery.kessl_ochre"] })}),
              (${claim}, 'free', 2, 10000, ${sql.json({ entries: ["entry-38"] })}),
              (${claim}, 'sponsored', 1, 100, ${sql.json({ items: ["drop.off_the_lift"] })})`;
    await sql`insert into game.allegiances (account_id, claim_id, faction) values (${a}, ${claim}, 'guild')`;
    await sql`insert into game.allegiance_history (account_id, claim_id, faction, declared_at)
              values (${a}, ${claim}, 'guild', now() - interval '1 hour')`;

    await t.step("a win pays Standing; the Contract advances; no Allegiance pays none", async () => {
      const match = await makeReport(sql, a, b);
      matches.push(match);
      assertEquals(await processReport(ctx, match, rules), "processed");
      const [s] =
        await sql`select standing, contracts from game.match_summaries where match_id = ${match} and account_id = ${a}`;
      assertEquals(s.standing, { faction: "guild", points: 3 });
      assertEquals(s.contracts, [{ contract_id: claim + "-snap", progress: 1, completed: false }]);
      const [other] = await sql`select standing, manifest from game.match_summaries
                                where match_id = ${match} and account_id = ${b}`;
      assertEquals(other.standing, null);
      // The Manifest is not faction-bound: both players tick the Free crate.
      assertEquals(other.manifest, { tonnage_t: 120, free: [1], sponsored: [] });
    });

    await t.step("completing the Contract grants its reward and +2 P; sponsoring pays a passed crate", async () => {
      await sql`insert into game.entitlements (account_id, entitlement) values (${a}, ${"sponsorship:" + claim})`;
      const match = await makeReport(sql, a, b);
      matches.push(match);
      assertEquals(await processReport(ctx, match, rules), "processed");
      const [s] = await sql`select standing from game.match_summaries where match_id = ${match} and account_id = ${a}`;
      assertEquals(s.standing, { faction: "guild", points: 5 });
      const items = await sql`select item_id, source from game.inventory where account_id = ${a} order by item_id`;
      assertEquals(items.map((i) => [i.item_id, i.source]), [
        ["drop.off_the_lift", "manifest"],
        ["livery.kessl_ochre", "manifest"],
        ["tape.guild_ochre_black", "contract"],
      ]);
      // The passed Sponsored crate was paid when the sponsorship was granted, not by this match.
      const [m] = await sql`select manifest from game.match_summaries where match_id = ${match} and account_id = ${a}`;
      assertEquals(m.manifest, { tonnage_t: 240, free: [], sponsored: [] });
      const entries =
        await sql`select 1 from game.ledger_entries where account_id = ${a} and entry_id = 'guild-entry-41'`;
      assertEquals(entries.length, 1);
    });

    await t.step("a forfeit pays no Standing and advances nothing", async () => {
      const match = await makeReport(sql, a, b, { forfeit_team: 2 });
      matches.push(match);
      assertEquals(await processReport(ctx, match, rules), "processed");
      const [s] =
        await sql`select standing, contracts from game.match_summaries where match_id = ${match} and account_id = ${a}`;
      assertEquals([s.standing, s.contracts], [{ faction: "guild", points: 0 }, []]);
    });

    await t.step("the weekly cap clips", async () => {
      await sql`insert into game.claim_standing_events (claim_id, faction, points, reason, account_id, week)
                values (${claim}, 'guild', 51, 'evidence', ${a}, 1)`;
      const match = await makeReport(sql, a, b);
      matches.push(match);
      await processReport(ctx, match, rules);
      const [s] = await sql`select standing from game.match_summaries where match_id = ${match} and account_id = ${a}`;
      assertEquals(s.standing, { faction: "guild", points: 1 });
    });

    await t.step("a match stored before a switch counts for the old faction, without Contracts", async () => {
      const match = await makeReport(sql, a, b);
      matches.push(match);
      await sql`update game.allegiances set faction = 'fray' where account_id = ${a} and claim_id = ${claim}`;
      await sql`insert into game.allegiance_history (account_id, claim_id, faction) values (${a}, ${claim}, 'fray')`;
      await processReport(ctx, match, rules);
      const [s] =
        await sql`select standing, contracts from game.match_summaries where match_id = ${match} and account_id = ${a}`;
      assertEquals([s.standing, s.contracts], [{ faction: "guild", points: 0 }, []]);
    });

    await t.step("after its grace a frozen Claim is tallied anyway, listing what never processed", async () => {
      const stuck = await makeReport(sql, a, b);
      matches.push(stuck);
      await sql`update game.claims set starts_at = now() - interval '3 days', ends_at = now() - interval '25 hours'
                 where claim_id = ${claim}`;
      await sql`update game.match_reports set received_at = now() - interval '2 days' where match_id = ${stuck}`;
      await claimsPass(ctx);
      const [done] = await sql`select state, outcome, review from game.claims where claim_id = ${claim}`;
      assertEquals([done.state, done.outcome.holder, done.outcome.variant], ["tallied", "guild", "base"]);
      assertEquals(done.review.anomalies.unprocessed_reports, [stuck]);
      assertEquals(done.outcome.anomalies, undefined);
      const [guild] = await sql`select points_total, active_players from game.claim_standing
                                where claim_id = ${claim} and faction = 'guild'`;
      assertEquals([Number(guild.points_total), guild.active_players], [60, 1]);
    });
  } finally {
    await sql.begin(async (tx) => {
      // Standing events are append-only; cleanup lifts that trigger alone, so cascades still run.
      await tx`alter table game.claim_standing_events disable trigger standing_events_append_only`;
      await tx`delete from game.claim_standing_events where claim_id = ${claim}`;
      await tx`alter table game.claim_standing_events enable trigger standing_events_append_only`;
      await tx`delete from game.allegiance_history where claim_id = ${claim}`;
      await tx`delete from game.manifest_paid where claim_id = ${claim}`;
      await tx`delete from game.manifest_progress where claim_id = ${claim}`;
      await tx`delete from game.manifest_crates where claim_id = ${claim}`;
      await tx`delete from game.claim_standing where claim_id = ${claim}`;
      await tx`delete from game.contract_progress where contract_id like ${claim + "%"}`;
      await tx`delete from game.contracts where claim_id = ${claim}`;
      await tx`delete from game.allegiances where claim_id = ${claim}`;
      await tx`delete from game.claims where claim_id = ${claim}`;
      if (matches.length) {
        await tx`delete from game.match_reports where match_id in ${tx(matches)}`;
        await tx`delete from game.matches where match_id in ${tx(matches)}`;
      }
      if (accounts.length) await tx`delete from auth.users where id in ${tx(accounts)}`;
    });
    await sql.end();
  }
});
