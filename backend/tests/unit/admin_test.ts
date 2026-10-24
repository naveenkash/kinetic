import { assertEquals } from "jsr:@std/assert@1.0.13";
import { parseArgs } from "../../tools/admin.ts";

Deno.test("admin commands need a staff name, a known command and their flags", () => {
  assertEquals(parseArgs(["claim:resolve", "--id", "c"]), "--staff is required: every change is audited under a name");
  assertEquals(typeof parseArgs(["--staff", "ana", "claim:explode"]), "string");
  assertEquals(parseArgs(["--staff", "ana", "claim:resolve"]), "claim:resolve needs --id");
  assertEquals(
    parseArgs(["--staff", "ana", "claim:resolve", "--id", "c", "--force", "yes"]),
    "claim:resolve does not take --force",
  );
  assertEquals(parseArgs(["--staff", "ana", "claim:resolve", "--id"]), "--id needs a value");
  assertEquals(parseArgs(["--staff", "ana", "claim:review", "--id", "c", "--holder", "htr", "--variant", "x"]), {
    staff: "ana",
    command: "claim:review",
    args: { id: "c", holder: "htr", variant: "x" },
  });
});
