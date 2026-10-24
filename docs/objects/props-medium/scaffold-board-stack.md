# Scaffold board stack

Medium prop. Mass tier Medium, 160 kg strapped. 390 × 45 × 23 cm (12 boards of 390 × 22.5 × 3.8 cm, two wide, six high, on two bearers, with two steel straps). Appears in the Anchor (the Scaffold, on every second deck; the Rim, in laydown rows), the Yard (the Floor, beside the trench bridges), and Hangtown (the Upper Hang, stacked at the Lift headframe).

## Role in play

A long, low Medium that becomes twelve Light planks when its straps go. Intact it is a battering ram. Broken it is a Scrapper's ammunition dump and a ×1.2 wood landing surface. 05 §6.5 makes the reading canon: single boards are Light, a strapped stack is Medium.

| Rig | Use |
|---|---|
| The Guillotine | Hang the stack lengthwise overhead (1.6 kN, within every class Hang load), Reel it up at 1.0 m/s, then recall or cut your own set line. It drops by gravity: 2.9 m of fall reaches its 7.5 m/s Stall speed; from 4 m it lands at 8.9 m/s, 1,424 N·s. |
| The Avalanche | Stacks are stacked. Haul the bottom one at or above 500 N: Rigger, Doubler, Breaker. |
| The Clothesline | A stack on its edge is a 23 cm trip wall; the strap loops are anchors at 0.25 m. |
| The Scatter | Break it on purpose. Twelve planks in the air is a screen. |

- **Scrapper:** breaks it first. Loose planks are Light, so they move 30% faster on a Scrapper line. The intact stack is 160 kg, over the 100 kg Medium bonus limit, so it does not. A Scrapper cannot start a stacked stack (400 N under 500 N).
- **Rigger:** Hangs it across a Scaffold gap as a bridge. The straps take the Dog.
- **Breaker:** Hauls it end-on into a doorway. 1,000 N on 160 kg is 6.25 m/s²; 15.8 m/s at the end of a 20 m line.
- **Climber:** a long perch. Jacks off the end and the stack rolls on its bearers the other way.
- **Doubler:** Hauls both ends toward two enemies. The stack turns sideways and sweeps.

## Appearance

Silhouette: a long rectangular bale. Twelve boards with ragged ends, two galvanized straps at the third points, two 10 × 10 cm bearers underneath. End-grain reads as a twelve-cell grid.

| Part | Material | Base color | Roughness | Metallic |
|---|---|---|---|---|
| Boards | `MI_Timber_Rough` | `#7A5C3A` | 0.78–0.90 | 0 |
| Board ends, end bands | `MI_Steel_Galvanized` | `#9EA3A6` | 0.35–0.55 | 1 |
| Straps | `MI_Steel_Galvanized` | `#9EA3A6` | 0.40–0.55 | 1 |
| Bearers | `MI_Timber_Dark` | `#4A3522` | 0.70–0.85 | 0 |
| Markings | `MI_Decal_Stencil` | `#C8912A` Guild, `#E8620C` HTR | inherits | 0 |

Wear: top boards silvered grey by sun, lower boards still brown. Boot wear down the center of the top face. Nail holes in rows. Concrete splatter on one end. End bands dented. Strap edges bright where they were tensioned.

Text: each board stencilled on its end band: `LG SCAF 38` (Guild) or `HTR-SC-2210` (HTR). The Anchor's stacks carry a white Office tag `SITE PROPERTY` on the top board.

Lighting: the long top face catches the Anchor's pale light and reads as a bright bar. End grain is dark. In the Yard, cutting dust `#2A2622` settles into the grain.

## Variants

| Variant | Difference |
|---|---|
| Guild | Ochre end stencils, older boards, one board split. |
| HTR | Orange end stencils, newer timber `#8E6D45`, straps tight. |
| Half stack | Six boards, 80 kg, 390 × 45 × 12 cm. Still Medium, and under 100 kg, so it gets the Scrapper bonus. |
| Loose | Spawns pre-broken as twelve planks in a loose pile. Scrap heap surface rule applies when the pile is over 40 cm deep (**PROPOSED**). |

## Physics

