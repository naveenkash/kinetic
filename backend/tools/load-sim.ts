// Load simulator (backend-architecture 25): plays many dev clients through login, crews,
// queue, match, join and the rewarded report, against a stack with the worker and fake
// servers running, and reports latencies and failures. Run against staging before each phase
// exit; locally it checks the loop end to end at small scale.
//
//   deno task load-sim --players 60 [--playlist scrap_3v3_casual] [--timeout-s 180]
//
// Env: SUPABASE_URL, FUNCTIONS_URL, SUPABASE_ANON_KEY (local defaults). Dev login must be on
// (ALLOW_DEV_LOGIN=true) and the login IP limit raised for one machine (KINETIC_LOGIN_IP_LIMIT).
import { createClient, type SupabaseClient } from "npm:@supabase/supabase-js@2.49.4";
import type { Database } from "../supabase/functions/_shared/database.types.ts";
import { loadLocalEnv } from "./env.ts";

const CREW = 3;
const POLL_MS = 1000;
const BUILD = "0.1.0";

type Db = SupabaseClient<Database, "api">;

interface Client {
  name: string;
  accountId: string;
  token: string;
  db: Db;
}

export interface Timings {
  [step: string]: number[];
}

export function summarize(timings: Timings, failures: Record<string, number>) {
  const pct = (xs: number[], p: number) => {
    const sorted = [...xs].sort((a, b) => a - b);
    return sorted.length ? sorted[Math.min(sorted.length - 1, Math.floor(p * sorted.length))] : null;
  };
  return {
    steps: Object.fromEntries(
      Object.entries(timings).map(([k, xs]) => [k, {
        n: xs.length,
        p50_ms: pct(xs, 0.5),
        p95_ms: pct(xs, 0.95),
        max_ms: xs.length ? Math.max(...xs) : null,
      }]),
    ),
    failures,
  };
}

export function parseFlags(argv: string[]): { players: number; playlist: string; timeoutS: number } | string {
  const flags: Record<string, string> = {};
  for (let i = 0; i < argv.length; i += 2) {
    if (!argv[i].startsWith("--") || argv[i + 1] === undefined) return `bad argument ${argv[i]}`;
    flags[argv[i].slice(2)] = argv[i + 1];
  }
  const players = Number(flags.players ?? 12);
  if (!Number.isInteger(players) || players <= 0 || players % (CREW * 2) !== 0) {
    return "--players must be a positive multiple of 6 (two crews of three per match)";
  }
  return { players, playlist: flags.playlist ?? "scrap_3v3_casual", timeoutS: Number(flags["timeout-s"] ?? 180) };
}

// Times a step; a failure is counted against the step and rethrown.
let failuresByStep: Record<string, number> = {};
async function timed<T>(timings: Timings, step: string, fn: () => Promise<T>): Promise<T> {
  const start = performance.now();
  try {
    const value = await fn();
    (timings[step] ??= []).push(Math.round(performance.now() - start));
    return value;
  } catch (e) {
    failuresByStep[step] = (failuresByStep[step] ?? 0) + 1;
    throw e;
  }
}

async function login(name: string): Promise<Client> {
  const res = await fetch(`${Deno.env.get("FUNCTIONS_URL")}/auth-steam`, {
    method: "POST",
    headers: { "content-type": "application/json" },
    body: JSON.stringify({ provider: "dev", name, build: BUILD }),
  });
  const body = await res.json();
  if (res.status !== 200) throw new Error(`login ${name}: ${JSON.stringify(body)}`);
  const db = createClient<Database, "api">(Deno.env.get("SUPABASE_URL")!, Deno.env.get("SUPABASE_ANON_KEY")!, {
    db: { schema: "api" },
    auth: { persistSession: false, autoRefreshToken: false },
    global: { headers: { Authorization: `Bearer ${body.access_token}` } },
  });
  return { name, accountId: body.account.account_id, token: body.access_token, db };
}

async function must<T>(what: string, p: PromiseLike<{ data: T; error: { message: string } | null }>): Promise<T> {
  const { data, error } = await p;
  if (error) throw new Error(`${what}: ${error.message}`);
  return data;
}

