# Chain coil

Light prop. Light tier, 25 kg. Coiled 40 cm diameter × 18 cm tall; 6 m of 10 mm grade-80 lifting chain with a hook each end. Appears in the Yard (the Floor, beside the rail trenches and on scrap heaps; the Rails, hung from the Gantry walkway), Ballast (the Bays, hanging from the magnetic racks and swinging on the Breathing; the Spine, hooked to ladder rungs), and Hangtown (the Upper Hang, coiled at cable anchor points). Not in the Anchor.

## Role in play

The heaviest Light prop in the game, and the only one that flips a player on a line. A chain coil is what a Scrapper reaches for when the barrels are gone. Rigging crews leave them everywhere because every load on the Lay was moved with one.

- Named rigs: the Scatter (Jack two coils apart into two doorways). The Guillotine, as the load on a set line: recall or cut your own set line and the coil drops by gravity; a second line (coil to enemy, Haul) only steers it. From 4 m it arrives at 8.9 m/s, 222 N·s, under stagger. A Light Guillotine is a tell and a tangle, not a kill. Overboard setup: a coil Hauled into the back of a Catch Plate near an edge puts a player over at 25 m/s.
- Scrapper: the primary weapon. Haul to the enemy's back plate at 25 m/s, flip, re-dog, repeat. The 30% Light-prop speed bonus closes the gap faster; the cap holds.
- Rigger: Hangs a coil from an overhead anchor, Reels it up out of sight (within Hang, holding Haul shortens the rod at 1.0 m/s, holding Jack lengthens it, bearing up to the class Hang load), and releases it as a set line. Twelve seconds is long enough for someone to walk under it; recall the set line and the coil drops.
- Breaker: uses it as the far Dog for a self-Haul when there is no wall. 25 kg does not move a Breaker; the Breaker moves it.
- Climber: a thrown coil in the Updraft drifts. Climbers Jack coils off the Scaffold at enemies below for the tell more than the hit.
- Doubler: the far end of a Snapback. At a full 1,200 N snap the coil receives 1,500 N·s: 60 m/s on 25 kg, capped at the 50 m/s terminal velocity, arriving with 1,250 N·s. That is a Stall, and the only way a chain coil scraps anyone. The snap impulse scales with the force at the snap (1,500 N·s × F / 1,200 N); a snap released before 960 N, which is 0.4 s into overspin, delivers under 1,200 N·s and is a flip.

## Appearance

Silhouette: a fat flat ring with two hooks hanging off it. Hanging, it is a dark vertical line with a hook at the bottom, which matters because it must not read as a singing line. Chain is matte and never emissive.

| Part | Material | Base color | Roughness | Metallic |
|---|---|---|---|---|
| Links (10 mm) | `MI_Chain_Oiled` | `#3C3E42` | 0.30 to 0.45 | 1 |
| Links, unmoved core of the coil | `MI_Steel_Rusted` | `#7A3F1C` | 0.80 | 0.3 |
| Hooks (clevis grab, 2) | `MI_Steel_Painted_Worn` | `#E8620C` HTR orange, or `#C8912A` Guild ochre | 0.40 to 0.65 | 1 where chipped |
| Rating tag (steel, 3 × 5 cm) | `MI_Steel_Galvanized` | `#9EA3A6` | 0.40 | 1 |

Wear story: the outer links are oiled and dark because they get used. The links at the center of the coil, which never leave the pile, are rusted orange. Hooks are polished to bare steel at the throat where loads sit. The rating tag is stamped `WLL 3.15 T` and the owner code: `HTR-RC-2208` on HTR coils, `LG-0097` on Guild coils. Hangtown coils have no tag and a loop of sisal rope through the end link.

Under arena lighting: chain is the darkest object in most frames. Specular highlights on the link crowns are the only read. On the Yard's Floor the oil film catches the floodlights. In Ballast's Hold a hanging coil swinging on the Breathing is a moving shadow against the seep-water sheen.

