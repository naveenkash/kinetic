# Scrap debris kit

Light prop kit. Light tier, 1 to 9 kg per piece, twelve pieces; a heap is 40 to 80 pieces in a 3 to 5 m mound 60 to 120 cm tall. Appears in the Yard (the Floor, between the rail islands: every scrap heap is this kit), Ballast (the Hold, crushed-cargo drifts; the Bays, under the racks), Hangtown (the Low Hang, under the shanties, and the Upper Hang anchor points), and the Anchor (the Rim, in the contractor compound, and the Scaffold base after Scaffold Collapse adds its own debris).

## Role in play

Scrap heaps are the Scrapper's ammunition and everyone's landing pad. 05 §7.3: a scrap heap doubles a player's stagger, flip, and Stall thresholds (×2.0): stagger at 10 m/s, flip at 16, Stall at 24, so a 24 m/s impact or a 29 m fall onto a heap is survivable where 12 m/s onto steel is not. 03 Yard: scrap heaps "come apart into a Scrapper's favorite ammunition." Both are true at once. A heap is a safe place to land until someone has Hauled the pieces out of it, and then it is a bare floor with a decal for the rest of the round (05 §9.2: prop pools do not replenish within a round).

No single piece hurts anyone. The heaviest (9 kg) at the Jack cap is 270 N·s, under stagger. The heap works by numbers: a Scatter that Jacks six pieces at once lands six hits in a second, and a Scrapper who Hauls piece after piece on instant recall is a nuisance that makes a player turn their back on the real threat.

- Named rigs: the Scatter, in volume. The Clothesline, with debris kicked onto the floor under the line. The Pendulum, as the landing: Climbers aim for heaps.
- Scrapper: the heap is their magazine. Light props travel 30% faster on a Scrapper line; the 25 m/s cap holds.
- Rigger: Hangs debris on set lines as noise and clutter around the real Guillotine load.
- Breaker: wades through. A Breaker Hauled onto a heap at 20 m/s survives (threshold 24 m/s on a heap).
- Climber: lands on heaps after Pendulums and Slingshots. Removing a heap removes the Climber's safe landing, which is a legitimate strategy.
- Doubler: a Snapback with a 9 kg piece receives 1,500 N·s at a full 1,200 N snap, 167 m/s on paper, capped at the 50 m/s terminal velocity, so it arrives with 450 N·s, a stagger. No kit piece can Stall anyone, even off a snap. The Doubler uses heaps as a landing after their own snap launch (10 m/s backward is fine on steel; the stagger is the cost).

## The twelve pieces

All Light tier. Dimensions in cm. Two Dog sockets each; surface class in brackets. Materials from the shared library.

| # | Piece | Dimensions (L × W × H) | Mass | Materials | Sockets |
|---|---|---|---|---|---|
| 1 | Hull plate offcut | 60 × 45 × 1.2 | 9 kg | `MI_Steel_Bare` `#5C5F63`, torch-blued edges `#3B4A5E`, `MI_Meridian_Cyan` ghost paint one face | `SKT_Dog_Top_01` [Metal], `SKT_Dog_End_01` [Metal] |
| 2 | Angle iron stub | 50 × 5 × 5 | 4 kg | `MI_Steel_Rusted` `#7A3F1C` | `SKT_Dog_Top_01` [Metal], `SKT_Dog_End_01` [Metal] |
| 3 | Rebar scrap, bent | 90 × 12 × 12 (bent) | 3 kg | `MI_Steel_Rusted` `#A8562A` | `SKT_Dog_End_01` [Metal], `SKT_Dog_End_02` [Metal] |
| 4 | Pipe section | 40 × 11 × 11 (100 mm tube) | 5 kg | `MI_Steel_Painted_Worn` `#5B6670` | `SKT_Dog_Top_01` [Metal], `SKT_Dog_End_01` [Metal] |
| 5 | Gear, cast | 32 × 32 × 6 | 8 kg | `MI_Steel_Bare` with `MI_Chain_Oiled` film | `SKT_Dog_Top_01` [Metal], `SKT_Dog_Rim_01` [Metal] |
| 6 | Drive dog shoe fragment | 45 × 20 × 15 | 9 kg | `MI_Steel_Bare`, `MI_Weave` `#1A2430` sliver on one face | `SKT_Dog_Top_01` [Metal], `SKT_Dog_SideN_01` [Metal] |
| 7 | Grating offcut | 50 × 30 × 3 | 6 kg | `MI_Steel_Galvanized` `#9EA3A6` | `SKT_Dog_Top_01` [Metal], `SKT_Dog_End_01` [Metal] |
| 8 | Plywood shard | 55 × 35 × 1.8 | 2 kg | `MI_Plywood_Weathered` `#9C8564` | `SKT_Dog_Top_01` [Wood], `SKT_Dog_End_01` [Wood] |
| 9 | Timber block | 30 × 15 × 15 | 3 kg | `MI_Timber_Dark` `#4A3522` | `SKT_Dog_Top_01` [Wood], `SKT_Dog_SideN_01` [Wood] |
| 10 | Concrete rubble | 25 × 20 × 18 | 7 kg | `MI_Concrete_Stained` `#6F6C66`, one rebar stub | `SKT_Dog_Top_01` [Concrete], `SKT_Dog_SideN_01` [Concrete] |
| 11 | Tire, small | 45 × 45 × 15 | 6 kg | `MI_Rubber_Black` `#1E1F20` | `SKT_Dog_Top_01` [Fabric], `SKT_Dog_Rim_01` [Fabric] |
| 12 | Wire tangle | 40 × 40 × 25 | 1 kg | `MI_Chain_Oiled` and `MI_Steel_Rusted` mix | `SKT_Dog_Top_01` [Metal], `SKT_Dog_SideN_01` [Metal] |

