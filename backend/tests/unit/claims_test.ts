import { assertEquals } from "jsr:@std/assert@1.0.13";
import {
  advanceContracts,
  arenaVariant,
  capped,
  claimWeek,
  freeScrapPlaces,
  holder,
  matchPoints,
  places,
  rewardLists,
  standings,
} from "../../worker/claims.ts";

Deno.test("Standing points follow game-modes 7", () => {
  const base = { mode: "scrap", rated: false, win: true, voided: false };
  assertEquals(matchPoints(base), 3);
  assertEquals(matchPoints({ ...base, win: false }), 1);
  assertEquals(matchPoints({ ...base, rated: true }), 4.5);
  assertEquals(matchPoints({ ...base, rated: true, win: false }), 1.5);
  assertEquals(matchPoints({ ...base, voided: true }), 0);
  assertEquals(matchPoints({ ...base, mode: "free_scrap", place: 3 }), 2);
  assertEquals(matchPoints({ ...base, mode: "free_scrap", place: 4 }), 1);
});

Deno.test("Free Scrap places share ties", () => {
  assertEquals(places([5, 3, 5, 1, 3]), [1, 3, 1, 5, 3]);
});

Deno.test("Free Scrap: the winner first, then scraps; no scraps, no top 3", () => {
  const p = (team: number, eliminations: number) => ({ team, eliminations });
  assertEquals(freeScrapPlaces([p(1, 2), p(2, 5), p(3, 2), p(4, 0)], 2), [2, 1, 2, Infinity]);
  assertEquals(freeScrapPlaces([p(1, 0), p(2, 0), p(3, 0)], 3), [Infinity, Infinity, 1]);
  assertEquals(freeScrapPlaces([p(1, 3), p(2, 1)], null), [1, 2]);
});

Deno.test("rewards are read defensively", () => {
  assertEquals(rewardLists({ items: ["a", 3], entries: "x" }), { items: ["a"], entries: [] });
  assertEquals(rewardLists(null), { items: [], entries: [] });
});

Deno.test("the weekly cap clips, never goes negative", () => {
  assertEquals([capped(3, 0), capped(3, 58), capped(3, 60), capped(3, 75)], [3, 2, 0, 0]);
});

Deno.test("Claim weeks run from the start", () => {
  const start = new Date("2026-11-01T00:00:00Z");
  assertEquals(claimWeek(start, new Date("2026-11-01T00:00:00Z")), 1);
  assertEquals(claimWeek(start, new Date("2026-11-07T23:59:59Z")), 1);
  assertEquals(claimWeek(start, new Date("2026-11-08T00:00:00Z")), 2);
});

Deno.test("Contracts advance on their counter and complete once", () => {
  const contracts = [
    { contract_id: "hang-20", counter_key: "lines_hung_used_by_others", target: 20 },
    { contract_id: "snap-1", counter_key: "snaps_survived", target: 1 },
    { contract_id: "done", counter_key: "snaps_survived", target: 1 },
  ];
  const progress = new Map([["hang-20", { progress: 18, completed: false }], ["done", {
    progress: 1,
    completed: true,
  }]]);
  assertEquals(advanceContracts(contracts, progress, { lines_hung_used_by_others: 3, snaps_survived: 1 }), [
    { contract_id: "hang-20", progress: 21, completed: true },
    { contract_id: "snap-1", progress: 1, completed: true },
  ]);
  assertEquals(advanceContracts(contracts, progress, {}), []);
});

Deno.test("S_f divides by at least 60% of the mean population", () => {
  const s = standings([
    { faction: "guild", points: 1000, active_players: 100, rated_wins: 0 },
    { faction: "htr", points: 300, active_players: 10, rated_wins: 0 },
  ]);
  // mean(A) = 110 / 5 = 22, so HTR divides by 13.2, not 10.
  assertEquals(s.find((x) => x.faction === "guild")!.standing, 10);
  assertEquals(Math.round(s.find((x) => x.faction === "htr")!.standing * 1000) / 1000, 22.727);
  assertEquals(s.find((x) => x.faction === "fray")!.standing, 0);
  assertEquals(holder(s), "htr");
});

Deno.test("ties go to Rated wins; a dead heat or an empty Claim has no holder", () => {
  const tied = (a: number, b: number) =>
    standings([
      { faction: "guild", points: 100, active_players: 10, rated_wins: a },
      { faction: "fray", points: 100, active_players: 10, rated_wins: b },
    ]);
  assertEquals(holder(tied(5, 2)), "guild");
  assertEquals(holder(tied(2, 5)), "fray");
  assertEquals(holder(tied(3, 3)), null);
  assertEquals(holder(standings([])), null);
});

Deno.test("each arena's outcome follows bible/04", () => {
  assertEquals(arenaVariant("HGT", "htr"), "district_removed");
  assertEquals(arenaVariant("HGT", "hangers"), "base");
  assertEquals(arenaVariant("BLS", "htr"), "crown_and_spine");
  assertEquals(arenaVariant("BLS", "guild"), "stabilized");
  assertEquals(arenaVariant("YRD", "htr"), "haulers_doubled");
  assertEquals(arenaVariant("YRD", "guild"), "base");
  assertEquals(arenaVariant("ANC", "fray"), "colors_fray");
});
