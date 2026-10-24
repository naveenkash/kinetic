# Force Volumes

One-line: environment, authored acceleration fields with no mass, five systems: the Updraft (the Anchor), the Pulse (the Anchor), water rush (Ballast tank ruptures), the Hold flood (Ballast standing water), and the gas jet (compressed-gas manifolds in all arenas and the Yard press header after its consequence). Force volumes never Stall directly; they move things into things that do (05 section 6.6).

## Shared rules

- **Force model (canon, 05 section 6.6 and CHANGELOG v0.2.2).** A volume applies a force authored as the acceleration it would give a 100 kg reference body, a_ref. A body of mass m receives a = a_ref x 100 / m, capped at 2 x a_ref. A Climber (90 kg) gets 1.11 a_ref, a Breaker (220 kg) 0.45 a_ref, a 45 kg propane cylinder the 2.0 cap, a 2,000 kg container 0.05 a_ref. Force volumes act on players and free bodies only: loose objects and loads held on a player's line. Not on kinematics, static world, cable-hung map objects (houses, the market ring, the Gantry load on its magnet), placed objects (racked, stacked, bolted, rated, or Heavy on a bare floor), lines, or Dogs.
- **Foot-hold.** Players inside a volume are airborne if their received acceleration exceeds their foot-hold (05 section 5.3: Climber 3, Scrapper 4, Rigger 5, Doubler 5, Breaker 5 m/s²; braced Rigger 9, braced Breaker 20). Below it they are pushed in a grounded skid.
- **Tells (05 section 11).** Arena-specific audio and particles, 1 s lead minimum before the volume applies force. Particles are the force's direction made visible: Weave scrap strips, dust, spray, mist.
- **Dogs in flight** are not acted on by the volume (players and loose objects only). The Updraft's Dog drift is an authored projectile effect: about 1 m of rise per 20 m of flight during the full phase (**PROPOSED**).
- **Stacking.** Overlapping volumes add as vectors.
- **Implementation.** Each is a `Kinetic_ForceVolume` overlap with a profile curve over time and a falloff curve over distance, evaluated on the server and predicted on clients from server time (all are either periodic or triggered by a replicated event).

## 1. The Updraft (the Breath, the Anchor)

Gusts rise from the Well on a 40 s cycle, strongest near the Stump (03). Heights use the Anchor's Rim datum (`arenas/the-anchor/layout.md`: Rim deck Z 0, Scaffold decks L1 to L5 at Z 6, 14, 22, 30, 38, Ring Z 42, Crown Z 60).

