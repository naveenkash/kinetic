# THE YARD: Events and Moving Parts

_Arena: The Yard (Collar Four Breaking Yard). File purpose: the Haulers' timetable, the Gantry's cycle, Shift Change, secondary cycles, the Lockout sudden-death event, every tell with its lead time, and reset rules._
_Sources: docs/bible/03-arenas.md, docs/bible/05-core-mechanics.md. Anything not in those files is marked **PROPOSED**._

Time notation: **e** is seconds elapsed since the Marshal calls "live." **c** is the Gantry's cycle time (0 to 120). Zone names are from layout.md.

## 1. The Haulers (docs/objects/kinematics/rail-hauler.md)

Three kinematic rail trucks, one per trench, 14 m long, 12 m/s, carrying a hull plate section whose top rides at +3 m above the Floor. South and North trenches run west to east. The Center trench runs east to west and crosses the press pit on two stringers at -4 m.

### Timetable (normal)

| Trench | First nose-in (e) | Period | Direction | Klaxon tone (**PROPOSED**) |
|---|---|---|---|---|
| South (Y -30) | 10 | 45 s | West to east | Single low horn |
| Center (Y 0) | 25 | 45 s | East to west | Two-tone |
| North (Y +30) | 40 | 45 s | West to east | Single high horn |

A train enters somewhere every 15 s. A full crossing is 13.3 s nose-in to nose-out and 14.5 s until the tail clears.

### One South crossing, second by second

| Time | Event |
|---|---|
| t0 - 3.0 | Klaxon, arena-wide, trench tone. The entry-end lamps (X -80 to -56) go amber. |
| t0 - 2.0 | Headlamp visible in the west plenum. |
| t0 | Nose enters at X -80. Lamps light 24 m ahead of the nose as it runs (2 s local warning at every point). |
| t0 + 2.0 | Nose at X -56, passing under West Mesa. |
| t0 + 2.5 | Nose under grate 1 (X -50). |
| t0 + 4.2 | Nose at X -30. The load sweeps the plank bridge on the first pass of the round (fractures to Light planks, 10 s life). |
| t0 + 5.8 | Nose under grate 2 (X -10). |
| t0 + 7.5 | Nose at X +10, second plank bridge. |
| t0 + 9.2 | Nose under grate 3 (X +30). |
| t0 + 12.5 | Nose under grate 4 (X +70). |
| t0 + 13.3 | Nose exits at X +80. |
| t0 + 14.5 | Tail clears. Lamps return to dim amber. |

The Center crossing mirrors this from X +80: nose at the pit's east stringer at t0 + 2.3, clear of the pit at t0 + 4.7, exit at t0 + 13.3.

### Contact rules

- Player in a trench when a train arrives: hit by a kinematic at 12 m/s. The Stall threshold against a kinematic is 9 m/s (05 section 7.1). Scrapped.
- Player on a trench lip: the load is 5 m wide inside a 6 m trench. No contact. A player leaning or ragdolled over the lip is clipped.
- Player on the Hauler deck or load: rides at 12 m/s. Stepping off onto the Floor is a 12 m/s relative impact against static. Stall. Stepping off onto a 2.7 m ledge is the same. Jump off the back: your 12 m/s forward is kept; Hang to something first, or wait for the end plenum, where the train stops for 2 s before despawning (**PROPOSED**) and the deck is level with the plenum floor.
- Hang line or set line crossing a trench: severed when the train passes through it (05 section 3.5: a kinematic severs any line it passes through). Snap plays, both Dogs release. Anything standing on the line falls into the trench. A line attached to the Hauler itself (the Caboose) is not severed by it.
- Rated hull section pulled into a trench ahead of a train (the Plow): the train carries it as a 12 m by 2.7 m wall whose ends overhang both lips by 3 m, at 12 m/s, until the end plenum, where it is dumped and removed.
- Interlock (**PROPOSED**): if a Center train would be on the pit stringers within 6 s of a Gantry drop, it holds at the east plenum and re-sounds its klaxon before entering. It fires once in Claim (the e 293 drop lands 4.3 s before the e 297 train) and never in Scrap, Haul, or Free Scrap.

