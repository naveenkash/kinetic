// The worker's Supabase Storage adapter against the local stack (supabase start).
import { assertEquals } from "jsr:@std/assert@1.0.13";
import { loadLocalEnv } from "../../tools/env.ts";
import { supabaseStorage } from "../../worker/storage.ts";

Deno.test("the Storage adapter uploads, lists and removes", async () => {
  await loadLocalEnv();
  const store = supabaseStorage();
  const folder = `storage-test-${crypto.randomUUID()}`;
  await store.upload("exports", `${folder}/a.json`, '{"ok":true}', "application/json");
  assertEquals(await store.list("exports", folder), ["a.json"]);
  await store.remove("exports", [`${folder}/a.json`]);
  assertEquals(await store.list("exports", folder), []);
});
