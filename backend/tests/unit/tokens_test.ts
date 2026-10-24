import { assertEquals, assertRejects } from "jsr:@std/assert@1.0.13";
import { exportJWK, generateKeyPair } from "npm:jose@5.9.6";
import {
  type Keyring,
  signJoinTicket,
  signServerCredential,
  verifyJoinTicket,
  verifyServerCredential,
} from "../../supabase/functions/_shared/tokens.ts";

async function keyring(kid: string): Promise<Keyring> {
  const { publicKey, privateKey } = await generateKeyPair("EdDSA", { crv: "Ed25519", extractable: true });
  return {
    privateJwk: { ...(await exportJWK(privateKey)), kid },
    publicJwks: { keys: [{ ...(await exportJWK(publicKey)), kid, alg: "EdDSA" }] },
  };
}

const ticket = {
  account_id: "a0000000-0000-4000-8000-000000000001",
  match_id: "m0000000-0000-4000-8000-000000000001",
  team: 2,
  slot: 5,
};

Deno.test("join ticket round-trips for its own match", async () => {
  const k = await keyring("k1");
  const token = await signJoinTicket(k, ticket);
  assertEquals(await verifyJoinTicket(k.publicJwks, token, ticket.match_id), ticket);
});

Deno.test("join ticket is rejected for another match", async () => {
  const k = await keyring("k1");
  const token = await signJoinTicket(k, ticket);
  await assertRejects(() => verifyJoinTicket(k.publicJwks, token, "m0000000-0000-4000-8000-000000000002"));
});

Deno.test("join ticket from another key is rejected", async () => {
  const k1 = await keyring("k1");
  const k2 = await keyring("k1");
  const token = await signJoinTicket(k1, ticket);
  await assertRejects(() => verifyJoinTicket(k2.publicJwks, token, ticket.match_id));
});

Deno.test("expired join ticket is rejected", async () => {
  const k = await keyring("k1");
  const token = await signJoinTicket(k, ticket, -10);
  await assertRejects(() => verifyJoinTicket(k.publicJwks, token, ticket.match_id));
});

Deno.test("a join ticket is not a server credential, and the reverse", async () => {
  const k = await keyring("k1");
  const join = await signJoinTicket(k, ticket);
  await assertRejects(() => verifyServerCredential(k.publicJwks, join));
  const server = await signServerCredential(k, { match_id: ticket.match_id, allocation_id: "alloc-1" }, 60);
  await assertRejects(() => verifyJoinTicket(k.publicJwks, server, ticket.match_id));
  assertEquals(await verifyServerCredential(k.publicJwks, server), {
    match_id: ticket.match_id,
    allocation_id: "alloc-1",
  });
});

Deno.test("a previous key still verifies during rotation", async () => {
  const old = await keyring("old");
  const current = await keyring("new");
  const rotated: Keyring = { ...current, publicJwks: { keys: [...current.publicJwks.keys, ...old.publicJwks.keys] } };
  const token = await signJoinTicket(old, ticket);
  assertEquals((await verifyJoinTicket(rotated.publicJwks, token, ticket.match_id)).slot, 5);
});
