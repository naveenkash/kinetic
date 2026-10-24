// Join tickets and per-match server credentials (backend-architecture section 11.1).
// Ed25519 JWTs signed by a KINETIC key held in function secrets and the worker's env.
import { createLocalJWKSet, importJWK, type JSONWebKeySet, type JWK, jwtVerify, SignJWT } from "npm:jose@5.9.6";

export const ISSUER = "kinetic-backend";
export const SERVER_AUDIENCE = "server";
export const JOIN_TICKET_TTL_S = 120;

export interface Keyring {
  privateJwk: JWK & { kid: string };
  publicJwks: JSONWebKeySet;
}

export interface JoinTicketClaims {
  account_id: string;
  match_id: string;
  team: number;
  slot: number;
}

export interface ServerClaims {
  match_id: string;
  allocation_id: string;
}

export function keyringFromEnv(get: (k: string) => string | undefined = (k) => Deno.env.get(k)): Keyring {
  const priv = get("KINETIC_SIGNING_PRIVATE_JWK");
  const pub = get("KINETIC_SIGNING_PUBLIC_JWKS");
  if (!priv || !pub) {
    throw new Error("KINETIC signing keys are not configured");
  }
  const privateJwk = JSON.parse(priv);
  if (!privateJwk.kid) {
    throw new Error("KINETIC_SIGNING_PRIVATE_JWK needs a kid");
  }
  return { privateJwk, publicJwks: JSON.parse(pub) };
}

async function sign(
  keyring: Keyring,
  claims: Record<string, unknown>,
  audience: string,
  subject: string,
  ttlSeconds: number,
): Promise<string> {
  const key = await importJWK(keyring.privateJwk, "EdDSA");
  return await new SignJWT(claims)
    .setProtectedHeader({ alg: "EdDSA", kid: keyring.privateJwk.kid })
    .setIssuer(ISSUER)
    .setAudience(audience)
    .setSubject(subject)
    .setIssuedAt()
    .setExpirationTime(`${ttlSeconds}s`)
    .setJti(crypto.randomUUID())
    .sign(key);
}

export function signJoinTicket(keyring: Keyring, c: JoinTicketClaims, ttlSeconds = JOIN_TICKET_TTL_S) {
  return sign(
    keyring,
    { match_id: c.match_id, team: c.team, slot: c.slot },
    `match:${c.match_id}`,
    c.account_id,
    ttlSeconds,
  );
}

export function signServerCredential(keyring: Keyring, c: ServerClaims, ttlSeconds: number) {
  return sign(keyring, { allocation_id: c.allocation_id }, SERVER_AUDIENCE, c.match_id, ttlSeconds);
}

export async function verifyJoinTicket(
  publicJwks: JSONWebKeySet,
  token: string,
  matchId: string,
): Promise<JoinTicketClaims> {
  const { payload } = await jwtVerify(token, createLocalJWKSet(publicJwks), {
    issuer: ISSUER,
    audience: `match:${matchId}`,
    algorithms: ["EdDSA"],
  });
  if (
    typeof payload.sub !== "string" || payload.match_id !== matchId ||
    typeof payload.team !== "number" || typeof payload.slot !== "number"
  ) {
    throw new Error("malformed join ticket");
  }
  return { account_id: payload.sub, match_id: matchId, team: payload.team, slot: payload.slot };
}

export async function verifyServerCredential(publicJwks: JSONWebKeySet, token: string): Promise<ServerClaims> {
  const { payload } = await jwtVerify(token, createLocalJWKSet(publicJwks), {
    issuer: ISSUER,
    audience: SERVER_AUDIENCE,
    algorithms: ["EdDSA"],
  });
  if (typeof payload.sub !== "string" || typeof payload.allocation_id !== "string") {
    throw new Error("malformed server credential");
  }
  return { match_id: payload.sub, allocation_id: payload.allocation_id };
}
