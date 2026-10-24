// Rehearses a season flip (backend-architecture 14; runbooks.md): runs one Claim through
// frozen, tallied, reviewed and resolved on a non-production database, and prints each step.
// It changes data (the Claim ends now), so it refuses production and wants the target named.
//
//   deno task flip-rehearsal --target staging --staff <name> --claim <id>
//       [--holder <faction> --variant <variant>]   the reviewer's call for a tie
//       [--ignore-checks true] [--force-tally true]
import postgres from "npm:postgres@3.4.5";
import type { WorkerContext } from "../worker/loop.ts";
import { loadLocalEnv } from "./env.ts";
import { refuseProduction, rehearsalTargetProblem, rehearsalText, rehearseFlip } from "./season.ts";

export const TARGETS = ["local", "staging"];

export function parseRehearsalArgs(argv: string[]): Record<string, string> | string {
  const flags: Record<string, string> = {};
  for (let i = 0; i < argv.length; i += 2) {
    if (!argv[i].startsWith("--") || argv[i + 1] === undefined || argv[i + 1].startsWith("--")) {
      return `expected --flag value pairs, at ${argv[i]}`;
    }
    flags[argv[i].slice(2)] = argv[i + 1];
  }
  const allowed = new Set(["target", "staff", "claim", "holder", "variant", "ignore-checks", "force-tally"]);
  const unknown = Object.keys(flags).filter((k) => !allowed.has(k));
  if (unknown.length) return `unknown --${unknown.join(", --")}`;
  for (const k of ["target", "staff", "claim"]) if (!flags[k]) return `--${k} is required`;
  if (!TARGETS.includes(flags.target)) return `--target must be ${TARGETS.join(" or ")}; never production`;
  if ((flags.holder === undefined) !== (flags.variant === undefined)) return "--holder and --variant go together";
  return flags;
}

if (import.meta.main) {
  const flags = parseRehearsalArgs(Deno.args);
  if (typeof flags === "string") {
    console.error(flags);
    Deno.exit(2);
  }
  if (flags.target === "local") await loadLocalEnv();
  const refusal = refuseProduction({
    KINETIC_ENV: Deno.env.get("KINETIC_ENV"),
    DATABASE_URL: Deno.env.get("DATABASE_URL"),
    KINETIC_PROD_DB_HOST: Deno.env.get("KINETIC_PROD_DB_HOST"),
  });
  if (refusal) {
    console.error(`refusing to rehearse: ${refusal}`);
    Deno.exit(2);
  }
  const url = Deno.env.get("DATABASE_URL") ?? "";
  const problem = rehearsalTargetProblem(flags.target, url, Deno.env.get("KINETIC_STAGING_DB_REFS"));
  if (problem) {
    console.error(`refusing to rehearse: ${problem}`);
    Deno.exit(2);
  }
  const sql = postgres(url, { max: 2, onnotice: () => {} });
  const ctx = {
    sql,
    functionsUrl: Deno.env.get("SUPABASE_URL") ? `${Deno.env.get("SUPABASE_URL")}/functions/v1` : "",
    log: (...a: unknown[]) => console.log(...a),
    ...({ fleet: null, keyring: null } as unknown as Pick<WorkerContext, "fleet" | "keyring">),
  } as WorkerContext;
  try {
    const result = await rehearseFlip(ctx, flags.claim, {
      staff: flags.staff,
      holder: flags.holder,
      variant: flags.variant,
      ignoreChecks: flags["ignore-checks"] === "true",
      forceTally: flags["force-tally"] === "true",
    });
    console.log(rehearsalText(result));
    Deno.exitCode = result.ok ? 0 : 1;
  } catch (e) {
    console.error(e instanceof Error ? e.message : e);
    Deno.exitCode = 1;
  } finally {
    await sql.end();
  }
}
