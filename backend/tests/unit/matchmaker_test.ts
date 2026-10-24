import { assert, assertEquals, assertThrows } from "jsr:@std/assert@1.0.13";
import { formMatches, type Member, type Playlist, type Ticket } from "../../worker/matchmaker.ts";

const NOW = new Date("2026-10-24T12:00:00Z");

const scrap: Playlist = {
  playlist_id: "scrap_3v3_casual",
  team_size: 3,
  teams: 2,
  regions: ["na-east", "eu-west", "local"],
  bot_fill_pools: ["apprentice"],
  bot_fill_after_s: 60,
};

const freeScrap: Playlist = { ...scrap, playlist_id: "free_scrap_8", team_size: 1, teams: 8, bot_fill_pools: [] };

let seq = 0;
function ticket(
  size: number,
  opts: { pool?: string; waited?: number; pings?: Record<string, number>; build?: string; playlist?: string } = {},
): Ticket {
  seq++;
  const members: Member[] = Array.from({ length: size }, (_, i) => ({
    account_id: `acct-${seq}-${i}`,
    harness: "Rigger",
    face: null,
    grade: 2,
  }));
  return {
    ticket_id: `t-${String(seq).padStart(4, "0")}`,
    crew_id: `crew-${seq}`,
    playlist_id: opts.playlist ?? scrap.playlist_id,
    grade_pool: opts.pool ?? "hand",
    build: opts.build ?? "0.1.0",
    members,
    pings_ms: opts.pings ?? { "na-east": 30 },
    created_at: new Date(NOW.getTime() - (opts.waited ?? 0) * 1000),
  };
}

Deno.test("two full stacks make one match with three slots per team", () => {
  const a = ticket(3);
  const b = ticket(3);
  const [m, ...rest] = formMatches([a, b], scrap, NOW);
  assertEquals(rest.length, 0);
  assertEquals(m.slots.length, 6);
  assertEquals(m.slots.filter((s) => s.team === 1).map((s) => s.slot), [1, 2, 3]);
  assertEquals(m.slots.filter((s) => s.team === 2).map((s) => s.slot), [4, 5, 6]);
  assert(m.slots.every((s) => !s.is_bot));
  assertEquals(new Set(m.ticket_ids), new Set([a.ticket_id, b.ticket_id]));
});

Deno.test("a stack meets an assembled team only after waiting 60 s", () => {
  const fresh = [ticket(3, { waited: 10 }), ticket(2, { waited: 10 }), ticket(1, { waited: 10 })];
  assertEquals(formMatches(fresh, scrap, NOW).length, 0);
  const waited = [ticket(3, { waited: 61 }), ticket(2, { waited: 10 }), ticket(1, { waited: 10 })];
  assertEquals(formMatches(waited, scrap, NOW).length, 1);
});

Deno.test("six solos form two assembled teams", () => {
  const solos = Array.from({ length: 6 }, () => ticket(1));
  const [m] = formMatches(solos, scrap, NOW);
  assertEquals(m.slots.filter((s) => !s.is_bot).length, 6);
});

Deno.test("the 150 ms cap never widens and regions must be shared", () => {
  assertEquals(formMatches([ticket(3, { pings: { "na-east": 151 } }), ticket(3)], scrap, NOW).length, 0);
  assertEquals(
    formMatches([ticket(3, { pings: { "eu-west": 20 } }), ticket(3, { pings: { "na-east": 20 } })], scrap, NOW).length,
    0,
  );
});

Deno.test("the region minimizes the highest ping", () => {
  const a = ticket(3, { pings: { "na-east": 20, "eu-west": 90 } });
  const b = ticket(3, { pings: { "na-east": 140, "eu-west": 80 } });
  assertEquals(formMatches([a, b], scrap, NOW)[0].region, "eu-west");
});

Deno.test("Grade pools widen to adjacent after 90 s and to all after 180 s", () => {
  assertEquals(formMatches([ticket(3, { pool: "hand" }), ticket(3, { pool: "journeyman" })], scrap, NOW).length, 0);
  assertEquals(
    formMatches([ticket(3, { pool: "hand", waited: 91 }), ticket(3, { pool: "journeyman" })], scrap, NOW).length,
    1,
  );
  assertEquals(
    formMatches([ticket(3, { pool: "apprentice", waited: 91 }), ticket(3, { pool: "journeyman" })], scrap, NOW)
      .length,
    0,
  );
  assertEquals(
    formMatches([ticket(3, { pool: "apprentice", waited: 181 }), ticket(3, { pool: "journeyman" })], scrap, NOW)
      .length,
    1,
  );
});

