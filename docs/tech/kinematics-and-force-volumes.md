# KINETIC: Kinematics, Force Volumes and Tells

_Status: proposal v0.1, 2026-10-24. Engine: UE 5.8.x. Behaviour follows `docs/bible/05-core-mechanics.md` sections 6.6, 6.7 and 11 and the arena notes in `docs/bible/03-arenas.md`._

## 1. Kinematic actors

Haulers, the Gantry, the Maintenance Ring, the Lift and its counterweight, the Breathing hull and the Shear Press ram are `AKineticKinematicActor`. Infinite mass, scripted path, nothing moves them (6.7).

### 1.1 Representation

| Actor | Arena | Path | Cycle | Tell lead |
|---|---|---|---|---|
| Haulers | Yard | Closed spline per trench, constant speed, scheduled departures | Per timetable; doubled during Shift Change | 3 s klaxon, then lights |
| Gantry | Yard | Linear rail spline, dwell at pick and drop. The load is a rated attachment (6.7) | Slow loop | 2 s before drop |
| Maintenance Ring | Anchor | Rotation about the Stump axis | 120 s per revolution | continuous |
| Lift and counterweight | Hangtown | Vertical spline, two cars opposite phase. The counterweight is a rated attachment (6.7) | Authored | continuous |
| Breathing hull | Ballast | Rotation ±6° about the hull's long axis | 90 s | continuous groan |
| Shear Press ram | Yard | Vertical timeline | Authored | audible everywhere |
| Market ring (Cable Day) | Hangtown | Rotation to 40° over 2.5 s | once | 5 s siren |
| Haul sled (ratchet rail) | Ballast | One-way rail spline up the Spine; position along the rail is a line-driven degree of freedom | Match-long | pawl clack per tooth |

Every kinematic except the Haul sled is a function of one number: server match time `t`. Position and rotation are evaluated as `Pose(t)` from a spline or curve asset. There is no per-frame transform replication.

