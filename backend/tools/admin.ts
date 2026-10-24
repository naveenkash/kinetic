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
//   reports         [--limit n]                     (the open report queue, oldest first)
//   report:handle   --id <uuid> --status (actioned|dismissed)
//   name:reset      --account <uuid>
//   word:block      --word <word>
//   word:allow      --word <word>                   (a word the filter wrongly refuses)
//   audit           [--limit n]
//
// Season flip (Phase 4; tools/season.ts, and `deno task flip-rehearsal` on staging):
//   claim:preview   --id                            (the tally the Claim would get now; read-only)
//   claim:checklist --id                            (readiness before the freeze)
//   variants:shipped --variants a,b                 (the arena variants the live build ships)
//
// Live ops (Phase 4):
//   health                                          (queues, matches, reports, jobs, purchases, the Claim)
//   player          --account <uuid> | --name <display name>
//   config:get      [--key]
//   config:set      --key --value <json>            (not builds, variants or flags: they have their own)
//   motd:set        --text <text>                   (an empty text clears it)
//   flag:set        --flag <name> --value <json>    (a feature flag in config `flags`)
//   playlist:set    --id [--enabled true|false] [--regions a,b] [--bot_fill_after_s n]
//   region:set      --id --enabled true|false
//   build:latest    --build <version>
//   build:min       --build <version> [--force true] (refuses while matches on older builds run)
//   identity:unlink --account <uuid> --provider (psn|xbox|nintendo|steam)
import postgres from "npm:postgres@3.4.5";
import { loadLocalEnv } from "./env.ts";
import { claimChecklist, claimPreview } from "./season.ts";

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
const VERSION = /^\d{1,6}(\.\d{1,6}){0,3}$/;
const bool = (v: string, flag: string) => {
  if (v !== "true" && v !== "false") throw new Error(`--${flag} must be true or false`);
  return v === "true";
};
// Keys with their own command, which checks them.
const OWN_COMMAND = /^(min_build|latest_build|motd|flags|variants_shipped|variant:.*)$/;

function parseJson(text: string): unknown {
  try {
    return JSON.parse(text);
  } catch {
    throw new Error("--value must be JSON (quote strings: '\"text\"')");
  }
}

async function setConfig(tx: Sql, key: string, value: unknown) {
  return await tx`insert into game.config (key, value) values (${key}, ${json(tx, value)})
                  on conflict (key) do update set value = excluded.value returning key, value`;
}

async function health(tx: Sql) {
  const queue = await tx`
    select playlist_id, grade_pool, build, count(*)::int as crews,
           round(extract(epoch from now() - min(created_at)))::int as oldest_s
      from game.mm_tickets group by playlist_id, grade_pool, build order by oldest_s desc`;
  const matches = await tx`
    select state, count(*)::int as n,
           count(*) filter (where state in ('waiting_for_players', 'in_progress')
                              and coalesce(last_heartbeat_at, created_at) < now() - interval '60 seconds')::int
             as no_heartbeat_60s,
           count(*) filter (where state = 'allocating' and created_at < now() - interval '2 minutes')::int
             as allocating_over_2m
      from game.matches where state not in ('completed', 'aborted') group by state order by state`;
  const builds = await tx`
    select build, count(*)::int as live_matches from game.matches
     where state in ('allocating', 'waiting_for_players', 'in_progress', 'reporting') group by build order by build`;
  const [reports] = await tx`
    select (count(*) filter (where status = 'stored'))::int as unprocessed,
           (count(*) filter (where status = 'quarantined'))::int as quarantined,
           round(extract(epoch from now() - min(received_at) filter (where status = 'stored')))::int as oldest_unprocessed_s
      from game.match_reports`;
  const [jobs] = await tx`
    select (count(*) filter (where status = 'queued'))::int as queued,
           (count(*) filter (where status = 'running'))::int as running,
           (count(*) filter (where status = 'failed' and requested_at > now() - interval '7 days'))::int as failed_7d
      from game.account_jobs`;
  const [purchases] = await tx`
    select (count(*) filter (where status = 'initiated' and next_check_at < now()))::int as checks_overdue,
           (select count(*) from game.support_flags where resolved_at is null)::int as support_flags_open
      from game.purchases`;
  const [claim] = await tx`
    select c.claim_id, c.state, c.ends_at,
           round(extract(epoch from now() - (select min(refreshed_at) from game.claim_standing s
                                              where s.claim_id = c.claim_id)))::int as standing_age_s
      from game.claims c where c.state = 'active'`;
  const config = await tx`select key, value from game.config where key in ('min_build', 'latest_build', 'motd')`;
  return { queue, matches, builds, reports, jobs, purchases, claim: claim ?? null, config };
}

