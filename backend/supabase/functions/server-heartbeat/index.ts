// POST /functions/v1/server-heartbeat — every 10 s from the game server.
// Body: { state: "waiting_for_players" | "in_progress", connected: ["<account_id>", ...] }
import { fail, isUuid, json } from "../_shared/http.ts";
import { dbFail, serveServerEndpoint } from "../_shared/server.ts";
import { serviceClient } from "../_shared/supabase.ts";

const MAX_CONNECTED_PLAYERS = 16;
const SERVER_STATES = ["waiting_for_players", "in_progress"];

serveServerEndpoint("server-heartbeat", async (claims, body) => {
  const connected = Array.isArray(body.connected) ? body.connected : [];
  if (connected.length > MAX_CONNECTED_PLAYERS || !connected.every(isUuid)) return fail("bad_request", 400);
  if (typeof body.state !== "string" || !SERVER_STATES.includes(body.state)) return fail("bad_state", 400);
  const { data, error } = await serviceClient().rpc("srv_heartbeat", {
    p_match: claims.match_id,
    p_allocation: claims.allocation_id,
    p_state: body.state,
    p_connected: connected,
  });
  if (error) return dbFail(error);
  return json(data);
});
