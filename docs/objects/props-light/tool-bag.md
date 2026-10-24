# Tool bag

Light prop. Light tier, 20 kg. 55 × 30 × 32 cm open-top canvas rigger's bag with two steel-reinforced handles. Appears in the Yard (the Floor, at the base of every floodlight mast and in the control shack; the Rails, on the crane cab step), Ballast (the Bays, on catwalk landings; the Crown, beside HTR's cutting gantry), the Anchor (the Scaffold, one per level per quadrant), and Hangtown (the Mid Hang, on porches). The most common prop in the game after the plank.

## Role in play

The tool bag is dense and soft. It staggers at the Haul cap and flips exactly at the Jack cap, and it lands on a player like a sack rather than a blade. It is also the arena's way of saying a crew was here five minutes ago.

- Named rigs: the Guillotine, as a decoy load or as padding on a real one. The Clothesline, dropped at the foot of the line so a sprinter who crouches hits the bag. The Scatter: two bags Jacked into two faces is two staggers at once.
- Scrapper: Hauls a bag into an enemy's chest to stagger them, then Hauls the barrel behind them. Light props travel 30% faster on a Scrapper line; the 25 m/s cap holds.
- Rigger: Hangs a bag from a set line at head height in a dark corner. It swings. Players flinch from it and walk into the real line.
- Breaker: steps over it.
- Climber: Jacks off a wall with a bag on the far Dog to add 20 kg of counter-mass to the launch (a self-Dog to a bag is a weak Slingshot, 20 kg against 90).
- Doubler: a bag on the far Dog of a full 1,200 N Snapback receives 1,500 N·s: 75 m/s on 20 kg, capped at the 50 m/s terminal velocity, arriving with 1,000 N·s. A hard flip and a shower of tools, not a Stall.

## Appearance

Silhouette: a loaf with two loops on top. Sags toward whichever side holds the heavy tools. Handles stand up. Tool handles (hammer, bar, spanner) stick out the top 10 to 20 cm.

| Part | Material | Base color | Roughness | Metallic |
|---|---|---|---|---|
| Body (18 oz canvas) | `MI_Canvas_Natural` | `#B9AE93`, Guild; `#4A4D50` HTR grey | 0.90 to 1.00 | 0 |
| Base (rubber) | `MI_Rubber_Black` | `#1E1F20` | 0.80 | 0 |
| Handles (steel core, leather wrap) | `MI_Steel_Bare` core, `MI_Timber_Dark` wrap stand-in | `#5C5F63`, `#4A3522` | 0.5, 0.75 | 1, 0 |
| Tool handles | `MI_Timber_Rough`, `MI_Steel_Painted_Worn` | `#7A5C3A`, `#E8620C` or `#C8912A` | 0.8, 0.5 | 0, 1 chipped |
| Rivets | `MI_Steel_Bare` | `#5C5F63` | 0.55 | 1 |

Wear story: the handles are black with hand grease. The canvas is dark along the top edge where it is grabbed and pale on the sides where the sun gets it. The base is scuffed grey. One corner is patched with `MI_Tarp_Blue` and sisal stitching on Guild and Hanger bags.

Stencils: Guild bags carry the hitch-knot glyph and an owner's Ledger number (`LG-1183`) in ochre on the end panel, plus a hand-written name on a taped strip. HTR bags are grey with `HTR` block letters in orange and a tool-crib number `CRIB 04`. Anchor bags are Guild. Hangtown bags are repurposed grain sacks with the sack's original print half visible.

Under arena lighting: natural canvas is one of the brighter non-emissive values in the Yard and Ballast. HTR grey bags disappear in the Yard's shadows; the orange letters do not. In Hangtown they read as another bundle on a porch.

## Variants

| Variant | Difference |
|---|---|
| `Guild` | Natural canvas, ochre glyph, name tape, tools with ochre handles. |
| `HTR` | Grey canvas, orange letters, clean. Tools orange. |
| `Hanger` | Grain sack, rope handles, mixed tools. 16 kg. |
| `Empty` | Slumped, 3 kg. Scrap heaps. Does not stagger. |
| `Spilled` | Bag on its side with tools out as a decal and three Light tool props (hammer 1.2 kg, bar 3 kg, spanner 0.8 kg). |

## Physics

- Tier: Light. Mass: 20 kg (Hanger 16 kg, Empty 3 kg).
- Collision: one box 55 × 30 × 32 cm with 4 cm chamfers; the handles are not collision.
- Center of mass: 12 cm above the base, offset 5 cm toward the heavy end.
- Friction 0.8, restitution 0.05 (**PROPOSED**). A bag lands and stays.
- Break-free: loose on floor 0 N.
- Haul acceleration with the far end static: 20 m/s² at 400 N (Climber, Scrapper), 30 at 600 N (Rigger), 40 at 800 N (Doubler), 50 at 1,000 N (Breaker). Distance to the 25 m/s Haul cap: 15.6 m, 10.4 m, 7.8 m, 6.3 m. A Scrapper's 20 m line reaches the cap at 15.6 m without the bonus and at 9.2 m with the 30% Light-prop bonus (×1.69 force, ×1.3 arrival speed; 05 §6.1), which pulls at 33.8 m/s².
- Cut recoil: 150 N·s per 1,000 N to both ends. Cutting a Breaker's line on a bag gives it 150 N·s, 7.5 m/s; it slumps over.
- Fracture: 500 N·s (**PROPOSED**). The bag does not break apart; it bursts and spills. A player's body counts against the threshold: a flipped 130 kg Rigger landing on a bag at 5 m/s is 650 N·s and bursts it. Fragments: the bag body (14 kg Light) and three Light tools, 10 s lifetime.