## 2. The Gantry (docs/objects/kinematics/gantry-crane.md)

A 60 m bridge spanning Y -30 to +30 at 36 m, traversing along X, with the load on a 6 m hoist line during traverse. Pick at the Pick Stack (X -50, Y +14). Drop into the press (X +45, Y 0). Load: a hull plate section, 2,800 kg, Heavy, on the electromagnet.

### The 120 s cycle

| c (s) | Gantry | Load | Trolley Y | Sound |
|---|---|---|---|---|
| 0.0 | Bridge at X -50 | Magnet seats on the Pick Stack top. Break-free clank (500 N, in-stack). | +14 | Magnet thump, stack creak |
| 2.0 | | Lift begins, 2.2 m/s | | Hoist motor |
| 10.0 | | Load top at 30 m, underside at 27.3 m | | |
| 10.0 to 70.0 | Traverse east at 1.6 m/s | Hanging, swings up to 1.5 m in the wind | +14 to 0, 0.23 m/s | Rail rumble, continuous |
| 40.0 | X = -2 | Over Marker Flat, 15 m east of the marker | +5 | |
| 70.0 | X = +45, over the pit | Lowering begins, 1.8 m/s | 0 | Hoist motor |
| 78.0 | | Load underside at 9.3 m. **Drop tell: lamps on the magnet go red, two-tone descending, 2 s** | | Tone |
| 80.0 | | Release. 1.8 s fall, 17.6 m/s into the ram. | | Impact, the press takes it |
| 85.0 | Return west at 2.7 m/s, magnet hoisted to 34 m | Empty | 0 to +14 | Rail rumble, higher pitch |
| 120.0 = 0.0 | X = -50 | Next pick | +14 | |

### Phase at "live"

The cycle is phased per mode so that at the end of the Shift Change siren the Gantry is 22 s into its loaded traverse, over the west half of Marker Flat (**PROPOSED**).

| Mode | Round | Shift Change siren ends at e | Phase c at live | Drop point at Shift Change |
|---|---|---|---|---|
| Scrap | 2:30 | 80 | 72 (lowering; first pit drop at e = 8) | X -15, Y +9 |
| Claim | 8:00 | 245 | 27 | X -15, Y +9 |
| Haul | 4:00 (per half) | 125 | 27 | X -15, Y +9 |
| Free Scrap | 6:00 | 185 | 87 | X -15, Y +9 |

In Scrap: drop into the pit at e 8, pick at e 48, lift to e 58, traverse e 58 to 118. Shift Change interrupts at e 80.

### Contact rules

- Standing on the bridge beam: free transport. At 1.6 m/s east, 2.7 m/s west. The end trucks shove a player off the rail head at that speed; neither is a stagger until Shift Change doubles them.
- Dog the load or the hoist cable: Hang and you ride under it. A Haul cannot hoist you to it (every pull tension is under body weight). Hold Haul inside the Hang to Reel (05 section 3.2): the rod shortens at 1.0 m/s and you climb to the load, 24 m in 24 s from NC2's top; hold Jack to let yourself down.
- Under the load at the drop: 2,800 kg at 17.6 m/s. Nothing survives it. The 2 s tone is the whole warning. The pit is the Well anyway.
- Load swing: a 1.5 m lateral swing on a 6 m hoist line at 30 m. Anyone perched on the load is carried, not thrown.
- Strip the load (Breaker only): the load is a rated attachment (05 section 6.7). A Breaker's Haul over its 1,000 N break-free releases it from the magnet and it falls from wherever the Gantry is, with the 1.0 s rated groan and no drop tone. The only place within a Breaker's 20 m line of the load is the bridge beam itself. The Gantry finishes its cycle empty.

## 3. Mid-match event: Shift Change

