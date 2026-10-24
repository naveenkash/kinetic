// POST /functions/v1/store-steam-init — starts a Steam purchase (backend-architecture 13.4 step 1).
// Body: { sku: "<sku>" }. Returns { order_id, amount, currency }; Steam then shows its overlay.
// Supabase verifies the player's access token before this runs (verify_jwt = true).
import { bearer, fail, json, readJson } from "../_shared/http.ts";
import { clientRpc, initPurchase } from "../_shared/purchases.ts";
import { steamStoreFromEnv } from "../_shared/steam_store.ts";
import { rateLimit, serviceClient, userClient } from "../_shared/supabase.ts";

const PURCHASES_PER_MINUTE = 5;
const RATE_WINDOW_S = 60;
const SKU = /^[a-z0-9_.-]{1,40}$/;

Deno.serve(async (req) => {
  if (req.method !== "POST") return fail("method_not_allowed", 405);
  const token = bearer(req);
  if (!token) return fail("not_authenticated", 401);
  const { data: userData, error: userError } = await userClient(token).auth.getUser();
  if (userError || !userData.user) return fail("not_authenticated", 401);
  const body = await readJson(req);
  if (!body || typeof body.sku !== "string" || !SKU.test(body.sku)) return fail("bad_request", 400);

  const steam = steamStoreFromEnv();
  if (!steam) return fail("store_unavailable", 503);
  const admin = serviceClient();
  if (!(await rateLimit(admin, `purchase:${userData.user.id}`, PURCHASES_PER_MINUTE, RATE_WINDOW_S))) {
    return fail("rate_limited", 429);
  }
  const result = await initPurchase(clientRpc(admin), steam, userData.user.id, body.sku);
  return result.ok ? json(result.body) : fail(result.error, result.status);
});
