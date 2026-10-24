import { assert, assertEquals } from "jsr:@std/assert@1.0.13";
import { parseReport } from "../../worker/report_parse.ts";
import {
  earners,
  factProblems,
  gradeFor,
  type PlayerState,
  type Report,
  type ReportFacts,
  type ReportPlayer,
  rewardPlayer,
  ropeTier,
  rulesetFor,
  tonnageLines,
  ZERO_CAREER,
} from "../../worker/rewards.ts";

const rules = rulesetFor(1);

const NO_FACTS: ReportFacts = {
  mass_distance_kg_m: 0,
  eliminations: 0,
  assists: 0,
  spotter_credits: 0,
  ringout_heights_m: [],
  rope_marks: [],
  counters: {},
  floor_seconds: 0,
  airborne_seconds: 0,
  hoisted_seconds: 0,
};

// menus-and-frontend 13's worked example: 3,120 t.m, 3 scraps, 2 Spotter credits, a 38 m
// ring-out and an assist pay 2,720 t.
const EXAMPLE: ReportFacts = {
  ...NO_FACTS,
  mass_distance_kg_m: 3_120_000,
  eliminations: 3,
  spotter_credits: 2,
  ringout_heights_m: [38],
  assists: 1,
};

const fresh: PlayerState = {
  ledger: { tonnage_total: 0, grade: 1, weight_balance: null, cleared: false, has_cutter_serial: false },
  rope_marks: {},
  career: ZERO_CAREER,
  entries: [],
};

function human(account: string, team: number, facts: Partial<ReportFacts> = {}): ReportPlayer & { account_id: string } {
  return {
    account_id: account,
    team,
    is_bot: false,
    abandoned: false,
    facts: { ...NO_FACTS, ...facts },
  };
}

function report(players: ReportPlayer[], winner: number | null = 1, noContest = false): Report {
  return {
    match_id: "m-1",
    winner_team: winner,
    forfeit_team: null,
    forfeit_penalty_free: false,
    no_contest: noContest,
    players,
  };
}

Deno.test("the frontend's worked example pays 2,720 t, line by line", () => {
  const lines = tonnageLines(EXAMPLE, rules);
  assertEquals(lines.map((l) => [l.key, l.tonnage]), [
    ["mass_moved", 1560],
    ["scraps", 360],
    ["spotter_credits", 360],
    ["ringouts", 380],
    ["assists", 60],
  ]);
  const p = human("a", 1, EXAMPLE);
  const r = rewardPlayer(report([p]), p, "Rigger", fresh, rules);
  assertEquals([r.tonnage, r.credit], [2720, 272]);
});

Deno.test("a Spotter credit pays more than a scrap, and a mover out-earns a cutter", () => {
  assert(rules.tonnage.per_spotter_credit > rules.tonnage.per_scrap);
  const mover = tonnageLines({ ...NO_FACTS, mass_distance_kg_m: 2_000_000 }, rules);
  const cutter = tonnageLines({ ...NO_FACTS, eliminations: 3 }, rules);
  const sum = (ls: { tonnage: number }[]) => ls.reduce((s, l) => s + l.tonnage, 0);
  assert(sum(mover) > sum(cutter));
});

Deno.test("bots, leavers and deleted accounts earn nothing", () => {
  const players: ReportPlayer[] = [
    human("a", 1, EXAMPLE),
    { ...human("b", 1, EXAMPLE), abandoned: true },
    { ...human("x", 2, EXAMPLE), account_id: null, is_bot: true },
    { ...human("y", 2, EXAMPLE), account_id: null },
  ];
  assertEquals(earners(report(players)).map((p) => p.account_id), ["a"]);
});

Deno.test("winners are the winning team's, and a no-contest has none", () => {
  const a = human("a", 1);
  const b = human("b", 2);
  assertEquals(rewardPlayer(report([a, b], 1), a, "Rigger", fresh, rules).win, true);
  assertEquals(rewardPlayer(report([a, b], 1), b, "Rigger", fresh, rules).win, false);
  assertEquals(rewardPlayer(report([a, b], null, true), a, "Rigger", fresh, rules).win, false);
});

