# Brick pallet

Medium prop. Mass tier Medium, 290 kg. 120 × 80 × 70 cm (a 120 × 80 × 14 cm timber pallet carrying 120 clay bricks at 2.3 kg each in six courses, shrink-wrapped and strapped). Appears in Hangtown (the Upper Hang, at both Collar rim landings where residents stage building material; the Mid Hang, one per house under construction), and the Anchor (the Rim, beside the Office gallery's half-built blast wall).

## Role in play

A half pallet of brick is a dense, dumb Medium with a trick: it fractures into 120 Light bricks. Intact, it is a Stall at walking speed and the heaviest thing a Scrapper line can start from a loose placement. Broken, it is a brick pile a Scrapper can throw by the handful and a hard, lumpy floor.

| Rig | Use |
|---|---|
| The Guillotine | Hang it over a swing bridge landing (2.8 kN, within every class Hang load), Reel it up at 1.0 m/s, then recall or cut your own set line. From 4 m it lands at 8.9 m/s, 2,581 N·s, a Stall. |
| The Avalanche | Pallets stack two high at the rim landings. Haul the bottom one at or above 500 N: Rigger, Doubler, Breaker. |
| The Anchor Drop | At a rim landing's edge, it is the heaviest unrated object on the Upper Hang. |
| The Scatter | Break it on a Jack. Bricks travel fast but weigh 2.3 kg. Harmless, blinding. |

- **Scrapper:** 400 N on 290 kg is 1.4 m/s²; 7.4 m/s over a 20 m line, a Stall. No bonus on the pallet (over 100 kg). The bricks it drops are Light and get the 30% bonus; a 2.3 kg brick at the 25 m/s cap is 58 N·s, under stagger. Scrappers throw them for the tell, not the hit. A Scrapper cannot start a stacked pallet.
- **Rigger:** a two-pallet stack is a wall. A Hang between two pallets is an anchored Clothesline.
- **Breaker:** Hauls a pallet into a hanging house's slack-cable anchor post and lets the mass do the cutting. 1,000 N is 3.4 m/s²; 11.7 m/s at the end of 20 m.
- **Climber:** the pallet top is a perch 70 cm up. Jacks off it and keeps 13.7 m/s of their 18 m/s V.
- **Doubler:** two pallets Hauled together over an enemy. Both burst. 480 N per line moves each at 1.7 m/s².

## Appearance

Silhouette: a tight rectangular block of brick with a visible running-bond pattern on all four faces, a sheet of cloudy shrink wrap, two plastic straps over the top, and a timber pallet with three bearers and chamfered deck boards.

| Part | Material | Base color | Roughness | Metallic |
|---|---|---|---|---|
| Bricks | `MI_Concrete_Stained` tinted to clay `#9E5B3C` | | 0.85–0.95 | 0 |
| Shrink wrap | `MI_HDPE_Blue` set to clear, roughness 0.2, opacity 0.35 | | | |
| Straps | `MI_HDPE_Black` | `#232426` | 0.45–0.60 | 0 |
| Pallet | `MI_Timber_Rough` | `#7A5C3A` | 0.78–0.90 | 0 |
| Markings | `MI_Decal_Stencil` | `#E6E4DD` | inherits | 0 |

Wear: the shrink wrap is torn on one corner where somebody pulled bricks out, exposing the course. Brick dust `#B87A5A` on the pallet deck. Pallet corners crushed by forks. A boot print on top. In the Anchor the wrap is intact and the pallet is new; in Hangtown it has been opened and re-strapped with rope.

Text: a paper slip under the wrap: `MERIDIAN MUTUAL / BUILDING GRANT / HANGTOWN DISTRICT 3`. On Anchor pallets, `HTR-BM-0417 / 120 PC` in white. Hangtown residents chalk a house number on the top course.

Lighting: the brick faces are mid-value and warm; the wrap picks up a broad specular from the Office lights and reads as a soft, cloudy highlight. In the Anchor's pale grime `#B8B2A4` the pallet reads as the only warm block on the Rim.

## Variants

| Variant | Difference |
|---|---|
| Wrapped | Default. Shrink wrap intact. 290 kg. |
| Opened | Wrap torn, top course half gone, re-strapped with rope. 245 kg. (**PROPOSED** mass) |
| Block | 60 concrete blocks instead of brick, `#8E8B84`, 290 kg. The Anchor's blast wall stock. Fragments are 4.6 kg blocks. |
| Spilled | Spawns pre-fractured as a brick pile 120 × 100 × 40 cm. |

## Physics

- Tier Medium, 290 kg.
- Collision: one box 120 × 80 × 56 cm for the brick block on top of a box 120 × 80 × 14 cm for the pallet.
- Center of mass: 40 cm above the floor at the footprint center. Tips at 35° on the long axis, 27° on the short. (**PROPOSED**)
- Friction 0.6 (pallet timber on steel and concrete), 0.65 on wood. Restitution 0.05. (**PROPOSED**)
- Break-free (05 §6.2): loose 0 N, anyone; in a two-high stack 500 N, Rigger, Doubler, Breaker; strapped to a rim landing post 800 N, Doubler and Breaker. Break-free is tested against active pull only (Haul, Jack, and the Kick); a Hang never unseats a placed object.
- Fracture: 1,000 N·s. Fragments: L1 six Medium courses at 46 kg each (120 × 80 × 9 cm) plus the Light pallet at 14 kg; L2 each course breaks at 400 N·s into 20 Light bricks at 2.3 kg with a 10 s lifetime. (**PROPOSED** thresholds; bricks are not in the 05 §6.5 table) A flipped 220 kg Breaker landing on it at 5 m/s is 1,100 N·s and bursts the wrap; player bodies count.

Line acceleration and arrival over a full line, other end static or braced (v = sqrt(2 a d)):

| Class | Tension | Acceleration | Line | Arrival | Impulse |
|---|---|---|---|---|---|
| Climber | 400 N | 1.4 m/s² | 50 m | 11.7 m/s | 3,393 N·s |
| Scrapper | 400 N | 1.4 m/s² | 20 m | 7.4 m/s | 2,146 N·s |
| Rigger | 600 N | 2.1 m/s² | 40 m | 12.9 m/s | 3,741 N·s |
| Doubler | 800 N | 2.8 m/s² | 30 m | 12.9 m/s | 3,741 N·s |
| Doubler overspin | 1,200 N | 4.1 m/s² | 30 m | 15.8 m/s | 4,582 N·s |
| Breaker | 1,000 N | 3.4 m/s² | 20 m | 11.7 m/s | 3,393 N·s |

Impact on a player (impulse = 290 kg × relative speed):

| Speed | Impulse | Result |
|---|---|---|
| 1.0 m/s | 300 N·s | Stagger |
| 2.0 m/s (walking) | 580 N·s | Stagger |
| 2.1 m/s | 600 N·s | Flip |
| 4.1 m/s | 1,200 N·s | Stall; a Scrapper needs 6.1 m of run-up, a Breaker 2.4 m |
| 5.2 m/s (Doubler Snap, 1,500 N·s) | 1,500 N·s | Stall |
| 11.7 m/s (7 m drop) | 3,393 N·s | Stall; pallet fractures on landing |
| 15.8 m/s (fastest Haul arrival, overspin) | 4,582 N·s | Stall; fractures |

A course (46 kg) is under 100 kg and gets the Scrapper bonus: 400 N is 8.7 m/s², 18.6 m/s over 20 m, times 1.3 is 24 m/s, 1,110 N·s, a flip. A course Stalls only on a Jack to the 30 m/s cap (1,380 N·s) or on a Snap (1,500 N·s, leaving at 33 m/s). A brick cannot Stall or flip under any line.

Jack (05 §3.2 v0.2): the pallet is Kicked to V × M / (M + m) within 0.3 s (the Kick), then T / 290 kg. Climber (V 18): pallet 4.3 m/s, Climber 13.7 back. Scrapper (V 14): pallet 3.9, Scrapper 10.2. Rigger (V 12): pallet 3.7, Rigger 8.3. Doubler (V 14): pallet 4.3, Doubler 9.7. Breaker (V 10): pallet 4.3, Breaker 5.7. Every Jack starts the pallet at or near its 4.1 m/s Stall speed.

Crush: no line reaches the 8 kN crush rule (05 §7.1: crush comes from kinematics and from stacked or falling weight over about 800 kg). The pallet weighs 2.8 kN; a two-high stack is 5.7 kN, under. A pallet crushes only when a kinematic drives it, or when it is the top of a three-pallet stack (8.5 kN) resting on a pinned player for 0.5 s.

## Interactions

- **Haul to it:** the pallet is heavier than every harness. 1,000 N on a 220 kg Breaker is 4.5 m/s², under the 5 m/s² foot-hold: the Breaker skids and does 57% of the closing. A Climber at 4.4 m/s² is airborne and does 76%.
- **Jack from it:** a strong launch. The split above leaves the pallet nearly still against anyone light.
- **Hang from it:** a Hang to a stacked or strapped pallet does not unseat it; the stack carries the load. A Hang to a loose pallet brings it along as 290 kg (2.8 kN) of dead weight. A player Hung to a pallet climbs it by Reel (hold Haul within Hang; the rod shortens at 1.0 m/s). A Climber or Scrapper line (Hang load 4 kN) holds it static but slips as soon as it swings. Rigger, Doubler, and Breaker lines hold it swinging. Two lines' Hang loads add (05 §6.2a).
- **Object to object:** a pallet Hauled into a market stall breaks the stall. Into a steel crate, both stop and the pallet's wrap tears (dented state).
- **Kinematic:** the Lift counterweight through a pallet at 3 m/s delivers 870 N·s to it: dented, shoved, not broken.
- **Water:** sinks. Loose bricks sink.
- **Updraft and Pulse:** force volumes act on players and free bodies only (05 §6.6: loose objects at break-free 0 and objects held on a player's line, not cable-hung map objects). A stacked or strapped pallet is unaffected until freed (an instance hung on a player's Hang line is a free body and swings in a gust). A loose 290 kg pallet feels 0.34× the authored acceleration (a_ref × min(100 / m, 2)) and shifts a hand's width on the Pulse (0.4 s outward, then a 0.6 s inhale toward the Stump, every 75 s from 0:55). A loose 2.3 kg brick feels 2× (cap) and leaves the Rim.
- **Sway (Hangtown):** on a rim landing it is on solid ground and does not move. On a Mid Hang house platform it moves with the house and shifts the house's center of mass by its own 290 kg, which matters on Cable Day.
- **Breathing:** not present in Ballast.
- **Cut recoil:** 150 N·s per 1,000 N, 0.5 m/s on the pallet. On a loose 2.3 kg brick a cut is 65 m/s, capped at the 50 m/s terminal velocity: 115 N·s, nothing, but it crosses the room.
- Slides, tips, does not roll or float.

## States

| State | Visual | Physics |
|---|---|---|
| Intact | Wrapped block on pallet. | One body, 290 kg. |
| Dented | Wrap torn open, top corner of bricks slumped 10 cm. | No change. |
| Broken | Six courses slid apart, pallet loose. | Six Medium bodies (46 kg, Scrapper bonus applies) and one Light. |
| Destroyed | Courses broken to bricks; a pile. | Up to 120 Light bodies for 10 s, then a static pile mesh; nothing replenishes within the round (05 §9.2). Hard surface ×1.0. |

## VFX and SFX hooks

- Impact: `SFX_Brick_Pallet_Hit_Steel`, `_Concrete`, `_Wood` (a dense thud with a clay chatter layer). `SFX_Brick_Splash` into water.
- Break-free: `SFX_BreakFree_Timber`, `NS_BreakFree_Dust_M` in brick-dust color. Under threshold the wrap crackles.
- Fracture L0: `SFX_Wrap_Tear`, `SFX_Course_Slide`, `NS_Brick_Dust_L`. Fracture L1: `SFX_Brick_Clatter` scaled by count, `NS_Brick_Scatter`.
- No emissive.

## Technical

- LOD0 6,000 tris (bricks as a displaced block with modeled corner bricks, wrap as a shell), LOD1 3,000, LOD2 1,200, LOD3 300. One 2K set plus `T_Trim_Timber_01` for the pallet.
- Pivot: bottom center of the pallet footprint. X+ is the long axis.
- Dog sockets: `SKT_Dog_Top_01`, `_02` (top course) `Concrete` class (clay brick uses the Spike Dog rules); `SKT_Dog_SideN_01`, `SKT_Dog_SideS_01`, `SKT_Dog_SideE_01`, `SKT_Dog_SideW_01` (mid height) `Concrete`; `SKT_Dog_Frame_01`, `_02` (pallet bearers at each end) `Wood`. Fragments: `SKT_Dog_F01_01` to `SKT_Dog_F06_01` (courses) `Concrete`; `SKT_Dog_F07_01` (pallet) `Wood`.
- Geometry collection `GC_BrickPallet`: L0 whole (damage threshold 1,000), L1 six courses plus pallet (threshold 400 per course, 100,000 for the pallet), L2 bricks (threshold 100,000; bricks do not break). `RemoveOnMaxSleep` 10 s on L2.
- Anchor fields: `FS_Anchor_BrickPallet` releases stacked instances at 500 N; `FS_Anchor_BrickPallet_Strap` releases strapped instances at 800 N.
- Replication RC-M intact; RC-GC on fracture with `ReplicationAbandonAfterLevel` = 2. Courses stay replicated (Medium); bricks go local.

## Reference search terms

Pallet of clay bricks shrink wrapped; brick pallet strapped construction site; opened brick pallet torn wrap; concrete block pallet 8 inch; brick dust pallet close up; hillside construction material staged photo.
