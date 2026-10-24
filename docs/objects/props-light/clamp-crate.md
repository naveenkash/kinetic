# Clamp crate

Light prop. Light tier, 28 kg. 60 × 40 × 30 cm open-top steel mesh crate holding about 20 scaffold couplers. Appears in the Anchor (the Scaffold, two per level per quadrant at the ladder landings; the Rim, in the contractor compound), the Yard (the Floor, beside the control shack; on scrap heaps as the `Spilled` variant), and Hangtown (the Upper Hang, at the cable anchor points where walkways are bolted). Not in Ballast.

## Role in play

The clamp crate is the Anchor's heavy Light prop: 28 kg at the top of the tier, flipping a player at the Haul cap. It is also the thing the Scaffold is held together with, so it sits beside the thing it holds together. When a Scaffold section fractures, its clamps (05 §6.5, Light clamps) are the same couplers this crate holds; the crate and the fragment pool share meshes.

- Named rigs: the Guillotine, as a real load on a set line: recall or cut your own set line and the crate drops by gravity; a second line (crate to enemy, Haul) only steers it. From 4 m it arrives at 8.9 m/s, 249 N·s, under stagger; from 7 m, 11.7 m/s, 328 N·s, a stagger. A Light Guillotine is a tell and a tangle, not a kill. The Scatter, decoy or real: two crates Jacked apart are two flips. Overboard setup: a crate to the back plate on the inner Scaffold edge.
- Scrapper: Hauls a crate into an enemy's back at 25 m/s for a flip, which on the Scaffold is a drop to the next level. Light props travel 30% faster on a Scrapper line; the 25 m/s cap holds.
- Rigger: Hangs a crate at head height from a set line over a ladder top. Twelve seconds, and the crate swings on the Updraft.
- Breaker: Hauls one as a 28 kg counter-mass for a self-Haul in the open where there is no wall to brace against. It barely helps; the Breaker's own mass does the work.
- Climber: a loose crate lifts in the Breath (it feels 2 times the authored acceleration, the per-body cap), so a Climber Hangs to one and rides it; between gusts it is a Hang pivot: Dog the crate, Dog the Scaffold, Hang, and the crate anchors a short swing.
- Doubler: a crate on the far Dog of a full 1,200 N Snapback receives 1,500 N·s: 54 m/s on 28 kg, capped at the 50 m/s terminal velocity, arriving with 1,400 N·s. A Stall. The snap impulse scales with the force at the snap (1,500 N·s × F / 1,200 N), so the crate Stalls from any snap over 960 N, which is 0.4 s into overspin. The Snapback is the only way a Light prop Stalls anyone, and this is the prop for it.

## Appearance

Silhouette: a low rectangular box of grid mesh, full of short steel rods with knuckles. Reads as "heavy small" because the couplers are visible through the mesh.

| Part | Material | Base color | Roughness | Metallic |
|---|---|---|---|---|
| Mesh crate (25 mm weld mesh, 12 mm frame) | `MI_Steel_Galvanized` | `#9EA3A6` | 0.35 to 0.55 | 1 |
| Couplers (drop-forged, 20) | `MI_Steel_Galvanized` worn to `MI_Steel_Bare` | `#9EA3A6` to `#5C5F63` | 0.45 to 0.65 | 1 |
| Coupler bolts and nuts | `MI_Steel_Rusted` | `#7A3F1C` | 0.80 | 0.3 |
| Handles (bar, 2 ends) | `MI_Steel_Bare` | `#5C5F63` | 0.45 | 1 |
| Rated-tape roll (1, in the Anchor variant) | `MI_Tape_Rated` | `#F2C230` / `#141414` | 0.55 | 0 |

Wear story: the galvanizing is dull with white zinc bloom. Couplers are bright where the jaws grip and rust-brown at the bolts. The crate's handles are polished by gloves. Concrete dust in every corner in the Anchor; oil in the Yard. One coupler is always a different maker's pattern (fiction: salvaged).

Stencils: Anchor crates carry a Guild Ledger stencil `LG-SCAF-07` in ochre on a steel tag wired to the mesh. Yard crates carry `C4PW` (Collar Four Press Works, **PROPOSED**). HTR crates, which appear only in the Rim compound, are `MI_HTR_Grey_Matte` painted mesh with `HTR-RC-5519` in orange.

Under arena lighting: galvanized mesh is a mid-grey with bright specular lines. In the Anchor it merges with scaffold tube and reads only by shape. The rated-tape roll inside the Anchor variant is a yellow point, which is intentional: it tells players that rated tape is applied by people and that the people were here.

