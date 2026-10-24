// The item catalog (backend-architecture 13.1): checks and loads backend/catalog/items.json.
// Pure checks here; tools/load-catalog.ts writes them to the database.

export const SIDEGRADE_SLOTS = ["line", "dog", "cutter"] as const;
export const COSMETIC_SLOTS = [
  "livery",
  "tape",
  "dogs",
  "cutter_skin",
  "rig_call",
  "drop",
  "scrap_card",
  "voice",
  "reserve_livery",
] as const;
const SOURCE_RULES = ["grade", "contract", "manifest", "shop", "admin"] as const;
const CURRENCIES = ["tonnage_credit", "sponsor_credit"] as const;
const ITEM_ID = /^[a-z_]+\.[a-z0-9_]+$/;
const MAX_GRADE = 6;

export interface CatalogItem {
  item_id: string;
  slot: string;
  source_rule: string;
  grade_gated?: number;
  metadata?: Record<string, unknown>;
}

export interface CatalogListing {
  item_id: string;
  currency: string;
  price: number;
}

export interface Catalog {
  items: CatalogItem[];
  shop_rotation: CatalogListing[];
}

function itemProblems(item: CatalogItem): string[] {
  const id = item.item_id;
  const sidegrade = (SIDEGRADE_SLOTS as readonly string[]).includes(item.slot);
  const problems: string[] = [];
  if (!ITEM_ID.test(id)) problems.push(`${id}: bad item id`);
  if (!sidegrade && !(COSMETIC_SLOTS as readonly string[]).includes(item.slot)) problems.push(`${id}: unknown slot`);
  if (!(SOURCE_RULES as readonly string[]).includes(item.source_rule)) problems.push(`${id}: unknown source rule`);
  if (sidegrade && item.source_rule !== "grade" && item.source_rule !== "contract") {
    problems.push(`${id}: a sidegrade is unlocked by Grade or a Contract only`);
  }
  if (sidegrade && item.grade_gated === undefined) problems.push(`${id}: a sidegrade needs a Grade`);
  if (item.source_rule === "grade" && item.grade_gated === undefined) {
    problems.push(`${id}: grade item without a Grade`);
  }
  const g = item.grade_gated;
  if (g !== undefined && (!Number.isInteger(g) || g < 1 || g > MAX_GRADE)) problems.push(`${id}: bad Grade`);
  if (!id.startsWith(`${item.slot}.`)) problems.push(`${id}: id does not start with its slot`);
  return problems;
}

function listingProblems(listing: CatalogListing, items: Map<string, CatalogItem>): string[] {
  const id = listing.item_id;
  const item = items.get(id);
  const problems: string[] = [];
  if (!item) return [`shop ${id}: not in the catalog`];
  if (item.source_rule !== "shop") problems.push(`shop ${id}: only shop items are sold`);
  if (!(CURRENCIES as readonly string[]).includes(listing.currency)) problems.push(`shop ${id}: unknown currency`);
  if (!Number.isInteger(listing.price) || listing.price <= 0) problems.push(`shop ${id}: bad price`);
  return problems;
}

export function catalogProblems(catalog: Catalog): string[] {
  const items = new Map(catalog.items.map((i) => [i.item_id, i]));
  const duplicates = catalog.items.length === items.size ? [] : ["duplicate item ids"];
  const baseline = SIDEGRADE_SLOTS.filter((s) => items.get(`${s}.standard`)?.grade_gated !== 1)
    .map((s) => `${s}.standard: the baseline must be unlocked at Grade 1`);
  const listed = catalog.shop_rotation.map((l) => l.item_id);
  const relisted = listed.length === new Set(listed).size ? [] : ["an item is listed twice"];
  return [
    ...duplicates,
    ...baseline,
    ...relisted,
    ...catalog.items.flatMap(itemProblems),
    ...catalog.shop_rotation.flatMap((l) => listingProblems(l, items)),
  ];
}
