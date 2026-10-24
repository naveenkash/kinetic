// Console identity bridges: a platform token is proved against the platform's keys, issuer and
// audience, and nothing weaker passes.
import { assertEquals, assertRejects } from "jsr:@std/assert@1.0.13";
import { exportJWK, generateKeyPair, SignJWT } from "npm:jose@5.9.6";
import {
  type ConsoleConfig,
  consoleConfig,
  isConsoleProvider,
  verifyConsoleToken,
} from "../../supabase/functions/_shared/console.ts";

const ISSUER = "https://auth.example-platform.test";
const AUDIENCE = "kinetic-game";

async function platform() {
  const { privateKey, publicKey } = await generateKeyPair("RS256");
  const jwk = { ...(await exportJWK(publicKey)), kid: "k1", alg: "RS256", use: "sig" };
  const config: ConsoleConfig = { jwks: { keys: [jwk] }, issuer: ISSUER, audience: AUDIENCE, subjectClaim: "sub" };
  const sign = (claims: Record<string, unknown>, opts: { exp?: string | null; iss?: string; aud?: string } = {}) => {
    let jwt = new SignJWT(claims).setProtectedHeader({ alg: "RS256", kid: "k1" }).setIssuedAt()
      .setIssuer(opts.iss ?? ISSUER).setAudience(opts.aud ?? AUDIENCE);
    if (opts.exp !== null) jwt = jwt.setExpirationTime(opts.exp ?? "5m");
    return jwt.sign(privateKey);
  };
  return { config, sign };
}

Deno.test("a platform token signed by the platform proves the player's id", async () => {
  const { config, sign } = await platform();
  const user = await verifyConsoleToken(await sign({ sub: "psn-1234567890", name: "rigger" }), {
    ...config,
    displayNameClaim: "name",
  });
  assertEquals(user, { providerUserId: "psn-1234567890", displayName: "rigger" });
});

Deno.test("the id can come from another claim, and a numeric id is kept as text", async () => {
  const { config, sign } = await platform();
  const user = await verifyConsoleToken(await sign({ nsa_id: 4242424242 }), { ...config, subjectClaim: "nsa_id" });
  assertEquals(user.providerUserId, "4242424242");
});

Deno.test("a numeric id past 2^53 is refused rather than rounded into someone else's", async () => {
  const { config, sign } = await platform();
  await assertRejects(
    async () => verifyConsoleToken(await sign({ nsa_id: 2 ** 53 + 2 }), { ...config, subjectClaim: "nsa_id" }),
    Error,
    "no_subject",
  );
  const user = await verifyConsoleToken(await sign({ nsa_id: "76561198000000001" }), {
    ...config,
    subjectClaim: "nsa_id",
  });
  assertEquals(user.providerUserId, "76561198000000001");
});

Deno.test("the wrong audience, issuer or keys, an expired token, or no expiry is refused", async () => {
  const { config, sign } = await platform();
  const other = await platform();
  await assertRejects(async () => verifyConsoleToken(await sign({ sub: "a" }, { aud: "another-game" }), config));
  await assertRejects(async () => verifyConsoleToken(await sign({ sub: "a" }, { iss: "https://evil.test" }), config));
  await assertRejects(async () => verifyConsoleToken(await other.sign({ sub: "a" }), config));
  await assertRejects(async () => verifyConsoleToken(await sign({ sub: "a" }, { exp: "-10m" }), config));
  await assertRejects(
    async () => verifyConsoleToken(await sign({ sub: "a" }, { exp: null }), config),
    Error,
    "no_expiry",
  );
});

Deno.test("a shared-secret token, a junk id or a malformed token is refused", async () => {
  const { config, sign } = await platform();
  const hs = await new SignJWT({ sub: "a" }).setProtectedHeader({ alg: "HS256" }).setIssuer(ISSUER)
    .setAudience(AUDIENCE).setExpirationTime("5m").sign(new TextEncoder().encode("a shared secret of 32 bytes!!!!"));
  await assertRejects(() => verifyConsoleToken(hs, config));
  await assertRejects(async () => verifyConsoleToken(await sign({ sub: "has spaces / and slashes" }), config));
  await assertRejects(async () => verifyConsoleToken(await sign({}), config), Error, "no_subject");
  await assertRejects(() => verifyConsoleToken("not-a-jwt", config), Error, "malformed_token");
  await assertRejects(() => verifyConsoleToken("a.b.c".repeat(5000), config), Error, "malformed_token");
});

Deno.test("a platform is disabled until its keys, issuer and audience are all set", () => {
  const env = (vars: Record<string, string>) => (name: string) => vars[name];
  assertEquals(consoleConfig("psn", env({})), null);
  assertEquals(consoleConfig("psn", env({ PSN_JWKS_URL: "https://k.test/jwks", PSN_ISSUER: ISSUER })), null);
  assertEquals(
    consoleConfig(
      "nintendo",
      env({
        NINTENDO_JWKS_URL: "https://k.test/jwks",
        NINTENDO_ISSUER: ISSUER,
        NINTENDO_AUDIENCE: AUDIENCE,
        NINTENDO_SUBJECT_CLAIM: "nsa_id",
      }),
    ),
    {
      jwksUrl: "https://k.test/jwks",
      issuer: ISSUER,
      audience: AUDIENCE,
      subjectClaim: "nsa_id",
      displayNameClaim: undefined,
    },
  );
  assertEquals([isConsoleProvider("xbox"), isConsoleProvider("steam"), isConsoleProvider(3)], [true, false, false]);
});
