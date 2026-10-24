# Paint can

Light prop. Light tier, 5 kg full, 1.2 kg empty. 18 cm diameter × 20 cm tall, 4 L steel can with a wire bail. Appears in Hangtown (the Mid Hang, on porches and the market platform in stacks of three to six; the Low Hang, by the garden terraces), the Yard (the Floor, inside and around the control shack; the Rails, on the crane cab step), and the Anchor (the Scaffold, in prep-crew caches beside the rated tape rolls). In Ballast only in HTR's Crown gantry cache.

## Role in play

The smallest prop in the game. It cannot stagger a player. It exists to be knocked over, to roll under feet, to mark where a Dog landed, and to leave a color on the floor when it breaks. A paint can is a tell generator: it makes noise and motion out of a line that hit nothing important.

- Named rigs: Clothesline bait (a can rolling across a doorway pulls the eye down). The Guillotine, as the thing hung in plain sight so the real load behind it goes unseen. The Scatter, as a splash that paints two players and makes them easy to track for the rest of the round.
- Scrapper: Hauls a can at an enemy's face as a flinch, then re-dogs. Instant recall means the can costs nothing. Light props travel 30% faster on a Scrapper line; the 25 m/s cap holds.
- Rigger: stacks four on a set line as a counterweight that swings and clatters, covering the sound of a second line being set.
- Breaker: steps on it.
- Climber: kicks a stack off the Scaffold to make noise on the Rim, then goes the other way.
- Doubler: a can on the far Dog of a Snapback bursts on arrival and paints the target. At the 50 m/s terminal velocity a 5 kg can carries 250 N·s. No impulse worth counting.

## Appearance

Silhouette: a short cylinder with a wire arc over it. A stack of them reads as a column of rings. Reads by its lid color more than its body.

| Part | Material | Base color | Roughness | Metallic |
|---|---|---|---|---|
| Can body (tinplate) | `MI_Steel_Bare` under a paper label | `#5C5F63` | 0.45 | 1 |
| Label (paper, 60% wrap) | `MI_Decal_Stencil` on `MI_Canvas_Natural` base | label color per variant | 0.75 | 0 |
| Lid | `MI_Steel_Bare` with a paint ring | `#5C5F63`, drip color per variant | 0.45, 0.35 on paint | 1, 0 |
| Bail (wire) | `MI_Steel_Galvanized` | `#9EA3A6` | 0.40 | 1 |
| Dried paint drips | `MI_Steel_Painted_Worn` paint layer | per variant | 0.30 to 0.40 | 0 |

Wear story: a dried skin of its own color runs down one side from the lid rim. The bail is bent where it was hung from a nail. The label is torn at the seam. Rust (`MI_Steel_Rusted`, `#7A3F1C`) rings the base where it sat in water. Hand polish on the bail.

Variants carry the colors of whoever bought them. Hangtown cans are hanger paint: teal `#2E8B8B`, red `#B5302A`, mustard `#D9A441`, violet `#6E4C9E`, with a hand-written price on the lid in chalk. Guild cans are ochre `#C8912A` and black `#1C1B19`, labeled `LAY GUILD STORES` with a Ledger stores number. Office cans are the rated-tape yellow `#F2C230` and black, labeled `OFFICE OF THE LAY / SITE MARKING / NOT FOR SALE`. HTR cans are orange `#E8620C` powder-coat touch-up, labeled `HTR-RC` and a batch code.

Under arena lighting: the lid paint ring is a saturated dot. In Hangtown that is the point. In the Anchor an Office yellow can on grey concrete is a bright mark and prep crews leave them near rated loads on purpose (fiction: that is where they were used).

## Variants

| Variant | Mass | Difference |
|---|---|---|
| `Full_Hanger` | 5 kg | Hanger color, chalk price. |
| `Full_Guild` | 5 kg | Ochre or black, stores label. |
| `Full_Office` | 5 kg | Yellow or black, Office label. |
| `Full_HTR` | 5 kg | Orange, batch code, no wear above 0.2. |
| `Empty` | 1.2 kg | Lid off, dry inside, dented. Scrap heaps. Rolls far. |
| `Stack_4` | 20 kg | Four full cans as one prop until it fractures into four. Hangtown porches. |

## Physics

- Tier: Light. Mass: 5 kg full, 1.2 kg empty, 20 kg for the stack.
- Collision: cylinder radius 9 cm × 20 cm tall. The bail is not collision.
- Center of mass: 9 cm above the base (full), 7 cm (empty).
- Friction 0.5, restitution 0.3 full, 0.5 empty (**PROPOSED**). Empty cans bounce and ring.
- Break-free: loose on floor 0 N. In a stack, the stack is one prop with 0 N; the cans separate on fracture.
- Haul acceleration with the far end static: 80 m/s² at 400 N (Climber, Scrapper), 120 at 600 N (Rigger), 160 at 800 N (Doubler), 200 at 1,000 N (Breaker). Distance to the 25 m/s Haul cap: 3.9 m, 2.6 m, 2.0 m, 1.6 m. A can is at the cap before the 0.4 s ramp finishes.
- Cut recoil: 150 N·s per 1,000 N to both ends. Cutting a Breaker's line seated in a full can gives the can 150 N·s, 30 m/s, and that is over its 120 N·s fracture value: the lid pops where it sits (**PROPOSED**: cut recoil counts against fracture thresholds). A Scrapper's line cut is 60 N·s, 12 m/s, and the can only jumps.
- Fracture: 120 N·s for a full can (**PROPOSED**): the lid pops and paint spills. Fragments: the can (1.2 kg Light) and the lid (0.2 kg Light), 10 s lifetime, plus a paint decal. The stack fractures into four cans at 200 N·s (**PROPOSED**).

