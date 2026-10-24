// Loads local settings for the worker, fake server and tests: the signing keys from
// supabase/functions/.env, plus local Supabase URLs. Real environments set these directly.
import { parse } from "jsr:@std/dotenv@0.225.3";

export const LOCAL_DEFAULTS: Record<string, string> = {
  DATABASE_URL: "postgresql://postgres:postgres@127.0.0.1:55422/postgres",
  SUPABASE_URL: "http://127.0.0.1:55421",
  FUNCTIONS_URL: "http://127.0.0.1:55421/functions/v1",
  // Public demo keys of the local Supabase CLI stack; not secrets.
  SUPABASE_ANON_KEY:
    "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJzdXBhYmFzZS1kZW1vIiwicm9sZSI6ImFub24iLCJleHAiOjE5ODM4MTI5OTZ9.CRXP1A7WOeoJeXxjNni43kdQwgnWNReilDMblYTn_I0",
  SUPABASE_SERVICE_ROLE_KEY:
    "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJzdXBhYmFzZS1kZW1vIiwicm9sZSI6InNlcnZpY2Vfcm9sZSIsImV4cCI6MTk4MzgxMjk5Nn0.EGIM96RAZx35lJzdJsyH-qQwv8Hdp7fsn3W0YpN81IU",
};

export async function loadLocalEnv(): Promise<void> {
  let fromFile: Record<string, string> = {};
  try {
    fromFile = parse(await Deno.readTextFile(new URL("../supabase/functions/.env", import.meta.url)));
  } catch {
    // Keys may come from the real environment instead.
  }
  for (const [key, value] of Object.entries({ ...LOCAL_DEFAULTS, ...fromFile })) {
    if (Deno.env.get(key) === undefined && value !== "") {
      Deno.env.set(key, value);
    }
  }
}
