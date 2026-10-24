# Cable reel (small)

Light prop. Light tier, 28 kg. 60 cm flange diameter, 40 cm wide, 20 cm core, plywood flanges on a steel axle, half-wound with 12 mm steel wire rope. Appears in the Yard (the Floor, in rows beside the control shack and at the trench heads; the Rails, in the crane cab store), Hangtown (the Upper Hang, at every cable anchor point, and under the Lift headframe), and Ballast (the Bays, chocked against the rack bases; the Crown, by HTR's cutting gantry). Not in the Anchor, where cable is Weave and nobody leaves it lying around.

## Role in play

The small reel is the Light prop that rolls. On its flanges it is a 60 cm wheel, and on any slope it goes. It flips a player at the Haul cap, it rolls down a Breathing lean on its own, and it is the clearest Light-tier tell for a tilt. On the Yard's Floor a reel rolling toward a trench is a free Caboose: it drops in, the Hauler arrives, and anyone who followed it is in the lane.

- Named rigs: the Avalanche, as the chock: reels wedge the bottom steel crate (Medium, 250 kg) of a crate stack in Ballast's Bays, and Hauling the reel out first lowers the break-free of the crate it chocked from 500 N to 0 (**PROPOSED** chock rule: a chocked stack's bottom unit counts as "in a rack or cradle," 200 N, instead of the 500 N stack value; pull the chock, 200 N, and the crate is loose on the floor at 0 N). This is the only route by which a Climber or Scrapper (400 N) starts an Avalanche, since a plain stack's 500 N is beyond them. It applies to Medium crate stacks only: the Bays' shipping containers are Heavy, and a Heavy object on a bare floor is fixed scenery that moves only on rollers, skids, or hanging (05 §6.1a); the containers move on their magnetic racks, not because a reel was pulled. Overboard, on wheels: a reel Hauled into the back of a player on a walkway rolls them off it. The Caboose, with the reel as the thing that goes in the trench first.
- Scrapper: Hauls a reel into a player at 25 m/s for a flip, then Hauls it back on instant recall. It rolls back on its own half the time. Light props travel 30% faster on a Scrapper line; the 25 m/s cap holds.
- Rigger: Hangs a reel from an overhead anchor on the lean side of the Bays, Reels it up clear of the catwalk (within Hang, holding Haul shortens the rod at 1.0 m/s, holding Jack lengthens it, bearing up to the class Hang load), and releases it as a set line. When the Breathing swings, the reel swings further than the hull (03 Ballast, the Pendulum) and clears a catwalk.
- Breaker: pulls the reel chock out of a stack before pulling the stack. Two lines, one Avalanche.
- Climber: a reel in the Hangtown Sway is a moving perch. Climbers Dog the axle end, Hang, and ride it along a walkway.
- Doubler: a reel on a full 1,200 N Snapback receives 1,500 N·s: 54 m/s on 28 kg, capped at the 50 m/s terminal velocity, arriving with 1,400 N·s. A Stall from any snap over 960 N (0.4 s into overspin; impulse is 1,500 N·s × F / 1,200 N). It rolls after it hits, which is worse.

## Appearance

Silhouette: two discs with a core between, rope visible through the gap. Standing on its flanges it is a wheel; on its side, a drum. The wire rope end is clamped to a flange with a U-bolt and 30 cm of it hangs free.

| Part | Material | Base color | Roughness | Metallic |
|---|---|---|---|---|
| Flanges (18 mm plywood, 2) | `MI_Plywood_Weathered` | `#9C8564` | 0.80 to 0.92 | 0 |
| Flange rims (steel band) | `MI_Steel_Galvanized` | `#9EA3A6` | 0.40 | 1 |
| Core (steel tube) and axle | `MI_Steel_Painted_Worn` | `#5B6670`; HTR `#E8620C` | 0.50 to 0.70 | 1 chipped |
| Wire rope (12 mm, 6×19) | `MI_Chain_Oiled` | `#3C3E42` | 0.35 | 1 |
| Rope clamp, tie bolts | `MI_Steel_Rusted` | `#7A3F1C` | 0.80 | 0.3 |

Wear story: the flanges are grey on the outer face and still brown on the inner. The rim band is dented from being rolled over grates. The rope is oiled on the outer wraps and rusted at the core where it has not moved. The axle ends are bright where the stand bearings ran. In Hangtown, sisal lashing through the flange holes and a hanger stencil (a kite) on one face.

Stencils: Guild reels are stenciled `LG WIRE ROPE 12 / 100 M` in ochre with a Ledger stores number. HTR reels carry `HTR-RC-3340` and a lot code in orange on a riveted plate. Ballast reels are pre-Severance `MERIDIAN` cargo, `MI_Meridian_Cyan` `#2A9FB5` at 0.9 wear, with climber hold codes `T-BAY-2`. Hangtown reels are unmarked or Guild hand-me-downs.

