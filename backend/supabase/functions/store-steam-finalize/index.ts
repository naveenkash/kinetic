// POST /functions/v1/store-steam-finalize — after Steam's authorization callback (13.4 steps 2
// and 3). Body: { order_id: "<digits>", authorized: true | false }. Finalizes with Steam and
// grants in one database transaction; a retry grants nothing more.
// Supabase verifies the player's access token before this runs (verify_jwt = true).
import { bearer, fail, json, readJson } from "../_shared/http.ts";
import { clientRpc, finalizePurchase } from "../_shared/purchases.ts";
import { steamStoreFromEnv } from "../_shared/steam_store.ts";
import { rateLimit, serviceClient, userClient } from "../_shared/supabase.ts";

const FINALIZES_PER_MINUTE = 10;
const RATE_WINDOW_S = 60;
// Order ids are bigints; 18 digits always fit.
const ORDER_ID = /^[0-9]{1,18}$/;

Deno.serve(async (req) => {
  if (req.method !== "POST") return fail("method_not_allowed", 405);
  const token = bearer(req);
  if (!token) return fail("not_authenticated", 401);
  const { data: userData, error: userError } = await userClient(token).auth.getUser();
  if (userError || !userData.user) return fail("not_authenticated", 401);
  const body = await readJson(req);
  if (
    !body || typeof body.order_id !== "string" || !ORDER_ID.test(body.order_id) || typeof body.authorized !== "boolean"
  ) {
    return fail("bad_request", 400);
  }

  const steam = steamStoreFromEnv();
  if (!steam) return fail("store_unavailable", 503);
  const admin = serviceClient();
  if (!(await rateLimit(admin, `finalize:${userData.user.id}`, FINALIZES_PER_MINUTE, RATE_WINDOW_S))) {
    return fail("rate_limited", 429);
  }
  const result = await finalizePurchase(clientRpc(admin), steam, userData.user.id, body.order_id, body.authorized);
  return result.ok ? json(result.body) : fail(result.error, result.status);
});