Fires with a 5 s siren at 1:15 remaining in Scrap (e = 75), at 4:00 remaining in Claim (e = 240), and at 2:00 remaining in each Haul half (e = 120), per 05 section 9.2. In Free Scrap it fires at 3:00 remaining (e = 180). The table below is written in Scrap time. Add 165 s for Claim, 45 s for Haul, and 105 s for Free Scrap.

| Time | What happens | Where it reads |
|---|---|---|
| e 75.0 | **Siren, 5 s.** LayCast PA: "Shift change." Every trench lamp goes steady red (no strobe, map rule 8). The Gantry's hoist brake alarm sounds. | Everywhere |
| e 77.0 | The Gantry's magnet lamps go red and the drop tone starts, 2 s early so the drop tell overlaps the siren. | Rails, Marker Flat |
| e 80.0 | Siren ends. **The Gantry releases its load wherever it is.** In the authored phase: from 30 m over (-15, +9). If the magnet is empty (a Breaker stripped the load, or the Pick Stack top was pulled off before the pick), nothing drops and the Gantry goes straight to its early pick at the East Pick Stack. | Marker Flat west |
| e 80.0 | **Hauler timetables reset.** South nose-in at e 83 (klaxon at 80), Center at e 90.5, North at e 98, then every 22.5 s per trench. A train somewhere every 7.5 s. | Trenches |
| e 82.4 | The load lands at 23 m/s. 12 m by 6 m by 2.7 m, flat. Anything under it is scrapped. It is fixed scenery from then on (05 section 6.1a). The heap at (-10, +6) scatters. A 3 m debris shove: force volume, radial, a_ref 12 m/s² for 0.5 s on a 100 kg body, scaled by 100 / m, players and loose objects only (05 section 6.6). | Marker Flat |
| e 80 to 94 | The Gantry returns east empty at 5.4 m/s (double speed) to the East Pick Stack (X +62). End trucks now shove at 5.4 m/s: a player on the rail head is staggered (over 4 m/s) and pushed off. | Rails |
| e 94 to 109 | Pick (e 94), lift 5 s, traverse 17 m west to the pit at 3.2 m/s, lower 5 s, drop tone at e 107, drop at e 109. | East Pick Stack, Press |
| e 109 to 129 | Return, pick again at e 129, lift, traverse, lower; normal speed resumes at e 140 mid-lower; drop at e 149. | Press |
| e 140 | Haulers return to the 45 s period, counted from each trench's last train. Lamps back to amber. The Gantry keeps picking east; the dropped load stays where it fell. | |

Demolition only goes one way. Nothing in this list makes the map safer.

### Zone by zone, after

| Zone | Before | After Shift Change |
|---|---|---|
| Trenches | One train per 15 s somewhere. | One per 7.5 s for 60 s. Crossing on foot at Floor level is a klaxon-to-klaxon read of 4.5 s. Then one per 15 s again, re-phased. |
| Marker Flat | Open Floor west of the marker. | A 2.7 m plate at X -21 to -9, Y +6 to +12: new cover between NC1 and the marker, a new ledge, and a dead heap. The Floor sightline from the Pick Stack to the marker is gone. |
| Pick Stack | 8 m, live top. | 5.4 m. Static. |
| East Pick Stack | 5.4 m, inert. | Live top (Heavy, 500 N break-free) until the Gantry takes it at e 94. Then 2.7 m. |
| The Press | A drop every 120 s. | A drop every 20 s for the next 60 s, then every 60 s, all from the east. The press approach is now under the Gantry's path. |
| Rails | Gantry at 1.6 to 2.7 m/s. | 3.2 to 5.4 m/s for 60 s. Rail-head contact staggers. |
| Haul Lane, docks | Trains beside you. | Trains beside you twice as often. Plank bridges are long gone. |
| Spawns | Spawn B is 71 m by walkway from a quiet pit. | Spawn B overlooks a busy one. Reserves dropping at B should expect Gantry traffic overhead at 30 m. |

## 4. Secondary cycles