Deno.test("Grades follow the thresholds and never drop", () => {
  assertEquals(gradeFor(0, rules), 1);
  assertEquals(gradeFor(2499, rules), 1);
  assertEquals(gradeFor(2500, rules), 2);
  assertEquals(gradeFor(400_000, rules), 6);
  const p = human("a", 1);
  const high = { ...fresh, ledger: { ...fresh.ledger, grade: 4, has_cutter_serial: true } };
  assertEquals(rewardPlayer(report([p]), p, "Rigger", high, rules).ledger.grade, 4);
});

Deno.test("reaching Hand is the Cutter ceremony, once", () => {
  const p = human("a", 1, EXAMPLE);
  const near = { ...fresh, ledger: { ...fresh.ledger, tonnage_total: 1000 } };
  const first = rewardPlayer(report([p]), p, "Rigger", near, rules);
  assertEquals([first.ledger.grade, first.cutter_ceremony, first.ledger.has_cutter_serial], [2, true, true]);
  assert(first.unlocks.includes("cutter-ceremony"));
  const after = { ...near, ledger: first.ledger, entries: first.unlocks };
  const second = rewardPlayer(report([p]), p, "Rigger", after, rules);
  assertEquals(second.cutter_ceremony, false);
  assert(!second.unlocks.includes("cutter-ceremony"));
});

Deno.test("the Weight starts at the full lease, is paid down, and clears once", () => {
  const p = human("a", 1, EXAMPLE);
  const first = rewardPlayer(report([p]), p, "Rigger", fresh, rules);
  assertEquals([first.weight_before, first.ledger.weight_balance], [45_000, 45_000 - 2720]);
  const nearlyClear = { ...fresh, ledger: { ...fresh.ledger, weight_balance: 1000 } };
  const clearing = rewardPlayer(report([p]), p, "Rigger", nearlyClear, rules);
  assertEquals([clearing.ledger.weight_balance, clearing.newly_cleared, clearing.ledger.cleared], [0, true, true]);
  assert(clearing.unlocks.includes("clear"));
  const cleared = { ...nearlyClear, ledger: clearing.ledger, entries: clearing.unlocks };
  const later = rewardPlayer(report([p]), p, "Rigger", cleared, rules);
  assertEquals([later.ledger.weight_balance, later.newly_cleared], [0, false]);
});

Deno.test("Rope Marks count per rig and climb Tape, Band and Mark", () => {
  assertEquals([0, 1, 9, 10, 49, 50].map((n) => ropeTier(n, rules)), ["none", "tape", "tape", "band", "band", "mark"]);
  const p = human("a", 1, {
    rope_marks: [{ rig: "Gallows" }, { rig: "Gallows" }, { rig: "Unknown" }, { rig: "Kiss" }],
  });
  const state = { ...fresh, rope_marks: { Gallows: 8 } };
  assertEquals(rewardPlayer(report([p]), p, "Rigger", state, rules).rope_marks, [
    { rig: "Gallows", count: 10, tier_before: "tape", tier: "band" },
    { rig: "Kiss", count: 1, tier_before: "none", tier: "tape" },
  ]);
});

Deno.test("career entries unlock on career totals, once", () => {
  const p = human("a", 1, { eliminations: 1, counters: { rated_tons_moved: 40 } });
  const r = rewardPlayer(report([p]), p, "Rigger", fresh, rules);
  assertEquals(r.unlocks, ["first-scrap", "rated-load-moved"]);
  const nearHundred = { ...fresh, career: { ...ZERO_CAREER, scraps: 99 }, entries: ["first-scrap"] };
  assertEquals(rewardPlayer(report([p]), p, "Rigger", nearHundred, rules).unlocks, [
    "rated-load-moved",
    "hundred-scraps",
  ]);
});

Deno.test("facts over the ruleset's ceilings are reported", () => {
  const p = human("a", 1, { eliminations: 41, ringout_heights_m: [401] });
  assertEquals(factProblems(report([p]), rules), [
    "player 0: eliminations 41 over 40",
    "player 0: ringout_height_m 401 over 400",
  ]);
  assertEquals(factProblems(report([human("a", 1, EXAMPLE)]), rules), []);
});

Deno.test("malformed facts are named", () => {
  const base = { match_id: "m", winner_team: 1, players: [] as unknown[] };
  const player = { account_id: "7d1b2c3a-0000-4000-8000-000000000000", team: 1, facts: {} };
  assertEquals(parseReport({ ...base, players: [{ ...player, facts: { eliminations: -1 } }] }), {
    problem: "malformed eliminations",
  });
  assertEquals(parseReport({ ...base, players: [{ ...player, facts: { rope_marks: [{}] } }] }), {
    problem: "malformed rope_marks",
  });
  assertEquals(parseReport({ ...base, winner_team: "1" }), { problem: "malformed winner_team" });
  assert("report" in parseReport({ ...base, players: [player] }));
});