## Variants

| Variant | Difference |
|---|---|
| `Coiled` | Default. Flat ring. Loose on floor or on a heap. |
| `Hung` | Straight, 6 m, hooked to a ceiling socket. Swings. Hangs from the rack frames in the Bays. |
| `HTR` | Orange hooks, clean tag, low wear. |
| `Guild` | Ochre hooks, ochre tape wrap on the top link, high wear. |
| `Hanger` | No hooks. Rope loop. Rustier. |

## Physics

- Tier: Light. Mass: 25 kg.
- Collision: coiled, a cylinder radius 20 cm × 18 cm tall. Hung, a chain of 12 capsules (radius 1.5 cm, 50 cm each) with ball constraints; the two end hooks are 8 cm spheres (**PROPOSED**).
- Center of mass: coiled, 8 cm above floor at the ring center. Hung, the midpoint of the chain.
- Friction 0.7, restitution 0.05 (**PROPOSED**). Chain does not bounce.
- Break-free: loose on floor 0 N. Hung from a hook, treated as "in a rack or cradle," 200 N, so any class unhooks it with a lurch.
- Haul acceleration with the far end static: 16 m/s² at 400 N (Climber, Scrapper), 24 at 600 N (Rigger), 32 at 800 N (Doubler), 40 at 1,000 N (Breaker). Distance to the 25 m/s Haul cap: 19.5 m, 13 m, 9.8 m, 7.8 m. A Scrapper's 20 m line just reaches the cap at full length; with the 30% Light-prop bonus (×1.69 force, ×1.3 arrival speed; 05 §6.1) it pulls at 27 m/s² and reaches the cap at 11.6 m.
- Cut recoil: 150 N·s per 1,000 N to both ends. Cutting a Breaker's line on a coil gives it 150 N·s, a 6 m/s twitch; a Scrapper's line, 60 N·s, 2.4 m/s. A hung coil swings when its line is cut.
- Fracture: none. Chain does not break on impact.

What it does to a player:

| Delivery | Speed | Impulse | Result |
|---|---|---|---|
| Haul, any class | 25 m/s cap | 625 N·s | Flip |
| Scrapper Haul | 25 m/s cap | 625 N·s | Flip |
| Jack | 30 m/s cap | 750 N·s | Flip |
| Fall from 7 m | 11.7 m/s | 293 N·s | Just under stagger |
| Hung coil swinging at 6 m/s | 6 m/s | 150 N·s | Nothing |
| Doubler Snapback, full 1,200 N | 50 m/s terminal | 1,250 N·s | Stall |
| Doubler Snapback at 960 N | 48 m/s | 1,200 N·s | Stall, at threshold |

On a Haul or Jack it cannot Stall anyone. 1,200 N·s needs 48 m/s and the caps are 25 and 30. A flipped player lands with their momentum preserved (05 §3.5), so a coil to the back plate at the edge of a trench is a ring-out setup, not a scrap. Only a Snapback makes it a scrap.

## Interactions

