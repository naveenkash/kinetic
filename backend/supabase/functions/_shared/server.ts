// Shared entry for the server-* functions: every call carries a per-match server
// credential, never a Supabase key (backend-architecture section 11).
import { bearer, fail, type JsonObject, readJson } from "./http.ts";
import { rateLimit, serviceClient } from "./supabase.ts";
import { keyringFromEnv, type ServerClaims, verifyServerCredential } from "./tokens.ts";

export type ServerHandler = (
  claims: ServerClaims,
  body: JsonObject,
) => Promise<Response>;

// Each endpoint allows this many calls per match per minute: a heartbeat every 10 s and a
// few player events need far less, so a leaked credential cannot flood the backend.
const CALLS_PER_MATCH_PER_MINUTE = 60;

export function serveServerEndpoint(endpoint: string, handler: ServerHandler) {
  Deno.serve(async (req) => {
    if (req.method !== "POST") return fail("method_not_allowed", 405);
    const token = bearer(req);
    if (!token) return fail("not_authenticated", 401);

    let claims: ServerClaims;
    try {
      claims = await verifyServerCredential(keyringFromEnv().publicJwks, token);
    } catch {
      return fail("bad_credential", 401);
    }
    const body = (await readJson(req)) ?? {};
    try {
      const key = `srv:${endpoint}:${claims.match_id}`;
      if (!(await rateLimit(serviceClient(), key, CALLS_PER_MATCH_PER_MINUTE, 60))) {
        return fail("rate_limited", 429);
      }
      return await handler(claims, body);
    } catch (e) {
      console.error(e);
      return fail("internal_error", 500);
    }
  });
}

// Maps a Postgres error from an srv_* function to an HTTP response.
export function dbFail(error: { message?: string }): Response {
  const code = error.message ?? "internal_error";
  const status = code === "unknown_match" || code === "unknown_player"
    ? 404
    : code === "match_closed"
    ? 409
    : code === "bad_state" || code === "bad_event"
    ? 400
    : 500;
  return fail(status === 500 ? "internal_error" : code, status);
}
