# Plank

Light prop. Light tier, 12 kg. 240 × 23 × 4 cm sawn softwood scaffold board. Appears in the Anchor (the Scaffold, as every walking surface, and as fragments after Scaffold Collapse), the Yard (the Floor, as trench bridges and on scrap heaps), and Hangtown (the Mid Hang, as swing-bridge decking and stall shelves; the Low Hang, as garden terrace edging). In Ballast only as a loose few in the Bays.

## Role in play

The plank is the unit of walking surface on the Lay. Where there is a plank there was a person. It is also the fragment that every piece of furniture and scaffolding breaks into, so the player meets more planks than any other prop. It hits the stagger line exactly at the Haul cap and never more.

- Named rigs: the Clothesline, as the thing a sprinter trips on before the line. The Guillotine, as a decoy load. Bridge play: a plank over a trench is a floor; a Dog in it and a Haul removes the floor.
- Scrapper: Hauls a plank out from under a player crossing a trench. The plank goes; the player drops. Light props travel 30% faster on a Scrapper line; the 25 m/s cap holds.
- Rigger: Hangs a plank at 1.0 m across a doorway on a set line. Players who crouch under the line walk into the plank.
- Breaker: ignores single planks. Pulls a stack (see the scaffold board stack) instead.
- Climber: a plank in the Updraft is a kite. Climbers Dog one, Hang, and ride the gust 3 m higher than the Jack alone.
- Doubler: a plank on the far Dog of a full 1,200 N Snapback receives 1,500 N·s: 125 m/s on 12 kg, capped at the 50 m/s terminal velocity, arriving with 600 N·s, a flip.

## Appearance

Silhouette: a long thin rectangle. Reads as wood by color before shape.

| Part | Material | Base color | Roughness | Metallic |
|---|---|---|---|---|
| Board faces | `MI_Timber_Rough` | `#7A5C3A` | 0.78 to 0.90 | 0 |
| Board faces, grey weathered top | `MI_Plywood_Weathered` blend | `#9C8564` | 0.90 | 0 |
| End bands (galvanized steel, 2) | `MI_Steel_Galvanized` | `#9EA3A6` | 0.40 | 1 |
| Nail heads | `MI_Steel_Rusted` | `#7A3F1C` | 0.85 | 0.3 |

Wear story: the top face is silvered by sun and polished dark along the center line where boots walk. The underside is still brown. Ends are split 5 to 10 cm back from the band. Boot prints in grey concrete dust in the Anchor; black oil in the Yard; chalk marks in Hangtown.

Stencils: the Anchor's planks carry a burned-in Guild Ledger brand `LG` and a batch number on one end band. Yard planks are branded `C4PW` (Collar Four Press Works, **PROPOSED** supplier). Hangtown planks are unbranded and some are painted a hanger color (`#2E8B8B` teal, `#B5302A` red) on one face.

Under arena lighting: the grey top face is a mid-value neutral. In the Anchor, concrete dust makes planks and floor the same value, so the end bands are the read. In Hangtown the painted faces are the brightest wood in frame.

## Variants

| Variant | Difference |
|---|---|
| `Standard` | 240 cm, two end bands. |
| `Short` | 120 cm, 6 kg. Stall shelves, terrace edging. |
| `Painted_Hanger` | One face in hanger paint. Hangtown. |
| `Broken` | Snapped at 150 cm, splintered end. Scrap heaps, Collapse debris. |
| `Bridge` | Standard with a nailed cleat at each end. Trench bridges. Placed in a cradle. |

## Physics

- Tier: Light. Mass: 12 kg (Short 6 kg).
- Collision: one box 240 × 23 × 4 cm.
- Center of mass: geometric center.
- Friction 0.55 on steel, 0.65 on concrete; restitution 0.15 (**PROPOSED**).
- Break-free: loose on floor 0 N. `Bridge` variant seated in trench cleats counts as "in a rack or cradle," 200 N: any class pulls a bridge.
- Haul acceleration with the far end static: 33 m/s² at 400 N (Climber, Scrapper), 50 at 600 N (Rigger), 67 at 800 N (Doubler), 83 at 1,000 N (Breaker). Distance to the 25 m/s Haul cap: 9.5 m, 6.3 m, 4.7 m, 3.8 m.
- Cut recoil: 150 N·s per 1,000 N to both ends. Cutting a Breaker's line on a plank gives it 150 N·s, 12.5 m/s; it skates across the floor.
- Fracture: 240 N·s (**PROPOSED**, under the 300 N·s furniture value because a single board is weaker than a stall). Fragments: 2 Light half-planks, 10 s lifetime.

What it does to a player:

| Delivery | Speed | Impulse | Result |
|---|---|---|---|
| Haul, any class | 25 m/s cap | 300 N·s | Stagger, exactly at threshold |
| Scrapper Haul | 25 m/s cap | 300 N·s | Stagger |
| Jack | 30 m/s cap | 360 N·s | Stagger |
| Fall from 7 m | 11.7 m/s | 140 N·s | Nothing |
| Doubler Snapback, full 1,200 N | 50 m/s terminal | 600 N·s | Flip, at threshold |

On a line a plank cannot flip or Stall. 05 §7.1 notes a 12 kg plank would need 100 m/s to Stall. Its job is the trip, the floor that leaves, and the ×1.2 wood surface a player lands on.

## Interactions

