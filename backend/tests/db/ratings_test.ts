// The Office Rating through the rewards pass against the local database: Rated playlists only,
// the leaver rated as a loss, bands recorded on the summary, casual matches unrated.
import { assert, assertEquals } from "jsr:@std/assert@1.0.13";
import postgres from "npm:postgres@3.4.5";
import type { WorkerContext } from "../../worker/loop.ts";
import { MU0 } from "../../worker/rating.ts";
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

// `a` and `c` (team 1) beat `b` (team 2); `c` left the match before it ended.
async function makeReport(
  sql: postgres.Sql,
  playlist: string,
  a: string,
  b: string,
  c: string,
  extra: Record<string, unknown> = {},
) {
  const match = crypto.randomUUID();
  await sql`insert into game.matches (match_id, playlist_id, arena, region, build, fleet, state)
            values (${match}, ${playlist}, 'YRD', 'local', '0.1.0', 'local', 'completed')`;
  await sql`insert into game.match_players (match_id, slot, team, account_id, harness) values
            (${match}, 1, 1, ${a}, 'Rigger'), (${match}, 2, 2, ${b}, 'Breaker'), (${match}, 3, 1, ${c}, 'Climber')`;
  const human = (account: string, team: number, abandoned = false) => ({
    account_id: account,
    team,
    is_bot: false,
    abandoned,
    facts: { eliminations: 1 },
  });
  const payload = {
    match_id: match,
    winner_team: 1,
    forfeit_team: null,
    no_contest: false,
    players: [human(a, 1), human(b, 2), human(c, 1, true)],
    ...extra,
  };
  await sql`insert into game.match_reports (match_id, payload) values (${match}, ${sql.json(payload)})`;
  return match;
}

Deno.test("the Office Rating", async (t) => {
  const sql = postgres(DB_URL, { max: 2, onnotice: () => {} });
  const ctx = context(sql);
  const accounts: string[] = [];
  const matches: string[] = [];
  try {
    const rules = await rulesetInForce(sql);
    for (const n of ["a", "b", "c"]) accounts.push(await makeAccount(sql, `rt_${n}_${tag}`));
    const [a, b, c] = accounts;

    await t.step("a casual match leaves Ratings alone", async () => {
      const match = await makeReport(sql, "scrap_3v3_casual", a, b, c);
      matches.push(match);
      assertEquals(await processReport(ctx, match, rules), "processed");
      assertEquals((await sql`select 1 from game.ratings where account_id in ${sql(accounts)}`).length, 0);
    });

    await t.step("a Rated match rates the roster, and the leaver loses", async () => {
      const match = await makeReport(sql, "scrap_3v3_rated", a, b, c);
      matches.push(match);
      assertEquals(await processReport(ctx, match, rules), "processed");
      const rows = await sql`select account_id, mu, games, band from game.ratings where account_id in ${sql(accounts)}`;
      const mu = (id: string) => rows.find((r) => r.account_id === id)!.mu;
      assert(mu(a) > MU0, "the winner gains");
      assert(mu(b) < MU0, "the loser loses");
      assert(mu(c) < MU0, "the leaver loses although the crew won");
      assertEquals(rows.map((r) => [r.games, r.band]), [[1, "Provisional"], [1, "Provisional"], [1, "Provisional"]]);
      const [s] = await sql`select rating from game.match_summaries where match_id = ${match} and account_id = ${a}`;
      assertEquals(s.rating, { band_before: "Provisional", band: "Provisional", games: 1 });
    });

    await t.step("a penalty-free forfeit spares the short crew, not the leaver", async () => {
      const before = await sql`select account_id, mu, games from game.ratings where account_id in ${sql(accounts)}`;
      const match = await makeReport(sql, "scrap_3v3_rated", a, b, c, {
        winner_team: 2,
        forfeit_team: 1,
        forfeit_penalty_free: true,
      });
      matches.push(match);
      assertEquals(await processReport(ctx, match, rules), "processed");
      const after = await sql`select account_id, mu, games from game.ratings where account_id in ${sql(accounts)}`;
      const row = (rows: typeof after, id: string) => rows.find((r) => r.account_id === id)!;
      assertEquals([row(after, a).mu, row(after, a).games], [row(before, a).mu, row(before, a).games]);
      assertEquals(row(after, c).games, row(before, c).games + 1);
      assert(row(after, c).mu < row(before, c).mu, "the leaver takes the full loss");
      assert(row(after, b).mu > row(before, b).mu, "the other crew is rated as winners");
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