Surface class drives Dog load rules: Mag Dogs hold full load on pieces 1 to 7 and 12; Spike Dogs on 8, 9, 10. Piece 11, rubber, uses the Fabric class as the plastics stand-in: Standard and Barb hold, Mag and Spike slide (**PROPOSED**).

## Appearance

A heap is a mound with hard edges sticking out of soft ones. Hull plate and grating lie flat on top; rebar and angle iron poke out at 30° to 60°; the tire and the wire tangle sit on the surface; rubble and timber fill the gaps. The heap's base is a `DECAL_Scrap_Stain` 1.5× the heap radius, oil-black in the Yard (`#2A2622`), algae-green in the Hold (`#3A4A3C`), warm dust in Hangtown (`#6E5A3C`), concrete-pale on the Rim (`#B8B2A4`).

Wear story: everything is cut, not broken. Torch edges on every steel piece, blued within 2 cm of the cut. Nothing is clean. Piece 6 is the only one with Weave visible, a 3 cm iridescent sliver, because a drive-dog shoe once gripped the Strand; it is the heap's one glint and players learn to see it. Piece 1 carries a quarter of a Meridian hexagon in cyan ghost paint in Ballast and a torched-off `HTR` in the Yard.

Under arena lighting: the heap is the darkest large mass on any floor. Specular on the galvanized grating and the oiled gear are the only highlights. It must never read as a volatile: no red, no yellow, no valves in the kit.

## Variants

| Variant | Pieces | Difference |
|---|---|---|
| `Heap_S` | 40 | 3 m across, 60 cm tall. ×2.0 surface volume. |
| `Heap_M` | 60 | 4 m, 90 cm. |
| `Heap_L` | 80 | 5 m, 120 cm. Yard trench islands. |
| `Drift_Hold` | 50 | Low, wet, pieces 1, 3, 7, 10, 12 weighted. Ballast Hold. Half-submerged. |
| `Collapse_Add` | 30 | Pieces 2, 7, 8, 9 plus Scaffold clamp fragments. Spawned by Scaffold Collapse. |
| `Loose` | 1 to 6 | Scattered singles on walkways and porches. No surface volume. |

## Physics

- Tier: Light, per piece. Masses in the table.
- Collision: pieces 1, 4, 7, 8, 9 are boxes at listed dimensions; 2 and 3 are capsules (radius 2.5 cm and 1 cm along a 3-segment bend); 5 and 11 are cylinders; 6 and 10 are 8-vertex convex hulls; 12 is a sphere radius 20 cm.
- Center of mass: geometric, except 6 (offset 5 cm toward the shoe face) and 10 (offset toward the rebar stub).
- Friction 0.7 steel, 0.65 wood, 0.8 concrete and rubber; restitution 0.1 to 0.3 (**PROPOSED**). Tire 0.5.
- Break-free: a piece on the heap surface is loose on floor, 0 N. A piece buried under others is "in a stack (friction)," 500 N, until the pieces above it are removed (**PROPOSED** heap rule: the pooled heap tracks a burial depth per piece; depth 0 is 0 N, depth 1 or more is 500 N). A Rigger (600 N), Doubler (800 N), or Breaker (1,000 N) can pull buried pieces; a Climber or Scrapper (400 N) takes surface pieces only. The Scrapper's magazine is the top layer.
- Haul acceleration with the far end static, heaviest piece (9 kg): 44 m/s² at 400 N, 67 at 600 N, 89 at 800 N, 111 at 1,000 N; distance to the 25 m/s cap 7.0 m, 4.7 m, 3.5 m, 2.8 m. Lightest piece (1 kg): 400 m/s² on the weakest line, at the cap in 0.8 m.
- Cut recoil: 150 N·s per 1,000 N to both ends. Cutting a Breaker's line on a 9 kg plate gives it 150 N·s, 17 m/s; on the 1 kg wire tangle, the recoil is capped by the 50 m/s terminal velocity. Pieces jump when lines on them are cut.
- Fracture: none. The pieces are already the fragments.