- Haul to it: it comes to you flat or spinning. End-on, the 23 cm face is hard to dodge.
- Jack from it: the Kick (the Jack's 0.3 s capacitor discharge, separate from the motor pull T) drives you apart to the class launch speed V, split by inverse mass. A 90 kg Climber (V 18 m/s) sends the plank off at 15.9 m/s and takes 2.1 m/s backward; a 110 kg Scrapper (V 14) sends it at 12.6 m/s and takes 1.4 m/s. Then T pushes the plank at 33 to 83 m/s² by class to the 30 m/s cap (a Scrapper's 30% bonus, ×1.69 force and ×1.3 arrival speed, applies to this push and to the Haul, never to V; 05 §6.1). The Kick never fires with a Dog in a Catch Plate: a Jack from the plank to an enemy is the T push alone, the plank leaving from rest at 33 m/s² on a 400 N line and reaching the cap in 0.9 s over 13.6 m, while the enemy skids back at T over their mass. A plank is not a Slingshot anchor unless it is wedged in a cradle, and a cradled bridge plank releases at 200 N, so it is not one either.
- Hang from it: break-free is tested against active pull only (Haul, Jack, and the Kick), so a Bridge plank's 200 N cradle value does not apply to a hanging player. The cleats bear down into the trench lip, so a Bridge plank holds a Pendulum from any harness up to the class Hang load (**PROPOSED** physical behaviour): a Climber can Hang from the underside of a bridge and swing the trench. A Haul or Jack on the same plank still pulls it out at 200 N. A plank loose on a floor comes along as 12 kg of dead weight.
- Object to object: a steel crate at 1 m/s onto a plank is 250 N·s, fracture. A plank Hauled into a propane cylinder at 25 m/s triggers it (impact over 8 m/s).
- Kinematic: a Hauler snaps bridge planks across a trench (03 Yard, "plank bridges"); they fracture and drop. A Hauler passing through a Hang line severs it (05 §6.7: the snap, 150 N·s per 1,000 N recoil to both ends, both Dogs release), so planks laid across a Hang-line footbridge drop with the line; lines attached to the Hauler itself are not severed. A plank bridge on a Hang line fractures at 600 N·s and a walkway on cable at 2,500 N·s (05 §6.5, canon); a single loose plank keeps its 240 N·s (**PROPOSED**). The Gantry load crushes planks. The Maintenance Ring sweeps loose planks off the Scaffold and severs any Clothesline it passes through.
- Water: floats, flat. In Ballast's Hold it rides the tilt-driven flow at the surface speed and becomes a moving ×1.2 surface.
- Updraft and Pulse: force volumes act on players and loose objects (break-free 0) at a_ref × 100 / m, with the factor capped at 2 per body (100 / 12 kg would be 8.3, so a loose plank feels 2 times the authored acceleration), speeds capped by the 50 m/s hard limit: it lifts, tumbles, and drifts on every gust. A `Bridge` plank in its cleats (200 N) is unaffected until pulled; a plank hung on a player's Hang line is a free body and swings in the gust. The Pulse (0.4 s outward, then a 0.6 s inhale toward the Stump, every 75 s from 0:55) throws loose planks out and draws them back; planks on the inner Scaffold edge go into the Well on the inhale, planks on the outer edge rain on the Rim.
- Breathing tilt: slides at 6° on steel, stays on wood.
- Chain reactions: knocks cans and chairs. Does not move barrels.

## States

| State | Visual | Physics |
|---|---|---|
| Intact | Whole board | 12 kg |
| Dented | Deep boot-edge gouge, one band missing | No change |
| Broken | Snapped at a knot, two halves | Two 6 kg Light props, 10 s lifetime unless authored |
| Destroyed | Splinters | Removed. Splinter decal persists 30 s |

## VFX and SFX hooks

- Impact on steel: `SFX_Plank_Steel` (hollow clap, bounce layer). Concrete: `SFX_Plank_Concrete` (slap). Wood: `SFX_Plank_Wood` (knock). Water: `SFX_Plank_Splash_Flat`.
- Footsteps on a plank: `SFX_Foot_Plank` with a flex creak when a Breaker (220 kg) stands mid-span.
- Break-free from cleats: `SFX_Wood_Pop`, `NS_Dust_S`.
- Fracture: `SFX_Plank_Snap`, `NS_Fracture_Wood_S` (20 splinters, 1.5 s), `DECAL_Splinters`.
- No emissive.

## Technical

- LOD0 1,200 tris (under budget; it is a box with chamfers and bands), LOD1 600, LOD2 200, LOD3 12. One 1K set shared across all plank variants and the scaffold board stack.
- Pivot: bottom center. X+ along the length.
- Dog sockets: `SKT_Dog_Top_01` (top face center, Wood), `SKT_Dog_End_01` (X+ end band, Metal), `SKT_Dog_End_02` (X- end band, Metal). Spike Dogs hold at full load in `Top_01`; Mag Dogs hold only in the end bands.
- Chaos: `GC_Plank`. L0 whole, L1 two halves. Damage threshold L0 = 240 (1 unit = 1 N·s, **PROPOSED** calibration). Anchor field `FS_Anchor_Plank_Bridge` for the Bridge variant, released at 200 N.
- Replication: RC-L. Planks that are fragments of a parent geometry collection go local after the parent's second cluster break.

## Reference search terms

Scaffold board 3.9 m end band galvanized; weathered scaffold plank grey top; trench bridge plank with cleats; splintered broken scaffold board; stacked scaffold boards site; boot-polished timber walkway.
