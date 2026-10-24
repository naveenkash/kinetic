# Load Cable

One-line: environment, static world (infinite mass) except during authored failures, 90 mm galvanized wire rope, up to 400 m span, Hangtown: eight main load cables between the Collar rims carrying the Upper Hang walkways, the house hangers, and the market platform's eight hanger ropes; also the Lift's hoist rope and the Gantry's hoist falls by material family.

## Role in play

Load cables are the Hangtown skeleton. They cannot be cut (no orange tape, no Cutter hold) and no line moves them. They fail two ways only: Cable Day (the scripted event at 4:00 remaining in Claim or 1:15 in Scrap) and an anchor pin pull (see `rated/anchor-pin.md`). Between failures they are the longest Dog anchors in the game, the Climber's road along the top, and the sightline that tells everyone where the walkways run.

| Harness | Relationship |
|---|---|
| Climber | Perches on the cable (1.5 s), re-dogs, perches again: the Climber crosses the ravine on the cable itself. |
| Scrapper | Dog anchor for Overboard from any walkway. |
| Rigger | Hang anchor for the Gallows over the Mid Hang: a 40 m line from a cable to a plate is a hoist with the whole ravine under it. |
| Doubler | Anchors one line to the cable above and one to an enemy; the snap launches the Doubler along the cable's direction. |
| Breaker | Cannot reach it except at the rims and the headframe. Ends it with the anchor pin. |

## Appearance

Silhouette: a catenary of heavy rope, sagging 14 m over a 400 m span, with hanger bars dropping every 2 m to the walkway and heavier hangers every 20 m to houses. Tyre-rubber wraps at every hanger. Bird spikes the residents added and birds ignore.

| Part | Material | Base color | Roughness | Metallic |
|---|---|---|---|---|
| Rope | galvanized wire rope, 6x36, greased | #6E7378 | 0.4 | 0.9 |
| Grease | black, thick at the low point | #1A1A1C | 0.3 | 0.2 |
| Hanger saddles | cast steel, U-bolted | #5A5E63 | 0.6 | 0.8 |
| Rubber wraps | tyre tread | #2A2C2E | 0.9 | 0.0 |
| Sockets | poured zinc, bright | #A8ADB1 | 0.45 | 0.9 |

Weathering: the rope is sixty years old at the rims (first strung L.4) and spliced in the middle with a Guild long splice from L.31, which is visibly thicker for 6 m. Grease has crept to the low point and dripped a black line down the ravine wall below. Markings: a stamped brass tag at each rim end (`CABLE 3  90 mm  L.4  RE-TENSIONED L.31  LG`), Office inspection paint rings every 50 m (white), and the residents' own distance marks in chalk. HTR's orange survey rings on the cable due for the cut. Lighting: none on the cable. The walkway strips beneath it light its underside; from below the cable reads as a dark line between lit hangers.

Read by tier: from the rims, eight dark arcs across the gap. From the Mid Hang, a thick line overhead with hangers dropping from it. From the Low Hang, almost invisible; the walkway strips mark where it is.

## Variants

1. **Walkway cable.** Carries a walkway run and two to four houses. Six of eight.
2. **Market cable.** Carries the market platform's hangers and two swing-bridge approach decks. Two of eight; one is the Cable Day casualty.
3. **Dead cable (seasonal cut district).** Hanging in a catenary from one rim into the ravine, socket gone from the other.
4. **Lift and Gantry ropes.** Same material family at 36 mm and 28 mm; kinematic, not cuttable, not world-static (they move with their machines).

## Physics

