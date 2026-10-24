// End-to-end Phase 1 loop against the local stack:
// login -> crews -> queue -> worker forms a match -> local fleet boots the fake server ->
// match_found over Realtime -> players join with tickets -> report stored.
//
// Needs: `supabase start`, `deno task functions` running, keys from `deno task keys`.
import { assert, assertEquals } from "jsr:@std/assert@1.0.13";
import { createClient, type RealtimeChannel, type SupabaseClient } from "npm:@supabase/supabase-js@2.49.4";
import type { Database } from "../../supabase/functions/_shared/database.types.ts";
import postgres from "npm:postgres@3.4.5";
import { loadLocalEnv } from "../../tools/env.ts";
import { keyringFromEnv } from "../../supabase/functions/_shared/tokens.ts";
import { LocalFleet } from "../../worker/fleet/local.ts";
import type { Fleet } from "../../worker/fleet/types.ts";
import { LEADER_LOCK_KEY, tick, type WorkerContext } from "../../worker/loop.ts";

await loadLocalEnv();
const SUPABASE_URL = Deno.env.get("SUPABASE_URL")!;
const FUNCTIONS_URL = Deno.env.get("FUNCTIONS_URL")!;
const ANON_KEY = Deno.env.get("SUPABASE_ANON_KEY")!;
const BUILD = "0.1.0";
const SUBSCRIBE_TIMEOUT_MS = 10_000;
const WAIT_TIMEOUT_MS = 20_000;
const POLL_INTERVAL_MS = 250;
const SLOW_FLEET_DELAY_MS = 2000;

interface Player {
  name: string;
  accountId: string;
  token: string;
  db: SupabaseClient<Database, "api">;
}

async function login(name: string): Promise<Player> {
  const res = await fetch(`${FUNCTIONS_URL}/auth-steam`, {
    method: "POST",
    headers: { "content-type": "application/json" },
    body: JSON.stringify({ provider: "dev", name, build: BUILD }),
  });
  const body = await res.json();
  assertEquals(res.status, 200, `login ${name}: ${JSON.stringify(body)}`);
  const db = createClient<Database, "api">(SUPABASE_URL, ANON_KEY, {
    db: { schema: "api" },
    auth: { persistSession: false, autoRefreshToken: false },
    global: { headers: { Authorization: `Bearer ${body.access_token}` } },
  });
  db.realtime.setAuth(body.access_token);
  return { name, accountId: body.account.account_id, token: body.access_token, db };
}

type ApiFunctions = Database["api"]["Functions"];

async function rpc<F extends keyof ApiFunctions>(p: Player, fn: F, args: ApiFunctions[F]["Args"]) {
  const { data, error } = await p.db.rpc(fn, args);
  if (error) throw new Error(`${p.name} ${fn}: ${error.message}`);
  return data;
}

function required<T>(value: T | null | undefined, what: string): T {
  if (value === null || value === undefined) throw new Error(`missing ${what}`);
  return value;
}

async function formCrew(lead: Player, members: Player[]) {
  for (const m of members) await rpc(lead, "crew_invite", { p_to: m.accountId });
  const { data: crew } = await lead.db.from("my_crew").select("crew_id").limit(1).single();
  for (const m of members) {
    await rpc(m, "crew_accept", { p_crew: required(crew?.crew_id, "crew id") });
    await rpc(m, "crew_set_pick", { p_harness: "Climber", p_ready: true });
  }
}

async function joinServer(address: string, joinTicket: string): Promise<Record<string, unknown>> {
  const [hostname, port] = address.split(":");
  const conn = await Deno.connect({ hostname, port: Number(port) });
  await conn.write(new TextEncoder().encode(JSON.stringify({ join_ticket: joinTicket }) + "\n"));
  const buf = new Uint8Array(4096);
  const n = await conn.read(buf);
  conn.close();
  return JSON.parse(new TextDecoder().decode(buf.subarray(0, n ?? 0)));
}

function subscribed(channel: RealtimeChannel): Promise<void> {
  return new Promise((resolve, reject) => {
    const timer = setTimeout(() => reject(new Error("realtime subscription timed out")), SUBSCRIBE_TIMEOUT_MS);
    channel.subscribe((status) => {
      if (status === "SUBSCRIBED") {
        clearTimeout(timer);
        resolve();
      } else if (status === "CHANNEL_ERROR" || status === "TIMED_OUT") {
        clearTimeout(timer);
        reject(new Error(`realtime subscription ${status}`));
      }
    });
  });
}

