# Catch Plates

_By Accord rule, harness armor must present dog-able surfaces. Riggers compare it to a judo gi: the uniform exists so you can be grabbed. Four plates: chest, back, left shoulder, right shoulder._

## Physical description

Rigid plates mounted to the harness over the chest, between the shoulder blades, and on each shoulder. A Dog seats in whichever plate is nearest its impact point. The plate determines where the line leaves the body, which determines how far the player must turn to see it (05 §5.5). Backs are the kill plate.

Every plate has a 1 cm **rim band** in team tint (Amber `#E69F00`, Sky `#56B4E9`, or Rose `#CC79A7`, per 05 §11a), lit at all times. During the Drop phase, before lines enable, the band renders Bone `#E8E6E0`. The band is the only place team light appears on the body apart from the Spool ring and the shoulder lamp.

| Class | Chest (W x H cm) | Back | Shoulders | Edge profile | Material |
|---|---|---|---|---|---|
| Climber | 26 x 22, convex | 24 x 30 fin, convex | 14 x 12 ovals | Rolled, no lip | Mirror stainless |
| Scrapper | 28 x 24 | 30 x 28, mounted low | L 15 x 13, R 13 x 11 | Mixed, by source plate | Mixed salvage |
| Rigger | 30 x 26 | 32 x 30 | 16 x 14 | 3 cm lip | Mild steel, brass rim |
| Doubler | 30 x 26 | 32 x 30 | 16 x 14 | 3 cm lip (Guild pattern), serial field ground | Steel, black |
| Breaker | 36 x 30 | 40 x 36 | 22 x 18, raked 15° | 5 mm chamfer | Anodized aluminum |

All dimensions are **PROPOSED** and are locked once chosen: a plate is a silhouette element. Minimum dimension on any plate is 22 cm, so a 16 cm Dog seats fully inside the rim.

## Seat sockets and line exit

| Plate | Socket | Bone | Turn to see a seated line |
|---|---|---|---|
| Chest | `S_Plate_Chest` | `spine_03` | 0°, already in view |
| Left shoulder | `S_Plate_ShoulderL` | `clavicle_l` | 60° to 90° |
| Right shoulder | `S_Plate_ShoulderR` | `clavicle_r` | 60° to 90° |
| Back | `S_Plate_Back` | `spine_03` (`spine_02` on the Scrapper, same socket name) | 150° or more |

- The seated Dog mesh snaps to the socket. A second Dog on the same plate seats at a 6 cm offset along the plate's long axis (**PROPOSED**).
- The line leaves the body from the socket, perpendicular to the plate face for the first 5 cm, then bends toward the other Dog.
- The HUD edge indicator is driven by the socket's direction relative to the camera, never by the line (05 §4.2).
- Self-dog seats Dog A or B in `S_Plate_Chest` instantly.

## How Dogs seat visually

| Plate type | Standard | Barb | Mag |
|---|---|---|---|
| Lipped (Rigger, Doubler, Kessl-pattern Scrapper) | Jaws bite under the lip; the Dog hangs from the rim | Flukes flare and hook the lip from any angle | Snaps flat to the field |
| Chamfered (Breaker, HTR cast-off Scrapper) | Jaws grip the chamfer; 0.5 cm magnetic snap-down | Flukes hook the chamfer | Snaps flat |
| Convex mirror (Climber) | Jaws grip the rolled edge; the Dog sits proud | Flukes hook the roll | Snaps flat with a visible reflection |
| Sand-cast (Basin Scrapper) | Jaws scrape, then bite a pore | As lipped | Snaps flat, slightly tilted |

Spike Dogs never seat in a plate: they slide off with a shriek. See `dogs.md`.

## Visual states

| State | Rim band | Field | Other |
|---|---|---|---|
| Idle | Team tint, 1 cm, steady, emissive 2 | Faction finish | Tallies and decals visible |
| Dog seats | Flashes white 0.2 s, then steady | | 10 sparks; seat clank |
| Line under force | Steady | | The line's glow lights the plate field; no extra plate emissive |
| Dog unseated by Cutter melee | Blinks twice | Scorch mark, fades 5 s | Dog pops 20 cm and recalls |
| Owner scrapped | Band goes red `#FF2A1A` (**PROPOSED**) and stays red while the body persists | Gel bloom at the plate edges | All lines release |
| Owner hoisted or airborne | Steady | | |

The red-band-on-scrapped state is the only time a non-team color appears in a rim band. It tells both crews at a glance which bodies are props.

## Readability and livery rules

- No livery, cloth, patch, or prop covers more than 15% of any plate (README rule 4).
- Plate field luminance stays between 25% and 70%. Mirror plates clamp specular to 70% of the line core's emissive.
- Tallies (white, painted by the prep crew) and the lease plate (lower-left chest) live on the plates. Tallies anchor at `S_Tally_01` to `S_Tally_08`.
- Office inspection dot: a 1 cm white dot in a plate corner on any plate not from the harness's original maker (Scrapper mostly). **PROPOSED**.
- Shoulder lamp at `S_Lamp_Shoulder` sits on the left shoulder plate's inner edge and never on the field.

## Audio

| Event | Sound |
|---|---|
| Dog seat | The seat clank, directional, one shared sample set, mixed 6 dB louder on the victim's client. Never customizable. |
| Cutter melee | Clang by plate material: steel ring, aluminum tonk, mirror ping, cast thud |
| Plate struck by object or world | Impact by material; above 300 N·s the stagger grunt; above 1,200 N·s the Stall sequence (05 §7.1) |

## Technical

| Item | Spec |
|---|---|
| Collision | One convex shape per plate, welded to its parent bone in `PA_Rigger_<Class>`. Separate from the capsule so Dog seating and Cutter melee can hit-test the plate and not the body. |
| Physical material | `PM_CatchPlate` on all plates; it selects the seat clank and the Dog seat clip. |
| Seat determination | Server: nearest plate socket to the impact point, ties to chest. Impact normal within 60° of the plate normal for Standard and Mag (**PROPOSED**); any angle for Barb. |
| Mesh | Plates are part of the class harness skeletal mesh, with a per-plate material slot so `TeamTint`, `Wear`, and `TallyMask` can be set per plate. |
| Material params | `TeamTint`, `RimState` (0 idle, 1 flash, 2 blink, 3 scrapped red), `FactionPrimary`, `Wear`, `TallyMask`, `ScorchMask`. |
| Replication | Seat events and `RimState` are replicated. Scorch and tallies are cosmetic and local. |

## Reference search terms

judo gi lapel grip, hockey shoulder pad caps, riot shield edge profile, motorcycle back protector plate, machined breastplate chamfer, sand-cast aluminum surface, polished stainless convex mirror.