- Hanging houses attach to a load cable's hangers at a rated 1,000 N (05 section 6.2): only a Breaker, a Doubler at overspin, or a cut slack cable drops one. A Hang line to a house never unseats it; lines cannot swing a hanging house dangerously on their own (a Breaker's 1,000 N deflects a 1,200 kg house to 0.76 m/s); swings come from gravity.
- Static world while intact. Infinite mass. Dogs seat anywhere along it (a 0.08 m Dog on a 90 mm rope: the seat hint is generous, 0.3 m radius). Lines to it are lines to the world.
- Perch surface: valid for the Climber's 1.5 s hold.
- Failure (Cable Day or pin pull): the cable becomes a spline mesh blending to its fallen catenary over 5.4 s from the failed end. It carries no simulation; its hangers release on the per-section timer and the walkway and houses respond as their own docs describe. A failing cable's socket whip at the rim is the one lethal moment of the cable itself: 300 kg of socket and rope at 20 m/s along an authored 8 m arc, 6,000 N s, scrapped (see `rated/anchor-pin.md`).
- Sway: the cables are the Sway's source. Their anchor proxies (not the cables) are animated: a 1.2 m lateral drift at mid-span on an 11 s period, phase offset 2 s per cable so neighboring cables are never in step (**PROPOSED**). Everything hung from a cable inherits its cable's Sway; a Hang line between two houses on different cables therefore changes length by up to 2.4 m over 5 s, which is 03's "different length every three seconds."
- Not cuttable: a Cutter hold on a load cable plays `SFX_Cutter_NoBite` and a sparks VFX and does nothing. The Visibility Clause fiction: Weave-cored rope, which the Cutter cannot melt. The brass tag says `WEAVE CORE` so the reason is legible.

## Interactions

- **Haul** self to the cable from a walkway: a grapple up 1.6 m to perch (Climber) or to hang (everyone else; hoisted, cannot move).
- **Jack** from the cable: a Slingshot down into the Mid Hang at V, or along the cable.
- **Hang** from the cable to an enemy on the walkway below, then hold Haul: the Gallows hoists by Reel, the rod shortening at 1 m/s under the class Hang load (05 section 3.2; only a Breaker's Haul lifts a Climber outright). A 1.6 m hoist lifts them off the walkway and leaves them hanging over the ravine until they cut, at which point they drop 1.6 m onto the walkway. Harmless. Hang from the cable to an enemy on a house 14 m below and Reel: 14 s of hoist, and the cut is a 14 m fall. That is the rig. A hanging player can Reel themselves up the same way.
- Chain: a failed cable's houses swing on their slack cables; those are now cuttable at both ends.

## States

| State | Visual |
|---|---|
| Intact | Even catenary, hangers vertical. |
| Stressed | Cable Day siren or pin groan: the whole cable sings a rising tone along its length; hanger saddles tick; walkway strips flicker from the failing end. |
| Failing | Socket releases, cable falls from one end over 5.4 s. |
| Failed | Catenary into the ravine from the surviving rim, hangers bare. |

## VFX and SFX hooks

`SFX_Cable_Sing_Spatial` (a sound emitted along the spline, not from a point), `SFX_Cable_Saddle_Tick`, `SFX_Cutter_NoBite`, `SFX_Cable_Fall_Whoosh`; `VFX_Cutter_Sparks_NoCut`, `VFX_Grease_Drip`, `VFX_Cable_Fall` (spline blend).

## Technical

- Spline mesh per cable, 90 mm radius, 2 m segments; LOD to a camera-facing ribbon beyond 60 m, to a line beyond 200 m.
- Textures: `T_WireRope_Tile` 1K (tiling along U), `T_Cable_Saddle` 1K.
- Pivot at the west socket. Sockets: `Socket_W`, `Socket_E`, `Hanger_<n>` every 2 m, `House_<n>` every 20 m, `Splice`.
- Implementation: spline with two control-point sets (intact, fallen); failure blends between them over 5.4 s from the failed end with an authored ease. Sway animates the anchor proxy actors that hangers attach to, not the cable mesh, which is cheaper and keeps the walkway chain deterministic. Collision: `Kinetic_WorldStatic` capsule chain while intact, `Kinetic_NoCollide` while falling, `Kinetic_WorldStatic` again when fallen (the dead catenary is a climbable Dog anchor into the ravine for anyone who wants to try).

## Reference search terms

suspension bridge main cable saddle, 6x36 wire rope close up, poured zinc socket, catenary cable across gorge, long splice wire rope, cable car track rope grease.