async function waitFor<T>(
  what: string,
  fn: () => Promise<T | null | undefined>,
  timeoutMs = WAIT_TIMEOUT_MS,
): Promise<T> {
  const deadline = Date.now() + timeoutMs;
  while (Date.now() < deadline) {
    const value = await fn();
    if (value) return value;
    await new Promise((r) => setTimeout(r, POLL_INTERVAL_MS));
  }
  throw new Error(`timed out waiting for ${what}`);
}

interface Ticketed {
  address: string;
  join_ticket: string;
}

// Holds the worker leader lock so a running `deno task worker` cannot race this test.
async function takeLeaderLock(): Promise<postgres.Sql> {
  const lock = postgres(Deno.env.get("DATABASE_URL")!, { max: 1, onnotice: () => {} });
  const [{ locked }] = await lock`select pg_try_advisory_lock(${LEADER_LOCK_KEY}) as locked`;
  if (!locked) {
    await lock.end();
    throw new Error("another worker holds the leader lock; stop `deno task worker` before this test");
  }
  return lock;
}

// Returns 2 s after the server starts, like a provider that waits for readiness: the
// server's boot heartbeat lands before the worker records its address.
function slowLocalFleet(): Fleet {
  const local = new LocalFleet(
    ["deno", "run", "-A", new URL("../../tools/fake-server.ts", import.meta.url).pathname],
    new URL("../../.logs", import.meta.url).pathname,
  );
  return {
    name: local.name,
    allocate: async (req) => {
      const alloc = await local.allocate(req);
      await new Promise((r) => setTimeout(r, SLOW_FLEET_DELAY_MS));
      return alloc;
    },
    deallocate: (ref) => local.deallocate(ref),
    shutdown: () => local.shutdown(),
  };
}

// Public signup is off: nobody can pre-register a Steam player's address.
async function assertSignupRefused() {
  const signup = await fetch(`${SUPABASE_URL}/auth/v1/signup`, {
    method: "POST",
    headers: { apikey: ANON_KEY, "content-type": "application/json" },
    body: JSON.stringify({ email: "steam-76561198000000001@users.kinetic.invalid", password: "hunter22hunter22" }),
  });
  await signup.body?.cancel();
  assert(signup.status >= 400, `signup should be refused, got ${signup.status}`);
}

// Six players, plus repeat and simultaneous logins that must all land on one account each.
async function loginPlayers(run: string): Promise<Player[]> {
  const names = ["a", "b", "c", "d", "e", "f"].map((x) => `it_${run}_${x}`);
  const players = await Promise.all(names.map(login));
  const [a] = players;
  const again = await Promise.all([login(a.name), login(a.name)]);
  assertEquals(again.map((p) => p.accountId), [a.accountId, a.accountId]);
  const fresh = await Promise.all([1, 2, 3].map(() => login(`it_${run}_g`)));
  assertEquals(new Set(fresh.map((p) => p.accountId)).size, 1);
  const forbidden = await a.db.rpc("srv_build_ok", { p_build: BUILD });
  assert(forbidden.error, "srv functions are not callable by players");
  return players;
}

// Members talk on their crew's chat topic; crew:<id> stays server-only.
async function assertCrewChat(sender: Player, listener: Player) {
  const { data: crewRow } = await sender.db.from("my_crew").select("chat_topic").limit(1).single();
  const chatTopic = required(crewRow?.chat_topic, "chat topic");
  const heard: unknown[] = [];
  const listening = listener.db.channel(chatTopic, { config: { private: true } })
    .on("broadcast", { event: "chat" }, (msg) => heard.push(msg.payload));
  const sending = sender.db.channel(chatTopic, { config: { private: true } });
  await subscribed(listening);
  await subscribed(sending);
  await sending.send({ type: "broadcast", event: "chat", payload: { text: "take up" } });
  await waitFor("crew chat", () => Promise.resolve(heard.length > 0));
}

// Queues both crews and drives the worker until the first leader hears match_found.
async function queueAndAwaitMatch(ctx: WorkerContext, leadA: Player, leadB: Player): Promise<
  Ticketed & {
    match_id: string;
  }
