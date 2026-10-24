# Bucket

Light prop. Light tier, 10 kg full of water, 2 kg empty. 30 cm top diameter, 26 cm base, 36 cm tall, 15 L HDPE bucket with a wire bail and plastic grip. Appears in Hangtown (the Low Hang garden terraces, the Mid Hang porches, under every roof water tank), Ballast (the Hold, floating in the seep water; the Bays, on catwalk landings), the Yard (the Floor, at the control shack and beside the welding cart as a quench bucket), and the Anchor (the Scaffold, one per level as a prep-crew wash bucket).

## Role in play

A bucket is a prop that changes mass when it tips. Full, it just reaches stagger at the Jack cap. Empty, it rolls, rings, and does nothing. Its real job is the spill: a tipped bucket leaves a wet patch that lowers friction, and the splash is a tell that a line hit something at the base of a wall.

- Named rigs: the Clothesline, as the thing on the floor under the line that a crouching player knocks over and slips on. The Scatter, decoy half: a Jacked bucket spraying water reads as a volatile vent for a quarter second. Bridge play: a wet patch on a plank bridge drops its friction.
- Scrapper: Hauls a full bucket into an enemy's chest at the Haul cap for a near-stagger and a face full of water, then re-dogs. Light props travel 30% faster on a Scrapper line; the 25 m/s cap holds.
- Rigger: hangs a full bucket from a set line over a doorway. When the line expires it drops, spills, and the doorway is wet for the next 20 s.
- Breaker: kicks it over on purpose. A wet steel floor under a braced Breaker does not matter to the Breaker; it matters to whoever tries to stand against the Breaker's Haul.
- Climber: a full bucket on a self-Dog Jack is a 10 kg counter-mass, which is nothing. Climbers use buckets as perch markers.
- Doubler: a bucket on a Snapback arrives empty: the water leaves at the snap. The 2 kg shell at the 50 m/s terminal velocity carries 100 N·s.

## Appearance

Silhouette: a truncated cone with a wire arc. Full, it sits flat and heavy; empty, it is usually on its side.

| Part | Material | Base color | Roughness | Metallic |
|---|---|---|---|---|
| Body (HDPE) | `MI_HDPE_Blue` | `#1F4FA3`; Hangtown variant `#D9A441` mustard; HTR variant `#4A4D50` grey | 0.40 to 0.55 | 0 |
| Bail (wire) | `MI_Steel_Galvanized` | `#9EA3A6` | 0.40 | 1 |
| Grip (plastic) | `MI_HDPE_Black` | `#232426` | 0.50 | 0 |
| Water surface (full) | translucent water material, depth-tinted | `#3A5F6E` | 0.05 | 0 |
| Scale and lime line inside | `MI_Decal_Grime` | `#D8D4C8` | 0.9 | 0 |

Wear story: subsurface 0.15 on the HDPE so the water level reads as a darker band from outside. The rim is scuffed white where it has been dragged. The grip is cracked. A dried tide line inside. In the Yard the quench bucket is black with slag and the water has an oil sheen. In Ballast the Hold buckets have algae (`#3A4A3C` grime) to the water line.

Stencils: Yard buckets carry a Guild stores stencil `LG STORES` in ochre on the side. HTR buckets carry `HTR-RC` and `WASH ONLY` in orange. Hangtown buckets have a hanger stencil (a kettle) or a child's name in paint marker. Anchor buckets are Office-issue grey with `OFFICE OF THE LAY` in white.

Under arena lighting: a blue bucket is a saturated point in otherwise neutral scenes. In the Anchor the Office grey version disappears against concrete, which is correct; the wet patch it leaves is the read.

## Variants

| Variant | Mass | Difference |
|---|---|---|
| `Full_Water` | 10 kg | Default. Water surface sloshes with motion. |
| `Empty` | 2 kg | Dry inside. Rolls on its side. Rings. |
| `Full_Sand` | 14 kg | Sand instead of water. Does not spill; dumps a sand decal on fracture. Yard fire station. |
| `Quench` | 10 kg | Black water, oil sheen, slag on the rim. Beside the welding cart. |
| `Hanger_Mustard` | 10 kg | Mustard body, painted name. |

