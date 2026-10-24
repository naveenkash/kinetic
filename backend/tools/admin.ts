// Admin console v1 (backend-architecture 14 and 17): the live-ops steps staff take, over the
// service connection, one transaction per command with the staff member named for the audit
// (admin.audit, written by trigger). A web console can replace this later on the same SQL.
//
//   deno task admin --staff <name> <command> [--flag value ...]
//
// Commands:
//   claim:create    --id --name --arena (YRD|HGT|BLS|ANC) --starts <iso> --ends <iso>
//   contract:add    --id --claim --faction --week --counter --target --title [--items a,b] [--entries x,y]
//   crate:add       --claim --track (free|sponsored) --tier --tonnage [--items a,b] [--entries x,y]
//   claim:review    --id [--holder <faction> --variant <variant>]
//   claim:resolve   --id
//   report:requeue  --match <uuid>
//   grant:item      --account <uuid> --item <item id>
//   grant:entitlement --account <uuid> --entitlement <kind:ref>
//   sanction        --account <uuid> --type <type> --reason <text> [--ends <iso>]
//   audit           [--limit n]
import postgres from "npm:postgres@3.4.5";
import { loadLocalEnv } from "./env.ts";

type Sql = postgres.TransactionSql;
type Args = Record<string, string>;

interface Command {
  required: string[];
  optional?: string[];
  run: (tx: Sql, a: Args) => Promise<unknown>;
}

const list = (v: string | undefined) => (v ? v.split(",").map((s) => s.trim()).filter(Boolean) : []);
const reward = (a: Args) => ({ items: list(a.items), entries: list(a.entries) });
const json = (tx: Sql, v: unknown) => tx.json(v as postgres.JSONValue);

export const COMMANDS: Record<string, Command> = {
  "claim:create": {
    required: ["id", "name", "arena", "starts", "ends"],
    run: (tx, a) =>
      tx`insert into game.claims (claim_id, name, arena, starts_at, ends_at)
         values (${a.id}, ${a.name}, ${a.arena}, ${a.starts}, ${a.ends}) returning claim_id, state`,
  },
  "contract:add": {
    required: ["id", "claim", "faction", "week", "counter", "target", "title"],
    optional: ["items", "entries"],
    run: (tx, a) =>
      tx`insert into game.contracts (contract_id, claim_id, faction, week, counter_key, target, title, reward)
         values (${a.id}, ${a.claim}, ${a.faction}, ${Number(a.week)}, ${a.counter}, ${Number(a.target)}, ${a.title},
                 ${json(tx, reward(a))}) returning contract_id`,
  },
  "crate:add": {
    required: ["claim", "track", "tier", "tonnage"],
    optional: ["items", "entries"],
    run: (tx, a) =>
      tx`insert into game.manifest_crates (claim_id, track, tier, tonnage_t, reward)
         values (${a.claim}, ${a.track}, ${Number(a.tier)}, ${Number(a.tonnage)}, ${json(tx, reward(a))})
         returning claim_id, track, tier`,
  },
  "claim:review": {
    required: ["id"],
    optional: ["holder", "variant"],
    run: (tx, a) =>
      tx`select api.srv_claim_review(${a.id}, current_setting('kinetic.staff'), ${a.holder ?? null},
                                     ${a.variant ?? null}) as result`,
  },
  "claim:resolve": {
    required: ["id"],
    run: (tx, a) => tx`select api.srv_claim_resolve(${a.id}) as result`,
  },
  "report:requeue": {
    required: ["match"],
    run: (tx, a) => tx`select api.srv_requeue_report(${a.match}) as result`,
  },
  "grant:item": {
    required: ["account", "item"],
    run: (tx, a) =>
      tx`insert into game.inventory (account_id, item_id, source, source_ref)
         values (${a.account}, ${a.item}, 'admin', current_setting('kinetic.staff')) returning item_id`,
  },
  "grant:entitlement": {
    required: ["account", "entitlement"],
    run: (tx, a) =>
      tx`insert into game.entitlements (account_id, entitlement, source_ref)
         values (${a.account}, ${a.entitlement}, current_setting('kinetic.staff')) returning entitlement`,
  },
  "sanction": {
    required: ["account", "type", "reason"],
    optional: ["ends"],
    run: (tx, a) =>
      tx`insert into game.sanctions (account_id, type, reason, ends_at, issued_by)
         values (${a.account}, ${a.type}, ${a.reason}, ${a.ends ?? null}, current_setting('kinetic.staff'))
         returning sanction_id`,
  },
  "audit": {
    required: [],
    optional: ["limit"],
    run: (tx, a) =>
      tx`select at, staff_id, action, target from admin.audit order by audit_id desc limit ${Number(a.limit ?? 20)}`,
  },
};

export interface Parsed {
  staff: string;
  command: string;
  args: Args;
}

// Parses `--staff <name> <command> --flag value ...`. Returns the problem as a string.
export function parseArgs(argv: string[]): Parsed | string {
  const flags: Args = {};
  const words: string[] = [];
  for (let i = 0; i < argv.length; i++) {
    if (!argv[i].startsWith("--")) {
      words.push(argv[i]);
      continue;
    }
    const value = argv[i + 1];
    if (value === undefined || value.startsWith("--")) return `${argv[i]} needs a value`;
    flags[argv[i].slice(2)] = value;
    i++;
  }
  const { staff, ...args } = flags;
  const [command] = words;
  if (!staff) return "--staff is required: every change is audited under a name";
  if (words.length !== 1 || !COMMANDS[command]) return `unknown command; one of ${Object.keys(COMMANDS).join(", ")}`;
  const spec = COMMANDS[command];
  const missing = spec.required.filter((k) => args[k] === undefined);
  if (missing.length > 0) return `${command} needs --${missing.join(", --")}`;
  const allowed = new Set([...spec.required, ...(spec.optional ?? [])]);
  const unknown = Object.keys(args).filter((k) => !allowed.has(k));
  if (unknown.length > 0) return `${command} does not take --${unknown.join(", --")}`;
  return { staff, command, args };
}

export async function runCommand(sql: postgres.Sql, parsed: Parsed): Promise<unknown> {
  return await sql.begin(async (tx) => {
    await tx`select set_config('kinetic.staff', ${parsed.staff}, true)`;
    return await COMMANDS[parsed.command].run(tx, parsed.args);
  });
}

if (import.meta.main) {
  const parsed = parseArgs(Deno.args);
  if (typeof parsed === "string") {
    console.error(parsed);
    Deno.exit(2);
  }
  if (Deno.env.get("KINETIC_ENV") !== "production") await loadLocalEnv();
  const sql = postgres(Deno.env.get("DATABASE_URL")!, { max: 1, onnotice: () => {} });
  try {
    console.log(JSON.stringify(await runCommand(sql, parsed), null, 2));
  } catch (e) {
    console.error(e instanceof Error ? e.message : e);
    Deno.exitCode = 1;
  } finally {
    await sql.end();
  }
}