> {
  const found: (Ticketed & { match_id: string })[] = [];
  const channel = leadA.db.channel(`user:${leadA.accountId}`, { config: { private: true } })
    .on("broadcast", { event: "match_found" }, (msg) => found.push(msg.payload as Ticketed & { match_id: string }));
  await subscribed(channel);
  await rpc(leadA, "queue_join", { p_playlist: "scrap_3v3_casual", p_pings: { local: 15 }, p_build: BUILD });
  await rpc(leadB, "queue_join", { p_playlist: "scrap_3v3_casual", p_pings: { local: 25 }, p_build: BUILD });
  return await waitFor("match_found", async () => {
    await tick(ctx);
    return found[0];
  });
}

// The reconnect path gives every other player a ticket.
async function reconnectTicket(p: Player): Promise<Ticketed> {
  const res = await fetch(`${FUNCTIONS_URL}/match-current`, {
    method: "POST",
    headers: { authorization: `Bearer ${p.token}` },
  });
  const body = await res.json();
  assertEquals(res.status, 200, JSON.stringify(body));
  return { address: body.match.address, join_ticket: body.join_ticket };
}

// The signature's first character is fully significant (its last one carries padding
// bits, so changing it can be a no-op).
async function assertTamperedTicketRefused(t: Ticketed) {
  const [head, body, sig] = t.join_ticket.split(".");
  const tampered = `${head}.${body}.${sig[0] === "A" ? "B" : "A"}${sig.slice(1)}`;
  assertEquals((await joinServer(t.address, tampered)).ok, false);
}

async function assertMatchRecorded(ctx: WorkerContext, matchId: string) {
  const { sql } = ctx;
  const report = await waitFor("match report", async () => {
    await tick(ctx);
    const [row] = await sql`select status, quarantine_reason from game.match_reports where match_id = ${matchId}`;
    return row;
  });
  assertEquals(report.status, "stored", `report quarantined: ${report.quarantine_reason}`);
  const [done] = await sql`select state from game.matches where match_id = ${matchId}`;
  assertEquals(done.state, "completed");
  const [joined] = await sql`
    select count(*)::int as n from game.match_players where match_id = ${matchId} and joined_at is not null`;
  assertEquals(joined.n, 6);
  const uploads = await sql`select bucket_id from storage.objects where name like ${matchId + "%"} order by bucket_id`;
  assertEquals(uploads.map((u) => u.bucket_id), ["match-events", "replays"]);
}

async function assertServerNeedsCredential() {
  const noCred = await fetch(`${FUNCTIONS_URL}/server-report`, { method: "POST", body: "{}" });
  assertEquals(noCred.status, 401);
  await noCred.body?.cancel();
}

// Leaves nothing queued or connected for the next run.
async function cleanUp(sql: postgres.Sql, players: Player[]) {
  if (players.length > 0) {
    await sql`delete from game.mm_tickets where crew_id in (
      select crew_id from game.crew_members where account_id in ${sql(players.map((p) => p.accountId))})`;
  }
  await Promise.all(players.map((p) => p.db.removeAllChannels()));
}

Deno.test({
  name: "Phase 1 loop: crews, queue, match, join, report",
  sanitizeOps: false,
  sanitizeResources: false,
  async fn() {
    const lock = await takeLeaderLock();
    const sql = postgres(Deno.env.get("DATABASE_URL")!, { max: 2, onnotice: () => {} });
    const fleet = slowLocalFleet();
    const ctx: WorkerContext = { sql, fleet, keyring: keyringFromEnv(), functionsUrl: FUNCTIONS_URL, log: () => {} };
    let players: Player[] = [];
    try {
      await assertSignupRefused();
      players = await loginPlayers(crypto.randomUUID().slice(0, 6));
      const [a, b, c, d, e, f] = players;
      await formCrew(a, [b, c]);
      await formCrew(d, [e, f]);
      await assertCrewChat(a, b);

      const found = await queueAndAwaitMatch(ctx, a, d);
      const [match] = await sql`select region from game.matches where match_id = ${found.match_id}`;
      assertEquals(match.region, "local");
      const others = await Promise.all([b, c, d, e, f].map(reconnectTicket));
      await assertTamperedTicketRefused(found);
      for (const t of [found, ...others]) {
        const reply = await joinServer(t.address, t.join_ticket);
        assertEquals(reply.ok, true, JSON.stringify(reply));
      }

      await assertMatchRecorded(ctx, found.match_id);
      await assertServerNeedsCredential();
    } finally {
      await cleanUp(sql, players);
      await fleet.shutdown();
      await sql.end();
      await lock.end();
    }
  },
});
