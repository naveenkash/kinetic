// POST /functions/v1/server-loadout — at each reset (the lobby, between rounds), the loadout a
// player takes into the next round, re-checked against what they own now.
// Body: { account_id: "<uuid>" }
import { fail, isUuid, json } from "../_shared/http.ts";
import { dbFail, serveServerEndpoint } from "../_shared/server.ts";
import { serviceClient } from "../_shared/supabase.ts";

serveServerEndpoint("server-loadout", async (claims, body) => {
  if (!isUuid(body.account_id)) return fail("bad_request", 400);
  const { data, error } = await serviceClient().rpc("srv_player_loadout", {
    p_match: claims.match_id,
    p_allocation: claims.allocation_id,
    p_account: body.account_id,
  });
  if (error) return dbFail(error);
  return json({ loadout: data });
});
