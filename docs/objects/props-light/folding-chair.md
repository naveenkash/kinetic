# Folding chair

Light prop. Light tier, 6 kg. Folded 46 × 6 × 84 cm; open 46 × 50 × 80 cm, seat at 44 cm. Appears in Hangtown (the Mid Hang market platform, house porches, the Low Hang terraces), the Yard (the Floor, around the control shack and in scrap heaps), and Ballast (the Bays, in crew break corners). Not staged in the Anchor.

## Role in play

A chair is clutter with a job. It is the lightest furniture in the game and it cannot hurt anyone by itself. It exists to be in the way, to be the thing a Dog lands in by mistake, and to make a market platform read as a place people lived an hour ago.

- Named rigs: Clothesline bait (a chair kicked across a doorway makes a sprinter look down while the line waits at 1.2 m). Guillotine clutter (hung from a set line to hide the real load behind it). The Scatter, as a decoy: Jack a chair at an enemy to make them cut a line that was never a threat. With a Dog in their Catch Plate the Kick does not fire, so this is the T push alone: the chair leaves from rest at 67 m/s² on a Scrapper's 400 N and reaches the 30 m/s cap in 0.45 s over 6.7 m, while the enemy skids back at 400 N over their mass.
- Scrapper: Hauls it in at full speed as a distraction, then re-dogs to a barrel while the enemy flinches. Light props travel 30% faster on a Scrapper's line.
- Rigger: Hangs three off a set line over a doorway. The chairs are visible; the second set line at knee height is not.
- Breaker: ignores it. A Breaker who Dogs a chair has misread the room.
- Climber: perches on a stack of them for 1.5 s to break line of sight, then Slingshots.
- Doubler: uses a chair as the far Dog of a Snapback. At a full 1,200 N snap the chair receives 1,500 N·s, which would be 250 m/s on 6 kg; it is capped at the 50 m/s terminal velocity and arrives carrying 300 N·s, a stagger. The Doubler gets the 10 m/s launch.

## Appearance

Silhouette: an X of thin tube with a flat seat and a flat back. Reads as a chair at any distance. Folded, it reads as a plank with a bend.

| Part | Material | Base color | Roughness | Metallic |
|---|---|---|---|---|
| Frame tube (19 mm) | `MI_Steel_Painted_Worn` | `#5B6670` grey-blue, Hangtown variant `#2E8B8B` teal | 0.45 clean to 0.70 at the feet | 1 where chipped |
| Seat and back (plywood) | `MI_Plywood_Weathered` | `#9C8564` | 0.80 to 0.92 | 0 |
| Rivets and hinge pins | `MI_Steel_Bare` | `#5C5F63` | 0.55 | 1 |
| Foot caps | `MI_Rubber_Black` | `#1E1F20` | 0.80 | 0 |

Wear story: the top rail of the back is polished to bare steel where hands grab it to fold it. Paint is chipped at the four feet and along the seat edge. Rust (`MI_Steel_Rusted`, `#7A3F1C`) only in the hinge knuckles. The plywood seat has a darker ring where someone set a mug down for a year.

Stencils: Hangtown chairs carry a hand-painted house number (two digits, `MI_Hanger_Paint` mustard `#D9A441`) on the back. Yard chairs carry a Lay Guild Ledger stencil `LG-0412` in ochre on the underside of the seat. Ballast chairs are HTR break-room issue: `HTR-RC-7731` in `MI_Decal_Stencil` orange on the rear leg.

Under arena lighting: the plywood is a mid-grey. The chair reads by its shadow more than its color. In the Anchor it is absent. In Hangtown it is the most colorful object in the frame and the teal is tuned to sit below the line glow.

## Variants

| Variant | Difference |
|---|---|
| `Open` | Default. Stands on four feet. |
| `Folded` | Flat. Leans against a wall or lies in a heap. Smaller collision. |
| `Hanger_Teal` | Teal frame, house number, one broken slat. Hangtown only. |
| `HTR_Grey` | Grey frame, orange tag, no wear above 0.3. Ballast only. |
| `Broken` | Missing back panel, one leg bent 20°. Scrap heaps. |

## Physics

- Tier: Light. Mass: 6 kg.
- Collision: open state, two boxes (seat 46 × 50 × 4 cm, back 46 × 4 × 36 cm) and four capsules for legs (radius 1.5 cm). Folded state, one box 46 × 6 × 84 cm.
- Center of mass: 32 cm above the floor, 2 cm behind the seat center (open).
- Friction 0.6, restitution 0.25 (**PROPOSED**).
- Break-free: loose on floor, 0 N. Any line moves it instantly.
- Haul acceleration with the far end static: 67 m/s² at 400 N (Climber, Scrapper), 100 at 600 N (Rigger), 133 at 800 N (Doubler), 167 at 1,000 N (Breaker). Distance to the 25 m/s Haul cap: 4.7 m, 3.1 m, 2.3 m, 1.9 m. Past that it coasts at the cap.
- Cut recoil: cutting a line delivers 150 N·s per 1,000 N of force to both ends. A Breaker's line cut while seated in a chair gives the chair 150 N·s, a 25 m/s jump; a Scrapper's line, 60 N·s, 10 m/s. Light props twitch when their line is cut.
- Fracture: 300 N·s (05 §6.5, furniture). Fragments: 2 Light planks (seat, back) and the frame as one Light piece. Planks live 10 s. A player's body counts against the threshold: a flipped 130 kg Rigger landing on a chair at 5 m/s is 650 N·s and breaks it.

