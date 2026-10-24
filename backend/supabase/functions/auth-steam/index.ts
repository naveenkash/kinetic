// POST /functions/v1/auth-steam
// Exchanges a Steam web API ticket (or, in local and dev only, a dev username) for a
// Supabase Auth session. backend-architecture section 8.
//
// Body: { provider: "steam", ticket: "<hex>", build: "0.1.0" }
//    or { provider: "dev", name: "alice", build: "0.1.0" }   (ALLOW_DEV_LOGIN=true only)
//
// Account safety: public signup is off, and an Auth user is only adopted for an identity
// if this function created it for exactly that identity: its app_metadata, which only the
// admin API can write, names the provider and provider user id. srv_ensure_account checks.
import { clientIp, fail, json, type JsonObject, readJson } from "../_shared/http.ts";
import { type EnsuredAccount, jsonObject } from "../_shared/rpc.ts";
import { verifySteamTicket } from "../_shared/steam.ts";
import { anonClient, type Client, rateLimit, serviceClient } from "../_shared/supabase.ts";

const DEV_NAME = /^[a-z0-9_]{3,24}$/;
const DEFAULT_LOGINS_PER_IP_PER_MINUTE = 10;
const LOGINS_PER_IDENTITY_PER_MINUTE = 5;
const RATE_WINDOW_S = 60;
const SESSION_ATTEMPTS = 3;
const SESSION_RETRY_BASE_MS = 50;
const SESSION_RETRY_JITTER_MS = 150;

interface Identity {
  provider: "steam" | "dev";
  providerUserId: string;
  displayName: string | null;
}

type Result<T> = { ok: true; value: T } | { ok: false; response: Response };
const ok = <T>(value: T): Result<T> => ({ ok: true, value });
const refuse = <T>(error: string, status: number): Result<T> => ({ ok: false, response: fail(error, status) });

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

async function findLinkedAccount(admin: Client, identity: Identity): Promise<string | null> {
  const { data, error } = await admin.rpc("srv_find_account", {
    p_provider: identity.provider,
    p_provider_user_id: identity.providerUserId,
  });
  if (error) throw new Error(`srv_find_account: ${error.message}`);
  return typeof data === "string" ? data : null;
}

// The Auth user for this identity and its address: the linked user, or one made here.
async function resolveAuthUser(admin: Client, identity: Identity): Promise<Result<{ userId: string; email: string }>> {
  const linkedId = await findLinkedAccount(admin, identity);
  if (linkedId) {
    const { data, error } = await admin.auth.admin.getUserById(linkedId);
    if (error || !data.user?.email) return refuse("internal_error", 500);
    return ok({ userId: linkedId, email: data.user.email });
  }

  // A synthetic address that is never mailed; Supabase Auth needs one to mint sessions.
  const email = `${identity.provider}-${identity.providerUserId}@users.kinetic.invalid`.toLowerCase();
  const { data: created, error: createError } = await admin.auth.admin.createUser({
    email,
    email_confirm: true,
    app_metadata: { kinetic_provider: identity.provider, kinetic_provider_user_id: identity.providerUserId },
  });
  if (created?.user) return ok({ userId: created.user.id, email });

  // A simultaneous first login made the user (GoTrue reports that race as a generic error,
  // not email_exists), or an earlier attempt stopped half way. Look it up without creating
  // anything; srv_ensure_account refuses it unless this function made it for this identity.
  const racedId = await findLinkedAccount(admin, identity);
  if (racedId) return ok({ userId: racedId, email });
  const { data: existingId, error: findError } = await admin.rpc("srv_find_auth_user", { p_email: email });
  if (findError) return refuse("internal_error", 500);
  if (typeof existingId === "string") return ok({ userId: existingId, email });

  console.error("createUser failed:", createError?.code, createError?.message);
  return refuse("try_again", 503);
}

async function ensureAccount(admin: Client, userId: string, identity: Identity): Promise<Result<EnsuredAccount>> {
  const { data, error } = await admin.rpc("srv_ensure_account", {
    p_user: userId,
    p_provider: identity.provider,
    p_provider_user_id: identity.providerUserId,
    p_display_name: identity.displayName ?? undefined,
  });
  if (error) {
    return error.message === "identity_conflict" ? refuse("identity_conflict", 409) : refuse("internal_error", 500);
  }
  const account = jsonObject<EnsuredAccount>(data);
  if (!account) return refuse("internal_error", 500);
  if (account.banned || account.status !== "active") return refuse("banned", 403);
  return ok(account);
}

// Mints a session from a one-time magic-link token, without sending email. Two logins at
// once can invalidate each other's token, so retry a few times with jitter.
async function mintSession(admin: Client, userId: string, email: string) {
  for (let attempt = 0; attempt < SESSION_ATTEMPTS; attempt++) {
    const { data: link, error: linkError } = await admin.auth.admin.generateLink({ type: "magiclink", email });
    if (!linkError && link?.properties?.hashed_token && link.user) {
      if (link.user.id !== userId) return null;
      const { data, error } = await anonClient().auth.verifyOtp({
        token_hash: link.properties.hashed_token,
        type: "magiclink",
      });
      if (!error && data.session) return data.session;
    }
    await new Promise((r) => setTimeout(r, SESSION_RETRY_BASE_MS + Math.random() * SESSION_RETRY_JITTER_MS));
  }
  return null;
}

async function login(req: Request): Promise<Response> {
  if (req.method !== "POST") return fail("method_not_allowed", 405);
  const body = await readJson(req);
  if (!body) return fail("bad_request", 400);

  const admin = serviceClient();
  const ipLimit = Number(Deno.env.get("KINETIC_LOGIN_IP_LIMIT") ?? DEFAULT_LOGINS_PER_IP_PER_MINUTE);
  if (!(await rateLimit(admin, `login-ip:${clientIp(req)}`, ipLimit, RATE_WINDOW_S))) {
    return fail("rate_limited", 429);
  }

  const build = typeof body.build === "string" ? body.build : "";
  const { data: buildOk, error: buildError } = await admin.rpc("srv_build_ok", { p_build: build });
  if (buildError) return fail("internal_error", 500);
  if (buildOk !== true) return fail("build_too_old", 426);

  const identity = await readIdentity(body);
  if (!identity.ok) return identity.response;
  const identityKey = `login-id:${identity.value.provider}:${identity.value.providerUserId}`;
  if (!(await rateLimit(admin, identityKey, LOGINS_PER_IDENTITY_PER_MINUTE, RATE_WINDOW_S))) {
    return fail("rate_limited", 429);
  }

  const user = await resolveAuthUser(admin, identity.value);
  if (!user.ok) return user.response;
  const account = await ensureAccount(admin, user.value.userId, identity.value);
  if (!account.ok) return account.response;
  const session = await mintSession(admin, user.value.userId, user.value.email);
  if (!session) return fail("session_failed", 503);

  return json({
    access_token: session.access_token,
    refresh_token: session.refresh_token,
    expires_in: session.expires_in,
    account: {
      account_id: account.value.account_id,
      display_name: account.value.display_name,
      grade: account.value.grade,
    },
  });
}

Deno.serve(async (req) => {
  try {
    return await login(req);
  } catch (e) {
    console.error(e);
    return fail("internal_error", 500);
  }
});