- Haul to it: the coil comes to you. If hung, it unhooks at 200 N and swings in.
- Jack from it: the Kick (the Jack's 0.3 s capacitor discharge, separate from the motor pull T) drives you apart to the class launch speed V, split by inverse mass. A 90 kg Climber (V 18 m/s) sends the coil off at 14.1 m/s and takes 3.9 m/s backward; a 110 kg Scrapper (V 14) sends it at 11.4 m/s and takes 2.6 m/s; a 220 kg Breaker (V 10) sends it at 9 m/s and takes 1 m/s. Then T pushes the coil at 16 to 40 m/s² by class to the 30 m/s cap (a Scrapper's 30% bonus, ×1.69 force and ×1.3 arrival speed, applies to this push and to the Haul, never to V; 05 §6.1). A coil is the one Light prop heavy enough to give a Climber a noticeable shove. The Kick never fires with a Dog in a Catch Plate: a Jack from the coil to an enemy is the T push alone, the coil leaving from rest at 16 m/s² on a 400 N line and reaching 25.3 m/s at the end of a Scrapper's 20 m line (a Breaker's 1,000 N reaches the cap in 0.75 s over 11.3 m), while the enemy skids back at T over their mass.
- Hang from it: break-free is tested against active pull only (Haul, Jack, and the Kick), never against a Hang line's static load, so a hung coil's 200 N hook value does not apply to a hanging player. The hook is a structural lifting eye and holds a Pendulum from any harness up to the class Hang load (**PROPOSED** physical behaviour). Swing the coil's 6 m of chain as a pivot off a Bays rack and you have a 6 m longer pendulum than the line alone. A Hang to a coil loose on a floor brings it along as 25 kg of dead weight.
- Object to object: a coil at 25 m/s into a propane cylinder is an impact over 8 m/s and triggers it. Into a market stall, 625 N·s, fracture. Into a steel crate, nothing.
- Kinematic: a Hauler drags a hung coil along the trench wall. The Gantry's electromagnet picks up any coil within 1 m of its load (**PROPOSED** behavior; the magnet is on).
- Water: sinks instantly. In the Hold it stays where it fell.
- Updraft and Pulse: not in the Anchor. If one gets there, force volumes act on players and loose objects (break-free 0) at a_ref × 100 / m: a loose coil feels 2 times the authored acceleration (the per-body cap; 100 / 25 kg would be 4) and lifts, speeds capped by the 50 m/s hard limit. A hooked coil (200 N) is unaffected by the Breath or the Pulse until it is unhooked; a coil hung on a player's Hang line is a free body and swings in the gust.
- Breathing tilt: a hung coil swings 6° with the hull plus its own period. A coiled coil slides at 6° on wet steel.
- Chain reactions: the standard first link of a Scrapper chain. Coil into cylinder into cart into player.

## States

| State | Visual | Physics |
|---|---|---|
| Intact | Coiled or hung | 25 kg |
| Dented | Not applicable; chain does not dent | |
| Broken | Not applicable | |
| Destroyed | Not applicable. A coil leaves the round only down the Well. | |

Chain has one state. Its tell is motion, not damage.

## VFX and SFX hooks

- Impact on steel: `SFX_Chain_Steel` (bright rattle, 3 layers, pitch by speed). Concrete: `SFX_Chain_Concrete` (dead rattle). Wood: `SFX_Chain_Wood`. Water: `SFX_Chain_Splash`.
- Movement: `SFX_Chain_Drag` loop while sliding; `SFX_Chain_Swing` creak on a hung coil at the top of each swing.
- Break-free lurch from a hook: `SFX_Hook_Slip` and `NS_Dust_S` at the hook.
- Impact on a player: `SFX_Chain_Plate` (chain on armor).
- No emissive. Confirm in the darkened Anchor sector that chain never reads as a line.

## Technical

- LOD0 2,000 tris (coiled, instanced link), LOD1 1,000, LOD2 400, LOD3 120 (a torus). Hung variant uses a spline mesh of one 1K link. One 1K set.
- Pivot: coiled, bottom center. Hung, the top hook eye (primary hang point).
- Dog sockets: `SKT_Dog_Top_01` (top of the coil, Metal), `SKT_Dog_Hook_01` (end hook A, Metal), `SKT_Dog_Hook_02` (end hook B, Metal). Hung variant adds `SKT_Dog_End_01` at the bottom hook.
- Chaos: not a geometry collection. Static mesh with a physics asset; hung variant is a constraint chain. Anchor field `FS_Anchor_ChainCoil_Hung` holds the hung variant until 200 N.
- Replication: RC-L. The hung variant replicates only the end positions; clients solve the chain locally.

## Reference search terms

Grade 80 lifting chain coil clevis grab hook; rigging chain sling WLL tag; coiled chain on shop floor oil; chain hanging from crane walkway; rusted chain pile scrapyard; chain sling with ochre tape wrap.