- Tier Medium, 160 kg (12 × 12 kg boards plus 16 kg of bearers and straps).
- Collision: one box 390 × 45 × 23 cm. Bearers add two boxes 45 × 10 × 10 cm underneath.
- Center of mass: geometric center at 11.5 cm above the bearers. Long axis makes it stable; it tips only end over end off a ledge. (**PROPOSED**)
- Friction 0.55 wood on steel, 0.6 on concrete, 0.65 wood on wood. Restitution 0.1. (**PROPOSED**)
- Break-free (05 §6.2): loose on floor 0 N, anyone; in a stack of stacks 500 N, Rigger, Doubler, Breaker; strapped to a Scaffold standard 800 N, Doubler and Breaker. Break-free is tested against active pull only (Haul, Jack, and the Kick); a Hang never unseats a placed object.
- Fracture: straps break at 800 N·s, matching the 05 §6.5 scaffold section value (**PROPOSED** for this object). Fragments: 12 Light planks at 12 kg, 2 Light straps at 1 kg, 2 Light bearers at 3 kg. All Light, 10 s lifetime unless picked up by a line. A flipped 130 kg Rigger landing on the stack at 6.2 m/s is 806 N·s and breaks the straps; player bodies count.

Line acceleration and arrival over a full line, other end static or braced (v = sqrt(2 a d)):

| Class | Tension | Acceleration | Line | Arrival | Impulse |
|---|---|---|---|---|---|
| Climber | 400 N | 2.5 m/s² | 50 m | 15.8 m/s | 2,528 N·s |
| Scrapper | 400 N | 2.5 m/s² | 20 m | 10.0 m/s | 1,600 N·s |
| Rigger | 600 N | 3.75 m/s² | 40 m | 17.3 m/s | 2,768 N·s |
| Doubler | 800 N | 5.0 m/s² | 30 m | 17.3 m/s | 2,768 N·s |
| Doubler overspin | 1,200 N | 7.5 m/s² | 30 m | 21.2 m/s | 3,392 N·s |
| Breaker | 1,000 N | 6.25 m/s² | 20 m | 15.8 m/s | 2,528 N·s |

Impact on a player (impulse = 160 kg × relative speed):

| Speed | Impulse | Result |
|---|---|---|
| 1.9 m/s | 300 N·s | Stagger |
| 2.0 m/s (walking) | 320 N·s | Stagger |
| 3.75 m/s | 600 N·s | Flip |
| 7.5 m/s | 1,200 N·s | Stall; a Scrapper needs 11 m of run-up, a Breaker 4.5 m |
| 9.4 m/s (Doubler Snap, 1,500 N·s) | 1,500 N·s | Stall; straps break |
| 11.7 m/s (7 m drop) | 1,872 N·s | Stall; fractures on landing |
| 21.2 m/s (fastest Haul arrival, overspin) | 3,392 N·s | Stall; fractures |

Order of events on a hit over 800 N·s: the player receives the full impulse, then the straps break and the planks continue past. A single plank (12 kg) needs 25 m/s for a stagger and cannot Stall.

Jack (05 §3.2 v0.2): the stack is Kicked to V × M / (M + m) within 0.3 s (the Kick), then accelerates at T / 160 kg. Climber (V 18): stack 6.5 m/s, Climber 11.5 back. Scrapper (V 14): stack 5.7, Scrapper 8.3. Rigger (V 12): stack 5.4, Rigger 6.6. Doubler (V 14): stack 6.3, Doubler 7.7. Breaker (V 10): stack 5.8, Breaker 4.2. Every Jack starts the stack at a flip speed; it reaches a Stall speed within 4 m on a Breaker line.

Crush: no line reaches the 8 kN crush rule (05 §7.1: crush comes from kinematics and from stacked or falling weight over about 800 kg). The stack weighs 1.6 kN. A Hauler pinning it against a trench wall crushes; nothing else does.

## Interactions

