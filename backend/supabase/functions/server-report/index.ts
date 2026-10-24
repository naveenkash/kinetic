// POST /functions/v1/server-report — one match report of facts at match end.
// Validation lives in api.srv_store_report; an invalid report is quarantined, not rewarded.
import { fail, json } from "../_shared/http.ts";
import { dbFail, serveServerEndpoint } from "../_shared/server.ts";
import { serviceClient } from "../_shared/supabase.ts";

const MAX_REPORT_BYTES = 256 * 1024;

serveServerEndpoint("server-report", async (claims, body) => {
  if (JSON.stringify(body).length > MAX_REPORT_BYTES) return fail("too_large", 413);
  const { data, error } = await serviceClient().rpc("srv_store_report", {
    p_match: claims.match_id,
    p_allocation: claims.allocation_id,
    p_payload: body,
  });
  if (error) return dbFail(error);
  return json(data);
});
