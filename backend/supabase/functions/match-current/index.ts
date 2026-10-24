// POST /functions/v1/match-current
// Reconnect path: returns the caller's running match and a fresh join ticket.
// Supabase verifies the player's access token before this runs (verify_jwt = true).
import { bearer, fail, json } from "../_shared/http.ts";
import { type CurrentMatch, jsonObject } from "../_shared/rpc.ts";
import { rateLimit, serviceClient, userClient } from "../_shared/supabase.ts";
import { keyringFromEnv, signJoinTicket } from "../_shared/tokens.ts";

const RECONNECTS_PER_MINUTE = 10;
const RATE_WINDOW_S = 60;

Deno.serve(async (req) => {
  if (req.method !== "POST") return fail("method_not_allowed", 405);
  const token = bearer(req);
  if (!token) return fail("not_authenticated", 401);

  const { data: userData, error: userError } = await userClient(token).auth.getUser();
  if (userError || !userData.user) return fail("not_authenticated", 401);
  const accountId = userData.user.id;

  const admin = serviceClient();
  if (!(await rateLimit(admin, `match-current:${accountId}`, RECONNECTS_PER_MINUTE, RATE_WINDOW_S))) {
    return fail("rate_limited", 429);
  }

  const { data, error } = await admin.rpc("srv_current_match", { p_account: accountId });
  if (error) return fail("internal_error", 500);
  const match = jsonObject<CurrentMatch>(data);
  if (!match) return json({ match: null });

  const joinTicket = await signJoinTicket(keyringFromEnv(), {
    account_id: accountId,
    match_id: match.match_id,
    team: match.team,
    slot: match.slot,
  });
  return json({
    match: { match_id: match.match_id, address: match.address, state: match.state },
    join_ticket: joinTicket,
  });
});