async function player(tx: Sql, a: Args) {
  if ((a.account === undefined) === (a.name === undefined)) throw new Error("player takes --account or --name");
  const [account] = a.account
    ? await tx`select account_id, display_name, status, created_at from game.accounts where account_id = ${a.account}`
    : await tx`select account_id, display_name, status, created_at from game.accounts
                where lower(display_name) = lower(${a.name!})`;
  if (!account) throw new Error("no such player");
  const id = account.account_id;
  return {
    account,
    identities: await tx`select provider, provider_user_id, created_at from game.identities where account_id = ${id}`,
    ledger:
      (await tx`select grade, tonnage_total, cleared_at, cutter_serial from game.ledgers where account_id = ${id}`)[
        0
      ] ??
        null,
    wallets: await tx`select currency, balance from game.wallets where account_id = ${id}`,
    sanctions: await tx`select type, reason, starts_at, ends_at, issued_by from game.sanctions
                         where account_id = ${id} and (ends_at is null or ends_at > now()) order by starts_at desc`,
    recent_matches: await tx`select m.match_id, m.playlist_id, m.arena, m.state, m.created_at, mp.team, mp.abandoned
                               from game.match_players mp join game.matches m using (match_id)
                              where mp.account_id = ${id} order by m.created_at desc limit 5`,
    reports_against: (await tx`select count(*)::int as open from game.reports where reported_id = ${id}
                                and status = 'open'`)[0].open,
  };
}

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
  "reports": {
    required: [],
    optional: ["limit"],
    run: (tx, a) =>
      tx`select r.report_id, r.created_at, r.reason, r.text, r.replay_path, r.match_id, a.display_name as reported
           from game.reports r left join game.accounts a on a.account_id = r.reported_id
          where r.status = 'open' order by r.created_at limit ${Number(a.limit ?? 20)}`,
  },
  "report:handle": {
    required: ["id", "status"],
    run: (tx, a) => tx`select api.srv_report_handle(${a.id}, ${a.status}, current_setting('kinetic.staff')) as result`,
  },
  "name:reset": {
    required: ["account"],
    run: (tx, a) => tx`select api.srv_reset_display_name(${a.account}) as result`,
  },
  "word:block": {
    required: ["word"],
    run: (tx, a) => tx`insert into game.blocked_words (word) values (${a.word}) returning word`,
  },
  "word:allow": {
    required: ["word"],
    run: (tx, a) => tx`insert into game.allowed_words (word) values (${a.word}) returning word`,
  },
  "audit": {
    required: [],
    optional: ["limit"],
    run: (tx, a) =>
      tx`select at, staff_id, action, target from admin.audit order by audit_id desc limit ${Number(a.limit ?? 20)}`,
  },
  "claim:preview": { required: ["id"], run: (tx, a) => claimPreview(tx, a.id) },
  "claim:checklist": { required: ["id"], run: (tx, a) => claimChecklist(tx, a.id) },
  "variants:shipped": {
    required: ["variants"],
    run: (tx, a) => setConfig(tx, "variants_shipped", list(a.variants)),
  },
  "health": { required: [], run: (tx) => health(tx) },
  "player": { required: [], optional: ["account", "name"], run: (tx, a) => player(tx, a) },
  "config:get": {
    required: [],
    optional: ["key"],
    run: (tx, a) =>
      a.key
        ? tx`select key, value from game.config where key = ${a.key}`
        : tx`select key, value from game.config order by key`,
  },
  "config:set": {
    required: ["key", "value"],
    run: (tx, a) => {
      if (OWN_COMMAND.test(a.key)) throw new Error(`${a.key} has its own command`);
      return setConfig(tx, a.key, parseJson(a.value));
    },
  },
  "motd:set": {
    required: ["text"],
    run: (tx, a) =>
      a.text.trim() === ""
        ? tx`delete from game.config where key = 'motd' returning key`
        : setConfig(tx, "motd", a.text.trim()),
  },
  "flag:set": {
    required: ["flag", "value"],
    run: async (tx, a) => {
      if (!/^[a-z0-9_.-]{1,64}$/.test(a.flag)) throw new Error("flag names are lowercase letters, digits, _ . -");
      const [current] = await tx`select value from game.config where key = 'flags'`;
      const flags = current && typeof current.value === "object" && !Array.isArray(current.value) ? current.value : {};
      return setConfig(tx, "flags", { ...flags, [a.flag]: parseJson(a.value) });
    },
  },
  "playlist:set": {
    required: ["id"],
    optional: ["enabled", "regions", "bot_fill_after_s"],
    run: async (tx, a) => {
      const enabled = a.enabled === undefined ? null : bool(a.enabled, "enabled");
      const regions = a.regions === undefined ? null : list(a.regions);
      const after = a.bot_fill_after_s === undefined ? null : Number(a.bot_fill_after_s);
      if (after !== null && !(Number.isInteger(after) && after >= 0)) {
        throw new Error("--bot_fill_after_s is whole seconds");
      }
      if (regions && regions.length === 0) throw new Error("--regions needs at least one region");
      if (regions) {
        const known = await tx`select region_id from game.regions where region_id in ${tx(regions)}`;
        if (known.length !== regions.length) throw new Error("--regions names a region that does not exist");
      }
      const rows = await tx`
        update game.playlists set enabled = coalesce(${enabled}, enabled), regions = coalesce(${regions}, regions),
                                  bot_fill_after_s = coalesce(${after}, bot_fill_after_s)
         where playlist_id = ${a.id} returning playlist_id, enabled, regions, bot_fill_after_s`;
      if (rows.length === 0) throw new Error("no such playlist");
      return rows;
    },
  },
  "region:set": {
    required: ["id", "enabled"],
    run: async (tx, a) => {
      const rows = await tx`update game.regions set enabled = ${bool(a.enabled, "enabled")} where region_id = ${a.id}
                            returning region_id, enabled`;
      if (rows.length === 0) throw new Error("no such region");
      return rows;
    },
  },
  "build:latest": {
    required: ["build"],
    run: async (tx, a) => {
      if (!VERSION.test(a.build)) throw new Error("--build is a version like 0.4.2");
      const [low] = await tx`select private.version_cmp(${a.build}, coalesce(private.config_text('min_build'), '0'))
                             as cmp`;
      if (low.cmp < 0) throw new Error("latest_build cannot be below min_build");
      return setConfig(tx, "latest_build", a.build);
    },
  },
  "build:min": {
    required: ["build"],
    optional: ["force"],
    run: async (tx, a) => {
      if (!VERSION.test(a.build)) throw new Error("--build is a version like 0.4.2");
      const [high] = await tx`select private.version_cmp(${a.build}, coalesce(private.config_text('latest_build'),
                                                                              ${a.build})) as cmp`;
      if (high.cmp > 0) throw new Error("min_build cannot be above latest_build: raise latest first");
      // Patch rollout (section 18): raise min_build once old matches drain.
      const [old] = await tx`select count(*)::int as n from game.matches
                              where state in ('allocating', 'waiting_for_players', 'in_progress')
                                and private.version_cmp(build, ${a.build}) < 0`;
      if (old.n > 0 && !(a.force && bool(a.force, "force"))) {
        throw new Error(`${old.n} matches on older builds are still running; wait for them, or pass --force true`);
      }
      return setConfig(tx, "min_build", a.build);
    },
  },
  "identity:unlink": {
    required: ["account", "provider"],
    run: (tx, a) => tx`select api.srv_unlink_identity(${a.account}, ${a.provider}) as result`,
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
