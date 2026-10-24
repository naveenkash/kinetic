// Small helpers shared by every Edge Function.
import type { Json } from "./database.types.ts";

export type JsonObject = { [key: string]: Json | undefined };

export function json(body: unknown, status = 200): Response {
  return new Response(JSON.stringify(body), {
    status,
    headers: { "content-type": "application/json" },
  });
}

export function fail(error: string, status: number): Response {
  return json({ error }, status);
}

export async function readJson(req: Request): Promise<JsonObject | null> {
  try {
    const body: Json = await req.json();
    return body !== null && typeof body === "object" && !Array.isArray(body) ? body : null;
  } catch {
    return null;
  }
}

export function bearer(req: Request): string | null {
  const header = req.headers.get("authorization") ?? "";
  const match = /^Bearer\s+(.+)$/i.exec(header);
  return match ? match[1].trim() : null;
}

// The client address for rate limiting. Headers a client can send are never trusted: the
// CDN's header only when TRUST_CDN_IP=true says a CDN that overwrites it is in front, else the
// x-forwarded-for entry the gateway appended last. CHECK the hosted gateway before launch.
export function clientIp(req: Request): string {
  if (Deno.env.get("TRUST_CDN_IP") === "true") {
    const cdn = req.headers.get("cf-connecting-ip")?.trim();
    if (cdn) return cdn;
  }
  const hops = (req.headers.get("x-forwarded-for") ?? "").split(",").map((h) => h.trim()).filter(Boolean);
  return hops.at(-1) ?? "unknown";
}

const UUID = /^[0-9a-f]{8}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{12}$/i;
export function isUuid(value: unknown): value is string {
  return typeof value === "string" && UUID.test(value);
}

// Postgres errors raised by `api` functions carry a short code as the message.
export function dbErrorCode(error: { message?: string } | null): string {
  return error?.message ?? "unknown_error";
}