## Physics

- Tier: Light. Mass: 10 kg full, 2 kg empty, 14 kg sand.
- Collision: a convex truncated cone, 30 cm top, 26 cm base, 36 cm tall. The bail is not collision.
- Center of mass: 15 cm above the base when full, 10 cm when empty.
- Friction 0.5, restitution 0.2 full, 0.4 empty (**PROPOSED**).
- Tip angle: a full bucket tips past 32° of lean (**PROPOSED**). On tipping, the water leaves over 0.5 s and the bucket becomes the Empty variant.
- Break-free: loose on floor 0 N.
- Haul acceleration, full, with the far end static: 40 m/s² at 400 N (Climber, Scrapper), 60 at 600 N (Rigger), 80 at 800 N (Doubler), 100 at 1,000 N (Breaker). Distance to the 25 m/s Haul cap: 7.8 m, 5.2 m, 3.9 m, 3.1 m. The bucket tips past 32° in the first 0.2 s of any Haul, so a full bucket never arrives full.
- Cut recoil: 150 N·s per 1,000 N to both ends. Cutting a Breaker's line on a full bucket gives it 150 N·s, 15 m/s; it tips and spills where it stands.
- Fracture: 350 N·s (**PROPOSED**): the bucket splits and the water leaves at once. Fragments: two 1 kg HDPE halves, 10 s lifetime, plus the spill.

What it does to a player:

| Delivery | Speed | Impulse | Result |
|---|---|---|---|
| Haul, full | 25 m/s cap | 250 N·s | Nothing. Soaks the player. |
| Scrapper Haul, full | 25 m/s cap | 250 N·s | Nothing |
| Jack, full | 30 m/s cap | 300 N·s | Stagger, exactly at threshold |
| Fall from 7 m, full | 11.7 m/s | 117 N·s | Nothing |
| Any delivery, empty | 30 m/s | 60 N·s | Nothing |
| Doubler Snapback, full 1,200 N, shell | 50 m/s terminal | 100 N·s | Nothing |

A bucket cannot flip or Stall a player. It is the spill, the splash, and the tell.

The wet patch: a 1.2 m radius decal where a bucket spills. Friction under players on the patch is 0.3 instead of the surface default for 20 s (**PROPOSED**). A player hit by a line while on a wet patch has their effective foot-hold reduced by 20% for the duration (**PROPOSED**; it makes the Clothesline and the Haul both better on wet floor). The patch does not change Stall thresholds.

## Interactions