- **Haul to it:** the player is pulled to the strap or board. 600 N on a 130 kg Rigger is 4.6 m/s², under the Rigger's 5 m/s² foot-hold, so the Rigger skids and the stack slides in at 3.75 m/s². Closing acceleration 8.4 m/s².
- **Jack from it:** the split above. A Climber keeps 11.5 m/s of V; the stack takes the rest.
- **Hang from it:** a Hang to a strapped or stacked stack does not unseat it (only Haul, Jack, and the Kick test break-free), so the strap loops are safe Pendulum anchors. A Hang to a loose stack brings it along as 160 kg of dead weight. A stack hung overhead swings end-first. As a bridge, the Hang holds the stack's 1.6 kN weight on any class line (lowest Hang load is 4 kN). Two Riggers' Hang loads add (05 §6.2a): 24 kN between them, for two stacks and a crew.
- **Object to object:** a stack sliding into a steel crate stops; into a market stall it fractures the stall (300 N·s needs only 1.9 m/s).
- **Kinematic:** a Hauler strike delivers the full relative impulse and breaks the straps every time. A Hauler passing through a Hang line that holds a stack as a trench bridge severs it (snap, both Dogs release); lines attached to the Hauler itself are not severed.
- **Water:** floats, boards up, straps intact. Broken planks float and spread.
- **Updraft and Pulse:** force volumes act on players and free bodies only (05 §6.6: loose objects at break-free 0 and objects held on a player's line, not cable-hung map objects). A strapped or stacked stack is unaffected until freed (an instance hung on a player's Hang line is a free body and swings in a gust). A loose 160 kg stack feels 0.63× the authored acceleration (a_ref × min(100 / m, 2)) and slides a little on the Pulse (0.4 s outward, then a 0.6 s inhale toward the Stump, every 75 s from 0:55). A loose 12 kg plank feels 2× (cap) and is lifted near the Stump, then drawn back on the inhale.
- **Breathing:** slides lengthwise at 6° on a steel deck only with a shove (friction 0.55 holds to 29°). Loose planks slide.
- **Cut recoil:** 150 N·s per 1,000 N, under 1 m/s on the stack. On a loose plank the same cut is 12.5 m/s: planks twitch when a line on them is cut.
- Slides, floats, does not roll. Loose planks roll a quarter turn and stop.

## States

| State | Visual | Physics |
|---|---|---|
| Intact | Twelve boards, two tight straps. | One body, 160 kg. |
| Dented | One strap slipped 20 cm, top board split along the grain. | No change. |
| Broken | Straps gone, planks fanned out. | Twelve Light bodies, 12 kg each. Pile surface ×1.2 wood on stagger, flip, and Stall; ×2.0 when over 40 cm deep. |
| Destroyed | Planks past 10 s lifetime despawn and do not replenish within the round (05 §9.2). | Bearers persist as Light props. |

## VFX and SFX hooks

- Impact: `SFX_Timber_Hit_Steel`, `_Concrete`, `_Wood`, `SFX_Timber_Splash`. Hollow knock layer scaled by impulse.
- Break-free: `SFX_BreakFree_Timber` scrape, `NS_BreakFree_Dust_M`. Under threshold, `SFX_Strap_Creak`.
- Fracture: `SFX_Strap_Snap` (a steel twang, not the line snap), `NS_Plank_Scatter` with splinters and dust, `NS_Impact_Wood_M`.
- No emissive.

## Technical

- LOD0 6,000 tris (boards individually modeled with chamfers), LOD1 3,000, LOD2 1,200 (a single box with end-grain texture), LOD3 300. One 2K set plus `T_Trim_Timber_01`.
- Pivot: bottom center of the bearers. X+ is the long axis.
- Dog sockets: `SKT_Dog_Top_01`, `_02`, `_03` (top face at quarter points) `Wood`; `SKT_Dog_End_01` (X+), `SKT_Dog_End_02` (X−) `Wood`; `SKT_Dog_Hook_01`, `_02` (strap loops) `Metal`; `SKT_Dog_SideN_01`, `SKT_Dog_SideS_01` `Wood`. Fragments: `SKT_Dog_F01_01` to `SKT_Dog_F12_01`, one per plank, `Wood`.
- Geometry collection `GC_ScaffoldBoardStack`: L0 whole (damage threshold 800), L1 twelve planks plus two straps plus two bearers (threshold 100,000, planks do not break further), L2 none. Damage propagation off above L1.
- Anchor fields: `FS_Anchor_ScaffoldBoardStack` releases stacked instances at 500 N; `FS_Anchor_ScaffoldBoardStack_Strap` releases strapped instances at 800 N.
- Replication RC-M intact; RC-GC on fracture with `ReplicationAbandonAfterLevel` = 1 so planks go local on clients.

## Reference search terms

Scaffold boards banded stack; scaffold plank end band galvanized; timber scaffold board silvered weathering; strapped lumber bundle construction site; scaffold board laydown yard; end grain lumber stack photo.
