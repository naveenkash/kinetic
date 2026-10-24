// Loads the built WebAssembly module in Node and plays a minute of a match with the player
// standing still: the module loads its tables, steps, and returns well-formed snapshots.
import createKinetic from "./public/kinetic.js";

const mod = await createKinetic();
const game = new mod.Game(42, 2, true);
const arena = JSON.parse(game.arena());
if (!Array.isArray(arena.solids) || arena.solids.length < 10) throw new Error("arena has no solids");
let snap;
for (let i = 0; i < 60 * 60; i++) {
  game.step(0, 0, 0, 0, 0, 0);
  if (i % 60 === 0) snap = JSON.parse(game.snapshot());
}
snap = JSON.parse(game.snapshot());
if (snap.players.length !== 6) throw new Error(`expected 6 players, got ${snap.players.length}`);
if (!["reset", "drop", "live", "sudden_death", "round_end", "match_over"].includes(snap.phase)) throw new Error(`bad phase ${snap.phase}`);
console.log(`smoke: ok, t=${snap.t.toFixed(1)} s, phase ${snap.phase}, ${snap.lines.length} lines, feed: ${snap.feed.at(-1)}`);
game.delete();
