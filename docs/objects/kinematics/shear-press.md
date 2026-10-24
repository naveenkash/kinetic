# Shear Press

One-line: kinematic, infinite mass, a 14 m x 14 m pit (X +38 to +52, Y -7 to +7) with sheer steel walls to Z -10 and a 10 m x 6 m x 2.5 m hydraulic ram cycling in it on a 20 s period, the Yard: center-east Floor, per `arenas/the-yard/layout.md`. The pit is the Yard's Well (see `environment/well-kill-volume.md`); the ram is its sound.

## Role in play

The press is the Yard's heartbeat and its ring-out. Every twelve seconds the ram comes down and the whole Yard hears it. Players do not fight the ram; nobody survives the pit long enough to meet it, because the kill volume sits on the pit floor. The ram's job is to be audible from everywhere (03), to give the Yard's Well a tell no other arena has, and to receive the Gantry's load. Its counterweight (see `rated/press-counterweight.md`) is the one way to stop it.

| Harness | Relationship |
|---|---|
| Climber | Perches on the press frame's crossheads, 6 m above the Floor, the best view of the east Floor. |
| Scrapper | Overboard into the pit from the west lip, where the lip has no rail. |
| Rigger | Hangs a Clothesline across the pit's north walkway. A Rigger holding the pit holds the Caboose's destination. |
| Doubler | Snapback off the frame to clear the pit in one jump. |
| Breaker | The only harness that can brace on the lip against anything pushing it in. The press counterweight is their lever. |

## Appearance

Silhouette: a four-post press frame straddling a rectangular pit, a crosshead beam carrying the ram's two cylinders, the ram itself a slab of steel with a shear blade on its leading edge, the pit walls of poured concrete with a steel-lined lip. A grated walkway on the north side, a rail on three sides, none on the west (the Haul truck's loading face). Red beacons on the frame posts pulse with the cycle.

| Part | Material | Base color | Roughness | Metallic |
|---|---|---|---|---|
| Frame posts | fabricated box steel, Guild red primer | #8C3B2E | 0.65 | 0.5 |
| Ram | forged steel, bright blade | #4B5055 / #B3B6B8 | 0.4 | 0.9 |
| Cylinders | chromed rods, grey bodies | #B3B6B8 / #6E7378 | 0.3 | 0.9 |
| Pit walls | concrete, oil-black lower half | #9A9590 to #2A2C2E | 0.8 | 0.0 |
| Lip lining | steel, chevroned | #F2C500 / #141414 | 0.65 | 0.6 |
| Beacons | red | #FF2A1A emissive | | |

Weathering: the pit's lower half is black with hydraulic oil and Weave dust; the ram face is polished bright by billets. Scrap welded to the frame where the Guild repaired it. Markings: `SHEAR PRESS 1  LAY GUILD  C4  CYCLE 12 s  STAND CLEAR` on the crosshead in white; a Guild chalk tally of cycles; an Office `WELL` stencil in 1 m yellow letters on every lip face, as on every Well in the game; a small brass plate on the north post listing names (the Guild's dead at this press, L.19 to L.60). Lighting: floodlights down into the pit from the frame, so the pit is the brightest hole on the map; the beacons; the Hook pad lights on the pit's south edge.

Read by tier: from the Rails, a lit rectangle with a slab moving in it. From the Stacks, the frame above the mesas and the beacons. From the Floor, a chevroned lip, a walkway, and a sound.

## Variants

1. **Cycling.** Default.
2. **Seated (post counterweight consequence).** Ram on the pit floor, beacons off, hydraulic mist across the east lip.
3. **Haul mode.** The west lip carries the Guild truck's loading ramp; the truck is the Haul delivery point 10 m west of the pit, placed so a spool that misses the ramp goes into the Well.

## Physics

