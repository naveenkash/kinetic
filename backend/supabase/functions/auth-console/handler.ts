// auth-console's logic, with its outside calls passed in so it can be tested without a stack.
//
// Link mode is the sensitive path: a linked identity signs in to the account from then on, so
// a stolen session alone must not be enough to link one. The request carries the session
// (bearer) and proof of an identity the account already has, made now: a Steam ticket, or a
// token from a platform already linked (or a dev name where dev login is on).
import { verifySteamTicket } from "../_shared/steam.ts";
import { bearer, fail, json, type JsonObject, readJson } from "../_shared/http.ts";
import {
  type ConsoleConfig,
  consoleConfig,
  type ConsoleUser,
  isConsoleProvider,
  verifyConsoleToken,
} from "../_shared/console.ts";
import { findLinkedAccount, type Identity, identityRateOk, preflight, signIn } from "../_shared/login.ts";
import type { Client } from "../_shared/supabase.ts";

const DEV_NAME = /^[a-z0-9_]{3,24}$/;

export interface ConsoleDeps {
  admin: Client;
  env: (name: string) => string | undefined;
  verifyConsole: (token: string, config: ConsoleConfig) => Promise<ConsoleUser>;
  verifySteam: typeof verifySteamTicket;
  // The sign-in steps, replaceable in tests.
  preflight: typeof preflight;
  signIn: typeof signIn;
  identityRateOk: typeof identityRateOk;
  findLinkedAccount: typeof findLinkedAccount;
}

export function realDeps(admin: Client): ConsoleDeps {
  return {
    admin,
    env: (name) => Deno.env.get(name),
    verifyConsole: verifyConsoleToken,
    verifySteam: verifySteamTicket,
    preflight,
    signIn,
    identityRateOk,
    findLinkedAccount,
  };
}

// Proves a console token; null when the platform is off or the token fails.
async function proveConsole(
  deps: ConsoleDeps,
  provider: unknown,
  token: unknown,
): Promise<Identity | "disabled" | null> {
  if (!isConsoleProvider(provider) || typeof token !== "string") return null;
  const config = consoleConfig(provider, deps.env);
  if (!config) return "disabled";
  try {
    const user = await deps.verifyConsole(token, config);
    return { provider, providerUserId: user.providerUserId, displayName: user.displayName };
  } catch {
    return null;
  }
}

// Proves the identity the link request offers as the account's own.
async function proveOwnIdentity(deps: ConsoleDeps, proof: unknown): Promise<Identity | null> {
  if (proof === null || typeof proof !== "object" || Array.isArray(proof)) return null;
  const p = proof as JsonObject;
  if (p.provider === "steam" && typeof p.ticket === "string") {
    const appId = deps.env("STEAM_APP_ID");
    const publisherKey = deps.env("STEAM_PUBLISHER_KEY");
    if (!appId || !publisherKey) return null;
    try {
      const steam = await deps.verifySteam(p.ticket, { appId, publisherKey });
      return steam.publisherBanned ? null : { provider: "steam", providerUserId: steam.steamId, displayName: null };
    } catch {
      return null;
    }
  }
  if (p.provider === "dev") {
    if (deps.env("ALLOW_DEV_LOGIN") !== "true" || typeof p.name !== "string" || !DEV_NAME.test(p.name)) return null;
    return { provider: "dev", providerUserId: p.name, displayName: p.name };
  }
  const proved = await proveConsole(deps, p.provider, p.token);
  return proved === "disabled" ? null : proved;
}

async function link(req: Request, body: JsonObject, deps: ConsoleDeps, identity: Identity): Promise<Response> {
  const token = bearer(req);
  if (!token) return fail("unauthorized", 401);
  const { data: who, error: whoError } = await deps.admin.auth.getUser(token);
  if (whoError || !who.user) return fail("unauthorized", 401);
  if (!(await deps.identityRateOk(deps.admin, identity))) return fail("rate_limited", 429);

  const own = await proveOwnIdentity(deps, body.proof);
  if (!own) return fail("proof_required", 403);
  if ((await deps.findLinkedAccount(deps.admin, own)) !== who.user.id) return fail("proof_mismatch", 403);

  const { data, error } = await deps.admin.rpc("srv_link_identity", {
    p_account: who.user.id,
    p_provider: identity.provider,
    p_provider_user_id: identity.providerUserId,
  });
  if (error) {
    const code = error.message;
    if (code === "identity_conflict" || code === "provider_already_linked") return fail(code, 409);
    if (code === "banned") return fail("banned", 403);
    if (code === "unknown_account") return fail("unauthorized", 401);
    return fail("internal_error", 500);
  }
  return json(data);
}

export async function handleConsole(req: Request, deps: ConsoleDeps): Promise<Response> {
  if (req.method !== "POST") return fail("method_not_allowed", 405);
  const body = await readJson(req);
  if (!body) return fail("bad_request", 400);
  if (!isConsoleProvider(body.provider)) return fail("bad_provider", 400);
  if (typeof body.token !== "string") return fail("bad_request", 400);

  const blocked = await deps.preflight(req, body, deps.admin);
  if (blocked) return blocked;
  const identity = await proveConsole(deps, body.provider, body.token);
  if (identity === "disabled") return fail("provider_disabled", 503);
  if (!identity) return fail("token_rejected", 401);
  return body.link === true ? await link(req, body, deps, identity) : await deps.signIn(deps.admin, identity);
}
