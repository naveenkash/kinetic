// auth-console's handler with its outside calls faked: sign-in, and link mode, which needs the
// session and proof of an identity the account already has.
import { assertEquals } from "jsr:@std/assert@1.0.13";
import { type ConsoleDeps, handleConsole } from "../../supabase/functions/auth-console/handler.ts";
import { syntheticEmail } from "../../supabase/functions/_shared/login.ts";
import type { Client } from "../../supabase/functions/_shared/supabase.ts";

const ACCOUNT = "11111111-1111-4111-8111-111111111111";
const ENV: Record<string, string> = {
  PSN_JWKS_URL: "https://keys.test/psn",
  PSN_ISSUER: "https://psn.test",
  PSN_AUDIENCE: "kinetic",
  XBOX_JWKS_URL: "https://keys.test/xbox",
  XBOX_ISSUER: "https://xbox.test",
  XBOX_AUDIENCE: "kinetic",
  STEAM_APP_ID: "480",
  STEAM_PUBLISHER_KEY: "key",
};

interface Seen {
  linked: unknown[];
  signedIn: unknown[];
}

function deps(opts: {
  user?: string | null;
  owners?: Record<string, string>; // "provider:id" -> account
  linkError?: string;
  env?: Record<string, string>;
} = {}): { deps: ConsoleDeps; seen: Seen } {
  const seen: Seen = { linked: [], signedIn: [] };
  const admin = {
    auth: {
      getUser: (token: string) =>
        Promise.resolve(
          token === "good" && opts.user !== null
            ? { data: { user: { id: opts.user ?? ACCOUNT } }, error: null }
            : { data: { user: null }, error: { message: "bad jwt" } },
        ),
    },
    rpc: (name: string, args: unknown) => {
      if (name !== "srv_link_identity") throw new Error(name);
      seen.linked.push(args);
      return Promise.resolve(
        opts.linkError
          ? { data: null, error: { message: opts.linkError } }
          : { data: { linked: true, already: false }, error: null },
      );
    },
  } as unknown as Client;
  const env = opts.env ?? ENV;
  return {
    seen,
    deps: {
      admin,
      env: (name) => env[name],
      verifyConsole: (token) => {
        if (!token.startsWith("ok.")) return Promise.reject(new Error("bad"));
        return Promise.resolve({ providerUserId: token.split(".")[1], displayName: null });
      },
      verifySteam: (ticket) =>
        ticket === "00112233445566778899"
          ? Promise.resolve({ steamId: "765611980", vacBanned: false, publisherBanned: false })
          : Promise.reject(new Error("steam_rejected")),
      preflight: () => Promise.resolve(null),
      signIn: (_admin, identity) => {
        seen.signedIn.push(identity);
        return Promise.resolve(new Response("{}", { status: 200 }));
      },
      identityRateOk: () => Promise.resolve(true),
      findLinkedAccount: (_admin, identity) =>
        Promise.resolve(
          (opts.owners ?? { "steam:765611980": ACCOUNT })[`${identity.provider}:${identity.providerUserId}`] ?? null,
        ),
    },
  };
}

function post(body: unknown, token?: string): Request {
  const headers: Record<string, string> = { "content-type": "application/json" };
  if (token) headers.authorization = `Bearer ${token}`;
  return new Request("http://local/auth-console", { method: "POST", headers, body: JSON.stringify(body) });
}

async function call(d: ConsoleDeps, body: unknown, token?: string) {
  const res = await handleConsole(post(body, token), d);
  return [res.status, (await res.json()).error ?? null];
}

const STEAM_PROOF = { provider: "steam", ticket: "00112233445566778899" };

Deno.test("a proved platform token signs in; a failed or unconfigured one does not", async () => {
  const { deps: d, seen } = deps();
  assertEquals(await call(d, { provider: "psn", token: "ok.p1", build: "1" }), [200, null]);
  assertEquals(seen.signedIn, [{ provider: "psn", providerUserId: "p1", displayName: null }]);
  assertEquals(await call(d, { provider: "psn", token: "bad.p1", build: "1" }), [401, "token_rejected"]);
  assertEquals(await call(d, { provider: "nintendo", token: "ok.n1", build: "1" }), [503, "provider_disabled"]);
  assertEquals(await call(d, { provider: "steam", token: "ok.s", build: "1" }), [400, "bad_provider"]);
});

Deno.test("linking needs the session and fresh proof of an identity the account has", async () => {
  const { deps: d, seen } = deps();
  const body = { provider: "psn", token: "ok.p1", build: "1", link: true };
  assertEquals(await call(d, body), [401, "unauthorized"]);
  assertEquals(await call(d, body, "stolen"), [401, "unauthorized"]);
  // A valid session alone is not enough.
  assertEquals(await call(d, body, "good"), [403, "proof_required"]);
  assertEquals(await call(d, { ...body, proof: { provider: "steam", ticket: "ffff" } }, "good"), [
    403,
    "proof_required",
  ]);
  // Proof of someone else's Steam account does not link to this one.
  const other = deps({ owners: { "steam:765611980": "22222222-2222-4222-8222-222222222222" } });
  assertEquals(await call(other.deps, { ...body, proof: STEAM_PROOF }, "good"), [403, "proof_mismatch"]);
  assertEquals(other.seen.linked.length, 0);
  // Fresh proof of this account's Steam identity links the platform.
  assertEquals(await call(d, { ...body, proof: STEAM_PROOF }, "good"), [200, null]);
  assertEquals(seen.linked, [{ p_account: ACCOUNT, p_provider: "psn", p_provider_user_id: "p1" }]);
});

Deno.test("a platform already linked can prove the account, dev only where dev login is on", async () => {
  const { deps: d } = deps({ owners: { "xbox:x1": ACCOUNT } });
  const body = { provider: "psn", token: "ok.p1", build: "1", link: true };
  assertEquals(await call(d, { ...body, proof: { provider: "xbox", token: "ok.x1" } }, "good"), [200, null]);
  assertEquals(await call(d, { ...body, proof: { provider: "dev", name: "ana" } }, "good"), [403, "proof_required"]);
  const dev = deps({ owners: { "dev:ana": ACCOUNT }, env: { ...ENV, ALLOW_DEV_LOGIN: "true" } });
  assertEquals(await call(dev.deps, { ...body, proof: { provider: "dev", name: "ana" } }, "good"), [200, null]);
});

Deno.test("the database's refusals map to answers", async () => {
  const body = { provider: "psn", token: "ok.p1", build: "1", link: true, proof: STEAM_PROOF };
  for (
    const [code, status] of [["identity_conflict", 409], ["provider_already_linked", 409], ["banned", 403]] as const
  ) {
    assertEquals(await call(deps({ linkError: code }).deps, body, "good"), [status, code]);
  }
  assertEquals(await call(deps({ linkError: "unknown_account" }).deps, body, "good"), [401, "unauthorized"]);
  assertEquals(await call(deps({ linkError: "boom" }).deps, body, "good"), [500, "internal_error"]);
});

Deno.test("console ids become an address Auth accepts, distinct by case; Steam's are unchanged", async () => {
  const a = await syntheticEmail({ provider: "psn", providerUserId: "Ab:c", displayName: null });
  const b = await syntheticEmail({ provider: "psn", providerUserId: "ab:c", displayName: null });
  assertEquals(/^psn-[0-9a-f]{40}@users\.kinetic\.invalid$/.test(a), true);
  assertEquals(a === b, false);
  assertEquals(
    await syntheticEmail({ provider: "steam", providerUserId: "76561198000000001", displayName: null }),
    "steam-76561198000000001@users.kinetic.invalid",
  );
});
