# Rail Hauler

One-line: kinematic, infinite mass, 14.0 m long x 3.2 m wide, deck at Z -0.4 (3.6 m above the trench's rail bed at Z -4), carrying a hull plate whose top rides at Z +3 above the Floor, the Yard: automated rail trucks running the three 160 m trenches (South and North west to east, Center east to west across the press pit on two stringers) on a fixed schedule, 12 m/s cruise, 45 s per trench (22.5 s during Shift Change), per `arenas/the-yard/events-and-moving-parts.md`.

## Role in play

The Hauler is the Caboose in its purest form (03). It is the Yard's timetable: klaxon, lights, train. You do not move it; it moves you. Dog an enemy to it and the schedule finishes the rig. Climbers ride it for free repositioning. Breakers stand on it and arrive like a delivery. Everyone else learns the chalked timetable at crossing 07 and crosses between trains.

| Harness | Relationship |
|---|---|
| Climber | Rides the deck; Jacks off it at the far end at 18 m/s plus the Hauler's 12 m/s onto the Stacks. |
| Scrapper | Lines a fuel drum to the Hauler's coupling: a 180 kg drum dragged at 12 m/s through a trench is a moving bomb that detonates on the first bridge bearer (over 10 m/s). |
| Rigger | Hangs a Clothesline across a trench at 1.2 m above the Floor. The next Hauler severs it (05 section 3.5: a kinematic severs any line it passes through; the load top rides at +3 m, so every line across a trench below Z 3 goes). A Hang footbridge across a trench lasts exactly until the next train, and whoever is on it falls into the trench ahead of that train. That is 03's Trench Bridges play. |
| Doubler | Two lines: enemy to Hauler, self to a mesa. Snap the second at the right moment to stay put. |
| Breaker | Stands on the deck braced. Nothing on the Yard out-pulls a 20 m/s² braced foot-hold plus a train. Steps off at the Rails-side stair. |

## Appearance

Silhouette: a low flatbed on two four-wheel bogies, a squat unmanned cab at the leading end with a klaxon horn and two amber beacons, a hull section chained on the deck. Office-yellow chevrons on both ends; Guild ochre body.

| Part | Material | Base color | Roughness | Metallic |
|---|---|---|---|---|
| Deck and frame | plate steel, Guild ochre | #B8752A | 0.6 | 0.6 |
| Cab | pressed steel, ochre over Consortium grey | #B8752A / #7C8488 | 0.55 | 0.6 |
| Bogies and wheels | cast steel, oil-black | #2A2C2E | 0.5 | 0.8 |
| Chevrons | Office yellow and black | #F2C500 / #141414 | 0.65 | 0.0 |
| Beacons | amber lens, lit | #FFB000 emissive | 0.2 | 0.0 |
| Headlights | two, 5000 K | emissive | | |

Weathering: forty years of continuous running. Ochre worn to grey on the deck's load footprint, oil film on everything below the frame, brake dust on the bogies. The cab's windows are painted over (unmanned since L.30). Markings: `C4 BREAKING YARD  HAULER 06` on the cab, Guild Ledger numbers of the crew who maintain it chalked inside the frame, HTR orange `PROPERTY DISPUTED` stencils sprayed on two of the six and painted over in ochre on one. Lighting: headlights on, beacons on while moving, the trench lights ahead flash red 3 s before arrival (05 section 11).

Read by tier: from the Rails, a lit rectangle sliding along a dark slot. From the Stacks, beacons flashing between mesas and a horn. From the Floor, a wall of ochre at chest height coming at you.

## Variants

1. **Loaded.** Hull section chained on, 3.2 m total height. The common one.
2. **Empty.** Flatbed only. A Climber's favorite: nothing to Slingshot into on the deck.
3. **Shift Change (double schedule).** Same mesh, 20 s headway, both directions in alternating trenches.
4. **Seasonal HTR win.** Haulers doubled permanently (04). Hauler 06 in HTR orange.

## Physics

- Kinematic, infinite mass, spline path, no suspension (03 engine notes). Nothing moves it. A line between it and anything else moves the anything else. It severs every line it passes through that is not attached to it: Clotheslines and Hang footbridges across a trench below Z 3, a Gallows hung from a plank bridge bearer into the trench, a Caboose line from a rider on one Hauler to a player on the Floor when a second Hauler crosses it at the pit stringers. The snap plays, both Dogs release.
- Speed: 12 m/s cruise. Acceleration out of the entry plenum 2.0 m/s² over 6 s to cruise; deceleration into the far plenum 2.0 m/s² over 6 s. Cruise length 110 m of the 160 m trench. The trench is 4 m deep and 6 m wide; the Hauler's deck is 3.6 m above its rail bed and its load top 3 m above the Floor, so a Hauler is taller than its trench and anything on a trench lip within 1.6 m of the rail is hit by the load at 12 m/s.
- Player hit by the leading face at 12 m/s: player hits kinematic, Stall at 9 m/s. Scrapped. By the side at a glancing 3 m/s closing: under the 4 m/s stagger, pushed. A player in the trench 3 s before arrival has the klaxon, the lights, and 36 m of warning.
- Player dragged by a Hauler (Caboose): the line accelerates them along the trench at T/m until they match the Hauler's 12 m/s (a Rigger on a Breaker's 1,000 N: 7.7 m/s², over their 5 m/s² foot-hold, airborne; a Breaker on a Scrapper's 400 N: 4.5 m/s², under their 5 m/s² foot-hold, a grounded skid down the trench floor). The kill is the first bridge bearer or the tunnel mouth: the Rigger passes 8 m/s (flip) at 1.0 s and 12 m/s (Stall) at 1.6 s after the ramp; the nearest bearer is never more than 15 m away. Panic window 1.6 to 2.2 s from Dog seat. Within target.
- Player on the deck: rides. At cruise, standing still on a kinematic is fine (no relative speed). Walking on the deck is allowed. Jumping off at 12 m/s onto concrete: player hits static at 12 m/s, scrapped. Jumping off onto a scrap heap (x2.0, 24 m/s) survives. The heaps along the Center trench are placed for exactly this. Waiting for the end plenum, where the train stops, is the other way off.
- Objects: a Hauler striking a Medium prop launches it at up to 12 m/s plus. A 180 kg drum at 12 m/s is 2,160 N s and a blast on arrival. A 2,800 kg billet crate or a rated hull section in a trench: the Hauler carries it ahead of itself (kinematic wins) as a wall whose ends overhang both lips by 3 m, at 12 m/s, to the end plenum, where it is dumped and removed. That is the Plow (see `structures/hull-plate-section.md`).
- Crush: a player between the Hauler's face and a trench end: scrapped. Between the side and the trench wall (0.3 m clearance): scrapped.

## Cycle and tells

Baseline period 45 s per trench, first trains at e 10 (South), 25 (Center), 40 (North), so there is a train somewhere every 15 s. Shift Change resets the timetable at e 80 to 22.5 s per trench (a train every 7.5 s) for 60 s. Canon per the arena events file.

| Time | Event |
|---|---|
| -3.0 s | Klaxon from the tunnel mouth (two tones, falling), trench lights on that trench flash red at 2 Hz. |
| -2.0 s | Headlights visible in the tunnel mouth. |
| 0.0 s | Hauler clears the tunnel mouth at 0 m/s, accelerating. |
| 6.0 s | Cruise, 12 m/s. Beacons steady amber. |
| 16.8 s | Begins decelerating 50 m from the far tunnel. |
| 22.8 s | Enters the far tunnel. Trench lights off. |
| 45.0 s | Next Hauler in this trench, same direction (each trench is one-way). |

Shift Change (1:15 remaining in Scrap, 4:00 in Claim, 5 s siren): period 22.5 s for 60 s, trench lamps steady red. Audio: `SFX_Hauler_Klaxon` (directional, 3 s lead per 05 section 11), `SFX_Hauler_Rail_Loop` (pitch by speed), `SFX_Hauler_Brake_Squeal` on deceleration. Visual: red trench lights, headlights, beacons. Lead 3 s, always, even in Shift Change.

## Interactions

- **Haul** enemy plate to Hauler (Caboose): they are dragged along the path. Counter: cut; or Hang to something heavier than the train (nothing is; Hang to the world).
- **Jack** off the deck: a flat launch along the trench plus the Hauler's own 12 m/s if you were riding (relative velocity is preserved on leaving a kinematic).
- **Hang** self to Hauler: ride on a rigid tether behind it like a water-skier along the trench floor. Legal, stupid, and the first bearer ends it.
- Chain: a Hauler hits a plank-bridge debris pile and showers Light planks at 12 m/s (290 N s each, staggers). A drum Caboosed to it blasts at the first bearer and launches that bridge's planks.

## States

Kinematic: no failure. Visual states are `Idle in tunnel`, `Approaching` (klaxon, lights), `Cruising`, `Braking`. A Hauler never derails (03).

## VFX and SFX hooks

`SFX_Hauler_Klaxon`, `SFX_Hauler_Rail_Loop`, `SFX_Hauler_Brake_Squeal`, `SFX_Hauler_Impact_Prop`; `VFX_Hauler_Headlight_Cone`, `VFX_Trench_Light_Red`, `VFX_Rail_Sparks` (braking), `VFX_Exhaust_Soot` (diesel-electric idle in tunnel).

## Technical

- LOD0 18,000 tris, LOD1 8,000, LOD2 2,500, LOD3 box with emissive beacons.
- Textures: `T_Hauler_Body` 4K, `T_Hauler_Bogie` 2K, `T_Yard_Decal_Atlas` 2K.
- Pivot at the deck center at rail height. Sockets: `Coupling_F/R` (Dog seat hints), `Deck_0..7`, `Cab_Klaxon`, `Beacon_L/R`, `Headlight_L/R`, `Load`.
- Implementation: kinematic body moving on a spline with a distance-over-time curve; server time-synced (clients evaluate the same curve at server time, no position replication). Physics step uses kinematic target transforms so contacts receive correct relative velocity. `Kinetic_Kinematic` channel. Line severing: the Hauler's swept volume (body plus load) is tested against every line segment not attached to it each step; intersecting lines snap. Plenums carry a `Kinetic_KillVolume` 2 m inside so a dragged player who somehow survives the bearers does not ride into unloaded space.
- Riders: deck is a moving platform base; characters inherit velocity. Line forces on a rider use the rider's own foot-hold against the deck.

## Reference search terms

automated rail flatcar slag yard, steel mill transfer car, trench rail cart industrial, unmanned shunter amber beacon, rail yard klaxon horn.
