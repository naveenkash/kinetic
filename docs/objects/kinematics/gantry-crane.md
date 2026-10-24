# Gantry Crane

One-line: kinematic, infinite mass (crane), 60 m span bridge (Y -30 to +30) on two portal legs, rails 36 m above the Floor (bridge top walkable, 1.5 m wide; cab at the north end at 40 m), trolley with a 2.4 m electromagnet on a 4-fall hoist; the Yard: one, traversing X -50 (the Pick Stack) to X +45 (the press pit) along a 160 m Yard, carrying a 2,800 kg hull plate section (Heavy, a rated attachment per 05 section 6.7) on a 120 s cycle. Geometry and schedule per `arenas/the-yard/layout.md` and `events-and-moving-parts.md`; heights here reflect the Rails at 36 m.

## Role in play

The Gantry is the Yard's slow clock. Its load is the Magnet Drop: a hull plate that falls from the hoist on a scripted release, over a Claim marker if you timed it. Players ride the load, Dog it, swing from it, and stand under it exactly once (03). The crane itself is the Rails tier: its bridge beam is the only high ground, 36 m up, and it moves. It is kinematic, so it severs any line it passes through (05 section 3.5): a Gallows hung across its path, a Hang footbridge between the rail walks, a Clothesline on the bridge beam's track, all snap as the bridge or the load goes through them. Lines attached to the Gantry or its load are not severed by it.

| Harness | Relationship |
|---|---|
| Climber | Owns the bridge deck. Hangs from the load for a free 2 m/s traverse at 25 m up. The Rails are where Climbers are made to leave by nothing but a Breaker on the control shack column. |
| Scrapper | Rides the load with a propane cylinder lined to it: a Magnet Drop with a blast on arrival. |
| Rigger | Hangs a Clothesline across the bridge deck walkway. A Rigger on the Rails owns a 60 m corridor. |
| Doubler | Overspins a line from an enemy to the load 2 s before release. |
| Breaker | Can release the load early (rated attachment, 05 section 6.7): any active pull reaching 1,000 N on the load (a Breaker alone, a Doubler at overspin, or a crew summing to it) held 0.6 s. Riding it down is a Stall unless they Jack off above 7.3 m. |

## Appearance

Silhouette: a double-girder box bridge on A-frame legs with rail bogies at the Floor's edge, a trolley at mid-span, four falls of rope to a round electromagnet, and a walkway along the bridge with a handrail. Guild ochre on the girders, Office chevrons on the legs, HTR orange on the trolley (HTR supplied it under dispute).

| Part | Material | Base color | Roughness | Metallic |
|---|---|---|---|---|
| Bridge girders | box section plate, Guild ochre | #B8752A | 0.6 | 0.6 |
| Legs | lattice, chevroned at the base | #B8752A, #F2C500 / #141414 | 0.6 | 0.6 |
| Trolley | HTR orange | #E8541E | 0.5 | 0.6 |
| Magnet | cast steel, black, bright contact face | #1A1A1C / #A8ADB1 | 0.5 | 0.9 |
| Rope | 28 mm wire, greased | #3C3F42 | 0.35 | 0.9 |
| Walkway | grating, handrail | #5E6366 | 0.6 | 0.9 |