What it does to a player:

| Delivery | Speed | Impulse | Result |
|---|---|---|---|
| Haul, any class | 25 m/s cap | 125 N·s | Nothing. The lid pops and the player is painted. |
| Scrapper Haul | 25 m/s cap | 125 N·s | Nothing |
| Jack | 30 m/s cap | 150 N·s | Nothing |
| Fall from 7 m | 11.7 m/s | 59 N·s | Nothing |
| Stack of 4 at Haul cap | 25 m/s | 500 N·s | Stagger (the stack fractures on arrival) |
| Doubler Snapback, full 1,200 N, single can | 50 m/s terminal | 250 N·s | Nothing |

A single can cannot stagger anyone. Stagger needs 300 N·s; a full can would need 60 m/s. The stack is the only version with a physical effect.

## Interactions

- Haul to it: it comes to you and bursts on your Catch Plate.
- Jack from it: the Kick (the Jack's 0.3 s capacitor discharge, separate from the motor pull T) drives you apart to the class launch speed V, split by inverse mass. A 90 kg Climber (V 18 m/s) sends the can off at 17.1 m/s and takes 0.9 m/s; a 110 kg Scrapper (V 14) sends it at 13.4 m/s and takes 0.6 m/s. You do not move in any way that matters. Then T pushes the can at 80 to 200 m/s² by class to the 30 m/s cap within 2 m (a Scrapper's 30% bonus, ×1.69 force and ×1.3 arrival speed, applies to this push and to the Haul, never to V; 05 §6.1) and it bursts on the first thing it hits. The Kick never fires with a Dog in a Catch Plate: a Jack from the can to an enemy is the T push alone, the can leaving from rest at 80 m/s² on a 400 N line and reaching the cap in 0.38 s over 5.6 m, while the enemy skids back at T over their mass.
- Hang from it: a Hang to a loose can brings it along as 5 kg of dead weight. Decorative.
- Object to object: a can into a propane cylinder at 25 m/s is an impact over 8 m/s and triggers it. This is the cheapest Kiss in the game and it should work. A can onto a market stall does nothing (125 N·s, under 300).
- Kinematic: a Hauler flattens it to a decal and the Empty prop. The Gantry load does the same.
- Water: a full can sinks; an empty can floats on its side and rides the Hold flow at surface speed.
- Updraft and Pulse: force volumes act on players and loose objects (break-free 0) at a_ref × 100 / m, with the factor capped at 2 per body (100 / m would be 20 for a full can and 83 for an empty one; both feel 2 times the authored acceleration), speeds capped by the 50 m/s hard limit: both lift and tumble on the first gust. A can hung on a player's Hang line is a free body and swings in the gust. The Pulse (0.4 s outward, then a 0.6 s inhale toward the Stump, every 75 s from 0:55) sends Office cans off the Scaffold in both directions; the ones that burst on the Rim leave yellow marks, which is a free map of where the Pulse throws things.
- Breathing tilt: rolls at 2° on steel. A can on the Bays' deck is the first thing that moves when the Breathing starts, which makes it the hull's clock for players who watch the floor.
- Chain reactions: knocks other cans. Nothing else.

## States

| State | Visual | Physics |
|---|---|---|
| Intact | Lid on, bail up | 5 kg |
| Dented | Side crease, bail bent flat | No change |
| Broken | Lid off, paint pooled, splash decal 60 cm | 1.2 kg Empty, lid 0.2 kg for 10 s |
| Destroyed | Flattened disc decal | Removed. Paint decal persists to round end. |

Paint on a player: a hit by a bursting can applies a `DECAL_Paint_<Color>` to the Catch Plate it struck, 40 cm, until the next Stall or round end. Cosmetic. Does not change Dog seating.

## VFX and SFX hooks

- Impact on steel: `SFX_Can_Steel` (tin clank, higher pitch when empty). Concrete: `SFX_Can_Concrete`. Wood: `SFX_Can_Wood`. Water: `SFX_Can_Plop`.
- Rolling: `SFX_Can_Roll` loop, pitch by angular speed. Empty cans ring; full cans slosh.
- Fracture: `SFX_Can_Lid_Pop`, `NS_Paint_Burst_<Color>` (40 drops, 0.5 s), `DECAL_Paint_Splash` 60 cm on the surface and 40 cm on any player within 1 m.
- Break-free lurch: none.
- No emissive. Office yellow is matte.

## Technical

- LOD0 800 tris (under budget), LOD1 400, LOD2 160, LOD3 48. One 1K set shared across all can variants; label from `T_Decal_Stencils_01`.
- Pivot: bottom center.
- Dog sockets: `SKT_Dog_Top_01` (lid center, Metal), `SKT_Dog_Handle_01` (bail crown, Metal), `SKT_Dog_Bot_01` (base center, Metal; it rolls, so the underside is reachable).
- Chaos: `GC_PaintCan_Full`. L0 whole, L1 can plus lid. Damage threshold L0 = 120 (1 unit = 1 N·s, **PROPOSED** calibration). `GC_PaintCan_Stack4`: L0 stack, L1 four full cans, L2 cans plus lids; thresholds L0 = 200, L1 = 120. No anchor field.
- Replication: RC-L. Paint decals are applied from the server fracture event so all clients agree on who is painted.

## Reference search terms

4 litre steel paint can wire bail dried drips; stacked paint tins market stall; paint can label torn rust ring; empty dented paint can; marking paint yellow black can; paint spill on concrete photo.
