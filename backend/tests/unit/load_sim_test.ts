import { assertEquals } from "jsr:@std/assert@1.0.13";
import { parseFlags, summarize } from "../../tools/load-sim.ts";

Deno.test("players come in matches of two crews of three", () => {
  assertEquals(parseFlags(["--players", "12"]), { players: 12, playlist: "scrap_3v3_casual", timeoutS: 180 });
  assertEquals(typeof parseFlags(["--players", "10"]), "string");
  assertEquals(typeof parseFlags(["--players"]), "string");
});

Deno.test("the summary gives p50, p95 and max per step", () => {
  const s = summarize({ login: [10, 20, 30, 40, 1000] }, { join: 1 });
  assertEquals(s.steps.login, { n: 5, p50_ms: 30, p95_ms: 1000, max_ms: 1000 });
  assertEquals(s.failures, { join: 1 });
});