What it does to a player:

| Delivery | Speed | Impulse | Result |
|---|---|---|---|
| Haul, any class | 25 m/s cap | 500 N·s | Stagger |
| Scrapper Haul | 25 m/s cap | 500 N·s | Stagger |
| Jack | 30 m/s cap | 600 N·s | Flip, exactly at threshold |
| Fall from 7 m | 11.7 m/s | 234 N·s | Nothing |
| Doubler Snapback, full 1,200 N | 50 m/s terminal | 1,000 N·s | Flip |

A tool bag cannot Stall anyone. 1,200 N·s needs 60 m/s and nothing falls faster than 50. The Jack flip is the reason to use it: a flipped player ragdolls with momentum preserved, which next to a trench is a ring-out.

## Interactions

- Haul to it: it comes to you and stops against you.
- Jack from it: the Kick (the Jack's 0.3 s capacitor discharge, separate from the motor pull T) drives you apart to the class launch speed V, split by inverse mass. A 90 kg Climber (V 18 m/s) sends the bag off at 14.7 m/s and takes 3.3 m/s backward, a short hop; a 110 kg Scrapper (V 14) sends it at 11.8 m/s and takes 2.2 m/s; a 220 kg Breaker (V 10) sends it at 9.2 m/s and takes 0.8 m/s. Then T pushes the bag at 20 to 50 m/s² by class to the 30 m/s cap (a Scrapper's 30% bonus, ×1.69 force and ×1.3 arrival speed, applies to this push and to the Haul, never to V; 05 §6.1). The Kick never fires with a Dog in a Catch Plate: a Jack from the bag to an enemy is the T push alone, the bag leaving from rest at 20 m/s² on a 400 N line and reaching 25 m/s at 15.6 m, the cap only on a line longer than 22.5 m, while the enemy skids back at T over their mass.
- Hang from it: a bag on a set line is a 200 N pendulum weight that swings for 10 s before friction settles it. A Hang from a player to a loose bag brings it along as 20 kg of dead weight.
- Object to object: a bag Hauled into a propane cylinder at 25 m/s triggers it (impact over 8 m/s). A bag onto a market stall at 25 m/s is 500 N·s, fracture. A bag landing on a scrap heap adds to the heap.
- Kinematic: a Hauler shoves it along the trench floor; the Gantry load flattens it to the Spilled variant.
- Water: a bag sinks in 2 s (tools), handles up. In Ballast's Hold it drifts on the bottom with the tilt flow.
- Updraft and Pulse: force volumes act on players and loose objects (break-free 0) at a_ref × 100 / m, with the factor capped at 2 per body (100 / 20 kg would be 5, so a bag feels 2 times the authored acceleration), speeds capped by the 50 m/s hard limit: it lifts on the Breath where a Climber does. A bag hung on a player's Hang line is a free body and swings in the gust. The Pulse (0.4 s outward, then a 0.6 s inhale toward the Stump, every 75 s from 0:55) shoves a bag at the inner Scaffold edge outward and then draws it back over the edge into the Well.
- Breathing tilt: friction 0.8 holds it to 6°. Bags do not slide on the Breathing. They are the one prop that stays put, which makes them useful markers.
- Chain reactions: a bag bursts over a scrap heap and adds three tools to the heap's pool.

## States

| State | Visual | Physics |
|---|---|---|
| Intact | Upright, tools showing | 20 kg |
| Dented | On its side, one handle torn | No change |
| Broken | Burst along a seam, tools out | 14 kg body plus 3 Light tools for 10 s |
| Destroyed | Flat canvas | 3 kg Empty variant |

## VFX and SFX hooks

- Impact on steel: `SFX_Bag_Steel` (dull thud with a metal rattle inside). Concrete: `SFX_Bag_Concrete` (thud, no ring). Wood: `SFX_Bag_Wood`. Water: `SFX_Bag_Splash_Sink`.
- Impact on a player: `SFX_Bag_Plate` plus `SFX_Tools_Rattle`.
- Fracture: `SFX_Canvas_Tear`, `NS_Spill_Tools` (12 small tool meshes as particles, 1 s, then the three real props).
- Break-free lurch: none.
- No emissive. The HTR orange letters are matte.

## Technical

- LOD0 2,000 tris, LOD1 1,000, LOD2 400, LOD3 120. One 1K set; tool handles share `T_Trim_Steel_01`.
- Pivot: bottom center. X+ toward the heavy end.
- Dog sockets: `SKT_Dog_Handle_01` (left handle crown, Metal), `SKT_Dog_Handle_02` (right handle crown, Metal), `SKT_Dog_Top_01` (canvas between the handles, Fabric), `SKT_Dog_SideE_01` (end panel with the stencil, Fabric). Mag Dogs hold full load only in the handles.
- Chaos: `GC_ToolBag`. L0 whole, L1 body plus three tools. Damage threshold L0 = 500 (1 unit = 1 N·s, **PROPOSED** calibration). No anchor field.
- Replication: RC-L.

## Reference search terms

Open top canvas rigger tool bag steel handles; Klein canvas tool bag worn; ironworker tool bag with spud wrench and bull pin; grain sack repurposed bag; tool bag on scaffold deck; tool bag grease-black handles.