Deno.test("Rated never mixes with casual pools", () => {
  const matches = formMatches(
    [ticket(3, { pool: "rated", waited: 999 }), ticket(3, { pool: "hand", waited: 999 })],
    scrap,
    NOW,
  );
  assertEquals(matches.length, 0);
});

Deno.test("bots fill apprentice matches after the wait, with a human on every team", () => {
  const before = [ticket(1, { pool: "apprentice", waited: 30 }), ticket(1, { pool: "apprentice", waited: 30 })];
  assertEquals(formMatches(before, scrap, NOW).length, 0);

  const after = [ticket(1, { pool: "apprentice", waited: 61 }), ticket(1, { pool: "apprentice", waited: 61 })];
  const [m] = formMatches(after, scrap, NOW);
  assertEquals(m.slots.length, 6);
  assertEquals(m.slots.filter((s) => s.is_bot).length, 4);
  for (const team of [1, 2]) {
    assert(m.slots.some((s) => s.team === team && !s.is_bot), `team ${team} has a human`);
  }
});

Deno.test("no bot fill for a lone crew or outside the bot pools", () => {
  assertEquals(formMatches([ticket(2, { pool: "apprentice", waited: 999 })], scrap, NOW).length, 0);
  const hand = [ticket(1, { pool: "hand", waited: 999 }), ticket(1, { pool: "hand", waited: 999 })];
  assertEquals(formMatches(hand, scrap, NOW).length, 0);
});

Deno.test("Free Scrap needs exactly eight players, one team each", () => {
  const seven = Array.from({ length: 7 }, () => ticket(1, { playlist: freeScrap.playlist_id }));
  assertEquals(formMatches(seven, freeScrap, NOW).length, 0);
  const eight = [
    ticket(2, { playlist: freeScrap.playlist_id }),
    ...Array.from({ length: 6 }, () => ticket(1, { playlist: freeScrap.playlist_id })),
  ];
  const [m] = formMatches(eight, freeScrap, NOW);
  assertEquals(m.slots.map((s) => s.team), [1, 2, 3, 4, 5, 6, 7, 8]);
});

Deno.test("every ticket is used at most once", () => {
  const tickets = Array.from({ length: 12 }, () => ticket(1));
  const matches = formMatches(tickets, scrap, NOW);
  assertEquals(matches.length, 2);
  const used = matches.flatMap((m) => m.ticket_ids);
  assertEquals(new Set(used).size, used.length);
});

Deno.test("tickets from different builds are rejected", () => {
  assertThrows(() => formMatches([ticket(3, { build: "0.1.0" }), ticket(3, { build: "0.2.0" })], scrap, NOW));
});

Deno.test("a Free Scrap player nobody can reach does not block the queue", () => {
  const isolated = ticket(1, { playlist: freeScrap.playlist_id, waited: 300, pings: { "eu-west": 20 } });
  const others = Array.from({ length: 8 }, () => ticket(1, { playlist: freeScrap.playlist_id }));
  const matches = formMatches([isolated, ...others], freeScrap, NOW);
  assertEquals(matches.length, 1);
  assert(!matches[0].ticket_ids.includes(isolated.ticket_id));
});

Deno.test("an isolated apprentice does not block bot fill for others", () => {
  const isolated = ticket(1, { pool: "apprentice", waited: 300, pings: { "eu-west": 20 } });
  const a = ticket(1, { pool: "apprentice", waited: 61 });
  const b = ticket(1, { pool: "apprentice", waited: 61 });
  const matches = formMatches([isolated, a, b], scrap, NOW);
  assertEquals(matches.length, 1);
  assertEquals(new Set(matches[0].ticket_ids), new Set([a.ticket_id, b.ticket_id]));
});

Deno.test("a player in two tickets is only ever matched once", () => {
  const older = ticket(3, { waited: 30 });
  const base = ticket(3, { waited: 10 });
  const stale = { ...base, members: [older.members[0], ...base.members.slice(1)] };
  const opponent = ticket(3, { waited: 20 });
  const matches = formMatches([older, stale, opponent], scrap, NOW);
  assertEquals(matches.length, 1);
  const accounts = matches[0].slots.map((s) => s.account_id);
  assertEquals(new Set(accounts).size, accounts.length);
  assert(!matches[0].ticket_ids.includes(stale.ticket_id));
});
