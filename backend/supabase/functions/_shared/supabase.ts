import { createClient, type SupabaseClient } from "npm:@supabase/supabase-js@2.49.4";
import type { Database } from "./database.types.ts";

function env(name: string): string {
  const value = Deno.env.get(name);
  if (!value) {
    throw new Error(`${name} is not set`);
  }
  return value;
}

// Typed against the `api` schema; regenerate database.types.ts with `deno task types`.
export type Client = SupabaseClient<Database, "api">;

const options = {
  db: { schema: "api" as const },
  auth: { persistSession: false, autoRefreshToken: false, detectSessionInUrl: false },
};

// Service client: only inside Edge Functions and the worker. Never in a client or game server.
export function serviceClient(): Client {
  return createClient<Database, "api">(env("SUPABASE_URL"), env("SUPABASE_SERVICE_ROLE_KEY"), options);
}

export function anonClient(): Client {
  return createClient<Database, "api">(env("SUPABASE_URL"), env("SUPABASE_ANON_KEY"), options);
}

// Acts as the calling player: their access token goes on every request.
export function userClient(accessToken: string): Client {
  return createClient<Database, "api">(env("SUPABASE_URL"), env("SUPABASE_ANON_KEY"), {
    ...options,
    global: { headers: { Authorization: `Bearer ${accessToken}` } },
  });
}

export async function rateLimit(
  client: Client,
  key: string,
  max: number,
  windowSeconds: number,
): Promise<boolean> {
  const { data, error } = await client.rpc("srv_rate_limit", {
    p_key: key,
    p_max: max,
    p_window_s: windowSeconds,
  });
  if (error) {
    throw new Error(`rate limit check failed: ${error.message}`);
  }
  return data === true;
}