## Variants

| Variant | Mass | Difference |
|---|---|---|
| `Full` | 28 kg | Default. 20 couplers. |
| `Full_Anchor` | 28 kg | Plus one rated-tape roll and a torque wrench handle sticking out. |
| `Half` | 16 kg | 10 couplers. Stagger at Haul cap, not flip. |
| `Empty` | 6 kg | Bare mesh. Scrap heaps. Does nothing. |
| `Spilled` | 6 kg crate plus 10 Light couplers | On its side on a heap, couplers out. Pre-fractured. |
| `HTR` | 28 kg | Grey painted mesh, orange tag. Rim compound only. |

## Physics

- Tier: Light. Mass: 28 kg (Half 16 kg, Empty 6 kg).
- Collision: one box 60 × 40 × 30 cm. Mesh is not modeled for collision; Dogs seat on the box face and visually snap to the nearest mesh wire.
- Center of mass: 12 cm above the base, centered.
- Friction 0.65, restitution 0.1 (**PROPOSED**). It lands and stays.
- Break-free: loose on floor 0 N. In the Rim compound, stacked three high, "in a stack (friction)," 500 N for the lower two: a Rigger (600 N), Doubler (800 N), or Breaker (1,000 N) starts them; a Climber or Scrapper (400 N) cannot, and takes the top crate only.
- Haul acceleration with the far end static: 14.3 m/s² at 400 N (Climber, Scrapper), 21.4 at 600 N (Rigger), 28.6 at 800 N (Doubler), 35.7 at 1,000 N (Breaker). Distance to the 25 m/s Haul cap: 21.9 m, 14.6 m, 10.9 m, 8.8 m. A Scrapper's 20 m line reaches 23.9 m/s at full length without the bonus and the cap at 12.9 m with the 30% Light-prop bonus (×1.69 force, ×1.3 arrival speed; 05 §6.1), which pulls at 24.1 m/s²; a Climber's 50 m line reaches it with room to spare.
- Cut recoil: 150 N·s per 1,000 N to both ends. Cutting a Breaker's line on a crate gives it 150 N·s, 5.4 m/s; it scoots.
- Fracture: 700 N·s (**PROPOSED**): the mesh splits at a weld seam and spills. A player's body counts against the threshold: a flipped 220 kg Breaker landing on a crate at 4 m/s is 880 N·s and splits it. Fragments: the crate (6 kg Light) and 10 Light couplers (1.1 kg each), 10 s lifetime. The couplers are the same `SM_Coupler_Light` as the Scaffold section's clamp fragments.

What it does to a player:

| Delivery | Speed | Impulse | Result |
|---|---|---|---|
| Haul, any class | 25 m/s cap | 700 N·s | Flip |
| Scrapper Haul | 25 m/s cap | 700 N·s | Flip |
| Jack | 30 m/s cap | 840 N·s | Flip |
| Fall from 7 m | 11.7 m/s | 328 N·s | Stagger |
| Doubler Snapback, full 1,200 N | 50 m/s terminal | 1,400 N·s | Stall |
| Doubler Snapback at 960 N | 42.9 m/s | 1,200 N·s | Stall, at threshold |

On a normal line a clamp crate cannot Stall. 840 N·s at the Jack cap is the most any Light prop delivers without a snap. A flip on the Scaffold is still a fall, and a fall off level three is a Stall on the Rim.

## Interactions

