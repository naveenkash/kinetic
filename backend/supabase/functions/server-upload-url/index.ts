// POST /functions/v1/server-upload-url — signed Storage upload URLs for the replay and
// the raw event file of this match only.
import { fail, json } from "../_shared/http.ts";
import { serveServerEndpoint } from "../_shared/server.ts";
import { serviceClient } from "../_shared/supabase.ts";

// Inside the local stack SUPABASE_URL is the internal gateway; KINETIC_PUBLIC_URL, when
// set, is the address a game server can reach. In production the two are the same.
function publicUrl<T extends { signedUrl: string }>(data: T): T {
  const external = Deno.env.get("KINETIC_PUBLIC_URL");
  if (!external) return data;
  const url = new URL(data.signedUrl);
  const base = new URL(external);
  url.protocol = base.protocol;
  url.host = base.host;
  return { ...data, signedUrl: url.toString() };
}

serveServerEndpoint("server-upload-url", async (claims) => {
  const admin = serviceClient();
  const { data: match, error } = await admin.rpc("srv_manifest", {
    p_match: claims.match_id,
    p_allocation: claims.allocation_id,
  });
  if (error || !match) return fail("unknown_match", 404);

  const replay = await admin.storage.from("replays").createSignedUploadUrl(`${claims.match_id}.replay`, {
    upsert: true,
  });
  const events = await admin.storage.from("match-events").createSignedUploadUrl(
    `${claims.match_id}.jsonl.gz`,
    { upsert: true },
  );
  if (replay.error || events.error) return fail("internal_error", 500);
  return json({ replay: publicUrl(replay.data), events: publicUrl(events.data) });
});
