# Billet Stack

One-line: structure, Heavy tier per unit, 2,800 kg per billet crate (2.4 m x 1.2 m x 1.2 m), stacked three or four high on pallets, the Yard Stacks tier: nine stacks along the Stacks' canyon edges and two on the Floor beside the press.

## Role in play

A billet stack is the Yard's slow kill. It topples slowly enough to read (03 engine notes ask for a settle time) and lands with enough mass to scrap anything under it. Breakers tip them. Everyone else learns where they are. Billet crates are also the Haul mode payload's cousins: the 2,000 kg spool is a billet on a pallet.

| Harness | Relationship |
|---|---|
| Climber | Stack tops are mid-tier perches with a view of the Rails. Jacking off a billet crate is a solid Slingshot. |
| Scrapper | Cannot move one (400 N tension against the 500 N stack break-free; 05 section 6.2). Fights around them. |
| Rigger | 600 N clears the 500 N break-free and rolls the bottom crate at 0.16 m/s² on its pallet; 0.3 m of roll unseats the two above and the tip does the rest. The line is the trigger; gravity is the engine. |
| Doubler | Overspin to 1,200 N starts a stack at twice a Rigger's rate, in the 1.3 s before the snap. |
| Breaker | The owner. Tip a stack across a canyon mouth and the Stacks have a new wall. |

## Appearance

Silhouette: steel-strapped timber crates, open-slat sides showing the dark Weave billets inside, on pallets of welded channel. Every crate has a Guild manifest stencil and an Office seal.

| Part | Material | Base color | Roughness | Metallic |
|---|---|---|---|---|
| Crate slats | hardwood, new | #B08D5A | 0.8 | 0.0 |
| Straps | steel banding | #8A8F93 | 0.4 | 0.9 |
| Billets | pressed Weave, 200 mm x 200 mm x 2.2 m | #1B1F26 with iridescent sheen | 0.3 | 0.6 |
| Pallet | steel channel on four dead rollers, Guild ochre | #B8752A | 0.6 | 0.7 |
| Seal | Office yellow wax disc, 100 mm | #F2C500 | 0.5 | 0.0 |

Weathering: the crates are new; the Yard makes them. Dust and rail-oil mist only. The billets inside are the most valuable thing on any map and look it: an oil-slick sheen under the floodlights, no rust, no dirt. Stencils: `LAY GUILD  COLLAR FOUR BREAKING YARD  LOT 61-0218  WEAVE  3 x 2.2 m  2,800 kg` and the Guild's hand-stamped Ledger number of the pressing crew. HTR orange tags on two stacks reading `CONTESTED`. Lighting: the floodlight masts rake the Stacks from 40 m up; billet sheen is the only specular highlight in the mid tier, which makes stacks landmarks.

Read by tier: from the Rails, pale crate tops in a grid. From the Stacks, chest-high walls with the Weave glinting through the slats. From the Floor, towers at the canyon mouths.

## Variants

1. **Four-stack.** 11,200 kg total, 4.8 m tall. Canyon-edge landmark.
2. **Three-stack.** 8,400 kg, 3.6 m. The common unit.
3. **Broken-strap.** Top crate's slats sprung, billets visible, Weave fragments (see `environment/weave-ribbon-fragment.md`) on the ground. Top crate fractures at 1,000 N s (**PROPOSED**) into Medium slat bundles and Heavy billets.
4. **Haul spool (mode payload).** Single billet on a Guild pallet with two Dog rings welded on, 2,000 kg per 05 section 9.1. Not stacked.

## Physics

