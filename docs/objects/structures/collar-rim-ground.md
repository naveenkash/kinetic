# Collar Rim Ground

One-line: structure, static world, infinite mass, two slabs each 60 m x 30 m (**PROPOSED** playable footprint) at the east and west ends of Hangtown: the sheared edges of Collar Three's two halves, the only solid ground on the map, the spawn zones, and the seats of the anchor pins.

## Role in play

The rims are where Hangtown's rules stop applying. No Sway, no slack cables, no Well underfoot. Both crews drop here (05 section 9.3: spawns at opposite ends of the high tier, never within 25 m of a Well edge). The rim is the Breaker's home and the Climber's launch pad. It holds the anchor pins (see `rated/anchor-pin.md`), which makes it the one place on the map where a Breaker can end a whole walkway, and the Lift headframe stands on the west rim.

| Harness | Relationship |
|---|---|
| Climber | Leaves it within 3 s of the drop. Returns only to re-launch. |
| Scrapper | Loots the Office prep stores on the rim (barrels, chain, carts) and carries them out on lines. |
| Rigger | Sets the room before leaving: Clotheslines at the walkway heads, a Gallows anchor on the headframe. |
| Doubler | Anchors one line to the rim and one to an enemy on the first walkway: the only tug of war a Doubler wins, because the rim does not move. |
| Breaker | Braces here against anything. Pulls anchor pins. Holds the Lift. |

## Appearance

Silhouette: a sheared Collar deck edge. A 30 m thick slab of concrete and Weave composite ends in a torn face that drops 400 m to the ravine. On top: Collar Three's original deck plating, the Office's prep compound (shipping containers as stores, a Hook landing pad, a Marshal gallery mast), and the eight cable anchor heads in a row along the ravine edge.

| Part | Material | Base color | Roughness | Metallic |
|---|---|---|---|---|
| Deck plating | Collar structural plate, 60 years weathered | #5A5E63 | 0.6 | 0.7 |
| Shear face | concrete and Weave lattice, torn | #9A9590 with #1B1F26 strands | 0.8 | 0.2 |
| Prep compound | Office containers, yellow | #F2C500 | 0.6 | 0.7 |
| Hook pad | painted H in white | #F5F5F0 | 0.7 | 0.0 |
| Anchor heads | cast steel, taped | #4B5055 | 0.6 | 0.8 |

Weathering: the deck plating is scarred by sixty years of cable installations: old anchor sockets, cut-off bolts, Guild ochre paint from the Ribbon Wars era under Office yellow. The shear face shows Weave strands pulled out like roots. Markings: Office spawn zone circles (10 m, white), Hook pad, `COLLAR THREE  WEST RIM  EL. 2,140` surveyed in yellow. Hangers' murals on the compound containers (see `environment/chalk-and-graffiti.md`). Floodlights: four Office masts, 30 m, aimed down onto the Upper Hang, not onto the rim, so the rim is darker than the walkways; a singing line is still the brightest thing on it.

Read by tier: from the far rim, a lit cliff edge 400 m away with the Lift headframe as a landmark. From the Mid Hang, a black wall above with cables leaving it. From the Low Hang, out of sight.

## Variants

1. **West rim.** Lift headframe, Marshal mast, four anchor heads, Guild and Hanger spawn in Claim.
2. **East rim.** Office compound, Hook pad, four anchor heads, HTR spawn in Claim. A longer approach to the market (two extra walkway segments).
3. **Cut district (seasonal).** East rim with two anchor heads empty, cables hanging into the ravine, compound expanded with HTR orange containers.

## Physics

- Static world. Infinite mass. Dogs seat in everything on it except the Marshal mast's gallery (`Kinetic_NoDog`, 05 section 3.1) and the Hook pad while a Hook is on it.
- Surfaces: deck plating and concrete are hard (x1.0 Stall multiplier). The Office keeps one Stall Gel pad (x4.0) under the Lift headframe where riggers step off the car, because the Lift arrives at 3 m/s and people fall off it; a prep-crew courtesy per 05 section 7.3.
- Edge: the ravine edge has a 0.4 m lip. Ledge grab works below 8 m/s. A player Jacked off the rim at 10 m/s or more does not grab.
- Prep stores: 6 fuel drums (cradled, 200 N break-free), 4 propane cylinders, 20 m of chain (Light, 4 kg per m in 2 m lengths), two hand carts (Medium, 60 kg). Staged where the Office's prep crew would keep them: beside the Hook pad, 20 m from spawn. Every Scrapper opener starts here.
- The rim does not Sway. A Hang line from the rim to a walkway section therefore changes length by the walkway's Sway amplitude alone (up to 1.2 m), and a rigid Hang set at the Sway's near extreme snaps taut at the far one, yanking whatever is on the far Dog by up to 1.2 m in 5 s. Mild, but a trap.

## Interactions

- **Haul** from the rim to the first walkway hanger: the standard non-Breaker exit. Haul an enemy from a walkway back to the rim: a Breaker's 1,000 N gets a Climber to 21 m/s and a Rigger to 17 m/s over 20 m, against a container wall (Stall at 12).
- **Jack** off the shear face from below (Climber returning): a vertical Slingshot at the Climber's 18 m/s launch speed (Updraft kit) up the wall to the rim.
- **Hang** from the Marshal mast: forbidden (no Dog). From the headframe: the Gallows anchor of choice, 18 m up.
- Chain: a fuel drum Kissed into a propane cylinder in the prep compound at 0:10 is the Hangtown opener everyone has seen. The compound is placed so the blast reaches neither spawn circle (4 m radius, 12 m from spawn).

## States

Static; no failure states. Seasonal variant only. The shear face has a `Weathered` material state that advances per season (more Weave strands exposed) as a cheap way of showing time.

## VFX and SFX hooks

- `SFX_Rim_Wind_Loop` (the only place on the map where wind is heard from a solid floor).
- `SFX_Hook_Rotor_Idle` on the Hook pad between rounds.
- `VFX_Rim_Dust_Edge` (wind-lifted grit at the lip).

## Technical

- LOD0 terrain mesh 40,000 tris per rim, LOD1 15,000, LOD2 4,000, far-side impostor at 400 m.
- Textures: `T_Collar_Deck_Plate` 4K tiling, `T_Shear_Face` 4K with Weave strand decal layer, `T_Office_Container` 2K, `T_Hangtown_Decal_Atlas` 2K.
- Pivot at the rim's ravine-edge center. Sockets: `Spawn_A`, `Spawn_B` (alternate), `Anchor_0..3`, `Headframe` (west), `HookPad`, `Marshal_Mast`, `Store_0..11`.
- Collision: `Kinetic_WorldStatic` complex-as-simple off, simplified hull for the deck plus a precise mesh for the lip. Nav mesh covers the rim for bot crews (apprenticeship).
- Replication: none; static. Spawn-contested logic (enemy within 15 m) per 05 section 9.3 lives on the game mode, not here.

## Reference search terms

sheared concrete deck edge cliff, dam crest cable anchorage, space station ring section cutaway, helipad on cliff industrial, exposed rebar and fiber composite tear.
