// The sign-in pipeline shared by auth-steam and auth-console (backend-architecture 8): rate
// limits, the build check, the Auth user for an identity, the account, and a session minted
// without sending email.
//
// Account safety: public signup is off, and an Auth user is only adopted for an identity
// if this pipeline created it for exactly that identity: its app_metadata, which only the
// admin API can write, names the provider and provider user id. srv_ensure_account checks.
// An identity linked to an account (a console linked to a Steam account) signs in to that
// account's Auth user; srv_link_identity linked it after proving it.
import { clientIp, fail, json, type JsonObject } from "./http.ts";
import { type EnsuredAccount, jsonObject } from "./rpc.ts";
import { anonClient, type Client, rateLimit, serviceClient } from "./supabase.ts";

const DEFAULT_LOGINS_PER_IP_PER_MINUTE = 10;
const LOGINS_PER_IDENTITY_PER_MINUTE = 5;
const RATE_WINDOW_S = 60;
const SESSION_ATTEMPTS = 3;
const SESSION_RETRY_BASE_MS = 50;
const SESSION_RETRY_JITTER_MS = 150;

export interface Identity {
  provider: "steam" | "dev" | "psn" | "xbox" | "nintendo";
  providerUserId: string;
  displayName: string | null;
}

export type Result<T> = { ok: true; value: T } | { ok: false; response: Response };
export const ok = <T>(value: T): Result<T> => ({ ok: true, value });
export const refuse = <T>(error: string, status: number): Result<T> => ({ ok: false, response: fail(error, status) });

const CONSOLE = new Set(["psn", "xbox", "nintendo"]);

// A synthetic address that is never mailed; Supabase Auth needs one to mint sessions. Steam
// and dev ids are digits and lowercase names. Console ids may hold characters an address
// cannot, or differ only by case, so they are hashed.
export async function syntheticEmail(identity: Identity): Promise<string> {
  if (!CONSOLE.has(identity.provider)) {
    return `${identity.provider}-${identity.providerUserId}@users.kinetic.invalid`.toLowerCase();
  }
  const digest = await crypto.subtle.digest("SHA-256", new TextEncoder().encode(identity.providerUserId));
  const hex = [...new Uint8Array(digest)].map((b) => b.toString(16).padStart(2, "0")).join("");
  return `${identity.provider}-${hex.slice(0, 40)}@users.kinetic.invalid`;
}

export async function findLinkedAccount(admin: Client, identity: Identity): Promise<string | null> {
  const { data, error } = await admin.rpc("srv_find_account", {
    p_provider: identity.provider,
    p_provider_user_id: identity.providerUserId,
  });
  if (error) throw new Error(`srv_find_account: ${error.message}`);
  return typeof data === "string" ? data : null;
}

// The Auth user for this identity and its address: the linked user, or one made here.
export async function resolveAuthUser(
  admin: Client,
  identity: Identity,
): Promise<Result<{ userId: string; email: string }>> {
  const linkedId = await findLinkedAccount(admin, identity);
  if (linkedId) {
    const { data, error } = await admin.auth.admin.getUserById(linkedId);
    if (error || !data.user?.email) return refuse("internal_error", 500);
    return ok({ userId: linkedId, email: data.user.email });
  }

  const email = await syntheticEmail(identity);
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

export async function ensureAccount(
  admin: Client,
  userId: string,
  identity: Identity,
): Promise<Result<EnsuredAccount>> {
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
export async function mintSession(admin: Client, userId: string, email: string) {
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

// The checks every sign-in makes before the identity is proved: the address's rate and the
// client's build.
export async function preflight(req: Request, body: JsonObject, admin: Client): Promise<Response | null> {
  const ipLimit = Number(Deno.env.get("KINETIC_LOGIN_IP_LIMIT") ?? DEFAULT_LOGINS_PER_IP_PER_MINUTE);
  if (!(await rateLimit(admin, `login-ip:${clientIp(req)}`, ipLimit, RATE_WINDOW_S))) {
    return fail("rate_limited", 429);
  }
  const build = typeof body.build === "string" ? body.build : "";
  const { data: buildOk, error: buildError } = await admin.rpc("srv_build_ok", { p_build: build });
  if (buildError) return fail("internal_error", 500);
  if (buildOk !== true) return fail("build_too_old", 426);
  return null;
}

export async function identityRateOk(admin: Client, identity: Identity): Promise<boolean> {
  const key = `login-id:${identity.provider}:${identity.providerUserId}`;
  return await rateLimit(admin, key, LOGINS_PER_IDENTITY_PER_MINUTE, RATE_WINDOW_S);
}

// Signs a proved identity in: its Auth user, its account, a session.
export async function signIn(admin: Client, identity: Identity): Promise<Response> {
  if (!(await identityRateOk(admin, identity))) return fail("rate_limited", 429);
  const user = await resolveAuthUser(admin, identity);
  if (!user.ok) return user.response;
  const account = await ensureAccount(admin, user.value.userId, identity);
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

export { serviceClient };
