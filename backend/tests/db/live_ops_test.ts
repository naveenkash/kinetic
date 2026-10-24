// Live ops from the console against the local database: config, the message of the day and
// feature flags reach the client's config; playlists and regions change under audit; builds
// rise only in order and only once old matches drain; the health and player views; and a
// console identity linked to an account signs in to it.
import { assert, assertEquals, assertRejects } from "jsr:@std/assert@1.0.13";
import postgres from "npm:postgres@3.4.5";
import { parseArgs, type Parsed, runCommand } from "../../tools/admin.ts";

const DB_URL = Deno.env.get("DATABASE_URL") ?? "postgresql://postgres:postgres@127.0.0.1:55422/postgres";
const tag = `${Date.now() % 1_000_000}`;
const staff = `ops-${tag}`;

function command(...argv: string[]): Parsed {
  const parsed = parseArgs(["--staff", staff, ...argv]);
  if (typeof parsed === "string") throw new Error(parsed);
  return parsed;
}

async function makeAccount(sql: postgres.Sql, name: string): Promise<string> {
  const id = crypto.randomUUID();
  await sql`insert into auth.users (id, email, aud, role, raw_app_meta_data)
            values (${id}, ${name + "@users.kinetic.invalid"}, 'authenticated', 'authenticated',
                    ${sql.json({ kinetic_provider: "dev", kinetic_provider_user_id: name })})`;
  await sql`select api.srv_ensure_account(${id}, 'dev', ${name}, ${name})`;
  return id;
}

