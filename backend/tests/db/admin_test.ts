// Admin console v1 against the local database: each command runs under the staff member's
// name, and every change lands in admin.audit, which cannot be edited.
import { assert, assertEquals, assertRejects } from "jsr:@std/assert@1.0.13";
import postgres from "npm:postgres@3.4.5";
import { parseArgs, type Parsed, runCommand } from "../../tools/admin.ts";

const DB_URL = Deno.env.get("DATABASE_URL") ?? "postgresql://postgres:postgres@127.0.0.1:55422/postgres";
const tag = `${Date.now() % 1_000_000}`;

function command(...argv: string[]): Parsed {
  const parsed = parseArgs(["--staff", `staff-${tag}`, ...argv]);
  if (typeof parsed === "string") throw new Error(parsed);
  return parsed;
}

Deno.test("admin console", async (t) => {
  const sql = postgres(DB_URL, { max: 2, onnotice: () => {} });
  const claim = `adm-${tag}`;
  const account = crypto.randomUUID();
  try {
    const name = `adm_${tag}`;
    await sql`insert into auth.users (id, email, aud, role, raw_app_meta_data)
              values (${account}, ${name + "@users.kinetic.invalid"}, 'authenticated', 'authenticated',
                      ${sql.json({ kinetic_provider: "dev", kinetic_provider_user_id: name })})`;
    await sql`select api.srv_ensure_account(${account}, 'dev', ${name}, ${name})`;
    await sql`insert into admin.staff (staff_id) values (${"staff-" + tag}), ('test-cleanup') on conflict do nothing`;
    // Far in the future, so it cannot overlap a Claim already on this database.
    const year = 2400 + (Number(tag) % 500);

    await t.step("a season is set up from the console, and every step is audited", async () => {
      await runCommand(
        sql,
        command(
          "claim:create",
          "--id",
          claim,
          "--name",
          "Console Claim",
          "--arena",
          "BLS",
          "--starts",
          `${year}-01-01T00:00:00Z`,
          "--ends",
          `${year}-02-01T00:00:00Z`,
        ),
      );
      await runCommand(
        sql,
        command(
          "contract:add",
          "--id",
          `${claim}-c1`,
          "--claim",
          claim,
          "--faction",
          "htr",
          "--week",
          "1",
          "--counter",
          "rated_tons_moved",
          "--target",
          "200",
          "--title",
          "Move two hundred tons.",
          "--entries",
          "htr-entry-9",
        ),
      );
      await runCommand(
        sql,
        command(
          "crate:add",
          "--claim",
          claim,
          "--track",
          "free",
          "--tier",
          "1",
          "--tonnage",
          "800",
          "--items",
          "tape.hangtown_stencil",
        ),
      );
      const audit = await sql`select action, target from admin.audit where staff_id = ${"staff-" + tag}
                              order by audit_id`;
      assertEquals(audit.map((r) => [r.action, r.target]), [
        ["insert", `claims:${claim}`],
        ["insert", `contracts:${claim}-c1`],
        ["insert", `manifest_crates:${claim}:free:1`],
      ]);
    });

    await t.step("grants and sanctions carry the staff name; a sidegrade cannot be granted", async () => {
      await runCommand(sql, command("grant:item", "--account", account, "--item", "livery.office_bone"));
      await runCommand(
        sql,
        command(
          "sanction",
          "--account",
          account,
          "--type",
          "chat_mute",
          "--reason",
          "test",
          "--ends",
          "2100-01-01T00:00:00Z",
        ),
      );
      await assertRejects(
        () => runCommand(sql, command("grant:item", "--account", account, "--item", "line.longline")),
        Error,
        "sidegrade_not_grantable",
      );
      const [s] = await sql`select issued_by from game.sanctions where account_id = ${account}`;
      assertEquals(s.issued_by, `staff-${tag}`);
      const audit = await sql`select target from admin.audit where staff_id = ${"staff-" + tag} and target like ${
        "%" + account + "%"
      }`;
      assertEquals(audit.length, 2);
    });

    await t.step("the audit cannot be edited", async () => {
      await assertRejects(() => sql`update admin.audit set staff_id = 'nobody' where staff_id = ${"staff-" + tag}`);
      await assertRejects(() => sql`delete from admin.audit where staff_id = ${"staff-" + tag}`);
      const rows = await runCommand(sql, command("audit", "--limit", "3")) as unknown[];
      assert(rows.length > 0);
    });
  } finally {
    await sql.begin(async (tx) => {
      await tx`select set_config('kinetic.staff', 'test-cleanup', true)`;
      await tx`delete from game.manifest_crates where claim_id = ${claim}`;
      await tx`delete from game.contracts where claim_id = ${claim}`;
      await tx`delete from game.claims where claim_id = ${claim}`;
      await tx`delete from auth.users where id = ${account}`;
    });
    await sql.end();
  }
});