Under arena lighting: plywood grey, like every other plank in the frame. The read is the circular silhouette and the wire-rope sheen. A rolling reel at night in the Yard is a dark disc with a bright rim, which is enough.

## Variants

| Variant | Mass | Difference |
|---|---|---|
| `Half` | 28 kg | Default. Half-wound. Rolls. |
| `Empty` | 11 kg | Bare core. Rolls faster and further. Stagger at Haul cap (275 N·s, just under; Jack 330 N·s, stagger). |
| `Chock` | 28 kg | Lying on a flange, wedged under a stack. Ballast Bays. Break-free 200 N. |
| `HTR` | 28 kg | Orange core, plated. Yard and Crown. |
| `Meridian` | 28 kg | Cyan ghost livery, 61 years of rot. Ballast only. |
| `Spooling` | 28 kg | Wire rope end tied to a wall socket; the reel pays out rope as it rolls. Visual spline only. Hangtown anchors. |

## Physics

- Tier: Light. Mass: 28 kg (Empty 11 kg).
- Collision: two cylinders (flanges) radius 30 cm × 2 cm, and a core cylinder radius 10 cm × 36 cm, all on the X axis. On its flanges it rolls about X.
- Center of mass: axle center.
- Friction 0.6, rolling resistance equivalent 0.03, restitution 0.2 (**PROPOSED**). On a 3° slope a standing reel starts rolling on its own.
- Break-free: loose on floor 0 N. `Chock` variant, in a rack or cradle, 200 N: any class pulls a chock.
- Haul acceleration with the far end static: 14.3 m/s² at 400 N (Climber, Scrapper), 21.4 at 600 N (Rigger), 28.6 at 800 N (Doubler), 35.7 at 1,000 N (Breaker). Distance to the 25 m/s Haul cap: 21.9 m, 14.6 m, 10.9 m, 8.8 m. On its flanges it rolls rather than slides, so it reaches these speeds with less scrape. A Scrapper's 20 m line reaches the cap at 12.9 m with the 30% Light-prop bonus (×1.69 force, ×1.3 arrival speed; 05 §6.1), which pulls at 24.1 m/s².
- Cut recoil: 150 N·s per 1,000 N to both ends. Cutting a Breaker's line on a reel gives it 150 N·s, 5.4 m/s, and on its flanges it rolls off with it.
- Fracture: 650 N·s (**PROPOSED**): a flange splits and the rope spills. Fragments: the core with one flange and the rope (20 kg Light, no longer rolls), one broken flange (4 kg Light), and a 4 kg wire-rope tangle (Light), 10 s lifetime for the smaller two.

What it does to a player:

| Delivery | Speed | Impulse | Result |
|---|---|---|---|
| Haul, any class | 25 m/s cap | 700 N·s | Flip |
| Scrapper Haul | 25 m/s cap | 700 N·s | Flip |
| Jack | 30 m/s cap | 840 N·s | Flip |
| Fall from 7 m | 11.7 m/s | 328 N·s | Stagger |
| Rolling down a 6° Breathing lean, 20 m | 4.5 m/s | 126 N·s | Nothing, but it is at the edge now |
| Doubler Snapback, full 1,200 N | 50 m/s terminal | 1,400 N·s | Stall |
| Doubler Snapback at 960 N | 42.9 m/s | 1,200 N·s | Stall, at threshold |

A reel cannot Stall on a normal line. A flipped player next to a trench or on a walkway is the reason to use it.

## Interactions