What a piece does to a player:

| Piece | Haul (25 m/s) | Jack (30 m/s) | Fall 7 m (11.7 m/s) | Result |
|---|---|---|---|---|
| Heaviest, 9 kg (1, 6) | 225 N·s | 270 N·s | 105 N·s | Nothing |
| Gear, 8 kg | 200 | 240 | 94 | Nothing |
| Lightest, 1 kg (12) | 25 | 30 | 12 | Nothing |

No piece reaches stagger (300 N·s) on a line, and the heaviest reaches 450 N·s off a full Snapback at terminal velocity. A Scatter of six pieces at the Jack cap into one player is six separate impacts under threshold and does not sum; 05 §7.1 counts each object hit on its own. The heap's physical effect on players is the surface rule only: stagger, flip, and Stall thresholds ×2.0 while the player lands on the heap's surface volume.

What the heap does for a player: a Climber falling 20 m (19.8 m/s) onto steel is scrapped; onto a heap (Stall at 24 m/s, flip at 16) they flip, land, and live. A player Hauled into a heap at 25 m/s is 1 m/s over the doubled threshold and is scrapped. Heaps make falls safe and Hauls almost safe.

## Interactions

- Haul to a piece: it comes to you. Nothing else.
- Jack from a piece: the Kick (the Jack's 0.3 s capacitor discharge, separate from the motor pull T) drives you apart to the class launch speed V, split by inverse mass. A 90 kg Climber (V 18 m/s) sends a 9 kg plate off at 16.4 m/s and takes 1.6 m/s. Then T pushes it at 44 to 111 m/s² by class to the 30 m/s cap (a Scrapper's 30% bonus, ×1.69 force and ×1.3 arrival speed, applies to this push and to the Haul, never to V; 05 §6.1). Nothing moves but the piece. The Kick never fires with a Dog in a Catch Plate: a Jack from a piece to an enemy is the T push alone, a 9 kg plate leaving from rest at 44 m/s² on a 400 N line and reaching the cap in 0.68 s over 10.2 m, while the enemy skids back at T over their mass.
- Hang from a piece: break-free is tested against active pull only (Haul, Jack, and the Kick), so a buried piece's 500 N value does not apply to a hanging player. Physically, loose scrap does not bear a harness: a buried piece pulls out of the heap under a hanging player and comes along as dead weight, 1 to 9 kg (**PROPOSED** physical behaviour); a surface piece is loose and comes along at once (canon for break-free 0). A heap anchors nothing.
- Object to object: a piece Hauled into a propane cylinder at 25 m/s is over 8 m/s and triggers it; the cheapest Kiss in the Yard. Pieces do nothing to Medium or Heavy objects.
- Kinematic: a Hauler scatters any heap in its trench (03 does not put heaps in trenches; the `Loose` variant may be). The Gantry magnet lifts steel pieces within 1 m of its load and drops them on the Press: pieces 1 to 7 and 12 (**PROPOSED**).
- Water: pieces 8, 9, 11 float; the rest sink. The `Drift_Hold` heap's floaters ride the tilt flow and re-collect at the nose each cycle.
- Updraft and Pulse: force volumes act on players and loose objects (break-free 0) at a_ref × 100 / m with the factor capped at 2 per body, speeds capped by the 50 m/s hard limit. Every kit piece is under 50 kg, so every surface piece feels 2 times the authored acceleration (100 / m would be 100 for the 1 kg wire tangle and 11 for a 9 kg plate; the cap flattens that). Surface pieces fly as one layer. Buried pieces (500 N) are unaffected until freed, so the Breath lifts the top layer off a Rim heap and leaves the body of the heap in place; a piece hung on a player's Hang line is a free body and swings in the gust. The Pulse (0.4 s outward, then a 0.6 s inhale toward the Stump, every 75 s from 0:55) throws loose pieces out and draws them back; Collapse debris at the Scaffold base migrates toward the Stump and the Well over a match, and the heap volumes shrink from the top down.
- Breathing tilt: surface pieces slide at 6° on steel; buried pieces hold. A Hold drift reshapes toward the nose each cycle and the ×2.0 volume moves with its centroid.
- Chain reactions: heaps end chains. A rolling barrel stops in a heap; a Hauled player stops on one; a Scatter's pieces stop in the next heap over.