What it does to a player:

| Delivery | Speed | Impulse | Result |
|---|---|---|---|
| Haul, any class | 25 m/s cap | 150 N·s | Nothing |
| Scrapper Haul | 25 m/s cap (30% faster to reach it) | 150 N·s | Nothing |
| Jack | 30 m/s cap | 180 N·s | Nothing |
| Fall from 7 m | 11.7 m/s | 70 N·s | Nothing |
| Doubler Snapback, full 1,200 N | 50 m/s terminal | 300 N·s | Stagger, at threshold |

On a line, a chair cannot stagger a player. Stagger needs 300 N·s; a 6 kg chair would need 50 m/s, which only the terminal-velocity cap of a Snapback reaches. It is a tell and a tangle.

## Interactions

- Haul to it as a player: the chair comes to you. You are not pulled.
- Jack from it: the Kick (the Jack's 0.3 s capacitor discharge, separate from the motor pull T) drives you and the chair apart to the class launch speed V, split by inverse mass. A 110 kg Scrapper (V 14 m/s) sends the chair off at 13.3 m/s and moves 0.7 m/s backward. A 90 kg Climber (V 18 m/s) sends it at 16.9 m/s and takes 1.1 m/s. Then T pushes the chair at 67 to 167 m/s² by class to the 30 m/s cap (a Scrapper's 30% bonus, ×1.69 force and ×1.3 arrival speed, applies to this push and to the Haul, never to V; 05 §6.1); the player gets nothing more worth counting. The Kick never fires with a Dog in a Catch Plate: a Jack from the chair to an enemy is the T push alone, the chair leaving from rest at 67 m/s² on a 400 N line and reaching the cap in 0.45 s over 6.7 m (0.18 s and 2.7 m on a Breaker's 1,000 N), while the enemy skids back at T over their mass. A chair is a Slingshot anchor only if it is wedged; a wedged chair is static and gives you the whole of V.
- Hang from it: a chair is loose on the floor, so a Hang to it brings it along as 6 kg of dead weight. It swings.
- Object to object: it fractures when a Medium object lands on it (a 120 kg cart at 2.5 m/s is 300 N·s). It does not move anything heavier than itself.
- Kinematic: a Hauler flattens it (fracture). The Gantry load crushes it. The Lift counterweight sweeps it off the Mid Hang.
- Water: it sinks slowly, plywood up, then rests. In Ballast's Hold a chair drifts 0.5 m/s with the tilt-driven water volume.
- Updraft and Pulse: not staged in the Anchor. If one gets there, force volumes act on players and loose objects (break-free 0) at a_ref × 100 / m, with the factor capped at 2 per body (100 / 6 kg would be 16.7, so a chair feels 2 times the authored acceleration), speeds capped by the 50 m/s hard limit: it leaves the floor on the first gust and tumbles. A chair hung on a player's Hang line is a free body and swings in the gust. The Pulse (0.4 s outward, then a 0.6 s inhale toward the Stump, every 75 s from 0:55) throws it out and draws it back; a chair on the inner Scaffold edge goes into the Well on the inhale.
- Breathing tilt: slides at 6° on steel. Open chairs tip over at 4° if facing the lean.
- Chain reactions: knocks a paint can, does not knock a barrel.

## States

| State | Visual | Physics |
|---|---|---|
| Intact | As described | 6 kg body |
| Dented | Back panel cracked, one leg bent, sits crooked | No change. Material swap only |
| Broken | Seat plank detached | Frame 3 kg, seat plank 1.5 kg, back plank 1.5 kg; planks are Light props for 10 s |
| Destroyed | Frame flattened, planks gone | Frame remains as a 3 kg flat prop until the round resets |

## VFX and SFX hooks

- Impact on steel: `SFX_Chair_Steel` (hollow tube clang, 2 layers). Concrete: `SFX_Chair_Concrete` (clatter, then a scrape). Wood: `SFX_Chair_Wood` (dull knock). Water: `SFX_Prop_Splash_S`.
- Break-free lurch: none. Break-free is 0.
- Fracture: `SFX_Plywood_Crack_S`, `NS_Fracture_Wood_S` (12 splinters, 1.5 s).
- Tumble: `SFX_Chair_Tumble` loop while angular speed is over 3 rad/s.
- No emissive.

## Technical

- LOD0 2,000 tris, LOD1 1,000, LOD2 400, LOD3 120. One 1K set (BC, N, ORM). Stencils from `T_Decal_Stencils_01`.
- Pivot: bottom center of the open footprint. X+ faces forward (toward the sitter's knees).
- Dog sockets: `SKT_Dog_Top_01` (top rail of the back, Metal), `SKT_Dog_Top_02` (seat center, Wood), `SKT_Dog_Frame_01` (front cross brace, Metal).
- Chaos: `GC_FoldingChair_Open`. L0 whole; L1 frame, seat plank, back plank; no L2. Damage threshold L0 = 300 (1 unit = 1 N·s, **PROPOSED** calibration). No anchor field; loose on floor.
- Replication: RC-L. Client predicted within 10 m of the local player or on their line; server correction at 10 Hz; sleeps after 3 s at rest.

## Reference search terms

Steel folding chair tubular plywood seat vintage; Samsonite folding chair 1960s rust; stacked folding chairs warehouse; folding chair bent leg junkyard; chair hand-painted number back market stall.
