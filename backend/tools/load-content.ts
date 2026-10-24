// The content deploy step (backend-architecture 12.3 and 13.1):
// - backend/ruleset/*.json into game.rulesets, for clients to read. A version already loaded
//   must be identical: a ruleset that has rewarded matches is never edited.
// - backend/catalog/items.json into game.items and the current shop rotation. Items are
//   upserted and never deleted: an item dropped from the catalog is retired, since
//   inventories may hold it. The rotation not tied to a Claim is replaced with this
//   catalog's, from now for ROTATION_DAYS.
//
// deno task content          (DATABASE_URL, the local database by default)
import postgres from "npm:postgres@3.4.5";
import { RULESETS } from "../ruleset/mod.ts";
import { type Catalog, catalogProblems } from "./catalog.ts";
import { loadLocalEnv } from "./env.ts";

const ROTATION_DAYS = 28;

if (Deno.env.get("KINETIC_ENV") !== "production") await loadLocalEnv();

const catalog: Catalog = JSON.parse(await Deno.readTextFile(new URL("../catalog/items.json", import.meta.url)));
const problems = catalogProblems(catalog);
if (problems.length > 0) {
  console.error(`catalog refused:\n  ${problems.join("\n  ")}`);
  Deno.exit(1);
}

const sql = postgres(Deno.env.get("DATABASE_URL")!, { max: 1, onnotice: () => {} });
try {
  await sql.begin(async (tx) => {
    for (const rules of RULESETS) {
      const json = tx.json(rules as unknown as postgres.JSONValue);
      const [row] = await tx<{ same: boolean }[]>`
        insert into game.rulesets (version, rules) values (${rules.version}, ${json})
        on conflict (version) do update set version = excluded.version
        returning rules = ${json} as same`;
      if (!row.same) throw new Error(`ruleset ${rules.version} differs from the one loaded: add a new version`);
    }
    const rows = catalog.items.map((i) => ({
      item_id: i.item_id,
      slot: i.slot,
      source_rule: i.source_rule,
      grade_gated: i.grade_gated ?? null,
      metadata: i.metadata ?? {},
    }));
    // The rotation goes first, so a catalog that takes an item out of the shop can update it.
    await tx`delete from game.shop_rotation where claim_id is null and ends_at > now()`;
    for (const r of rows) {
      await tx`
        insert into game.items (item_id, slot, source_rule, grade_gated, metadata, retired)
        values (${r.item_id}, ${r.slot}, ${r.source_rule}, ${r.grade_gated}, ${
        tx.json(r.metadata as postgres.JSONValue)
      },
                false)
        on conflict (item_id) do update set slot = excluded.slot, source_rule = excluded.source_rule,
          grade_gated = excluded.grade_gated, metadata = excluded.metadata, retired = false`;
    }
    await tx`update game.items set retired = true where item_id not in ${tx(rows.map((r) => r.item_id))}`;
    for (const l of catalog.shop_rotation) {
      await tx`
        insert into game.shop_rotation (item_id, currency, price, starts_at, ends_at)
        values (${l.item_id}, ${l.currency}, ${l.price}, now(), now() + make_interval(days => ${ROTATION_DAYS}))`;
    }
  });
  console.log(
    `content loaded: ${RULESETS.length} rulesets, ${catalog.items.length} items, ${catalog.shop_rotation.length} in the shop`,
  );
} finally {
  await sql.end();
}