## Pooling and the surface volume (the Yard engine note)

03 Yard: scrap heaps are "pooled small-prop clusters, budgeted per sector." The rule:

- Each arena sector has a pool of 120 debris pieces (**PROPOSED**). A heap draws from the sector pool on load. 05 §9.2 (canon): scrap heaps and other prop pools do not replenish within a round; what is thrown stays thrown until reset. A piece Hauled out of a heap stays where it comes to rest (a Light prop under RC-L, asleep after 3 s) and nothing respawns on the heap's surface until the round resets.
- The heap's surface volume (the ×2.0 modifier on stagger, flip, and Stall) is a box volume sized to the heap's authored footprint. It shrinks as pieces leave: when fewer than 40% of the heap's pieces remain inside the footprint, the volume is removed and the floor is bare steel (×1.0) with the stain decal (**PROPOSED**). A heap can therefore be dismantled, and a Climber's landing taken away, by a crew that spends twenty lines on it.
- Round reset restores all heaps.

## States

| State | Visual | Physics |
|---|---|---|
| Intact | Full mound | Surface volume active, ×2.0 on stagger, flip, Stall |
| Dented | Crater where a player or load landed; pieces displaced within the footprint | No change; volume active |
| Broken | Under 40% of pieces in the footprint | Volume removed; ×1.0 floor; stain decal |
| Destroyed | Stain only | Pieces stay where they landed (05 §9.2, no replenish); the heap respawns only on round reset |

## VFX and SFX hooks

- Impact per piece on steel, concrete, wood, water: `SFX_Scrap_<Class>_<Surface>` with classes `Plate`, `Bar`, `Pipe`, `Cast`, `Grate`, `Wood`, `Rubble`, `Tire`, `Wire`.
- Player landing on a heap: `SFX_Land_Scrap` (a settle of twenty pieces over 0.6 s), `NS_Dust_Scrap` (dark dust and three flying small pieces as particles).
- Heap volume removed: `SFX_Scrap_Settle_Last` and the stain decal gains a 0.5 s dust puff. No fracture; the pieces are the fragments.
- Buried piece break-free: `SFX_Scrap_Lurch` (a cascade of three impacts as neighbors shift), `NS_Dust_S`.
- Emissive: piece 6's Weave sliver, `MI_Weave`, emissive 0; the read is specular and iridescence only. Nothing in the kit glows.

## Technical

- LOD0 per piece: 1 at 400 tris, 2 at 120, 3 at 200, 4 at 160, 5 at 600, 6 at 500, 7 at 300 (masked card for the grid), 8 at 60, 9 at 36, 10 at 240, 11 at 400, 12 at 300. All pieces LOD1 at half, LOD2 at a quarter, LOD3 at a 12-tri hull. The kit shares one 1K atlas set (BC, N, ORM); the heap stain is a separate 1K decal.
- Pivot: bottom center of each piece's resting footprint. Piece 11 (tire) pivots at the axle center since it rolls.
- Dog sockets: two per piece as listed. Surface class per socket as listed.
- Chaos: not geometry collections. Each piece is a static mesh with a physics asset, spawned by `BP_ScrapHeap` from the sector pool using Chaos rigid-body instancing. The heap volume is a `PhysicsVolume` subclass `APV_ScrapSurface` with `StallMultiplier = 2.0`, applied to the stagger, flip, and Stall thresholds alike. Burial depth is computed once at spawn from overlap counts above each piece, and recomputed when a piece leaves. Anchor field `FS_Anchor_ScrapHeap` holds buried pieces at 500 N.
- Replication: RC-L. The heap's piece list and volume state replicate; piece transforms replicate only while awake. The pool does not refill within a round (05 §9.2); the only pool write is the round reset.

## Reference search terms

Scrapyard steel offcut pile torch cut edges; demolition debris heap rebar concrete plywood; shipbreaking yard scrap mound; hull plate offcut blued torch edge; cast gear rusted on scrap pile; galvanized grating offcut; small tire on junk pile; wire tangle scrap; scrap pile oil stain concrete floor.
