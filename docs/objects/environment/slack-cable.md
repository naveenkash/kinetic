# Slack Cable

One-line: environment, cuttable constraint, 24 mm wire rope or 30 mm polyester rope, 2 to 12 m lengths, orange tape at both ends, Hangtown only: every hanging house has two, every swing bridge has two (one per end hitch), and the market platform's approach decks have one each. About 70 per round.

## Role in play

The slack cable is the one place in the game where the Cutter is a demolition tool (03, 04 open question 4). Any Cutter cuts one in 0.6 s of continuous contact (05 section 4.5). Cut a house's slack cable and the house drops a tier or into the Well. Cut a swing bridge's near end and it swings. The orange tape and the visible fray are the whole readability contract: if it is orange, it cuts; if it is not, it does not.

| Harness | Relationship |
|---|---|
| Climber | Cuts from a perch on the cable above, reaching down. A Climber's 0.20 s warm-up plus the 0.6 s hold is 0.8 s of standing still, which Climbers hate. |
| Scrapper | The fastest hold start (0.15 s warm-up). Cut-Loose and Eviction are theirs. |
| Rigger | Hangs a Clothesline across the approach to a slack hitch so the cutter has to crouch to reach it. |
| Doubler | Cuts with one hand while a line holds an enemy on the bridge with the other. |
| Breaker | Slowest warm-up (0.30 s). Can also skip the Cutter: a house attaches at a rated 1,000 N (CHANGELOG v0.2.1), so a Breaker's 1,000 N Haul tears a house's slack hitch; a bridge hitch is 800 N (chained). |

## Appearance

Silhouette: a short rope from a house frame or bridge bearer up to a hanger saddle on a load cable or a main hanger, with a visible belly of slack (it is a secondary, not the load path), orange tape wrapped 0.3 m at each end, and a frayed section near the lower hitch where strands have sprung.

| Part | Material | Base color | Roughness | Metallic |
|---|---|---|---|---|
| Rope (steel) | 24 mm wire rope, rusted | #7A3B1E over #6E7378 | 0.7 | 0.6 |
| Rope (fiber) | 30 mm polyester, sun-bleached | #D9CBB0 | 0.9 | 0.0 |
| Tape | orange with a white reflective thread | #FF8A00 / #F5F5F0 | 0.6 | 0.0 |
| Fray | sprung strands, bright | #B3B6B8 | 0.5 | 0.8 |
| Hitch | shackle or knot on an eye | #5A5E63 | 0.6 | 0.8 |

Weathering: the slack cables are the residents' own work, mismatched and old. The Office's prep crew applies the orange tape before every match and the tape is clean against the dirty rope, which makes it read. The fray is authored on the lower 0.5 m so the cut point is where a standing player can reach it (1.0 to 1.6 m above the deck). Markings: a prep-crew paint dot (white) on the hitch, numbered; HTR's orange survey ring on the load cable above matches nothing on the slack cable, deliberately, so HTR orange and tape orange are never on the same object. Lighting: the reflective thread catches lantern and line light; within 5 m of a player the tape's emissive rises 20% (the set-line rule from 05 section 11 applied to cuttables, **PROPOSED**).

Read by tier: from the Upper Hang, orange flecks on the houses below. From the Mid Hang, a hitch at shoulder height beside every doorway and bridge end. From the Low Hang, orange above you.

## Variants

1. **House secondary (steel).** 24 mm, 3 to 6 m, with the taped cuttable segment at least 1.5 m above the hanger eye on single-cable small houses (the cutter stands on the roof), from the house's upper frame corner to the hanger. Two per house. Cut one: the house drops to hang from the other at a 25 degree list (authored pose). Cut both: the house falls a tier or to the Well on its pre-authored path.
2. **Bridge hitch (fiber).** 30 mm, 1.5 m, at each end of a swing bridge. Cut the near end: the Cut-Loose.
3. **Approach deck hitch.** As bridge hitch, on the market's four approach decks.
4. **Decoy (dressing).** Orange tape on a rope that holds nothing: a laundry line. Cutting it drops laundry (Light, x2.0 surface). Four per round, so the tape is a promise of cuttability, not a promise of a kill.

