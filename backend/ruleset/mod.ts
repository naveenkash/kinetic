// Economy rulesets, one JSON file per version, owned by the game designer (backend-architecture
// 12.3). A version is never edited once a match has been rewarded with it: add a new file, add
// it here, and set the `ruleset` config key to switch. Every number is PROPOSED until tuned.
import type { Ruleset } from "../worker/rewards.ts";
import v1 from "./1.json" with { type: "json" };

export const RULESETS: readonly Ruleset[] = [v1 as Ruleset];
