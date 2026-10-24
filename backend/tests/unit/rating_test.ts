import { assert, assertAlmostEquals, assertEquals } from "jsr:@std/assert@1.0.13";
import { band, MU0, ordinal, rate, type RatedPlayer, SIGMA0 } from "../../worker/rating.ts";

const fresh = { mu: MU0, sigma: SIGMA0 };
const player = (key: string, team: number, abandoned = false): RatedPlayer => ({ key, team, rating: fresh, abandoned });

Deno.test("one against one matches OpenSkill's Bradley-Terry reference", () => {
  const r = rate([player("a", 1), player("b", 2)], { rank: { 1: 1, 2: 2 }, forfeit_team: null });
  assertAlmostEquals(r.get("a")!.mu, 27.6351, 1e-3);
  assertAlmostEquals(r.get("b")!.mu, 22.3649, 1e-3);
  assert(r.get("a")!.sigma < SIGMA0 && r.get("b")!.sigma < SIGMA0);
  assertAlmostEquals(r.get("a")!.sigma, r.get("b")!.sigma, 1e-9);
});

Deno.test("crews of three gain and lose by their share of the uncertainty", () => {
  const players = [
    player("a1", 1),
    player("a2", 1),
    { ...player("a3", 1), rating: { mu: 25, sigma: 2 } },
    player("b1", 2),
    player("b2", 2),
    player("b3", 2),
  ];
  const r = rate(players, { rank: { 1: 1, 2: 2 }, forfeit_team: null });
  assert(r.get("a1")!.mu > 25 && r.get("b1")!.mu < 25);
  assert(r.get("a1")!.mu - 25 > r.get("a3")!.mu - 25, "a settled rating moves less");
});

Deno.test("a draw between equals moves no means", () => {
  const r = rate([player("a", 1), player("b", 2)], { rank: { 1: 1, 2: 1 }, forfeit_team: null });
  assertAlmostEquals(r.get("a")!.mu, 25, 1e-12);
});

Deno.test("a forfeit costs a loss plus 20%; a leaver takes the full loss", () => {
  const base = rate([player("a", 1), player("b", 2)], { rank: { 1: 2, 2: 1 }, forfeit_team: null });
  const forfeit = rate([player("a", 1), player("b", 2)], { rank: { 1: 2, 2: 1 }, forfeit_team: 1 });
  assertAlmostEquals(25 - forfeit.get("a")!.mu, 1.2 * (25 - base.get("a")!.mu), 1e-9);
  assertAlmostEquals(forfeit.get("b")!.mu, base.get("b")!.mu, 1e-12);
  const left = rate([player("a", 1, true), player("a2", 1), player("b", 2)], {
    rank: { 1: 1, 2: 2 },
    forfeit_team: null,
  });
  assert(left.get("a")!.mu < 25, "the leaver loses although the crew won");
  assert(left.get("a2")!.mu > 25);
});

Deno.test("bands: Provisional for ten games, then by the conservative ordinal", () => {
  assertEquals(band(fresh, 9), "Provisional");
  assertAlmostEquals(ordinal(fresh), 0, 1e-9);
  assertEquals(band(fresh, 10), "Inspected");
  assertEquals(band({ mu: 20, sigma: 1 }, 50), "Rated");
  assertEquals(band({ mu: 30, sigma: 1 }, 50), "Sanctioned");
  assertEquals(band({ mu: 45, sigma: 1 }, 50), "Championship");
});
