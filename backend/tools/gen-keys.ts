// Generates a KINETIC Ed25519 signing key and writes supabase/functions/.env for local dev.
// Usage: deno task keys            (refuses to overwrite an existing file)
//        deno task keys --force
import { exportJWK, generateKeyPair } from "npm:jose@5.9.6";

const target = new URL("../supabase/functions/.env", import.meta.url);
const force = Deno.args.includes("--force");

try {
  await Deno.stat(target);
  if (!force) {
    console.log(`${target.pathname} exists; pass --force to replace it.`);
    Deno.exit(0);
  }
} catch {
  // Not there yet.
}

const { publicKey, privateKey } = await generateKeyPair("EdDSA", { crv: "Ed25519", extractable: true });
const kid = `kinetic-${new Date().toISOString().slice(0, 10)}-${crypto.randomUUID().slice(0, 8)}`;
const privateJwk = { ...(await exportJWK(privateKey)), kid, alg: "EdDSA", use: "sig" };
const publicJwk = { ...(await exportJWK(publicKey)), kid, alg: "EdDSA", use: "sig" };

const lines = [
  "# Local development only. Never commit. Production secrets live in Supabase function secrets.",
  `KINETIC_SIGNING_PRIVATE_JWK=${JSON.stringify(privateJwk)}`,
  `KINETIC_SIGNING_PUBLIC_JWKS=${JSON.stringify({ keys: [publicJwk] })}`,
  "ALLOW_DEV_LOGIN=true",
  "KINETIC_PUBLIC_URL=http://127.0.0.1:55421",
  "# Every local client shares the Docker gateway address, so the per-IP login limit is raised.",
  "KINETIC_LOGIN_IP_LIMIT=200",
  "STEAM_APP_ID=",
  "STEAM_PUBLISHER_KEY=",
  "",
];
await Deno.writeTextFile(target, lines.join("\n"), { mode: 0o600 });
console.log(`Wrote ${target.pathname} with key ${kid}.`);
