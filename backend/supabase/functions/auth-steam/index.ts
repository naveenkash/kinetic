// POST /functions/v1/auth-steam
// Exchanges a Steam web API ticket (or, in local and dev only, a dev username) for a
// Supabase Auth session. backend-architecture section 8.
//
// Body: { provider: "steam", ticket: "<hex>", build: "0.1.0" }
//    or { provider: "dev", name: "alice", build: "0.1.0" }   (ALLOW_DEV_LOGIN=true only)
//
// The pipeline after the identity is proved (rate limits, Auth user, account, session) is
// shared with auth-console: _shared/login.ts.
import { fail, type JsonObject, readJson } from "../_shared/http.ts";
import { type Identity, ok, preflight, refuse, type Result, serviceClient, signIn } from "../_shared/login.ts";
import { verifySteamTicket } from "../_shared/steam.ts";

const DEV_NAME = /^[a-z0-9_]{3,24}$/;

// Reads the provider from the body; for Steam, proves the ticket with Steam.
async function readIdentity(body: JsonObject): Promise<Result<Identity>> {
  if (body.provider === "dev") {
    if (Deno.env.get("ALLOW_DEV_LOGIN") !== "true") return refuse("provider_disabled", 403);
    if (typeof body.name !== "string" || !DEV_NAME.test(body.name)) return refuse("bad_name", 400);
    return ok({ provider: "dev", providerUserId: body.name, displayName: body.name });
  }
  if (body.provider !== "steam") return refuse("bad_provider", 400);
  if (typeof body.ticket !== "string") return refuse("bad_request", 400);

  const appId = Deno.env.get("STEAM_APP_ID");
  const publisherKey = Deno.env.get("STEAM_PUBLISHER_KEY");
  if (!appId || !publisherKey) return refuse("provider_disabled", 503);
  try {
    const steam = await verifySteamTicket(body.ticket, { appId, publisherKey });
    if (steam.publisherBanned) return refuse("banned", 403);
    return ok({ provider: "steam", providerUserId: steam.steamId, displayName: null });
  } catch {
    return refuse("ticket_rejected", 401);
  }
}

async function login(req: Request): Promise<Response> {
  if (req.method !== "POST") return fail("method_not_allowed", 405);
  const body = await readJson(req);
  if (!body) return fail("bad_request", 400);

  const admin = serviceClient();
  const blocked = await preflight(req, body, admin);
  if (blocked) return blocked;
  const identity = await readIdentity(body);
  if (!identity.ok) return identity.response;
  return await signIn(admin, identity.value);
}

Deno.serve(async (req) => {
  try {
    return await login(req);
  } catch (e) {
    console.error(e);
    return fail("internal_error", 500);
  }
});
