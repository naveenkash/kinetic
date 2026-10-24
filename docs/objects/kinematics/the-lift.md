# The Lift

One-line: kinematic, infinite mass (car, rope, headframe) with a rated attachment (the counterweight, 8,000 kg, 2.2 m x 1.5 m x 1.0 m concrete), car 3 m x 3 m cage at (30, 0), running Z 2 (bottom landing) to Z 30 (upper landing) at 1.6 m/s, the counterweight on guides at (14, 0) through the Mid Hang, Hangtown: one. Geometry and cycle per `arenas/hangtown/layout.md` and `events-and-moving-parts.md`; the counterweight's own sheet is `heavy/lift-counterweight.md`.

## Role in play

The Lift is the honest route up (03) and the only vertical machine in Hangtown. The car runs Low to Upper on the west side; its counterweight runs the opposite direction through the Mid Hang in an open guide frame, passing the market ring's east edge. The car is a slow, lit, predictable box that a Breaker rides and a Climber ignores. Both are kinematic and sever any line they pass through (05 section 3.5): a Hang across the Lift run, a Gallows hung from the headframe through the car's path, a Clothesline across the counterweight's notch, all snap on the next pass; lines attached to the car or the block ride with them. The counterweight is a rated attachment per 05 section 6.7: kinematic on its guide until any active pull on it reaches 1,000 N (a Breaker alone, a Doubler at overspin, or a crew summing to it) held 0.6 s, then a simulated rated load that leaves its guide and takes the Lift with it.

| Harness | Relationship |
|---|---|
| Climber | Never rides. Perches on the headframe sheave beam, the highest point on the west rim. |
| Scrapper | Rides the counterweight down on a Hang line for a fast Mid-to-Low drop. Lines props to the car as a moving Guillotine weight. |
| Rigger | A Clothesline across the car door at the Upper landing. The car arrives, the door is a wall. |
| Doubler | Overspins the counterweight from a Mid Hang bridge 20 m away; 1.3 s above 1,000 N commits it, and the recoil throws the Doubler toward a house, not the Well. |
| Breaker | Rides the car. Pulls the counterweight from the Mid Hang walkway beside its guide, braced. |

## Appearance

Silhouette: a timber-and-mesh cage car on a single 36 mm rope over a headframe sheave on the west rim, guided by two cables; the counterweight is a raw concrete block with cast-in lifting eyes in a steel guide frame of two channels, running through a notch cut in the Mid Hang's house row. Lanterns on the car. Yellow-and-black tape in a band around the counterweight and on the guide channels at walkway height.

| Part | Material | Base color | Roughness | Metallic |
|---|---|---|---|---|
| Car frame | timber, Hanger blue | #2F6FAD | 0.8 | 0.0 |
| Car mesh | expanded steel, rust | #7A3B1E | 0.7 | 0.6 |
| Rope and guides | wire rope, greased | #3C3F42 | 0.35 | 0.9 |
| Headframe | salvaged crane lattice, ochre | #B8752A | 0.6 | 0.6 |
| Counterweight | cast concrete, iron-stained | #8E8A82 with #7A3B1E runs | 0.85 | 0.0 |
| Guide channels | steel channel, taped | #6E7378, #F2C500 / #141414 | 0.6 | 0.8 |
| Lanterns | 2700 K | #F4B860 emissive | | |

Weathering: the car's timber is replaced plank by plank in different blues. The counterweight has a chalk tally of years on its face (`L.22` to `L.61`) and a hand-painted saint. Markings: `VILCA LIFT  12 PERSONS  NO GOATS` on the car gate; Office inspection plate; HTR `CUT 06` orange on the live guide channel; a condemnation notice wired to the headframe. Lighting: car lanterns (6, 60 lm), a red beacon at the headframe, the counterweight's tape lit by a lamp at the walkway notch.

Read by tier: from the rims, a lit cage rising on a line against the ravine. From the Mid Hang, a concrete block sliding past a walkway twice a minute with a lantern-lit car on the far side. From the Low Hang, the car arriving overhead and settling onto the landing timbers.

## Variants

1. **Standard.** Car and counterweight on the cycle.
2. **Dropped (post-consequence).** Counterweight in the Low Hang landing's timbers, rope slack, car jammed against the headframe sheave 36 m up.
3. **Cut district (seasonal).** Lift removed; headframe stands with no rope. Dressing.

## Physics

- Car and rope: kinematic, infinite mass. Speed 1.6 m/s. Car floor is a moving platform; riders inherit velocity. A player struck by the car's underside at 3 m/s: under the 4 m/s kinematic stagger, pushed. Crush between the car and the Upper landing lip (0.2 m clearance): over 8 kN for 0.5 s, scrapped. The landing has a tape stripe.
- Counterweight: kinematic at 1.6 m/s on the guide while attached. Passing the Mid Hang walkway notch it presents a 1.5 m face 0.3 m from the handrail. A player leaning over the rail is pushed (3 m/s, no stagger) or pinned against the rail post (crush, scrapped). The notch has a crush pocket at each guide channel.
- Rated attachment: break-free 1,000 N on any active pull (Haul, Jack push, or Kick; never a Hang), commit 1,000 N sustained 0.6 s (README) pulled outward from the guide (away from the Well side). On release the block leaves the guide and free-falls as a simulated 8,000 kg body; the car, suddenly 8,000 kg lighter on its rope, is hauled up to the headframe by the rope's spring-back and the sheave's scripted overspeed (authored, 6 m/s), slamming the stop beam.
- Fall: from the block's position at commit (anywhere in its 36 m travel). From mid-travel, 18 m: 1.9 s, 18.8 m/s. It passes through the Low Hang below its guide (a garden terrace, Medium props and drip nets, authored breakaway) and continues to the Well. The impact on anyone in its 2.2 m x 1.5 m footprint: scrapped. A Hang line from a player to the block: the Anchor Drop with 400 m of fall.
- Riders in the car at the slam: the car hits the stop beam at 6 m/s. Player hits kinematic at 6 m/s: flipped, not scrapped. The car then hangs jammed at the Upper landing with its gate 1.2 m above the deck: a mantle for everyone but a Breaker (1.8 m mantle, so they can).