Deno.test("reaching Band on a rig writes its Ledger entry, once", () => {
  const p = human("a", 1, { rope_marks: [{ rig: "Anchor Drop" }, { rig: "Kiss" }] });
  const state = { ...fresh, rope_marks: { "Anchor Drop": 9, Kiss: 49 } };
  assertEquals(rewardPlayer(report([p]), p, "Rigger", state, rules).unlocks, ["rope-band-anchor-drop"]);
  const owned = { ...state, entries: ["rope-band-anchor-drop"] };
  assertEquals(rewardPlayer(report([p]), p, "Rigger", owned, rules).unlocks, []);
});

Deno.test("every Contract counter has a ceiling", () => {
  const p = human("a", 1, { counters: { rated_tons_moved: 1.5e308, new_counter: 1001, snaps_survived: 3 } });
  assertEquals(factProblems(report([p]), rules), [
    "player 0: counters.rated_tons_moved 1.5e+308 over 5000",
    "player 0: counters.new_counter 1001 over 1000",
  ]);
});

Deno.test("a huge ring-out list is reported, not a crash", () => {
  const p = human("a", 1, { ringout_heights_m: new Array(200_000).fill(1) });
  assertEquals(factProblems(report([p]), rules), ["player 0: ringouts 200000 over 40"]);
});

Deno.test("account ids are made canonical; counts are whole; facts are required", () => {
  const base = { match_id: "m", winner_team: 1 };
  const upper = "7D1B2C3A-0000-4000-8000-00000000000A";
  const parsed = parseReport({ ...base, players: [{ account_id: upper, team: 1, facts: {} }] });
  assert("report" in parsed);
  assertEquals(parsed.report.players[0].account_id, upper.toLowerCase());
  assertEquals(parseReport({ ...base, players: [{ account_id: "not-a-uuid", team: 1, facts: {} }] }), {
    problem: "malformed account_id",
  });
  assertEquals(parseReport({ ...base, players: [{ account_id: null, team: 1, facts: { eliminations: 2.5 } }] }), {
    problem: "malformed eliminations",
  });
  assertEquals(
    parseReport({ ...base, players: [{ account_id: null, team: 1, facts: { counters: { snaps_survived: 0.5 } } }] }),
    { problem: "malformed counters.snaps_survived" },
  );
  assert(
    "report" in
      parseReport({
        ...base,
        players: [{ account_id: null, team: 1, facts: { counters: { rated_tons_moved: 0.5 } } }],
      }),
  );
  assertEquals(parseReport({ ...base, players: [{ account_id: null, team: 1 }] }), { problem: "malformed facts" });
  assert("report" in parseReport({ ...base, players: [{ account_id: null, team: 1, is_bot: true }] }));
});

// Golden file (backend-architecture 25): the engine-free core's sample match report, rewarded
// for fresh players under ruleset 1. Regenerate with UPDATE_GOLDEN=1 only for a deliberate
// change, and review the diff.
Deno.test("golden: the KineticCore sample report under ruleset 1", async () => {
  const payload = JSON.parse(await Deno.readTextFile(new URL("../fixtures/sample-report.json", import.meta.url)));
  const parsed = parseReport(payload);
  assert("report" in parsed);
  assertEquals(factProblems(parsed.report, rules), []);
  // The roster's harness, which the pass reads from match_players.
  const roster = new Map<string, string>(
    payload.players.map((p: { account_id: string; harness: string }) => [p.account_id, p.harness]),
  );
  const rewards = earners(parsed.report).map((p) =>
    rewardPlayer(parsed.report, p, roster.get(p.account_id)!, fresh, rules)
  );
  const golden = new URL("../fixtures/sample-report.rewards.v1.json", import.meta.url);
  if (Deno.env.get("UPDATE_GOLDEN") === "1") {
    await Deno.writeTextFile(golden, JSON.stringify(rewards, null, 2) + "\n");
  }
  assertEquals(rewards, JSON.parse(await Deno.readTextFile(golden)));
});