- Haul to it: it rolls to you if it is on its flanges, which is faster than it flies. 28 kg pulls a 90 kg Climber 24% of the gap.
- Jack from it: the Kick (the Jack's 0.3 s capacitor discharge, separate from the motor pull T) drives you apart to the class launch speed V, split by inverse mass. A 90 kg Climber (V 18 m/s) sends the reel off at 13.7 m/s and takes 4.3 m/s backward, a real hop; a 110 kg Scrapper (V 14) sends it at 11.2 m/s and takes 2.8 m/s; a 220 kg Breaker (V 10) sends it at 8.9 m/s and takes 1.1 m/s. Then T pushes the reel at 14.3 to 35.7 m/s² by class to the 30 m/s cap (a Scrapper's 30% bonus, ×1.69 force and ×1.3 arrival speed, applies to this push and to the Haul, never to V; 05 §6.1), and it rolls away and keeps rolling. The Kick never fires with a Dog in a Catch Plate: a Jack from the reel to an enemy is the T push alone, the reel leaving from rest at 14.3 m/s² on a 400 N line and reaching 23.9 m/s at the end of a 20 m line (a Breaker's 1,000 N reaches the cap in 0.84 s over 12.6 m), while the enemy skids back at T over their mass.
- Hang from it: a reel on a set line is a 270 N pendulum weight that spins on its axle as it swings. A Hang to a loose reel brings it along as 28 kg of dead weight, and it rolls toward you. A Hang to the `Chock` variant is not tested against its 200 N break-free (that is for active pull only: Haul, Jack, and the Kick); the reel is wedged under 250 kg and more, so it holds a hanging player until the stack above it moves (**PROPOSED** physical behaviour).
- Object to object: a reel rolling into a propane cylinder at 8 m/s triggers it; reels on a Breathing lean reach 4.5 m/s, under threshold, so a rolling reel alone does not set off racked cylinders. A Hauled reel does. A reel at 25 m/s into a market stall fractures it. A reel into a steel crate stops.
- Kinematic: a Hauler crushes a reel in a trench. The Gantry magnet lifts a reel by its steel core if within 1 m. A reel on the Hangtown Lift platform rides up; on the Sway, reels on walkways roll 20 cm each way and stop against the kick-boards.
- Water: floats on its flanges, core up, half out. In Ballast's Hold it rides the tilt flow and rolls when it grounds.
- Updraft and Pulse: not in the Anchor. If one gets there, force volumes act on loose objects (break-free 0) at a_ref × 100 / m, with the factor capped at 2 per body (100 / 28 kg would be 3.6, so a loose reel feels 2 times the authored acceleration), speeds capped by the 50 m/s hard limit, and it lifts; a `Chock` reel (200 N) is unaffected until pulled, and a reel hung on a player's Hang line is a free body and swings in the gust.
- Breathing tilt: the headline interaction. At 6° every unchocked reel in the Bays rolls toward the lean, reaching 4.5 m/s over 20 m, and stops against containers, catwalk kick-boards, or nothing. After the Shear there is nothing on one side. Reels go down the Well on every cycle until the Bays are clear of them, which takes about four cycles. This is a tell and a hazard and it is cheap.
- Chain reactions: a reel into a reel is a stop. A reel into a paint-can stack is a clatter. A reel chock pulled from a stack is the first move of an Avalanche.

## States

| State | Visual | Physics |
|---|---|---|
| Intact | Two flanges, half wound | 28 kg, rolls |
| Dented | Rim band kinked, flange cracked, wobbles when rolling | No change; a 0.5 Hz wobble on the roll |
| Broken | One flange split, rope spilled in a tangle | 20 kg core piece that does not roll; two Light fragments 10 s |
| Destroyed | Both flanges gone | 12 kg bare core, rolls; rope tangle decal |

## VFX and SFX hooks

- Impact on steel: `SFX_Reel_Steel` (wood-and-steel knock with a rim ring). Concrete: `SFX_Reel_Concrete`. Wood: `SFX_Reel_Wood`. Water: `SFX_Reel_Splash_Float`.
- Rolling: `SFX_Reel_Roll_Steel` and `SFX_Reel_Roll_Grate` loops, pitch by speed. The grate loop is the Yard's reel tell.
- Break-free from a chock: `SFX_Wood_Scrape_Lurch`, `NS_Dust_S`.
- Fracture: `SFX_Plywood_Split_L`, `NS_Fracture_Wood_M`, `NS_WireRope_Spill` (spline unspool, 1 s).
- No emissive.

## Technical

- LOD0 2,000 tris, LOD1 1,000, LOD2 400, LOD3 120 (two discs and a tube). One 1K set shared with the large Haul spool's wood where possible; rope from `T_Trim_Steel_01`.
- Pivot: axle center (authored state is rolling). X+ along the axle.
- Dog sockets: `SKT_Dog_End_01` (X+ axle end, Metal), `SKT_Dog_End_02` (X- axle end, Metal), `SKT_Dog_Rim_01` (flange rim at top, Metal band), `SKT_Dog_Top_01` (top of the wound rope, Metal). A Dog in an axle end lets the reel spin freely under Haul; a Dog in the rim does not, and the reel drags.
- Chaos: `GC_CableReel_S`. L0 whole, L1 core with flange A, flange B, rope tangle. Damage threshold L0 = 650 (1 unit = 1 N·s, **PROPOSED** calibration). Anchor field `FS_Anchor_Reel_Chock` for the Chock variant, released at 200 N; releasing it also drops the chocked crate's anchor field from its 200 N chocked value to loose (0 N).
- Replication: RC-L. Rolling reels are the Light prop most likely to desync; raise correction to 15 Hz while angular speed is over 2 rad/s (**PROPOSED**).

## Reference search terms

Wire rope reel plywood flange 60 cm; small cable drum half wound steel wire; cable drums stacked construction site; cable reel rolling on floor; wire rope drum rusted core; cable drum chock under crate.