- Tier: Heavy, 2,800 kg, on roller pallets (friction 0.02 to start, 0.005 rolling, 05 section 6.1a and CHANGELOG v0.2.2: Avalanche stacks sit on roller pallets; Heavy objects on a bare floor are fixed scenery). T / m: Rigger 0.21 m/s², Doubler 0.29, Breaker 0.36, Doubler overspin 0.43, against 0.2 m/s² to start and 0.05 rolling: a Rigger's line starts a bottom crate and rolls it at 0.16 m/s² net; a Climber's or Scrapper's 400 N (0.14 m/s²) does not, and the 500 N stack break-free says the same. The line unseats the crate; the stack's own geometry does the rest.
- Break-free: 500 N (stack). A Climber or Scrapper line (400 N) sings and the stack creaks.
- Settle time: at break-free the bottom crate lurches 0.3 m on its roller pallet (the break-free tell) and keeps rolling under the line; the crates above begin to tip at that point. The tip is a rotation about the far pallet edge. For a three-stack, center of mass 1.8 m up and 0.6 m in: it passes the tipping point at 18 degrees, which takes 1.4 s under gravity from rest (**PROPOSED** authored; Chaos sleep thresholds raised so it does not snap over). Total from break-free to impact: about 2.6 s. Readable.
- Impact: the top crate of a three-stack lands from 3.6 m at 8.4 m/s. On a player: 23,500 N s. On a plank bridge: fragments it. On a Hauler: nothing (kinematic), the crate bounces off at up to 12 m/s and is now a 2,800 kg projectile that scraps anything it touches at 0.43 m/s or more.
- Crates do not fracture except the broken-strap variant. They dent nothing. They are the stop on the map.
- Crush: a crate settling against a player pinned to a hull section applies 2,800 kg x 9.8 = 27 kN. Over 8 kN for 0.5 s: scrapped. This is how most billet kills actually resolve: not the fall, the pin.

## Interactions

- **Haul** bottom crate toward you: the Avalanche. Stand to the side, not in the line; the stack falls toward the puller.
- **Jack** from a crate: a Slingshot off the most solid mid-tier object. Jack between two bottom crates: the Scatter, both stacks tip outward, a canyon becomes a corridor of falling crates.
- **Hang** from a crate: a Hang line never unseats a placed object (05 section 6.2), so any class uses a stacked crate as a Pendulum pivot or a Gallows anchor up to the class Hang load. A crate's own weight (27 kN) reaches the line only once the crate is falling, and then every Hang load slips 0.5 m per 0.1 s with a creak: a 1.5 s countdown, not a catch. The Guillotine is for Medium loads.
- Chain: a tipped stack lands on a scrap heap and scatters Light junk at up to 8 m/s, staggering a 6 m radius. A crate into the press pit is a Well entry for anything tethered to it: the Anchor Drop with 2,800 kg.
- Gantry: the magnet picks hull sections, not billets. Billets do not ride.

## States

| State | Visual |
|---|---|
| Intact | Square, strapped, sealed. |
| Stressed | Under a line below 500 N: straps ping, slats creak, dust off the top crate, pallet scrapes 20 mm and stops. |
| Failing | Bottom crate slides, upper crates tilt through 18 degrees over 1.4 s, seals crack. |
| Failed | Crates on their sides, one straps-up, billets visible, dust settling. |

## VFX and SFX hooks

- `SFX_Billet_Strap_Ping`, `SFX_Pallet_Scrape`, `SFX_Billet_Tip_Groan` (long, pitch falls as angle grows), `SFX_Billet_Impact_Floor`.
- `VFX_Billet_Dust`, `VFX_Weave_Sheen` (material parameter pulse on impact).
- Broken-strap variant: `VFX_Slat_Splinter`, Weave fragment spawn.

## Technical

- LOD0 1,900 tris per crate, LOD1 800, LOD2 200, LOD3 box. Billets inside are a parallax interior material at LOD1 and below.
- Textures: `T_Billet_Crate` 2K, `T_Weave_Billet` 2K (iridescent clearcoat), `T_Yard_Decal_Atlas` 2K.
- Pivot bottom-center of the pallet. Sockets: `Strap_0..3` (Dog seats), `Lift_Ring_A/B` (Haul spool variant), `Seal`.
- Physics: rigid bodies with authored stack constraints (as container stacks). Angular damping raised to 2.0 during the tip phase and restored at 18 degrees to produce the settle. High sleep threshold so a nudged stack does not wobble. Geometry collection only on the broken-strap top crate.
- Collision: `Kinetic_Heavy`. Haul spool variant adds `Tag.Payload` and the Well reset rule from 05 section 9.4.

## Reference search terms

steel strapped timber crate, scrap yard bale stack, aluminium billet stack, pallet of ingots, wooden crate tipping slow motion.
