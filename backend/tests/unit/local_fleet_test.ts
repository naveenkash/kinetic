import { assert, assertEquals } from "jsr:@std/assert@1.0.13";
import { LocalFleet } from "../../worker/fleet/local.ts";

Deno.test({
  name: "game-server processes never inherit the worker's secrets",
  sanitizeResources: false,
  async fn() {
    const logDir = await Deno.makeTempDir();
    Deno.env.set("KINETIC_SIGNING_PRIVATE_JWK", "secret-signing-key");
    Deno.env.set("SUPABASE_SERVICE_ROLE_KEY", "secret-service-key");
    Deno.env.set("DATABASE_URL", "postgresql://secret");
    const fleet = new LocalFleet(
      ["deno", "eval", "console.log('ENV' + JSON.stringify(Deno.env.toObject()))"],
      logDir,
    );
    try {
      const alloc = await fleet.allocate({ matchId: "m1", region: "local", build: "0.1.0", env: { MATCH_ID: "m1" } });
      assert(alloc.port > 0);
      let text = "";
      for (let i = 0; i < 100 && !text.includes("ENV"); i++) {
        await new Promise((r) => setTimeout(r, 100));
        text = await Deno.readTextFile(`${logDir}/m1.log`).catch(() => "");
      }
      const env = JSON.parse(text.slice(text.indexOf("ENV") + 3).split("\n")[0]);
      assertEquals(env.MATCH_ID, "m1");
      assertEquals(env.PORT, String(alloc.port));
      for (const secret of ["KINETIC_SIGNING_PRIVATE_JWK", "SUPABASE_SERVICE_ROLE_KEY", "DATABASE_URL"]) {
        assertEquals(env[secret], undefined, `${secret} leaked to the game server`);
      }
    } finally {
      await fleet.shutdown();
      await Deno.remove(logDir, { recursive: true });
    }
  },
});
