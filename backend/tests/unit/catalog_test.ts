import { assertEquals } from "jsr:@std/assert@1.0.13";
import { type Catalog, catalogProblems } from "../../tools/catalog.ts";

const baseline: Catalog["items"] = [
  { item_id: "line.standard", slot: "line", source_rule: "grade", grade_gated: 1 },
  { item_id: "dog.standard", slot: "dog", source_rule: "grade", grade_gated: 1 },
  { item_id: "cutter.standard", slot: "cutter", source_rule: "grade", grade_gated: 1 },
];

Deno.test("the shipped catalog passes", async () => {
  const catalog = JSON.parse(await Deno.readTextFile(new URL("../../catalog/items.json", import.meta.url)));
  assertEquals(catalogProblems(catalog), []);
});

Deno.test("sidegrades are never sold or shipped, and only shop items are listed", () => {
  const catalog: Catalog = {
    items: [
      ...baseline,
      { item_id: "line.longline", slot: "line", source_rule: "shop" },
      { item_id: "dog.barb", slot: "dog", source_rule: "manifest" },
      { item_id: "tape.chevron", slot: "tape", source_rule: "manifest" },
    ],
    shop_rotation: [
      { item_id: "tape.chevron", currency: "tonnage_credit", price: 300 },
      { item_id: "tape.missing", currency: "tonnage_credit", price: 300 },
    ],
  };
  assertEquals(catalogProblems(catalog), [
    "line.longline: a sidegrade is unlocked by Grade or a Contract only",
    "line.longline: a sidegrade needs a Grade",
    "dog.barb: a sidegrade is unlocked by Grade or a Contract only",
    "dog.barb: a sidegrade needs a Grade",
    "shop tape.chevron: only shop items are sold",
    "shop tape.missing: not in the catalog",
  ]);
});

Deno.test("ids, Grades, prices and the baseline are checked", () => {
  const catalog: Catalog = {
    items: [
      { item_id: "line.standard", slot: "line", source_rule: "grade", grade_gated: 2 },
      { item_id: "dog.standard", slot: "dog", source_rule: "grade", grade_gated: 1 },
      { item_id: "cutter.standard", slot: "cutter", source_rule: "grade", grade_gated: 1 },
      { item_id: "Tape.Bad", slot: "tape", source_rule: "shop" },
      { item_id: "livery.x", slot: "livery", source_rule: "grade" },
      { item_id: "drop.y", slot: "livery", source_rule: "shop" },
      { item_id: "tape.z", slot: "tape", source_rule: "shop" },
    ],
    shop_rotation: [{ item_id: "tape.z", currency: "gold", price: 0 }],
  };
  assertEquals(catalogProblems(catalog), [
    "line.standard: the baseline must be unlocked at Grade 1",
    "Tape.Bad: bad item id",
    "Tape.Bad: id does not start with its slot",
    "livery.x: grade item without a Grade",
    "drop.y: id does not start with its slot",
    "shop tape.z: unknown currency",
    "shop tape.z: bad price",
  ]);
});