Deno.test("live ops", async (t) => {
  const sql = postgres(DB_URL, { max: 2, onnotice: () => {} });
  const saved = await sql<{ key: string; value: unknown }[]>`
    select key, value from game.config where key in ('flags', 'motd', 'min_build', 'latest_build', ${`ops-test-${tag}`})`;
  const [playlist] = await sql`select playlist_id, enabled, regions, bot_fill_after_s from game.playlists
                               where playlist_id = 'scrap_3v3_casual'`;
  const accounts: string[] = [];
  const matches: string[] = [];
  try {
    await sql`insert into admin.staff (staff_id) values (${staff}), ('test-cleanup') on conflict do nothing`;
    const a = await makeAccount(sql, `ops_a_${tag}`);
    const b = await makeAccount(sql, `ops_b_${tag}`);
    accounts.push(a, b);

    await t.step("the message of the day and feature flags reach the client's config", async () => {
      await runCommand(sql, command("motd:set", "--text", "Shift Change at 1800."));
      await runCommand(sql, command("flag:set", "--flag", `voice_${tag}`, "--value", "true"));
      await runCommand(sql, command("flag:set", "--flag", `limit_${tag}`, "--value", "3"));
      const [{ config }] = await sql`select api.get_config() as config`;
      assertEquals(config.motd, "Shift Change at 1800.");
      assertEquals([config.flags[`voice_${tag}`], config.flags[`limit_${tag}`]], [true, 3]);
      assert("variants" in config && "claim" in config);
      await runCommand(sql, command("motd:set", "--text", " "));
      const [{ config: cleared }] = await sql`select api.get_config() as config`;
      assertEquals(cleared.motd, null);
    });

    await t.step("config:set takes JSON and leaves keys with their own command alone", async () => {
      await runCommand(sql, command("config:set", "--key", `ops-test-${tag}`, "--value", '{"a":1}'));
      const [row] = await runCommand(sql, command("config:get", "--key", `ops-test-${tag}`)) as { value: unknown }[];
      assertEquals(row.value, { a: 1 });
      for (const key of ["min_build", "flags", "motd", "variant:YRD", "variants_shipped"]) {
        await assertRejects(
          () => runCommand(sql, command("config:set", "--key", key, "--value", "1")),
          Error,
          "own command",
        );
      }
      await assertRejects(
        () => runCommand(sql, command("config:set", "--key", `ops-test-${tag}`, "--value", "not json")),
        Error,
        "must be JSON",
      );
    });

    await t.step("playlists and regions change under audit", async () => {
      await runCommand(sql, command("playlist:set", "--id", "scrap_3v3_casual", "--enabled", "false"));
      const [{ config }] = await sql`select api.get_config() as config`;
      assert(!config.playlists.some((p: { playlist_id: string }) => p.playlist_id === "scrap_3v3_casual"));
      await runCommand(sql, command("playlist:set", "--id", "scrap_3v3_casual", "--enabled", "true"));
      await assertRejects(
        () => runCommand(sql, command("playlist:set", "--id", "scrap_3v3_casual", "--regions", "nowhere")),
        Error,
        "region that does not exist",
      );
      await assertRejects(() => runCommand(sql, command("playlist:set", "--id", "nope")), Error, "no such playlist");
      await assertRejects(
        () => runCommand(sql, command("playlist:set", "--id", "scrap_3v3_casual", "--regions", " , ")),
        Error,
        "at least one region",
      );
      await assertRejects(
        () => runCommand(sql, command("playlist:set", "--id", "scrap_3v3_casual", "--enabled", "maybe")),
        Error,
        "true or false",
      );
      const audit = await sql`select action from admin.audit where staff_id = ${staff}
                              and target = 'playlists:scrap_3v3_casual'`;
      assertEquals(audit.length, 2);
      await runCommand(sql, command("region:set", "--id", "local", "--enabled", "false"));
      const [{ config: noLocal }] = await sql`select api.get_config() as config`;
      assert(!noLocal.regions.some((r: { region_id: string }) => r.region_id === "local"));
      await runCommand(sql, command("region:set", "--id", "local", "--enabled", "true"));
      await assertRejects(
        () => runCommand(sql, command("region:set", "--id", "nope", "--enabled", "true")),
        Error,
        "no such region",
      );
      const regionAudit =
        await sql`select action from admin.audit where staff_id = ${staff} and target = 'regions:local'`;
      assertEquals(regionAudit.length, 2);
    });

    await t.step("builds rise in order, and min_build waits for old matches to drain", async () => {
      await assertRejects(
        () => runCommand(sql, command("build:min", "--build", "99.0.0")),
        Error,
        "above latest_build",
      );
      await runCommand(sql, command("build:latest", "--build", "99.0.0"));
      const match = crypto.randomUUID();
      matches.push(match);
      await sql`insert into game.matches (match_id, playlist_id, arena, region, build, fleet, state)
                values (${match}, 'scrap_3v3_casual', 'YRD', 'local', '0.1.0', 'local', 'in_progress')`;
      await assertRejects(() => runCommand(sql, command("build:min", "--build", "99.0.0")), Error, "still running");
      await runCommand(sql, command("build:min", "--build", "99.0.0", "--force", "true"));
      const [{ ok }] = await sql`select api.srv_build_ok('0.1.0') as ok`;
      assertEquals(ok, false);
      await assertRejects(
        () => runCommand(sql, command("build:latest", "--build", "98.0.0")),
        Error,
        "below min_build",
      );
      await assertRejects(() => runCommand(sql, command("build:min", "--build", "x.y")), Error, "version");
    });

    await t.step("the health view and the player lookup", async () => {
      const health = await runCommand(sql, command("health")) as Record<string, unknown>;
      for (const k of ["queue", "matches", "builds", "reports", "jobs", "purchases", "claim", "config"]) {
        assert(k in health, k);
      }
      const builds = health.builds as { build: string; live_matches: number }[];
      assert(builds.some((r) => r.build === "0.1.0" && r.live_matches >= 1));
      const found = await runCommand(sql, command("player", "--name", `OPS_A_${tag}`)) as Record<string, unknown>;
      assertEquals((found.account as { account_id: string }).account_id, a);
      assertEquals((found.identities as { provider: string }[]).map((i) => i.provider), ["dev"]);
      await assertRejects(() => runCommand(sql, command("player")), Error, "--account or --name");
    });

    await t.step("a console identity linked to an account signs in to it", async () => {
      const psn = `psn-${tag}`;
      const linked = await sql`select api.srv_link_identity(${a}, 'psn', ${psn}) as r`;
      assertEquals(linked[0].r.already, false);
      assertEquals((await sql`select api.srv_link_identity(${a}, 'psn', ${psn}) as r`)[0].r.already, true);
      // Signing in with it reaches the same account, though its Auth user was made for dev.
      const [{ r: account }] = await sql`select api.srv_ensure_account(${a}, 'psn', ${psn}) as r`;
      assertEquals(account.account_id, a);
      // It cannot be taken by another account, nor sign another Auth user in.
      await assertRejects(() => sql`select api.srv_link_identity(${b}, 'psn', ${psn})`, Error, "identity_conflict");
      await assertRejects(() => sql`select api.srv_ensure_account(${b}, 'psn', ${psn})`, Error, "identity_conflict");
      // One per platform, and only console platforms link this way.
      await assertRejects(
        () => sql`select api.srv_link_identity(${a}, 'psn', ${psn + "-2"})`,
        Error,
        "provider_already_linked",
      );
      await assertRejects(() => sql`select api.srv_link_identity(${a}, 'steam', '765611')`, Error, "bad_provider");
      // An unproved identity on an Auth user made for something else still cannot sign in.
      await assertRejects(
        () => sql`select api.srv_ensure_account(${b}, 'xbox', ${"x-" + tag})`,
        Error,
        "identity_conflict",
      );

      // The player's link is audited by platform and account, never by platform id.
      const [linkAudit] = await sql`select target, after from admin.audit where staff_id = 'player:'
                                    and after ->> 'account_id' = ${a} order by audit_id desc limit 1`;
      assertEquals([linkAudit.target, JSON.stringify(linkAudit.after).includes(psn)], [`identities:psn:${a}`, false]);

      await runCommand(sql, command("identity:unlink", "--account", a, "--provider", "psn"));
      // Only console platforms unlink: the identity an account was made for would come back.
      await assertRejects(
        () => runCommand(sql, command("identity:unlink", "--account", a, "--provider", "dev")),
        Error,
        "bad_provider",
      );
      // Staff changes too: by platform and account, never the platform id.
      const audit = await sql`select action, before, after from admin.audit where staff_id = ${staff}
                              and target = ${"identities:psn:" + a}`;
      assertEquals(audit.map((r) => r.action), ["delete"]);
      assertEquals(JSON.stringify(audit).includes(psn), false);

      // An account made on a console keeps its last identity.
      const c = crypto.randomUUID();
      accounts.push(c);
      const xbox = `xb-${tag}`;
      await sql`insert into auth.users (id, email, aud, role, raw_app_meta_data)
                values (${c}, ${"xbox-" + tag + "@users.kinetic.invalid"}, 'authenticated', 'authenticated',
                        ${sql.json({ kinetic_provider: "xbox", kinetic_provider_user_id: xbox })})`;
      await sql`select api.srv_ensure_account(${c}, 'xbox', ${xbox})`;
      await assertRejects(
        () => runCommand(sql, command("identity:unlink", "--account", c, "--provider", "xbox")),
        Error,
        "last_identity",
      );

      // A banned account links nothing.
      await sql`insert into game.sanctions (account_id, type, reason) values (${b}, 'account_ban', 'test')`;
      await assertRejects(() => sql`select api.srv_link_identity(${b}, 'nintendo', ${"n-" + tag})`, Error, "banned");
    });
  } finally {
    await sql.begin(async (tx) => {
      await tx`select set_config('kinetic.staff', 'test-cleanup', true)`;
      if (matches.length) await tx`delete from game.matches where match_id in ${tx(matches)}`;
      if (accounts.length) await tx`delete from auth.users where id in ${tx(accounts)}`;
      await tx`delete from game.config where key in ('flags', 'motd', 'min_build', 'latest_build', ${`ops-test-${tag}`})`;
      for (const row of saved) {
        await tx`insert into game.config (key, value) values (${row.key}, ${tx.json(row.value as postgres.JSONValue)})`;
      }
      await tx`update game.playlists set enabled = ${playlist.enabled}, regions = ${playlist.regions},
                                         bot_fill_after_s = ${playlist.bot_fill_after_s}
                where playlist_id = 'scrap_3v3_casual'`;
    });
    await sql.end();
  }
});
