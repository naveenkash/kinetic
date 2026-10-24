# Walkway on Cable

One-line: structure, destructible in 2 m sections, Heavy tier per 10 m run (640 kg), sections 2.0 m x 1.0 m, hung 1.6 m below a main load cable on steel hangers, Hangtown Upper Hang: eight runs, one per main cable, each up to 180 m long between the Collar rims and the Lift headframe.

## Role in play

The walkways are the Upper Hang's roads and the only continuous path between the two Collar rims. They hang from the main load cables, so they move with the Sway as a whole and do not swing locally. A run is a long, narrow, well-lit corridor forty meters above the Mid Hang with no walls: the best Clothesline site in the game and the worst place to meet a Breaker. Pull an anchor pin (see `rated/anchor-pin.md`) and a whole run drops.

| Harness | Relationship |
|---|---|
| Climber | Runs along the main cable above the walkway instead. Perches on hangers. |
| Scrapper | Fast lines between hangers. Overboard works in both directions on every meter. |
| Rigger | Two set lines close a walkway for 12 s. The Gallows from the main cable above, hoisted by Reel at 1 m/s under the 12 kN Hang load. |
| Doubler | Snap mid-run: the enemy goes sideways, there is nothing sideways. |
| Breaker | Walks the walkway braced when needed; nothing on the Upper Hang out-pulls a braced Breaker. The honest route is the Lift plus the walkway. |

## Appearance

Silhouette: a chain of grating panels on light steel stringers, hung by paired round-bar hangers from the main cable every 2 m, with a single tube handrail on each side at 1.0 m and a toe-board. Office floodlight strips clipped to the handrail every 10 m. From a distance the run reads as a lit thread under a dark cable.

| Part | Material | Base color | Roughness | Metallic |
|---|---|---|---|---|
| Grating | pressed steel plank, perforated | #6E7378 | 0.55 | 0.85 |
| Stringers | angle steel, Hanger-painted | #2F6FAD blue (Vilca's Hang) | 0.6 | 0.6 |
| Hangers | 20 mm round bar, galvanized | #8A8F93 | 0.5 | 0.9 |
| Handrail | tube, worn to bare metal at hand height | #8A8F93 / #B3B6B8 | 0.4 | 0.9 |
| Floodlight strip | Office cool white, 4000 K | emissive | | |

Weathering: hangers are wrapped where they meet the cable with rubber from tyres to stop fretting. Bird nests in the handrail elbows. Chalk kilometer marks every 10 m (`0.14`, `0.15`) in the residents' hand, and the Office's own yellow stencils over them. Laundry lines tied to the handrail with clothes still on them (Light props). On the HTR claim side, condemnation notices wired to the handrail every 20 m and a line of orange survey spray on the grating.

Read by tier: from the rims, a lit line into the dark. From the Mid Hang, a glowing ceiling edge with feet visible through the grating. From the Low Hang, a thin bright thread above the houses.

## Variants

1. **Standard run.** Grating, two rails. 64 kg per 2 m section, 640 kg per 10 m cluster.
2. **Lift approach.** 2.0 m wide for 12 m near the headframe, double hangers. 1,100 kg per 10 m.
3. **Condemned run.** Grating missing every fifth panel, one rail only. Gaps are 2 m: a Breaker's jump covers 0.8 m of height and roughly 2.4 m of distance at run speed, so a Breaker must sprint. Fracture 1,200 N s (**PROPOSED**).
4. **Cut district (seasonal).** Hangers only, no grating. Cables hanging empty.

## Physics

- Tier: Heavy per 10 m cluster (640 kg). Fracture threshold 2,500 N s per 10 m cluster (canon, 05 section 6.5). Fragments: Medium grating panels (64 kg) and Light hanger bar.
- Anchoring: hangers are rigid to the main load cable, which is static except during Cable Day and anchor-pin consequences. A walkway does not swing locally; its lateral motion is the Sway of the main cable, up to 1.2 m at mid-span with an 11 s period (**PROPOSED**).
- Worked: a Breaker landing from 3 m (7.7 m/s) delivers 1,690 N s to a 10 m cluster, no fracture. From 5 m (9.9 m/s), 2,180 N s, no fracture, but the Breaker is at 9.9 m/s against static: flipped, not scrapped. A hanging house swung into a walkway (2,600 kg at 4 m/s, 10,400 N s) takes the cluster out.
- Fall from the Upper Hang to the Mid Hang: 12 m, 15.3 m/s. Scrapped on a roof (wood x1.2 raises the Stall to 14.4 m/s, still scrapped). Laundry or drip nets (x2.0, 24 m/s) save. The Low Hang nets are 24 m down: 21.7 m/s, saved by a net, scrapped by anything else.
- A Clothesline at 1.2 m across a 1.0 m walkway cannot be stepped around. Crouch (passes under lines above 1.0 m) or slide (0.7 m).

## Interactions

- **Haul** a player along a walkway: they skid toward you on grating. Haul from the walkway to a house roof below: you drop 12 m on a line; release at 2 m above the roof or take the landing.
- **Jack** between you and an enemy on a walkway: Overboard in the purest form, and a T push only (no Kick when a Dog is in a player's plate, 05 section 3.2 interlock). The push splits by mass: a Climber (90 kg) Jacking a Breaker (220 kg) takes 71% of the separation and goes off the other side of a 1.0 m walkway first.
- **Hang** from the main cable above to yourself: the Pendulum along the run's axis is a 40 m arc through the Mid Hang.
- Chain: an anchor pin pulled at the rim drops the main cable; the walkway run folds down its length over 3 s in a pre-authored sequence, sections detaching from the rim end first. Anyone on it goes with it.
- Cable Day: the failed main cable's walkway drops in the same way.

## States

| State | Visual |
|---|---|
| Intact | Level, strips lit, laundry drifting. |
| Stressed | During a pin pull or Cable Day siren: hangers ring, strips flicker in sequence along the run, grating lifts 50 mm at the near end. |
| Failing | Hangers part from the rim end at 10 m per 0.3 s, grating panels peel away downward. |
| Failed | Cable hangs in the ravine, hangers bare, grating gone. Strips out. The Mid Hang below is now lit only by lanterns and lines. |

## VFX and SFX hooks

- `SFX_Walkway_Grating_Step` (hollow, carries), `SFX_Hanger_Ring` on impact above 300 N s.
- `SFX_Walkway_Peel` (long, rising) for the authored drop; `VFX_Grating_Shed`, `VFX_Strip_Fail_Sequence`.
- `SFX_Laundry_Flap` ambient.

## Technical

- LOD0 1,100 tris per 2 m section, LOD1 400, LOD2 ribbon strip, LOD3 emissive line only. Instanced; one ISM per run.
- Textures: `T_Walkway_Trim` 2K shared with Hangtown steel, `T_Hangtown_Decal_Atlas` 2K.
- Pivot at the section's near-end deck center. Sockets: `Hanger_L`, `Hanger_R`, `Rail_L`, `Rail_R`, `Strip` (every fifth), `Laundry_0..1`.
- Chaos: geometry collection per 10 m cluster. Level 0 cluster (anchored by the main cable's anchor field), level 1 sections, level 2 panels and bar. The authored drop does not use fracture: it drives a per-section release timer along the run and lets each section fall as a Medium body, with a budget of 20 simulating sections and the rest swapped to a pre-baked vertex animation at distance.
- Collision: `Kinetic_WorldDynamic` (rides the Sway), `Kinetic_Prop` fragments.

## Reference search terms

cable suspended walkway grating, Capilano suspension bridge hangers, mine headframe walkway, perforated steel plank grating, favela cable walkway night lights.
