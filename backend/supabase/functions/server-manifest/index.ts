// POST /functions/v1/server-manifest — called once by the game server at boot.
import { json } from "../_shared/http.ts";
import { dbFail, serveServerEndpoint } from "../_shared/server.ts";
import { serviceClient } from "../_shared/supabase.ts";

serveServerEndpoint("server-manifest", async (claims) => {
  const { data, error } = await serviceClient().rpc("srv_manifest", {
    p_match: claims.match_id,
    p_allocation: claims.allocation_id,
  });
  if (error) return dbFail(error);
  return json(data);
});