| Cycle | Period | Detail |
|---|---|---|
| Press ram | 20 s | Rises -10 to -6.5 m over 8 s, holds 2 s with a clang, falls 10 s. Audible everywhere; the clang is the map's metronome. The kill plane at -6 m does not move. |
| Trench lamps | Continuous | Dim amber. Bright amber 24 m ahead of a train. Red during Shift Change. |
| Plank bridges | One-shot | Six crossings (fracture 600 N·s) that exist until the first train in their trench, or until a Breaker jumps onto one. South's go at e 14, Center's at e 29 and 31, North's at e 44. |
| Pick Stack tops | One-shot each | Live Heavy bodies that a Rigger, Doubler, or Breaker (over the 500 N stack break-free) can Avalanche off the stack before the Gantry arrives. If one is pulled off, the Gantry picks nothing and runs its cycle empty (**PROPOSED**); after Shift Change it picks R2 instead. |
| Press counterweight | One-shot (rated) | A Breaker's Haul, a Doubler at overspin, or crew lines summing to 1,000 N on one axis: 1.0 s groan, cable pays out, the 6,000 kg block swings north over Marker Flat on an 8 m radius (2 swings, 6 s), then lands at (45, +14) as a permanent block. Anything under the swing arc at 3 to 6 m height is hit at up to 9 m/s by 6,000 kg: scrapped. The press keeps cycling. |
| Control shack | One-shot (rated) | A Breaker's Haul, a Doubler at overspin, or crew lines summing to 1,000 N on either column: 1.0 s groan and dust, then the shack drops 10.5 m onto the North Dock (X -66 to -58, Y +34 to +44). Anyone on it hits at 14.3 m/s: scrapped. Anyone under it: crushed. Roof floodlights die; the west third of the Rails (X -80 to -27) and the west end of Marker Flat go dark. Lines light that sector. |
| The Plow | Event (rated) | A rated hull section Hauled (Breaker, Doubler at overspin, or crew lines summing to 1,000 N) off its greased skids slides down them to the nearest lip and drops in (authored slide at 0.2 to 0.27 m/s², 4 s, visible). The next train carries it. |
| Demolition charges | One-shot | Impact over 6 m/s by an object or a player on either charge at (+20, +30, 3) (Dogs seat in a charge without triggering it): 5 m blast, sheds the north leg's ladder cage from 0 to 12 m. The honest route up that leg is gone for the round. |

## 5. Sudden death: Lockout (**PROPOSED**)

Fires when a Scrap round times out with equal riggers standing (05 section 9.1). The fiction: end of shift, the Yard drops its blast curtains to clear the lot.

| Time | Event |
|---|---|
| T+0 | Marshal: "Time. Lockout." Siren, 5 s. Trench lamps red. |
| T+5 | The four corner floodlight masts cut, west pair then east pair 2 s apart. The shack and press lights stay. Line emissives are the light on the docks. |
| T+5 | Two Lockout curtains (prop-manifest.md, kinematic, 90 m by 36 m) unpark from X = ±80 and advance inboard at 1.0 m/s, lit from behind. Lines they pass through are severed (05 section 3.5); lines attached to a curtain itself are not. Props are pushed ahead of them and into the trenches. A player pinned between a curtain and anything solid is crushed (over 8 kN for 0.5 s). |
| T+5 | All Haulers run continuously: 22.5 s per trench, staggered 7.5 s. The Gantry parks at X = 0 with the magnet hoisted. |
| T+25 | Curtains at X ±60. Both spawn mesas and the shack are behind them. |
| T+45 | Curtains at X ±40. The East Pick Stack and the pit's east half are swept. |
| T+75 | Curtains at X ±10 and hold. Playable band: X -10 to +10, all Y, all heights: the marker, the spool start, the east half of NC2, grate 2 at X -10. The center trench runs through it. |
| T+90 | If more than one crew remains: curtains resume and close to X ±1 by T+99. The round is a draw and is replayed (**PROPOSED**). |

