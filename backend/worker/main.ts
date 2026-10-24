// kinetic-worker entry point. Run two instances; the one holding the advisory lock is the
// leader and runs the tick every second, the other waits to take over.
//
// The lock lives on its own single-connection client. If that connection closes for any
// reason, the lock is gone: the instance stops leading at once and competes again.
// DATABASE_URL must be a direct or session-mode connection, never the transaction pooler.
//
// Env: DATABASE_URL, FUNCTIONS_URL, KINETIC_SIGNING_PRIVATE_JWK, KINETIC_SIGNING_PUBLIC_JWKS,
//      FLEET (local | edgegap), LOCAL_SERVER_CMD (JSON array, local fleet only).
import postgres from "npm:postgres@3.4.5";
import { keyringFromEnv } from "../supabase/functions/_shared/tokens.ts";
import { loadLocalEnv } from "../tools/env.ts";
import { EdgegapFleet } from "./fleet/edgegap.ts";
import { LocalFleet } from "./fleet/local.ts";
import type { Fleet } from "./fleet/types.ts";
import { LEADER_LOCK_KEY, tick, type WorkerContext } from "./loop.ts";

const TICK_MS = 1000;
const LEADER_PING_TIMEOUT_MS = 2000;

if (Deno.env.get("KINETIC_ENV") !== "production") await loadLocalEnv();

// A worker must never die on a stray rejection from a client library.
globalThis.addEventListener("unhandledrejection", (e) => {
  console.error("[worker] unhandled rejection:", e.reason);
  e.preventDefault();
});

function makeFleet(): Fleet {
  const kind = Deno.env.get("FLEET") ?? "local";
  if (kind === "edgegap") return new EdgegapFleet();
  const command = JSON.parse(
    Deno.env.get("LOCAL_SERVER_CMD") ??
      JSON.stringify(["deno", "run", "-A", new URL("../tools/fake-server.ts", import.meta.url).pathname]),
  );
  return new LocalFleet(command, new URL("../.logs", import.meta.url).pathname);
}

const databaseUrl = Deno.env.get("DATABASE_URL")!;
const sql = postgres(databaseUrl, { max: 4, onnotice: () => {} });
const fleet = makeFleet();

let lockClient: postgres.Sql | null = null;
let leading = false;

async function tryLead(): Promise<boolean> {
  const client = postgres(databaseUrl, {
    max: 1,
    // 0 disables both timers: recycling this connection would silently drop the lock.
    max_lifetime: 0,
    idle_timeout: 0,
    onnotice: () => {},
    onclose: () => {
      if (leading) console.error("[worker] leader connection closed; leadership lost");
      leading = false;
    },
  });
  try {
    const [{ locked }] = await client<{ locked: boolean }[]>`select pg_try_advisory_lock(${LEADER_LOCK_KEY}) as locked`;
    if (locked) {
      lockClient = client;
      leading = true;
      return true;
    }
  } catch (e) {
    console.error("[worker] lock attempt failed:", e instanceof Error ? e.message : e);
  }
  await client.end({ timeout: 1 }).catch(() => {});
  return false;
}

async function stopLeading() {
  leading = false;
  const client = lockClient;
  lockClient = null;
  // End the connection rather than returning it anywhere: that is what releases the lock.
  await client?.end({ timeout: 1 }).catch(() => {});
}

const ctx: WorkerContext = {
  sql,
  fleet,
  keyring: keyringFromEnv(),
  functionsUrl: Deno.env.get("FUNCTIONS_URL")!,
  isLeader: () => leading,
};

let stopping = false;
const stop = async () => {
  if (stopping) return;
  stopping = true;
  console.log("[worker] stopping");
  await stopLeading();
  await fleet.shutdown();
  await sql.end({ timeout: 5 });
  Deno.exit(0);
};
Deno.addSignalListener("SIGINT", stop);
Deno.addSignalListener("SIGTERM", stop);

console.log(`[worker] fleet=${fleet.name}; waiting for leadership`);
while (!stopping) {
  const started = Date.now();
  if (!leading) {
    if (lockClient) await stopLeading();
    if (await tryLead()) console.log("[worker] leader");
  }
  if (leading) {
    try {
      // Proves the lock connection is still up before acting as leader. A dropped
      // connection fires onclose first, which has already cleared `leading`.
      await Promise.race([
        lockClient!`select 1`,
        new Promise((_, reject) =>
          setTimeout(() => reject(new Error("leader ping timed out")), LEADER_PING_TIMEOUT_MS)
        ),
      ]);
    } catch (e) {
      console.error("[worker] leader ping failed:", e instanceof Error ? e.message : e);
      await stopLeading();
    }
  }
  if (leading) {
    try {
      await tick(ctx);
    } catch (e) {
      console.error("[worker] tick failed:", e instanceof Error ? e.message : e);
    }
  }
  await new Promise((r) => setTimeout(r, Math.max(0, TICK_MS - (Date.now() - started))));
}