- Kinematic, infinite mass. Ram stroke 3.5 m: it rises from Z -10 to Z -6.5 over 8 s, holds 2 s with a clang, and falls over 10 s. Peak speed 0.7 m/s. It severs any line it passes through (05 section 3.5): a Hang into the pit below Z -6.5 is cut on the next rise.
- Kill volume: the pit below Z -6 (see `environment/well-kill-volume.md`); the ram's top of stroke is -6.5 and never reaches it. Entering it is a ring-out, at 6 m of fall (10.8 m/s) from the lip. Hooks spin up 0.5 s before entry per 05 section 7.5, from a hatch in the pit's north wall at Z -5.
- The ram and players: in practice none. A player hanging from a line into the pit above Z -6 (a Hang from the north rail, 5.5 m down) is above the ram's reach; a line that dips below -6.5 is severed on the ram's rise and its owner enters the kill volume. The ram never crushes a player against the floor because the floor is inside the kill volume.
- Objects: the ram shears anything on the pit floor. Objects entering the pit are destroyed on reaching the floor (pooled props despawn; Heavy bodies play a crush and despawn at the next ram dwell). The Gantry's hull section lands on the pit floor at 22 m/s, sits 0 to 12 s, and is removed at the next ram contact with `SFX_Press_Shear_Heavy`, the loudest cue on the map.
- Press frame: static world. Dogs seat. The crossheads are a 6 m perch.
- Lip: steel, x1.0. The west lip has no rail and a 0.1 m lip; ledge grab works under 8 m/s. Overboard here is a T push, not a Kick: the Kick never fires when a Dog is in a player's plate (05 section 3.2 interlock). A Breaker's 1,000 N on a Climber is 11 m/s², airborne at once, 4.7 m/s after a second, over the 0.1 m lip and grabbing it (under 8 m/s) unless the push continues: the Jack keeps pushing at T until line length, so the grab fails on the second second. The victim has that second. That is the panic window Overboard is built to give.

## Cycle and tells

20 s cycle, continuous from round Live, canon per the arena events file.

| Time | Ram | Tell |
|---|---|---|
| 0 to 8 s | Rises from Z -10 to -6.5. | `SFX_Press_Rise` (hydraulic whine), ram lamps in the pit brighten. |
| 8 to 10 s | Holds at the top. | `SFX_Press_Clang` at 8.0 s: audible across the whole Yard at a fixed level, not attenuated beyond 40 m (authored: it is the map's metronome). Beacons solid. |
| 10 to 20 s | Falls to Z -10. | `SFX_Press_Descent` (falling whine), floor dust puffs within 10 m of the pit at 20 s. |

Audio lead: the clang is the time reference players count from; the rise whine is 8 s long. Visual: beacons, pit floodlights dim 20% during descent (hydraulic draw), dust. The press is not a lethal system in itself; its tells exist for navigation and for the Well.

Shift Change: no change to the press. Counterweight consequence: see `rated/press-counterweight.md`; the ram seats and the beacons go out, and the Yard loses its clock for the rest of the round, which is the intended cost.

## Interactions

- **Haul** enemy to the frame post across the pit: they cross the pit on the line, airborne, and hit the post at up to 25 m/s (static, Stall at 12). Cut mid-crossing: they fall into the Well. Either way.
- **Jack** off a frame post: a Slingshot along the east Floor.
- **Hang** from the north rail into the pit: a hiding place 5.5 m down, above the kill volume. A Reel (hold Haul inside Hang, 1 m/s) brings the hider back up; a Reel the wrong way (hold Jack) lowers them into the ram's reach, where the next rise severs the line. Legal, and the Fray does it on stream with a countdown.
- Chain: the Claim marker Hauled into the pit resets to its authored position (05 section 9.4) after a `SFX_Press_Shear_Heavy`; the Haul spool does the same.

## States

Kinematic: `Dwell Up`, `Descending`, `Dwell Down`, `Rising`, `Seated` (post-consequence).

## VFX and SFX hooks

`SFX_Press_Idle_Hum`, `SFX_Press_Descent`, `SFX_Press_Clang` (global), `SFX_Press_Rise`, `SFX_Press_Shear_Heavy`, `SFX_Hook_Rotor_Spinup` (pit pad); `VFX_Pit_Dust_Puff`, `VFX_Beacon_Pulse`, `VFX_Pit_Flood_Dim`, `VFX_Hydraulic_Mist` (seated state).

## Technical

- LOD0 frame 24,000 tris, ram 3,000, pit 5,000; LOD1 half; LOD2 quarter; LOD3 posts and slab.
- Textures: `T_Press_Frame` 4K, `T_Press_Ram` 2K, `T_Pit_Concrete` 2K, `T_Yard_Decal_Atlas` 2K.
- Pivot at the pit floor center. Sockets: `Ram`, `Crosshead_N/S`, `Post_0..3` (Dog seat hints), `Beacon_0..3`, `Flood_0..3`, `HookPad`, `KillVolume`, `Truck_Ramp` (Haul).
- Implementation: ram on a server time-synced timeline (height curve, eased). Kill volume actor owned by the Well system, not the press, so a counterweight consequence that stops the ram does not touch the ring-out. Object removal in the pit: overlap on the pit floor flags props for despawn at the next ram contact event. Line severing: the ram's swept volume is tested against line segments each step.
- Collision: frame `Kinetic_WorldStatic`, ram `Kinetic_Kinematic`, pit floor `Kinetic_KillVolume` overlap plus `Kinetic_WorldStatic` blocking.

## Reference search terms

scrap metal shear press pit, hydraulic guillotine shear ram, baler press scrap yard, four post press frame, steel lined pit lip chevron.