The playable area shrinks from 160 m to 20 m over 70 s. The trenches stay live the whole time. A crew that holds NC2's top (8 m) is above the Floor traffic but inside the curtain's height, so the curtain still comes for them.

## 6. Tells (05 section 11)

| System | Audio | Visual | Lead |
|---|---|---|---|
| Hauler approach | Trench-tone klaxon, arena-wide | Entry-end lamps amber, then lamps 24 m ahead of the nose | 3.0 s at the entry end, 2.0 s locally |
| Gantry drop | Two-tone descending | Magnet lamps red | 2.0 s |
| Gantry traverse | Rail rumble | Moving shadow under the load | Continuous |
| Shift Change | Siren, PA | Lamps red, brake alarm lamp on the cab | 5.0 s |
| Rated load (counterweight, columns, R1 to R4) | Structural groan | Dust, tape flutter, authored debris | 1.0 s minimum before consequence |
| Billet stack topple | Scrape, then a slow groan | 1.5 s visible lean (**PROPOSED** settle time) | 1.5 s |
| Break-free (any racked or stacked object) | Lurch, scrape | Shudder | 0.2 s |
| Press ram | Hydraulic rise, clang at top | Ram lamps in the pit | 20 s period, continuous |
| Plank bridge under load | Creak | Sag | 0.3 s |
| Volatile armed | Hiss | Venting | 0.3 s |
| Gas manifold Jacked | Valve shriek | Vapor | 1.0 s |
| Ring-out imminent (pit) | Hooks spin up in the north wall hatch | | 0.5 s |
| Lockout curtain | Chain-mail drag, continuous | Backlit wall advancing | Continuous, 70 s to close |
| Line under force, set lines, overspin, Hang slip | Per 05 section 11 | Per 05 section 11 | Per 05 section 11 |

## 7. Reset rules

Per 05 section 9.2, at round end:

1. All props return to authored positions and states. Scrap heaps re-pool. Plank bridges are re-laid.
2. Rated-load consequences reset: the counterweight is rehung, the shack rebuilt on both columns with its floodlights, R1 to R4 back on their blocks, the ladder cage restored.
3. Gantry loads dropped on the Floor are removed. Both Pick Stack tops are restored.
4. The Hauler timetable restarts from "live" (South e 10, Center e 25, North e 40). The Gantry restarts at its mode phase.
5. Lockout curtains park at X = ±80. Corner masts relight.
6. Set lines are cleared. Volatiles are replaced.

Within a round nothing resets. A plank bridge swept at e 14 is gone until the next round. The Shift Change plate stays. The shack stays down.

## 8. In the engine (PROPOSED where noted)

`AKineticYardDirector` (game/Source/KineticGameplay) runs this document. Where it departs from it:

- **Clock.** The match's mid-match event (FScrapMatch: 1:15 remaining on a clock that includes the 6 s Drop) is Shift Change's payload, the siren 5 s before it; every Shift Change time above is taken relative to that moment rather than to e 80. The Gantry's phase at "live" is set so that it is 22 s into its loaded traverse at the payload, as section 2 asks.
- **Picks.** Each pick takes a fresh 2,800 kg plate; the Pick Stack tops are not separate bodies yet, so they cannot be Avalanched off before the Gantry comes.
- **After Shift Change** the Gantry cycles the East Pick Stack every 35 s for 60 s, then every 60 s (section 3's times are not all consistent with one another).
- **The Plow.** A committed hull section slides to its lip over 4 s, then lies across the trench on both lips; the next train in that trench carries it 3 m ahead of its nose until the plenum.
- **Lockout.** The corner masts' lights are not built yet, so the curtains do not darken them; the round replays at T+90 (FScrapRules), so the final close to ±1 m is never reached in Scrap.
- **Not yet built:** the press counterweight's hit at up to 9 m/s is judged on the kinematic table (a Stall at 9 m/s, a flip below); the billet stack topple, the Hauler's 2 s stop in the end plenum and the Interlock.