## Cycle and tells

40 s cycle (**PROPOSED**).

| Time | Car | Counterweight | Tell |
|---|---|---|---|
| 0 s | Low landing, gate open, 8 s dwell. | Upper position, 36 m up in the guide. | Gate bell, two strokes. |
| 8 s | Gate closes, ascends at 3 m/s. | Descends at 3 m/s. | `SFX_Lift_Rope_Loop`, lanterns sway. |
| 14 s | | Passes the Mid Hang notch (walkway at 18 m). | Guide channel rumble audible on the walkway from 2 s before. |
| 20 s | Upper landing, gate open, 8 s dwell. | Low position. | Bell. |
| 28 s | Descends. | Ascends. | |
| 34 s | | Passes the notch upward. | Rumble. |
| 40 s | Low landing. | Upper. | Bell. |

Audio: gate bell, rope loop (pitch steady; the Lift has one speed), guide rumble 2 s before the block reaches the notch, headframe sheave squeal at each reversal. Visual: car lanterns, the block's lit tape band moving against the dark guide, the headframe beacon switching green (moving) to red (dwell). Lead: 2 s of rumble at the notch; the car is visible for its whole travel.

Authored consequence (counterweight): commit, 1.0 s groan (guide channels ring, the rope hums up its whole length, Marshal: "Counterweight's off its guide"), release, fall, car slam at the headframe 1.2 s after release, Low Hang terrace breakaway at the impact time, block into the Well. The Lift is out for the round; the car is a jammed static box at the top.

## Interactions

- **Haul** self to the car floor from the Mid Hang: a grapple onto a moving platform (1.6 m/s upward: you are hoisted until your feet find the floor, or you Reel up the last meters at 1 m/s).
- **Jack** off the counterweight face as it passes: a Slingshot across the Mid Hang at the Kick's class V (18 m/s for a Climber), plus the block's 3 m/s if the Jack is along its travel.
- **Hang** to the counterweight and step off: a 1.6 m/s elevator in either direction, Mid Hang to the bottom in about 9 s. The Scrapper's shortcut. Hang an enemy to it going down: they are pulled through the terrace's drip nets (x2.0) and stop at the sole plate with the block at 1.6 m/s: pushed, not killed, unless they are between the block's top and the jib guard at the top of its travel (crush, scrapped). Going up: hoisted and left hanging under the guard.
- Chain: a house swung into the guide frame (3,500 N s large-house fracture against a kinematic frame) breaks the house, not the frame.

## States

Kinematic: `Dwell Low`, `Ascending`, `Dwell Upper`, `Descending`. Counterweight: `Attached`, `Stressed` (line under 1,000 N or under 0.6 s: tape flutters, guide shoes chatter), `Failing` (groan), `Dropped`.

## VFX and SFX hooks

`SFX_Lift_Bell`, `SFX_Lift_Rope_Loop`, `SFX_Guide_Rumble`, `SFX_Sheave_Squeal`, `SFX_CWT_Guide_Ring`, `SFX_Rope_Hum_36m`, `SFX_Car_Slam`; `VFX_Lantern_Sway`, `VFX_Beacon_State`, `VFX_Guide_Dust`, `VFX_Terrace_Breakaway`; Marshal VO.

## Technical

- LOD0 car 6,000 tris, counterweight 900, headframe 9,000, guide frame 3,000 per 10 m; LOD1 half; LOD2 quarter; LOD3 boxes.
- Textures: `T_Lift_Car` 2K, `T_Lift_CWT` 2K, `T_Hangtown_Steel_Trim` 2K, `T_Hangtown_Decal_Atlas` 2K.
- Pivots: car at floor center; counterweight at base center. Sockets: car `Floor`, `Gate`, `Lantern_0..5`, `Roof_Dog_0..3`; counterweight `Face_Walkway` (Haul target), `Eye_0..3`, `Tape_Band`; headframe `Sheave`, `Beacon`, `Stop_Beam`.
- Implementation: car and counterweight on one server time-synced timeline with mirrored height curves. The counterweight is a kinematic body with a `Kinetic_RatedLoad` query shape; the commit hands it to simulation (`Kinetic_RatedLoad` dynamic, 8,000 kg, gravity on) and switches the car to a 1.2 s slam timeline. The Low Hang terrace under the guide is a geometry collection with an anchor field cleared by the consequence 0.1 s before the block's arrival time.
- Collision: car `Kinetic_Kinematic`; counterweight `Kinetic_Kinematic` attached, `Kinetic_RatedLoad` dropped.

## Reference search terms

mine shaft cage hoist headframe, counterweight guide rails open shaft, favela inclined elevator, rope elevator concrete counterweight, timber cage lift lanterns.
