// Console identity bridges (backend-architecture 8 and 26, Phase 4): proves a PlayStation,
// Xbox or Nintendo identity from the token the console's platform SDK gives the game client.
//
// Each platform issues a signed token (a JWT) for the game's server, checked here against the
// platform's published signing keys (JWKS), issuer and audience. Which claim carries the
// player's stable id differs by platform, so it is configured too. A platform with any of its
// settings unset is disabled: sign-in answers provider_disabled, as Steam does without keys.
//
// CHECK each platform's partner documentation before launch (it is under NDA, so none of these
// values are in the repository):
// - PlayStation: the ID token from the PSN authorization flow for a game server; its JWKS,
//   issuer, audience (the game's client id) and the account-id claim.
// - Xbox: XSTS tokens issued for a relying party are encrypted (JWE) with the relying party's
//   certificate. Decrypting them needs that key and is not built yet; until it is, Xbox needs a
//   signed token from a token service in front of this one.
// - Nintendo: the NSA ID token; its JWKS, issuer, audience (the application id) and the NSA id
//   claim.
import { createLocalJWKSet, createRemoteJWKSet, type JSONWebKeySet, jwtVerify } from "npm:jose@5.9.6";

export const CONSOLE_PROVIDERS = ["psn", "xbox", "nintendo"] as const;
export type ConsoleProvider = typeof CONSOLE_PROVIDERS[number];

export interface ConsoleConfig {
  jwksUrl?: string;
  jwks?: JSONWebKeySet; // Tests: a local key set instead of fetching one
  issuer: string;
  audience: string;
  subjectClaim: string;
  displayNameClaim?: string;
  clockToleranceS?: number;
}

export interface ConsoleUser {
  providerUserId: string;
  displayName: string | null;
}

const ENV_PREFIX: Record<ConsoleProvider, string> = { psn: "PSN", xbox: "XBOX", nintendo: "NINTENDO" };
const MAX_TOKEN_LENGTH = 16384;
const DEFAULT_TOLERANCE_S = 30;
const SUBJECT = /^[A-Za-z0-9._:-]{1,128}$/;

export function isConsoleProvider(value: unknown): value is ConsoleProvider {
  return typeof value === "string" && (CONSOLE_PROVIDERS as readonly string[]).includes(value);
}

// The provider's settings from the environment, or null when it is not configured.
export function consoleConfig(
  provider: ConsoleProvider,
  env: (name: string) => string | undefined,
): ConsoleConfig | null {
  const p = ENV_PREFIX[provider];
  const jwksUrl = env(`${p}_JWKS_URL`);
  const issuer = env(`${p}_ISSUER`);
  const audience = env(`${p}_AUDIENCE`);
  const subjectClaim = env(`${p}_SUBJECT_CLAIM`) ?? "sub";
  if (!jwksUrl || !issuer || !audience) return null;
  return { jwksUrl, issuer, audience, subjectClaim, displayNameClaim: env(`${p}_NAME_CLAIM`) };
}

const remoteSets = new Map<string, ReturnType<typeof createRemoteJWKSet>>();

export async function verifyConsoleToken(token: string, config: ConsoleConfig): Promise<ConsoleUser> {
  if (token.length === 0 || token.length > MAX_TOKEN_LENGTH || token.split(".").length !== 3) {
    throw new Error("malformed_token");
  }
  let keys;
  if (config.jwks) {
    keys = createLocalJWKSet(config.jwks);
  } else if (config.jwksUrl) {
    keys = remoteSets.get(config.jwksUrl) ?? createRemoteJWKSet(new URL(config.jwksUrl));
    remoteSets.set(config.jwksUrl, keys);
  } else {
    throw new Error("no_keys");
  }
  const { payload } = await jwtVerify(token, keys, {
    issuer: config.issuer,
    audience: config.audience,
    clockTolerance: config.clockToleranceS ?? DEFAULT_TOLERANCE_S,
    // Asymmetric signatures only: a token signed with a shared secret proves nothing here.
    algorithms: ["RS256", "RS384", "RS512", "PS256", "PS384", "PS512", "ES256", "ES384", "ES512", "EdDSA"],
  });
  if (payload.exp === undefined) throw new Error("no_expiry");
  // 64-bit platform ids sent as JSON numbers are already rounded past 2^53: two players would
  // share one id, so only an exact integer, or text, is taken.
  const subject = payload[config.subjectClaim];
  const id = typeof subject === "number" && Number.isSafeInteger(subject) ? String(subject) : subject;
  if (typeof id !== "string" || !SUBJECT.test(id)) throw new Error("no_subject");
  const name = config.displayNameClaim ? payload[config.displayNameClaim] : undefined;
  return { providerUserId: id, displayName: typeof name === "string" ? name : null };
}