| Parameter | Value |
|---|---|
| Shape | Vertical column about the Stump axis from Z 0 to Z 70; below Z 0 the shaft is still air |
| Direction | Up |
| Period | 40 s, full from 32 to 37 s of each cycle |
| Profile over time | 27 to 32 s build (ease in), 32 to 37 s full, 37 to 40 s decay, 0 to 27 s calm |
| a_ref by radius at full | r 12 to 16 (the Well mouth, the core): 14 m/s². Linear to 0 at r 24 (the Scaffold's outer rail). Above Z 60 the core also falls to 0 inside r 8, so the Crown center is calm. |
| Tell | 1.0 s before the build: an intake roar from the Well, pitch rising; dust, chalk, and Weave strips lift; Rim banners rise. 5 s from first tell to full. |

Worked at full in the core: a Climber receives 15.6 m/s², net 5.8 m/s² upward: lifted, 52 m in 5 s from rest inside the mouth. A Scrapper 12.7 (net 2.9 up). A Rigger 10.8 (net 1.0 up, a slow rise). A Breaker 6.4 (net 3.4 down, falls at a third of gravity). At r 20 the core value is 7.0: a Climber floats (7.8 against 9.8), a Breaker (3.2) stands. The Ascension: a Climber's Kick at 18 m/s straight up from L5's inner edge (Z 38) during the full phase rises under net acceleration of about 8.6 m/s² upward on top of the launch, passes the Crown lip (Z 60) at 25 m/s or more, and must Hang to the Crown within 7 m of it or Pendulum off a crane boom tip; without the Breath the same launch rises 30 m and clears the lip at about 9 m/s. Hang loads swing: a Medium barrel on a 10 m Hang at r 18 takes 10 m/s² of lift and goes slack. Dogs fired across the full Breath drift upward about 1 m per 20 m of flight (an authored projectile effect, not the volume acting on the Dog; force volumes act on players and loose objects only).

## 2. The Pulse (the Anchor)

The Stump's residual tension releases on a fixed clock: a radial shove outward, then an inhale back toward the Stump (CHANGELOG v0.2.2, canon). The Marshals count it down.

| Parameter | Value |
|---|---|
| Shape | Horizontal radial field about the Stump axis, r 3 to 44, Z -15 to 70 |
| Period | 75 s, first Pulse at 0:55 elapsed (canon). A Scrap round gets two (55 s and 130 s); a Claim match gets six (55, 130, 205, 280, 355, 430). |
| Shove | 0.4 s, outward. a_ref 16 m/s² at r 16 and inward of it, linear to 0 at r 44. On the Crown: 0 inside r 3 (the eye), rising to 16 at r 8. (**PROPOSED** falloff; the timings are canon.) |
| Inhale | 0.6 s, immediately after the shove, toward the Stump. a_ref 8 m/s² from r 12 to 24 (the Well mouth and the Scaffold decks), linear to 0 at r 30. On the Crown it pulls toward the eye and is harmless. |
| Tell | Marshal count from five; the Stump's hum climbs and its glow pulses brighter on each count; bass thud at 0.0; whip-crack at 0.4 s; reverse whoosh for the inhale. 5 s lead. |

Worked at r 17 (a deck's inner meter, a_ref 15.4): Climber 17.1 m/s² for 0.4 s, 6.8 m/s outward, airborne (over 3); Scrapper 14 m/s², 5.6 m/s; Rigger 11.8, 4.7 m/s, airborne unbraced, holds braced (9 is under 11.8: **the braced Rigger also goes**); Doubler 11.8, airborne; Breaker 7.0, airborne unbraced (5), holds braced (20). Only a braced Breaker, or anyone Hung to a radial anchor, stays put at the inner edge. An airborne Climber at 6.8 m/s crosses the 8 m deck in 1.2 s and meets the outer rail at 6.8 m/s: a stagger against static, over the rail only if they were already past mid-deck. Then the inhale: a_ref 8 for 0.6 s moves a Climber 5.3 m/s inward, a Breaker 2.2 m/s. It catches whoever cut their Hang early or stood up after the shove inside 3 m of the inner edge, and the inner edge is the Well mouth. Loose objects: a 100 kg barrel gets 6.4 m/s outward then 4.8 m/s back; a 45 kg propane cylinder gets the 2 x cap, 30.8 m/s² for 0.4 s, 12.3 m/s outward, over its 8 m/s trigger, and detonates on first contact. The Pulse is the Anchor's Scatter. The Claim marker on its runners at the Crown center is inside the eye and feels nothing until Hauled past r 3.

The Scrap tie-break "shrink event" in 05 section 9.1 (the Anchor's is Held Breath, see `arenas/the-anchor/events-and-moving-parts.md`) is a separate arena system and is not the Pulse.

## 3. Water rush (Ballast tank rupture)

A water tank (Heavy, fracture 1,500 N s) releases a force volume (05 section 6.5).

| Parameter | Value |
|---|---|
| Shape | A wedge from the tank's rupture face: 4 m wide at the tank, 8 m wide at 12 m, 1.2 m tall, aimed downslope toward the nose at the moment of rupture (the Breathing's current lean decides the exact direction, within 15 degrees of the nose axis) |
| Direction | Along the wedge |
| Duration | 4 s |
| a_ref over time | 14 m/s² at 0 s, linear to 0 at 4 s |
| Falloff | Full to 8 m, linear to 0 at 12 m |
| Tell | 1.0 s: the tank's shell pieces part with a spray burst before the volume applies (the fracture is the tell; the rush is delayed 1.0 s, authored) |

Worked: a Climber at 4 m in the first second receives 15.6 m/s²: airborne and carried 7 m in 1 s. A Breaker receives 6.4: over their 5 m/s² foot-hold, airborne and carried 3 m; braced (20) they wade. Props: a 180 kg drum receives 7.8 m/s², 5 m/s after a second, and is carried into the nose sump. The Hold flood signature play (03): rupture a tank as a Scrapper and ride the rush out while the enemy is carried into the nose. The nose is the Well: a player carried 12 m toward it from a tank 10 m from the lip is over the edge at 3 s unless they Hang to the deck. Two tanks rupture in the drive dog piston's consequence (see `rated/drive-dog-piston.md`).

## 4. The Hold flood (Ballast standing water)

The Hold's 0.6 m seep water (1.4 m in the nose sump) shifts with the Breathing and shoves whatever is standing in it (03).

| Parameter | Value |
|---|---|
| Shape | The Hold's water footprint, 0.6 m tall (1.4 m in the sump) |
| Direction | Along the deck, downslope, following the hull's pitch angle |
| Profile | a_ref = 6 m/s² x sin(angle) / sin(6 degrees), continuous; zero at level, full at the 6 degree peaks, so a 90 s period with peaks at 22.5 s (toward the nose) and 67.5 s (toward the Spine) |
| Tell | The water's surface tilt and foam line (continuous); `SFX_Hold_Water_Shift` rises with tilt rate |

Worked at peak lean toward the nose: Climber 6.7 m/s² (airborne, swept), Scrapper 5.5 (swept), Rigger 4.6 (a grounded skid toward the nose at up to the water's speed, holds braced), Doubler 4.6 (skid), Breaker 2.7 (wades). Everyone is pushed toward the nose for 20 s of every 90, and the nose is a ramp into the Well on that lean. The sump (1.4 m, x3.0) is the one soft landing in the Hold and sits at the low end of the shove, 6 m from the lip: the design is that the Hold's safest surface is also the one the water carries you past. Blasts under more than 1 m of water (the sump) deliver half impulse and no fire (05 section 6.4).

## 5. Gas jet (manifolds; the press header)

A compressed-gas manifold is fixed world geometry; a Jack over 600 N applied to it, or a blast, vents a 6 m directional jet at 15 m/s for 3 s that pushes and does not kill (05 section 6.4). Manifolds sit where cutting crews keep gas: four in Ballast's Bays, three on the Yard Floor, two in the Anchor Rim's contractor compound, none in Hangtown.

| Parameter | Value |
|---|---|
| Shape | A cone from the vent flange, 0.4 m at the flange to 2.5 m at 6 m |
| Direction | Along the vent's authored axis (always across a walkway or toward an edge; never into a wall) |
| Duration | 3 s (the press header: the rest of the round) |
| Profile | Velocity-targeting: force drives the body toward 15 m/s along the axis with a_ref = 12 m/s² x (1 minus v_along / 15); zero once the body moves at 15 m/s with the jet |
| Tell | 1.0 s hiss and a valve-relief shriek before the jet; frost forms on the flange |

Worked: a Climber at the flange receives 13.3 m/s² initially: airborne, at 10 m/s within 1.2 s, across a 4 m catwalk and off it. A Breaker receives 5.5: at or over their 5 m/s² foot-hold, airborne for the first meter until the jet's velocity targeting eases off; braced (20) they stand in it. Props: a propane cylinder (45 kg) receives the 2 x cap, 24 m/s², reaches 15 m/s in 0.9 m, and leaves at its 8 m/s trigger speed: a manifold with a cylinder cradled beside it is a launcher. The Yard's press header (see `rated/press-counterweight.md`) is the only jet with no end.

## States and VFX

| Volume | Idle | Tell | Active | Visual |
|---|---|---|---|---|
| Updraft | still air | inhale rumble, dust lifts | gust | Weave strips spiraling, dust columns, banner lift |
| Pulse | hum | countdown, hum rises | shove | Stump flash, dust sheet, props sliding outward |
| Water rush | tank intact | spray burst | wedge of white water | foam front, debris carried |
| Hold flood | water level | surface tilts | current | foam line at the low edge, ripples |
| Gas jet | frost-free flange | hiss, frost | jet | white vapor cone, dust blown |

## Technical

- `BP_ForceVolume` base with children per system. Profile and falloff as curve assets. Force applied per physics step to overlapping `Kinetic_Prop`, `Kinetic_Heavy`, `Kinetic_RatedLoad` (dynamic only), and player bodies (airborne ragdolls receive force; grounded characters receive a drag per 05 section 5.1 against their foot-hold).
- Periodic volumes (Updraft, Pulse, Hold flood) are evaluated from server time with no replication. Triggered volumes (water rush, gas jet) spawn from a replicated event with a server timestamp.
- Particle budgets: Updraft 20 strips plus 4 dust columns; Pulse one dust sheet (a mesh) plus 40 slide decals; water rush one GPU fluid surface per rush, max 2; Hold flood a tilt-driven plane; gas jet one cone emitter each, max 3 live.
- Debug: a console view draws a_ref isolines and per-class airborne radii.

## Reference search terms

thermal updraft dust devil, blast wave dust ring slow motion, tank rupture water surge industrial, sloshing water ship hold, compressed gas venting frost, cooling tower updraft debris.