- Haul to it: it comes to you, tips in flight, and arrives empty. You are wet.
- Jack from it: the Kick (the Jack's 0.3 s capacitor discharge, separate from the motor pull T) drives you apart to the class launch speed V, split by inverse mass. A 90 kg Climber (V 18 m/s) sends a full bucket off at 16.2 m/s and takes 1.8 m/s; a 110 kg Scrapper (V 14) sends it at 12.8 m/s and takes 1.2 m/s. Then T pushes the bucket at 40 to 100 m/s² by class to the 30 m/s cap (a Scrapper's 30% bonus, ×1.69 force and ×1.3 arrival speed, applies to this push and to the Haul, never to V; 05 §6.1), spilling as it goes, and it arrives empty. The Kick never fires with a Dog in a Catch Plate: a Jack from the bucket to an enemy is the T push alone, the bucket leaving from rest at 40 m/s² on a 400 N line and reaching the cap in 0.75 s over 11.3 m, while the enemy skids back at T over their mass.
- Hang from it: a full bucket on a set line is a 100 N pendulum weight. When the set line expires the bucket falls and spills under the doorway. A Hang from a player to a loose bucket brings it along as dead weight, tipped and empty.
- Object to object: a full bucket into a propane cylinder at 25 m/s is an impact over 8 m/s and triggers it. A bucket's spill under a steel crate does nothing to the crate. A spill on a plank bridge drops plank friction to 0.3 for 20 s.
- Kinematic: a Hauler flattens it. The Gantry's magnet ignores HDPE.
- Water: empty, it floats upright until it takes water over the rim, then sinks. Full, it sinks. In Ballast's Hold buckets ride the tilt-driven flow at the surface and bump against the nose bulkhead with the lean.
- Updraft and Pulse: force volumes act on players and loose objects (break-free 0) at a_ref × 100 / m, with the factor capped at 2 per body (100 / m would be 10 for a full bucket and 50 for an empty one; both feel 2 times the authored acceleration), speeds capped by the 50 m/s hard limit: a full bucket lifts, tips past 32° in the air, and spills on the Scaffold planks below; the empty shell tumbles on. The Pulse (0.4 s outward, then a 0.6 s inhale toward the Stump, every 75 s from 0:55) spills every full bucket on the Scaffold, which is why the prep-crew wash buckets are a once-per-Pulse wet patch. A bucket hung on a player's Hang line is a free body and swings in the gust, spilling.
- Breathing tilt: a full bucket slides at 5° and tips at 6° on the lean, which is exactly the hull's maximum. On every Breathing cycle, every full bucket in the Bays tips at the peak. This is intended: it is a metronome.
- Chain reactions: a spilled bucket under a scrap heap does nothing. A spilled bucket under a market stall's feet does nothing to the stall.

## States

| State | Visual | Physics |
|---|---|---|
| Intact | Full, water surface visible | 10 kg |
| Dented | Rim crushed oval, grip missing | No change |
| Broken | Tipped, empty, on its side | 2 kg Empty variant plus wet patch |
| Destroyed | Split in two | Two 1 kg halves, 10 s; wet patch |

## VFX and SFX hooks

- Impact on steel: `SFX_Bucket_Steel` (plastic clonk, water slosh layer when full). Concrete: `SFX_Bucket_Concrete`. Wood: `SFX_Bucket_Wood`. Water: `SFX_Bucket_Splash`.
- Tip and spill: `SFX_Water_Spill_15L`, `NS_Water_Spill_Bucket` (fluid sheet, 0.5 s, then `DECAL_Wet_Patch` 1.2 m with a 20 s fade).
- Rolling empty: `SFX_Bucket_Roll` loop, hollow plastic.
- Player hit by a full bucket: `SFX_Bucket_Plate` plus `NS_Water_Splash_Player` and a 0.3 s water-on-lens overlay (cosmetic).
- Fracture: `SFX_HDPE_Crack`.
- No emissive.

## Technical

- LOD0 1,200 tris, LOD1 600, LOD2 240, LOD3 64. One 1K set shared across all bucket variants; water surface is a separate material on an inner cap mesh.
- Pivot: bottom center.
- Dog sockets: `SKT_Dog_Handle_01` (bail crown, Metal), `SKT_Dog_Rim_01` (rim at X+, the usual Haul grab, hits the Dog and tips the bucket), `SKT_Dog_Bot_01` (base center, Fabric-class surface override to plastic; Mag Dogs do not hold on HDPE). Surface class for the body is `Fabric` as the nearest plastic stand-in: Standard and Barb hold, Mag and Spike slide (**PROPOSED** rule for plastics).
- Chaos: `GC_Bucket`. L0 whole, L1 two halves. Damage threshold L0 = 350 (1 unit = 1 N·s, **PROPOSED** calibration). No anchor field. The tip-to-empty state swap is a Blueprint on the lean angle, not a fracture.
- Replication: RC-L. The spill event and wet patch are server-authoritative so friction agrees across clients.

## Reference search terms

15 litre blue plastic bucket wire bail scuffed; builder's bucket full of water tipped; quench bucket welding shop slag; bucket on scaffold deck; bucket under leaking roof tank shanty; wet floor spill on steel plate photo.