## Physics

- Constraint, not a body. It holds its structure in the `Anchored` state (03 engine notes: houses held in an anchored state until their cable is cut or their strain threshold is hit).
- Cut: 0.6 s of continuous Cutter contact (a hold). Interrupting the hold resets it (**PROPOSED**; no partial-cut memory, so a cutter staggered at 0.5 s starts over). The Scrapper's class bonus applies to warm-up, not to the hold.
- Break-free by line: house secondaries 1,000 N (a hanging house attaches at rated strength; only a Breaker, a Doubler at overspin, or a cut drops one), bridge and approach-deck hitches 800 N (chained class, **PROPOSED**). The tear comes with the standard lurch and a 0.2 s tell; below threshold the hitch creaks. A torn house hitch follows the README's authored-consequence spec (commit 0.6 s, groan 1.0 s) because the result is a falling house.
- Strain: a house swung into another (over its fracture threshold) breaks the house, not the slack cable; a house whose load cable fails (Cable Day) keeps its slack cables and swings from them, so they become the house's only hold and are now the quickest Eviction on the map.
- Not solid. Players and objects pass through a slack cable. Dogs seat in the hitch hardware (`Kinetic_WorldStatic` shackle), not in the rope.

## Interactions

- **Cutter hold** 0.6 s: the structure responds per its doc. `SFX_SlackCable_Cut_Hold` builds over the hold; `SFX_SlackCable_Part` at release. Marshal: "Cable cut in the Mid Hang" (first cut per round only).
- **Haul** on a house hitch (Breaker, or Doubler overspin): tears it after the commit and groan. The Breaker's Eviction without a knife. On a bridge hitch, a Doubler's single line at 800 N also tears it.
- **Hang** to a slack cable's hitch: a Dog anchor on the house frame, effectively. Nothing special.
- Chain: cutting a bridge's near end while an enemy is Hung to the bridge's far rail: they swing with it into the house below (3,500 N s large-house fracture from a 420 kg bridge at 15 m/s: the wall goes, the enemy goes through it).

## States

| State | Visual |
|---|---|
| Intact | Belly of slack, tape clean, fray visible. |
| Stressed | Under a Breaker's line: belly gone, hitch creaks, tape stretches. |
| Failing | Strands part one by one through the 0.6 s hold; sparks under the Cutter on steel, smoke on fiber. |
| Failed | A 0.5 m tail hanging from the saddle above, tape on it; the lower hitch swinging on the house. |

## VFX and SFX hooks

`SFX_SlackCable_Cut_Hold` (0.6 s build, universal across cables), `SFX_SlackCable_Part`, `SFX_Hitch_Creak`, `SFX_Hitch_Tear`; `VFX_Cutter_Sparks_Steel`, `VFX_Cutter_Smoke_Fiber`, `VFX_Tape_Proximity_Glow`.

## Technical

- Spline mesh, 1 m segments, with a physics-driven belly (a two-bone sag, not cloth). LOD to a ribbon at 30 m.
- Textures: `T_WireRope_Tile` 1K, `T_Fiber_Rope_Tile` 1K, `T_Tape_Orange` 512 with a reflective-thread mask.
- Pivot at the lower hitch. Sockets: `Hitch_Low` (Dog seat), `Hitch_High`, `CutZone` (a 0.5 m capsule, the only region where the hold registers), `Tape_Low`, `Tape_High`.
- Implementation: `Kinetic_Cuttable` overlap capsule on `CutZone`; the Cutter's hold trace checks it every frame, accumulates 0.6 s, fires the owning structure's `OnSlackCut(index)`. Replicated as a server event; the hold progress is predicted client-side with a cancel on mismatch. Line break-free uses the structure's constraint strength (800 N) on the hitch socket.
- Collision: rope `Kinetic_NoCollide`; hitch hardware `Kinetic_WorldStatic`.

## Reference search terms

frayed wire rope sprung strands, orange safety tape on rope, shackle hitch rope eye, rope secondary safety line scaffold, polyester mooring rope bleached.