Chaos Vehicles was considered for the Haulers and rejected. It is a wheeled suspension simulation whose plugin still lives under `Engine/Plugins/Experimental` and whose wheel outputs are not replicated ([API](https://dev.epicgames.com/documentation/unreal-engine/API/Plugins/ChaosVehicles/FWheelsOutput)). The Yard note already says determinism matters more than suspension. Chaos Modular Vehicles in 5.8 adds client-predicted, server-authoritative vehicles ([5.8 notes](https://dev.epicgames.com/documentation/unreal-engine/unreal-engine-5-8-release-notes)), which solves a problem we do not have.

### 1.1a Rail-constrained kinematic with a line-driven degree of freedom: the Ballast Haul sled

Ballast's Haul mode runs the 2,000 kg spool up the Spine on a counterbalanced ratchet rail. The sled is a kinematic with one free coordinate, `s`, its distance along the rail spline. Nothing else about it is simulated.

| Parameter | Value |
|---|---|
| Moving mass (sled, spool, counterbalance referred to the rail) | 4,000 kg |
| Pawl drag | 200 N, opposing motion along the rail |
| Rail direction | One-way. The pawl holds `s` against any backward force, so `ds/dt >= 0` |
| Driving force | Vector sum of active pull (Haul, Jack push, Kick) from every line seated on the sled or spool, projected onto the rail tangent |
| Hard caps | 25 m/s Haul closing cap and 50 m/s body cap apply; in practice the sled never exceeds 2 m/s |

Per physics step, on the server and in each client's predicted world:

```text
F_rail = dot(sum of active line forces on the sled, rail_tangent(s))
F_net  = F_rail - 200 N                 // pawl drag; counterbalance cancels gravity
if F_net <= 0 and v_s <= 0: v_s = 0     // pawl holds, no backward motion
else: v_s = max(0, v_s + (F_net / 4000 kg) * dt)
s = clamp(s + v_s * dt, 0, rail_length)
sled pose = spline(s), applied as a kinematic target so riders and contacts see its velocity
```

A single Climber (400 N) nets 200 N, 0.05 m/s² and takes a long time; three lines adding as vectors (6.2a) is the intended crew play. Lines seated on the sled are `Kinematic` endpoints with invMass 0 for everything except the rail coordinate, so a Haul from a player to the sled moves the player toward it as usual, and only the rail-tangent component of the force moves the sled. Objects and players standing on the sled ride it through the normal kinematic carry path. The sled replicates `s` and `v_s` at 10 Hz (two floats) and clients predict between updates from the same rule; a break-free style tell (a pawl clack per rail tooth, every 0.25 m) is driven from `s` on each client. `s` resets to zero at round reset.

### 1.2 Time sync

- The server's `AGameStateBase` replicates match time. Clients estimate server time with the engine's server world time plus half RTT, smoothed. Target error under 20 ms, which at a Hauler speed of 12 m/s is 24 cm.
- Kinematic poses are evaluated on the physics thread at the fixed step from the same `t`, so collisions with simulated bodies are consistent on server and client. Rendering interpolates between physics poses.
- Clients that drift more than 50 ms resnap `t` over 0.5 s rather than jumping.
- Event starts (a Hauler departure, a Gantry drop) are replicated once as `(EventId, StartTime)`. Everything after is derived.

### 1.3 Movement and collision

- Each kinematic is a Chaos kinematic rigid body moved with target-pose updates per physics step, so it carries velocity. Simulated bodies and flight bodies that it hits receive the full relative impact (6.7) and the Impact Monitor uses the kinematic thresholds (4 / 6 / 9 m/s).
- CMC characters standing on a kinematic use CMC based movement: the capsule inherits the base's linear and angular velocity. Standing on a Hauler, riding the Ring, and standing on the tilting hull all work through this path.
- A line between a kinematic and anything else moves the anything else: the solver treats the kinematic endpoint as invMass 0 and its anchor as moving with `Pose(t)`. The Caboose is this rule with a Hauler.
- **Kinematics sever any line they pass through** (05 section 3.5): a Hauler through a trench bridge, the Gantry load through a Gallows, the Ring through a set line. Lines are not solid to kinematics, so this is not a collision; it is a server-authoritative per-tick swept test of each kinematic's collision shapes against every live and set line segment, four sub-samples per physics step. A crossing is a cut at the crossing point with the 4.2 snap and recoil; both Dogs release; reliable multicast. Lines with a Dog seated in that kinematic are exempt. The Haul sled is a kinematic and severs lines it sweeps through, except its own.

### 1.4 Carried objects on moving platforms

- Simulated objects on a kinematic are carried by contact friction. Friction coefficient 0.8 for Hauler decks and the Gantry load. Heavy objects that are meant to move sit on rollers (0.02) or skids (0.05); a Heavy on a bare floor is fixed scenery (6.1a). The Claim marker is on runners (0.05) and the Haul spool on rollers (0.02).
- Kinematics are the game's crush source (7.1, v0.2.1): the Impact Monitor evaluates the 8 kN for 0.5 s test only where a kinematic or a Heavy body is one of the two opposing contacts. Lines cannot crush. Bodies resting on a kinematic are kept awake while the kinematic's speed exceeds 0.1 m/s so they do not float when it starts.
- Rotating platforms (the Ring, the market ring, the Breathing hull) need angular carry. Chaos handles it through contact, but sleeping bodies on a slowly tilting surface can lag. The arena timeline wakes all bodies inside the hull volume at each Breathing reversal.
- **The tilted hull (Ballast)**: the hull is one kinematic root. Per the arena note, contents are children simulating in its frame. Implementation: all Ballast props and GCs are world-space simulated bodies; the hull's collision moves them by contact as it rotates ±6°. On the lean, containers that have broken free slide on the racks' dead rollers (friction 0.02, 6.1a), chains swing, and the anchor field break-free values for the low side drop by 100 N (see `chaos-destruction.md` section 4). CMC characters follow the deck through based movement. Hang lines fixed to the hull at both ends move with it; a Hang between the hull and a static gorge point changes length, so it slips or snaps taut exactly as the arena note describes.
- **Rated attachments** (6.7): the Gantry load and the Lift counterweight are kinematic while carried, moving with `Pose(t)` and treated as invMass 0 by the line solver. They become simulated rated loads when the summed line force on them reaches the 1,000 N break-free (a Breaker, or a Doubler at overspin), which releases them from the path, or when the script releases them (the Gantry drop point, early during Shift Change). Players can ride, Dog and swing from them in either state. Shift Change drops whatever the Gantry magnet holds at that moment; an empty magnet drops nothing and the next pick comes early (v0.2.2).

## 2. Force volumes

Updraft, Pulse, water rush and gas jet are `AKineticForceVolume`. Each is authored as `a_ref`, the acceleration a 100 kg reference body would feel, and applies a **constant force of `100 kg × a_ref` to each affected body**, so a body of mass `m` accelerates at `a_ref × 100 / m` (05 section 6.6). A 90 kg Climber in a 14 m/s² updraft feels 15.6 m/s²; a 220 kg Breaker 6.4 m/s²; a 45 kg cylinder 28 m/s² (31 before 05 6.6's cap at 2 × a_ref); a 2,000 kg container 0.7 m/s². Volumes act on **players, loose objects (break-free 0) and objects held by a player's line** (05 6.6: a Hang-held load swings in a gust). Racked, stacked, bolted, rated and cable-hung map objects, kinematics and geometry-collection clusters still anchored are skipped until something else frees them. CMC characters receive the force through `MOVE_Dragged` and go Airborne when `a_ref × 100 / m` exceeds their foot-hold. Volumes never Stall directly.

| Volume | Arena | Shape | Profile | Timing |
|---|---|---|---|---|
| Updraft | Anchor | Cylinder around the Stump | `a_ref` 14 m/s² upward at the Stump falling linearly to 0 at 60 m radius, scaled by a 0 to 1 gust curve | 40 s cycle, 1 s tell |
| Pulse | Anchor | Sphere | `a_ref` 20 m/s² radially outward for 0.4 s (the shove), then `a_ref` 8 m/s² inward toward the Stump for 0.6 s (the inhale), falloff to 0 at the Rim | every 75 s from 0:55, Marshal countdown |
| Water rush | Ballast | Box in the Hold, direction set by hull tilt | `a_ref` 12 m/s² toward the nose, 6 s after a tank rupture | event |
| Gas jet | any manifold (fixed world geometry, no line moves it) | Cone 6 m long | `a_ref` chosen per arena so a 100 kg body reaches 15 m/s in 3 s (5 m/s²), capped at 15 m/s along the jet | on a Jack over 600 N applied to it, or blast |
| Fire patch | fuel drum | Disc 3 m | Outward push to a 4 m/s target, stagger on entry, 6 s | event |

Implementation: the constant force `100 kg × a_ref` is applied in the line solver's sim callback (same thread, same step) by iterating the players and loose bodies overlapping the volume; anchored, placed and kinematic bodies are filtered by the placement data before the overlap test. For predicted bodies the client applies the same profile from the same clock, so an Updraft Slingshot predicts correctly. Chaos Fields were considered for these and rejected for players: Fields are built for geometry collections and are awkward to drive from gameplay per body. Fields remain the tool for destruction (anchors, strain, kill).

Dogs fired across the Updraft drift: Dogs are not bodies, so the flight step samples `a_ref` directly and adds it as acceleration (arena note).

No volume pushes a body past the 50 m/s hard velocity cap (6.3). Players inside a volume whose acceleration exceeds foot-hold go Airborne; braced players use the brace foot-hold, which is how "everyone braces" for the Pulse works without a special case.

## 3. Tell scheduling

Every lethal system has an audio tell, a visual tell and a lead time (05 section 11). Tells are driven from the same server clock as the kinematics so they can never be late.

### 3.1 Timeline asset

`DA_ArenaTimeline_<Arena>` holds an ordered list of events. Arena geometry (datums, heights, the Spine's forward 72 m, the Anchor Rim at 240 m with the Crown 60 m above it) is authored in `docs/arenas/*/layout.md`, which is the authority the timeline and spline assets are built against.

```
FTellEvent {
  Name            (Hauler_T2_Departure, Gantry_Drop, Shear, Pulse, ...)
  TriggerTime     (absolute match time, or recurring: period, phase)
  LeadAudio       (s)   LeadVisual (s)
  AudioCue        (MS_ asset)   VisualCue (NS_ or light sector)
  Location        (actor ref or world point, for directionality)
  Payload         (what happens at TriggerTime: kinematic event, field, volume)
}
```

The server owns the timeline. On match start it replicates the timeline id and the match start time. Recurring events need nothing else. One-off events (the mid-match event at 1:15 remaining in a Scrap round, 4:00 remaining in Claim, 2:00 remaining in each Haul half, 3:00 remaining in Free Scrap) are replicated as `(EventId, TriggerTime)` when scheduled, which is at round start, so clients know 5 s or more ahead.

### 3.2 Client scheduler

Each client runs `UKineticTellScheduler`. Each frame it compares estimated server time with `TriggerTime - Lead` for every pending event and fires the cue when crossed. Cues are fired locally, with no RPC, so a 150 ms client hears the klaxon within 20 ms of a 30 ms client.

Lead times, minimums from 05 section 11 (the data asset may exceed them, never undercut them):

| System | Audio | Visual | Lead |
|---|---|---|---|
| Dog seated in your plate | seat clank, directional | edge indicator | 0 s |
| Line under force | singing, pitch = force | emissive = force | continuous |
| Overspin | octave jump at 1.0 s | white flicker | 0.8 s to snap |
| Set line | low hum | dim, brightens within 5 m | continuous |
| Hang load slipping | creak | jitter | 0.3 s per 0.5 m |
| Break-free | lurch, scrape | shudder | 0.2 s |
| Rated load moving | structural groan | dust, debris | 1.0 s before consequence |
| Kinematic approach | klaxon, then lights | lights | 3 s Hauler, 2 s Gantry drop |
| Mid-match event | siren | sag or sparks | 5 s |
| Force volume | gust or rumble | particles | 1 s |
| Ring-out imminent | Hooks spin up | | 0.5 s |
| Volatile armed by impact | hiss | venting | 0.3 s |

Event-driven tells (break-free, slip, volatile armed, ring-out) are not on the timeline. They are fired by the server as reliable events with the server frame they occurred on, and the client plays them immediately. Their lead times are enforced on the server: a rated load's consequence field cannot fire until 1.0 s after the groan event was sent; a volatile cannot blast until 0.3 s after the hiss. Latency therefore shortens a 150 ms client's effective lead by at most 150 ms, which is why the bible's minimums are minimums.

### 3.3 Validation

- Automation test `Tell.LeadTimes`: runs every arena timeline headless and asserts each payload fires no earlier than its lead after its cue.
- The editor tool `Tell Timeline Inspector` (`project-structure.md`) draws the timeline with cue and payload markers so designers can see a Hauler whose klaxon is 2.5 s.
- Light sectors: tower collapse in the Anchor toggles a light sector; the inspector shows the lines' emissive in that sector is unchanged, which the arena note asks to confirm first.

## 4. Engine-free implementation

[game/Source/KineticCore](../../game/README.md) implements this document without Unreal. The tests are in `KinematicsTest.cpp`.

**Kinematics:** `Kinematics.h`.
- Every pose is a function of match time:
  - `FRailRun` with `FTimetable` covers Haulers. A train is 14 m long: its nose leaves at 13.3 s and its tail clears at 14.5 s. Its pose faces along the path, as a rotation about Up. Between runs it has no pose at all, because it is out of the arena. Departure segments represent Shift Change: the Yard resets the timetables at e 80, cancelling trains not yet departed.
  - `FCycle` covers the Gantry's keyframed 120 s cycle and its phase at live; keys at one time are a step.
  - `FSpin` covers the Maintenance Ring, and `FRock` the Breathing hull.
- `FHaulSled` is the 1.1a ratchet rule, with a pawl tooth every 0.25 m.
- `FServerClock` is the 1.2 estimate:
  - small errors are smoothed;
  - an error over 50 ms, seen in two samples in a row, is slewed out over 0.5 s, so one late packet is ignored;
  - once synced, the estimate never runs backward.
- `SweptSever` is the 1.3 test: a kinematic's box is swept in four substeps against a line segment. `FRailRun::SweepPoses` gives poses only while the same train is in the arena at both ends of the step, so a despawn or a spawn is never swept across the trench.

**Force volumes:** `ForceVolumes.h` gives each section 2 profile's a_ref at a point and time:
- the Updraft, with a gust curve that wraps at its cycle;
- the Pulse, a 0.4 s shove then a 0.6 s inhale every 75 s from 0:55;
- the water rush, the gas jet and the fire patch.

Every volume has a height band. The Pulse's is Z -15 to 70; the Updraft's is per arena. `BodyAcceleration` applies 05 6.6's mass scaling and 2 × a_ref cap, to players, loose bodies and line-held bodies only. Rulings this document left open (**PROPOSED**):

| What | Value |
|---|---|
| Water rush duration | 4 s |
| Gas jet cone half-angle | 15° |
| Fire patch push | 8 m/s² until a body moves outward at 4 m/s |
| Fire patch height | 2.5 m above the disc |

The Updraft gust curve is per-arena data; its rise from 0 is the gust start that the 1 s tell leads.

**Tells:** `Tells.h`.
- `MinimumLead` is 05 section 11's table.
- `TimelineProblems` is the `Tell.LeadTimes` check: a lead under its minimum, or an event that repeats before its own cue.
- `FTellScheduler` fires each cue once when the estimated server time crosses it.
  - A leading cue whose payload has already passed is dropped: a client that joins late, or a frame gap over both, hears no klaxon for a train that has gone.
  - A zero-lead cue fires as its moment is crossed.
  - Events added mid-match with `AddEvent` (the one-off events of 3.1) join as a late client would.
- `FTellGate` holds an event tell's consequence (a rated load's groan 1.0 s, a volatile's hiss 0.3 s) for each source, on its latest cue, with `Clear` and `Reset` for round reset.
- `MidMatchEventTime` places the mid-match event on the round clock, which runs from the start of the Drop (05 9.2, game-modes 2.2).

**Two time bases, and an open inconsistency for design.** Kinematics and the Yard's documents run on match time e, seconds since "live". The round clock counts from the start of the 6 s Drop, so round time = e + 6.

The Yard's events document puts the Shift Change siren at e 75 to 80, and its siren-end values for each mode put the payload 5 s after the clock reads the "remaining" time, counted from live. The canonical rule puts the payload at the remaining time on the round clock (05 9.2; game-modes 3, where Claim's siren is at 4:05 and the event at 4:00). Every Yard value is therefore 11 s late: 5 s for the siren, 6 s for the Drop.

| Mode | Yard document (e) | Canonical rule (e) |
|---|---|---|
| Scrap | 80 | 69 |
| Claim | 245 | 234 |
| Haul | 125 | 114 |
| Free Scrap | 185 | 174 |

The same 11 s applies to everything the Yard times from Shift Change:
- the Hauler timetable reset (e 80, then 83, 90.5 and 98, and back to 45 s at 140);
- the Claim interlock (e 293 and 297);
- the Gantry phases at live.

The code and tests use the Yard's values for the Yard timetable and the canonical rule for `MidMatchEventTime`. Before integration, design should either re-derive the arena's e-values or amend the rule.
