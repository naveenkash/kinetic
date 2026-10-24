// The flip rehearsal's guard rails: it never runs against production, and wants its target
// named; the season tooling lists every variant an arena can draw.
import { assertEquals } from "jsr:@std/assert@1.0.13";
import { parseRehearsalArgs } from "../../tools/flip-rehearsal.ts";
import { possibleVariants, projectRef, refuseProduction, rehearsalTargetProblem } from "../../tools/season.ts";

Deno.test("the rehearsal refuses production", () => {
  assertEquals(refuseProduction({ KINETIC_ENV: "production" }), "KINETIC_ENV is production");
  assertEquals(
    refuseProduction({
      DATABASE_URL: "postgresql://u:p@db.prod.example:5432/postgres",
      KINETIC_PROD_DB_HOST: "db.prod.example",
    }),
    "DATABASE_URL is the production database",
  );
  assertEquals(
    refuseProduction({
      DATABASE_URL: "postgresql://u:p@db.staging.example:5432/postgres",
      KINETIC_PROD_DB_HOST: "db.prod.example",
    }),
    null,
  );
  assertEquals(
    refuseProduction({ DATABASE_URL: "not a url", KINETIC_PROD_DB_HOST: "db.prod.example" }),
    "DATABASE_URL does not parse",
  );
});

Deno.test("the rehearsal wants a named non-production target, a staff name and a Claim", () => {
  const base = ["--staff", "ana", "--claim", "w-1"];
  assertEquals(typeof parseRehearsalArgs(["--target", "staging", ...base]), "object");
  assertEquals(
    parseRehearsalArgs(["--target", "production", ...base]),
    "--target must be local or staging; never production",
  );
  assertEquals(parseRehearsalArgs(base), "--target is required");
  assertEquals(
    parseRehearsalArgs(["--target", "local", ...base, "--holder", "htr"]),
    "--holder and --variant go together",
  );
  assertEquals(parseRehearsalArgs(["--target", "local", ...base, "--force", "x"]), "unknown --force");
  assertEquals(parseRehearsalArgs(["--target"]), "expected --flag value pairs, at --target");
});

Deno.test("every variant an arena can take", () => {
  assertEquals(possibleVariants("YRD"), ["base", "haulers_doubled"]);
  assertEquals(possibleVariants("BLS"), ["base", "crown_and_spine", "stabilized"]);
  assertEquals(possibleVariants("ANC").length, 6);
});

Deno.test("the target proves itself by facts the connection string cannot fake", () => {
  const local = "postgresql://postgres:postgres@127.0.0.1:55422/postgres";
  const direct = "postgresql://postgres:pw@db.stagingref123.supabase.co:5432/postgres";
  const pooled = "postgresql://postgres.stagingref123:pw@aws-0-eu-west-1.pooler.supabase.com:6543/postgres";
  const prod = "postgresql://postgres.prodref456:pw@aws-0-eu-west-1.pooler.supabase.com:6543/postgres";
  assertEquals([projectRef(direct), projectRef(pooled), projectRef(prod)], [
    "stagingref123",
    "stagingref123",
    "prodref456",
  ]);
  assertEquals(rehearsalTargetProblem("local", local, undefined), null);
  assertEquals(rehearsalTargetProblem("local", direct, undefined), "--target local needs a database on this machine");
  // Staging only where its project is listed; nothing is listed by default.
  assertEquals(rehearsalTargetProblem("staging", pooled, undefined)?.includes("not set"), true);
  assertEquals(rehearsalTargetProblem("staging", direct, "stagingref123"), null);
  assertEquals(rehearsalTargetProblem("staging", pooled, " other , stagingref123 "), null);
  // The production pooler shares the host but not the project; a session option changes nothing.
  assertEquals(
    rehearsalTargetProblem("staging", prod, "stagingref123"),
    "project prodref456 is not listed in KINETIC_STAGING_DB_REFS",
  );
  assertEquals(
    rehearsalTargetProblem("staging", prod + "?options=-c%20kinetic.environment%3Dstaging", "stagingref123")?.includes(
      "prodref456",
    ),
    true,
  );
  assertEquals(rehearsalTargetProblem("staging", "nonsense", "x"), "DATABASE_URL does not parse");
});