Weathering: the ochre is chalked by sun on top faces and oiled black under the trolley track. Lightning scars on the leg tops. The legs carry a cage ladder each (the honest route from the docks to the Rails, 36 m, 23 s at a Breaker's 1.6 m/s) and the north leg's cage can be blown off by the demolition charge at its foot. Markings: `LAY GUILD  C4  GANTRY 1  SWL 40 t` on the girder face in 1.2 m white letters, readable from the Floor; HTR's `UNIT ON LOAN  HTR-M-22` on the trolley; a Guild chalk board on the walkway listing today's lifts. Lighting: two 4000 K floodlights under the trolley lighting the load; red magnet lamps and a two-tone descending tone that start 2 s before release (05 section 11, Gantry drop lead; no strobe, per map rule 8); walkway strip lights; red aircraft beacons at the leg tops.

Read by tier: from the Rails, you are on it. From the Stacks, a lit load crossing overhead with its own shadow sliding over the mesas. From the Floor, girders against sky and a strobe when it matters.

## Variants

1. **Loaded traverse.** Hull plate on the magnet, load underside about 21 m above the Floor (hoisted 11 m above the 8 m Pick Stack top plus the plate's thickness), swinging up to 1.5 m in the wind.
2. **Empty return.** Magnet raised to 32 m, faster (2.7 m/s).
3. **Shift Change early drop.** The load releases wherever the crane is when the siren ends, with the standard 2 s tone. An empty magnet drops nothing; the next pick comes early from the East Pick Stack (canon, CHANGELOG v0.2.2).
4. **Seasonal colors.** Not applicable (the Anchor's cranes take the season colors, not the Yard's).

## Physics

- Crane: kinematic, infinite mass. Bridge traverse 1.6 m/s east loaded, 2.7 m/s west empty (3.2 and 5.4 during Shift Change); trolley 0.23 m/s across the span; hoist 2.2 m/s up, 1.8 m/s down. A player hit by an end truck at 1.6 to 2.7 m/s: under the 4 m/s kinematic stagger, shoved off the rail head; at 5.4 m/s during Shift Change, staggered and pushed off. Crush between a leg and a mesa (0.5 m clearance on the north leg's path past NC1, authored): over 8 kN for 0.5 s, scrapped. The one place the crane kills by itself.
- Load: a simulated rigid body (2,800 kg) on a constraint to the magnet (03 engine notes), released by script. While hanging it swings as a pendulum of length 11 m (period 6.7 s) with the crane's acceleration and with lines on it. A line does not usefully swing it (a Breaker's 1,000 N gives 0.36 m/s², a 0.4 m offset). The load is a rated attachment (05 section 6.7): any active pull reaching 1,000 N on it (a Breaker alone, a Doubler at overspin, or a crew summing to it on one axis), sustained 0.6 s (README), releases it from the magnet early after a 1.0 s groan and with no drop tone. That is the counter to a Magnet Drop timed by someone else: drop it now, where it is. Force volumes do not act on it while it hangs (cable-hung map object).
- Release over the pit: the hoist lowers to 9.3 m underside, 2 s tone, then the magnet drops the load: 1.8 s fall, 17.6 m/s into the ram. On a player: scrapped at any contact. A Shift Change drop from traverse height (21 m underside) lands at 20.3 m/s on the Floor: 4 m impact decal, no fracture (hull plates do not fracture), fixed scenery thereafter (05 section 6.1a), with a 0.5 s debris shove. On a billet stack: tips it.
- Players on the load at release: airborne with it, 17.6 to 20.3 m/s into a stopped object: scrapped unless they Jack off above 7.3 m (12 m/s at the ground) or Hang to the magnet (then they hang under the trolley, hoisted). Hanging players: look, cut, fire, cannot move (05 section 5.2); they climb the line by Reel at 1 m/s (05 section 3.2) to the trolley, 11 m, 11 s.
- Shift Change: the drop tone starts 2 s before the siren ends and the load releases wherever the crane is (in the authored phase, from 30 m over Marker Flat west). If the magnet is empty nothing drops; the Gantry returns east at double speed and picks early from the East Pick Stack, then drops into the press every 20 s for the rest of the 60 s event.

## Cycle and tells

120 s cycle (canon per `arenas/the-yard/events-and-moving-parts.md`), phased per mode so that the Shift Change siren ends 22 s into a loaded traverse. Scrap phase at Live: c = 72 (lowering over the pit; first pit drop at e = 8 s, pick at e = 48, traverse e 58 to 118, Shift Change interrupts at e = 80).

| c (s) | Gantry | Load | Tell |
|---|---|---|---|
| 0 | Bridge at X -50, magnet seats on the Pick Stack top (Z 8). | Break-free clank (500 N, in-stack). | Magnet thump, stack creak. |
| 2 to 10 | | Lift at 2.2 m/s to traverse height. | Hoist motor. |
| 10 to 70 | Traverse east at 1.6 m/s, trolley drifting Y +14 to 0. | Hanging, swinging up to 1.5 m. | Rail rumble, the moving shadow. |
| 70 | X +45, over the pit. | Lowering at 1.8 m/s. | Hoist motor. |
| 78 | | Underside at 9.3 m. **Magnet lamps go red, two-tone descending, 2 s.** | The drop tell (05 section 11, 2 s). |
| 80 | | Release. 1.8 s fall, 17.6 m/s into the ram. | Impact; the press takes it. |
| 85 to 120 | Return west at 2.7 m/s, magnet hoisted to 32 m. | Empty. | Rail rumble, higher pitch. |

In Claim (8:00) the pit takes a load at e 8, 128, 248, 368 (the Shift Change at 4:00 remaining interrupts the fourth traverse) and the Pick Stack tops run out: both Pick Stack tops are one-shot Heavy bodies, and if a crew Avalanches one off its stack before the Gantry arrives, the Gantry picks nothing and runs the cycle empty.

Audio tells: hoist motor, rail rumble (audible across the Yard, pitch by speed), magnet thump, the two-tone descending drop tone (2 s), release clack. Visual: load lights, the shadow, the red magnet lamps. Leads: 2 s for the drop, continuous for the traverse. There is no strobe (map rule 8: nothing strobes).

## Interactions

- **Haul** enemy to the load (Caboose, airborne version): they are dragged along at 2 m/s, 25 m up, until release.
- **Jack** off the load: a Slingshot from 25 m up. The Climber's descent to any mesa.
- **Hang** self to the magnet or hoist cable, step off the load: an 11 m pendulum ride under the trolley. A Haul cannot hoist you to it (every pull tension is under body weight); Hang from a mast platform as it passes and let the hoist lift you, or Reel up the line at 1 m/s.
- Chain: the load lands on a fuel drum cradle beside the pit: drums launched above 10 m/s, blast, fire patch at the Well's lip.

## States

Kinematic: no failure. Load states: `Picking`, `Carrying`, `DropTone`, `Released`, `Empty`.

## VFX and SFX hooks

`SFX_Gantry_Rail_Rumble`, `SFX_Hoist_Whine`, `SFX_Magnet_Energize`, `SFX_Magnet_DropTone`, `SFX_Magnet_Release_Clack`, `SFX_Load_Impact_Floor`; `VFX_Magnet_Lamps_Red`, `VFX_Load_Shadow` (a real shadow; confirm the directional light budget), `VFX_Load_Impact_Dust`.

## Technical

- LOD0 crane 60,000 tris, LOD1 25,000, LOD2 8,000, LOD3 girder boxes. Load uses the hull section LODs.
- Textures: `T_Gantry_Girder` 4K, `T_Gantry_Trolley` 2K, `T_Gantry_Magnet` 2K, `T_Yard_Decal_Atlas` 2K.
- Pivot at the bridge center at rail height. Sockets: `Trolley`, `Magnet_Face`, `Rope_0..3`, `Walkway_0..23` (Dog seat hints at 2.5 m), `Leg_N_Base`, `Leg_S_Base`, `Strobe`, `Flood_L/R`.
- Implementation: crane as a three-level kinematic (bridge on a spline, trolley on a lateral curve, hoist on a height curve), all driven by one server time-synced timeline. Load as a Chaos rigid body on a 6-DOF constraint locked to the magnet socket; release destroys the constraint. Shift Change release starts the drop tone 2 s before the siren ends and destroys the constraint when it ends. Line severing: the bridge beam, end trucks, trolley, hoist falls, and hanging load carry a swept volume tested against every line segment not attached to them each step; intersecting lines snap with the 05 section 4.2 recoil. Collision: crane `Kinetic_Kinematic`; load `Kinetic_Heavy` (hanging) then `Kinetic_WorldStatic` once it comes to rest on the Floor (fixed scenery, 05 section 6.1a).

## Reference search terms

double girder gantry crane scrap yard, electromagnet lifting scrap crane, rail mounted gantry crane legs, steel mill overhead crane trolley, crane magnet drop slow motion.
