// POST /functions/v1/server-player-event — on join, leave and abandon.
// Body: { account_id: "<uuid>", event: "joined" | "left" | "abandoned" }
import { fail, isUuid, json } from "../_shared/http.ts";
import { dbFail, serveServerEndpoint } from "../_shared/server.ts";
import { serviceClient } from "../_shared/supabase.ts";

serveServerEndpoint("server-player-event", async (claims, body) => {
  if (!isUuid(body.account_id) || typeof body.event !== "string") return fail("bad_request", 400);
  const { error } = await serviceClient().rpc("srv_player_event", {
    p_match: claims.match_id,
    p_allocation: claims.allocation_id,
    p_account: body.account_id,
    p_event: body.event,
  });
  if (error) return dbFail(error);
  return json({ ok: true });
});
