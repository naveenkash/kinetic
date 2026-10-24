// Shapes of the JSON returned by the srv_* functions. The generated types only say `Json`
// for jsonb results; these interfaces state what each SQL function builds, and jsonObject
// narrows a result to one after checking it is an object.
import type { Json } from "./database.types.ts";

export interface EnsuredAccount {
  account_id: string;
  display_name: string;
  status: string;
  grade: number;
  banned: boolean;
}

export interface CurrentMatch {
  match_id: string;
  address: string;
  team: number;
  slot: number;
  state: string;
}

export function jsonObject<T>(value: Json | null | undefined): T | null {
  const isObject = value !== null && typeof value === "object" && !Array.isArray(value);
  return isObject ? value as unknown as T : null;
}
