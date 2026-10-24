// Fake game server for local development and the integration test, until the Unreal
// dedicated server exists. It does what the real server must do with the backend:
// fetch the manifest, heartbeat, verify join tickets locally, report joins, play for a
// while, upload files, send the match report and exit.
//
// Clients connect over TCP and send one line: {"join_ticket":"..."}. The real server uses
// the Unreal net driver instead; the backend contract is the same.
//
// Env: MATCH_ID, SERVER_CREDENTIAL, FUNCTIONS_URL, JOIN_TICKET_PUBLIC_KEYS, PORT,
//      FAKE_MATCH_SECONDS (default 3), FAKE_HEARTBEAT_S (default 2), FAKE_JOIN_WAIT_S (default 60)
import { verifyJoinTicket } from "../supabase/functions/_shared/tokens.ts";

const env = (k: string) => {
  const v = Deno.env.get(k);
  if (!v) throw new Error(`${k} is not set`);
  return v;
};
const matchId = env("MATCH_ID");
const credential = env("SERVER_CREDENTIAL");
const functionsUrl = env("FUNCTIONS_URL");
const publicJwks = JSON.parse(env("JOIN_TICKET_PUBLIC_KEYS"));
const port = Number(env("PORT"));
const matchSeconds = Number(Deno.env.get("FAKE_MATCH_SECONDS") ?? 3);
const heartbeatS = Number(Deno.env.get("FAKE_HEARTBEAT_S") ?? 2);
const joinWaitS = Number(Deno.env.get("FAKE_JOIN_WAIT_S") ?? 60);

const MAX_LINE_BYTES = 8192;
const JOIN_POLL_MS = 250;

const log = (...a: unknown[]) => console.log(`[fake-server ${matchId.slice(0, 8)}]`, ...a);

async function call(fn: string, body: unknown = {}): Promise<Record<string, unknown>> {
  const res = await fetch(`${functionsUrl}/${fn}`, {
    method: "POST",
    headers: { authorization: `Bearer ${credential}`, "content-type": "application/json" },
    body: JSON.stringify(body),
  });
  const text = await res.text();
  if (!res.ok) throw new Error(`${fn} ${res.status}: ${text}`);
  return text ? JSON.parse(text) : {};
}

interface ManifestPlayer {
  slot: number;
  team: number;
  account_id: string | null;
  is_bot: boolean;
}
const manifest = await call("server-manifest") as { players: ManifestPlayer[]; teams: number };
const humans = manifest.players.filter((p) => !p.is_bot);
const joined = new Map<string, number>(); // account_id -> joined at (ms)
let state: "waiting_for_players" | "in_progress" = "waiting_for_players";
let startedAt = 0;
log(`manifest: ${manifest.players.length} slots, ${humans.length} humans`);

// The heartbeat contract for any game server: from boot, every heartbeat interval, until
// the report is sent. The backend marks the server ready on the first heartbeat after the
// worker has recorded its address, so the boot heartbeat alone is not enough.
const heartbeat = () => call("server-heartbeat", { state, connected: [...joined.keys()] });
await heartbeat();
const hb = setInterval(() => heartbeat().catch((e) => log("heartbeat failed", e.message)), heartbeatS * 1000);

const listener = Deno.listen({ hostname: "127.0.0.1", port });
log(`listening on ${port}`);

async function handle(conn: Deno.Conn) {
  const reply = async (body: unknown) => {
    await conn.write(new TextEncoder().encode(JSON.stringify(body) + "\n"));
    conn.close();
  };
  try {
    const buf = new Uint8Array(MAX_LINE_BYTES);
    const n = await conn.read(buf);
    const line = new TextDecoder().decode(buf.subarray(0, n ?? 0)).trim();
    const { join_ticket } = JSON.parse(line);
    const claims = await verifyJoinTicket(publicJwks, join_ticket, matchId);
    const slot = humans.find((p) => p.account_id === claims.account_id && p.slot === claims.slot);
    if (!slot) return await reply({ ok: false, error: "not_on_roster" });
    if (!joined.has(claims.account_id)) {
      joined.set(claims.account_id, Date.now());
      await call("server-player-event", { account_id: claims.account_id, event: "joined" });
    }
    await reply({ ok: true, slot: slot.slot, team: slot.team });
  } catch (e) {
    await reply({ ok: false, error: e instanceof Error ? e.message : "bad_ticket" }).catch(() => {});
  }
}

(async () => {
  for await (const conn of listener) handle(conn);
})().catch(() => {});

const deadline = Date.now() + joinWaitS * 1000;
while (state === "waiting_for_players") {
  await new Promise((r) => setTimeout(r, JOIN_POLL_MS));
  if (joined.size === humans.length) {
    state = "in_progress";
    startedAt = Date.now();
    await heartbeat();
    log("all players joined; live");
  } else if (Date.now() > deadline) {
    log("not everyone joined in time; exiting");
    clearInterval(hb);
    Deno.exit(0);
  }
}

await new Promise((r) => setTimeout(r, matchSeconds * 1000));
clearInterval(hb);

const urls = await call("server-upload-url") as Record<string, { signedUrl: string }>;
for (const [kind, url] of Object.entries(urls)) {
  const res = await fetch(url.signedUrl, { method: "PUT", body: `fake ${kind} for ${matchId}\n` });
  if (!res.ok) log(`upload ${kind} failed: ${res.status}`);
}

const elapsed = (Date.now() - startedAt) / 1000;
const report = {
  match_id: matchId,
  build: "0.1.0",
  ruleset: 1,
  event_schema: 1,
  winner_team: 1,
  rounds: [{ winner_team: 1, seconds: Math.round(elapsed), stalls: 4 }],
  players: manifest.players.map((p) => ({
    account_id: p.account_id,
    team: p.team,
    is_bot: p.is_bot,
    connected_seconds: p.is_bot ? elapsed : (Date.now() - (joined.get(p.account_id!) ?? Date.now())) / 1000,
    abandoned: false,
    facts: { mass_distance_kg_m: 1000 * p.slot, eliminations: p.slot % 2, assists: 0, spotter_credits: 0 },
  })),
};
log("report:", JSON.stringify(await call("server-report", report)));
listener.close();
Deno.exit(0);
