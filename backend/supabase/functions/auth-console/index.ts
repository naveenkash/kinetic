// POST /functions/v1/auth-console
// Signs in with a PlayStation, Xbox or Nintendo identity, or links one to the signed-in
// account so a player keeps one Ledger across platforms (backend-architecture 8.4, Phase 4).
//
// Sign in: { provider: "psn" | "xbox" | "nintendo", token: "<platform token>", build: "0.1.0" }
// Link:    { ..., link: true, proof: { provider: "steam", ticket: "<hex>" } }
//          with Authorization: Bearer <the account's access token>. The proof is an identity
//          the account already has, proved now (a Steam ticket, or { provider, token } for a
//          platform already linked), so a stolen session alone cannot link anything.
//
// A token is proved against the platform's signing keys (_shared/console.ts). A platform whose
// settings are not configured answers provider_disabled. Sign-in runs the same pipeline as
// auth-steam (_shared/login.ts). Linking fails if the identity belongs to another account, the
// account already has one on that platform, or the account is banned. The logic is in
// handler.ts.
import { fail } from "../_shared/http.ts";
import { serviceClient } from "../_shared/supabase.ts";
import { handleConsole, realDeps } from "./handler.ts";

Deno.serve(async (req) => {
  try {
    return await handleConsole(req, realDeps(serviceClient()));
  } catch (e) {
    console.error(e);
    return fail("internal_error", 500);
  }
});