- Haul to it: it comes to you, flat. 28 kg against a 90 kg Climber moves the Climber 24% of the gap, which is noticeable: a Climber who Hauls a crate while airborne is pulled toward it.
- Jack from it: the Kick (the Jack's 0.3 s capacitor discharge, separate from the motor pull T) drives you apart to the class launch speed V, split by inverse mass. A 90 kg Climber (V 18 m/s) sends the crate off at 13.7 m/s and takes 4.3 m/s backward, a real push; a 110 kg Scrapper (V 14) sends it at 11.2 m/s and takes 2.8 m/s; a 220 kg Breaker (V 10) sends it at 8.9 m/s and takes 1.1 m/s. Then T pushes the crate at 14.3 to 35.7 m/s² by class to the 30 m/s cap (a Scrapper's 30% bonus, ×1.69 force and ×1.3 arrival speed, applies to this push and to the Haul, never to V; 05 §6.1). The Kick never fires with a Dog in a Catch Plate: a Jack from the crate to an enemy is the T push alone, the crate leaving from rest at 14.3 m/s² on a 400 N line and reaching 23.9 m/s at the end of a 20 m line (a Breaker's 1,000 N reaches the cap in 0.84 s over 12.6 m), while the enemy skids back at T over their mass.
- Hang from it: a crate on a set line is a 270 N pendulum weight. A Hang to a crate loose on a floor brings it along as 28 kg of dead weight. A Hang to a crate in a Rim compound stack is not tested against the 500 N stack break-free (that is for active pull only: Haul, Jack, and the Kick); the lower crates bear the weight of the ones above them, so a hanging player hangs from the stack until the top crate slides off its neighbor at 15° of line angle from vertical (**PROPOSED** physical behaviour).
- Object to object: a crate Hauled into a propane cylinder at 25 m/s triggers it. A crate at 25 m/s into a Scaffold section is 700 N·s, under the section's 800 N·s fracture; two crates together, or one off a Doubler snap, break it. A crate onto a market stall fractures the stall.
- Kinematic: the Maintenance Ring sweeps crates off the Scaffold top. A Hauler shoves them. The Gantry magnet lifts a steel crate within 1 m of its load (**PROPOSED**; the magnet is on).
- Water: sinks. Mesh fills instantly.
- Updraft and Pulse: force volumes act on players and loose objects (break-free 0) at a_ref × 100 / m, with the factor capped at 2 per body (100 / 28 kg would be 3.6, so a loose crate feels 2 times the authored acceleration), speeds capped by the 50 m/s hard limit: it lifts on the Breath and slides on the Pulse. Crates in a Rim compound stack (500 N) are unaffected until freed; a crate hung on a player's Hang line is a free body and swings in the gust. The Pulse (0.4 s outward, then a 0.6 s inhale toward the Stump, every 75 s from 0:55) shoves a loose crate toward the outer Scaffold edge and then draws it back toward the Stump; one that goes over the outer edge lands on the Rim as a 328 N·s stagger on anyone below, one that goes over the inner edge on the inhale is gone. Prep crews placed them near ladders for a reason; the Pulse clears them on purpose.
- Breathing tilt: not in Ballast.
- Chain reactions: a crate fracturing over a scrap heap adds ten couplers to the heap's pool, which raises nothing but the clutter count.

## States

| State | Visual | Physics |
|---|---|---|
| Intact | Full mesh crate | 28 kg |
| Dented | One side mesh bowed in, handle bent | No change |
| Broken | Seam split, couplers out | 6 kg crate plus 10 Light couplers for 10 s |
| Destroyed | Flattened mesh | 6 kg Empty, remains to round end |

## VFX and SFX hooks

- Impact on steel: `SFX_Crate_Mesh_Steel` (clang with a loose-coupler rattle inside). Concrete: `SFX_Crate_Mesh_Concrete`. Wood: `SFX_Crate_Mesh_Wood`. Water: `SFX_Crate_Splash_Sink`.
- Impact on a player: `SFX_Crate_Plate` (heavy, flat).
- Break-free from a stack: `SFX_Steel_Scrape_S`, `NS_Dust_S`.
- Fracture: `SFX_Mesh_Tear`, `NS_Spill_Couplers` (20 coupler particles, 1 s, then 10 real props).
- No emissive.

## Technical

- LOD0 2,000 tris (mesh as a masked card box, couplers as 20 instanced 60-tri meshes), LOD1 1,000, LOD2 400 (couplers baked to a card), LOD3 120. One 1K set; couplers share `T_Trim_Steel_01`.
- Pivot: bottom center.
- Dog sockets: `SKT_Dog_Top_01` (center of the open top, on the couplers, Metal), `SKT_Dog_Handle_01` (X+ end handle, Metal), `SKT_Dog_Handle_02` (X- end handle, Metal), `SKT_Dog_SideN_01` (long side mesh, Metal). Mag Dogs hold full load everywhere on it.
- Chaos: `GC_ClampCrate`. L0 whole, L1 crate shell plus 10 couplers (couplers as one cluster that releases together). Damage threshold L0 = 700 (1 unit = 1 N·s, **PROPOSED** calibration). Anchor field `FS_Anchor_ClampCrate_Stack` for the Rim compound stacks, released at 500 N.
- Replication: RC-L. Spilled couplers go local immediately (fragments under Light).

## Reference search terms

Scaffold coupler crate steel mesh stillage; drop forged double coupler pile galvanized; scaffolding fittings bin site; swivel coupler rusted bolt; wire mesh stillage with handles; scaffold clamps spilled on deck.