async function formCrew(crew: Client[]): Promise<void> {
  const [lead, ...rest] = crew;
  for (const m of rest) await must("invite", lead.db.rpc("crew_invite", { p_to: m.accountId }));
  const row = await must("my_crew", lead.db.from("my_crew").select("crew_id").limit(1).single());
  const crewId = row?.crew_id;
  if (!crewId) throw new Error(`${lead.name}: no crew`);
  for (const m of rest) {
    await must("accept", m.db.rpc("crew_accept", { p_crew: crewId }));
    await must("pick", m.db.rpc("crew_set_pick", { p_harness: "Rigger", p_ready: true }));
  }
}

async function currentMatch(c: Client): Promise<{ address: string; join_ticket: string; match_id: string } | null> {
  const res = await fetch(`${Deno.env.get("FUNCTIONS_URL")}/match-current`, {
    method: "POST",
    headers: { authorization: `Bearer ${c.token}` },
  });
  const body = await res.json();
  return res.status === 200 && body.match ? { ...body.match, join_ticket: body.join_ticket } : null;
}

async function until<T>(deadline: number, fn: () => Promise<T | null>): Promise<T> {
  while (Date.now() < deadline) {
    const v = await fn().catch(() => null);
    if (v) return v;
    await new Promise((r) => setTimeout(r, POLL_MS));
  }
  throw new Error("timed out");
}

async function join(address: string, ticket: string): Promise<void> {
  const [hostname, port] = address.split(":");
  const conn = await Deno.connect({ hostname, port: Number(port) });
  try {
    await conn.write(new TextEncoder().encode(JSON.stringify({ join_ticket: ticket }) + "\n"));
    const buf = new Uint8Array(4096);
    const n = await conn.read(buf);
    const reply = JSON.parse(new TextDecoder().decode(buf.subarray(0, n ?? 0)));
    if (!reply.ok) throw new Error(`join refused: ${reply.error}`);
  } finally {
    conn.close();
  }
}

// One crew of three: form, queue (the leader), wait for the match, join, wait for rewards.
async function playCrew(crew: Client[], playlist: string, deadline: number, timings: Timings): Promise<void> {
  await timed(timings, "crew", () => formCrew(crew));
  const [lead] = crew;
  await timed(
    timings,
    "queue_join",
    () => must("queue", lead.db.rpc("queue_join", { p_playlist: playlist, p_pings: { local: 20 }, p_build: BUILD })),
  );
  const match = await timed(timings, "queue_to_match", () => until(deadline, () => currentMatch(lead)));
  await timed(timings, "join", () =>
    Promise.all(crew.map(async (c) => {
      const m = c === lead ? match : await until(deadline, () => currentMatch(c));
      await join(m.address, m.join_ticket);
    })));
  await timed(timings, "match_to_rewards", () =>
    until(deadline, async () => {
      const { data } = await lead.db.from("my_match_summaries").select("match_id").eq("match_id", match.match_id);
      return data && data.length > 0 ? true : null;
    }));
}

async function main() {
  const flags = parseFlags(Deno.args);
  if (typeof flags === "string") {
    console.error(flags);
    Deno.exit(2);
  }
  await loadLocalEnv();
  const run = crypto.randomUUID().slice(0, 6);
  const timings: Timings = {};
  failuresByStep = {};
  const failures = failuresByStep;
  const names = Array.from({ length: flags.players }, (_, i) => `ls_${run}_${i}`);
  const clients = (await Promise.all(names.map((n) =>
    timed(timings, "login", () => login(n)).catch((e) => {
      console.error(e instanceof Error ? e.message : e);
      return null;
    })
  ))).filter((c): c is Client => c !== null);
  const crews = Array.from(
    { length: Math.floor(clients.length / CREW) },
    (_, i) => clients.slice(i * CREW, i * CREW + CREW),
  );
  const deadline = Date.now() + flags.timeoutS * 1000;
  await Promise.all(crews.map((crew) =>
    playCrew(crew, flags.playlist, deadline, timings).catch((e) => {
      console.error(e instanceof Error ? e.message : e);
    })
  ));
  console.log(JSON.stringify(summarize(timings, failures), null, 2));
  if (Object.keys(failures).length > 0) Deno.exitCode = 1;
}

if (import.meta.main) await main();
