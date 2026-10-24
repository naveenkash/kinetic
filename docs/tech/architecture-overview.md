# KINETIC Technical Architecture: Overview

_Status: proposal v0.1, 2026-10-24. Engine: Unreal Engine 5.8.x (latest hotfix). Canonical gameplay numbers live in `docs/bible/05-core-mechanics.md`; this document never restates a number it can link to there._

## 1. What the engine must do

Six players (eight in Free Scrap) on a dedicated server. Every elimination is a physics event: a Stall threshold crossed by relative speed, impulse, crush force, or a ring-out volume. The one tool is the Spool: a Line between two Dogs that acts as a capped pull toward zero length (Haul), the Kick, a capacitor discharge that drives the ends apart to a class launch speed, followed by a motor push to max length (Jack), or a rigid distance constraint (Hang). Players are kinematic characters while grounded and simulated bodies while Dragged, Airborne, Hoisted, or Flipped. Arenas have destructibles (geometry collections), kinematics on scripted paths, force volumes, and a mid-match scripted collapse.

The fairness contract is the panic window: 1.0 to 2.0 s from Dog seat to point of no return, and any line on your body can be cut by looking at it. The netcode has to keep that window honest at 30, 80 and 150 ms.

## 2. System map

```
                         DEDICATED SERVER (authoritative)
 +-----------------------------------------------------------------------------+
 |  Match (GameMode, round flow, Marshal, Scrap Cards, replay record)          |
 |  Tell Scheduler (arena timeline -> audio/visual tells, lead times)           |
 |  Kinematic Director (Haulers, Gantry, Ring, Lift, Breathing, Shear Press)    |
 |  Force Volume Manager (updraft, Pulse, water rush, gas jet)                  |
 |  Destruction Director (GC damage, anchor fields, scripted events)            |
 |                                                                             |
 |  Chaos physics thread @ 60 Hz fixed, async                                  |
 |   +-- Line Solver (sim callback): Haul force, Jack velocity drive, Hang XPBD  |
 |   +-- Impact Monitor: contact impulses, relative speed, crush timer          |
 |   +-- Flight bodies (one rigid body per airborne player)                     |
 |   +-- Rigid props, GC clusters, kinematic targets                            |
 |                                                                             |
 |  Game thread @ 60 Hz: CMC characters (grounded), Spool, Cutter, GAS-lite,    |
 |  hit validation + 150 ms rewind buffer, relevancy/priority                   |
 +-------------------------------+---------------------------------------------+
                                 | UDP, Iris or legacy replication, 30/60 Hz
       +-------------------------+--------------------------+
       v                                                    v
 OWNING CLIENT                                       OTHER CLIENTS
 Predicts: own CMC moves, own Dog flight,            Interpolate: everyone else's
 own Line visuals, own Cutter swing, own Flight       bodies, lines, props (Predictive
 body (Resimulation mode)                             Interpolation), GC cosmetic breaks
 Camera and HUD always local                          Tells from replicated timeline clock
```

## 3. Authority model

Server-authoritative physics. The server owns every outcome that ends a round: Stall, ring-out, Dog seat, cut, break-free, fracture. Clients predict what they can feel and interpolate what they can only see.

| Object class | Authority | Client treatment | Why |
|---|---|---|---|
| Own character, grounded | Server, client-predicted | CharacterMovementComponent (CMC) move prediction, plus a custom `Dragged` movement mode fed by the replicated line force | CMC prediction is production-proven; Mover is still Experimental in 5.8 ([Mover docs](https://dev.epicgames.com/documentation/en-us/unreal-engine/mover-in-unreal-engine)) |
| Own flight body (Airborne, Hoisted, Flipped) | Server | Physics replication mode **Resimulation**; client runs about half an RTT ahead and rewinds on mismatch ([Networked Physics overview](https://dev.epicgames.com/documentation/en-us/unreal-engine/networked-physics-overview)) | Slingshot and Pendulum must feel immediate |
| Other players' bodies | Server | **Predictive Interpolation** | Cheaper, handles pushes without full resim |
| Own Dog projectile | Server validates the seat | Client-predicted swept trace; server rewinds target plates up to 150 ms | Hit registration must feel instant |
| Lines (all) | Server | Own line: predicted force and visuals from local inputs; enemy lines: replicated state, interpolated endpoints | The line is the game's main readable |
| Light and Medium props | Server | Predictive Interpolation when near a player; Default mode when far | Interaction fidelity where it matters |
| Heavy props, Rated loads | Server | Predictive Interpolation; break-free is a server event | Mass moves slowly; interpolation hides latency |
| Geometry collections | Server decides breaks | Client simulates fragments locally after the server break event; abandon replication below a cluster level | Fragment-level replication is too expensive |
| Kinematics | Server clock | Client evaluates the same spline/timeline from a replicated start time | Deterministic, zero per-frame bandwidth |
| Force volumes | Server clock | Same timeline evaluation; client applies locally to predicted bodies | Same |
| Tells | Server clock | Client schedules audio/visual from timeline lead times | Tells may never be late |

Rule: the client never decides a Stall. It may show a pending Stall (the 0.2 s harness lock from 05 section 7.4) while the server confirms.

## 4. Tick model

| Clock | Rate | Notes |
|---|---|---|
| Physics (Chaos, async fixed step) | **60 Hz** (16.667 ms) | Server and all clients identical. Required for networked physics prediction ([roadmap](https://portal.productboard.com/epicgames/1-unreal-engine-public-roadmap/c/381-asynchronous-physics-simulation)) |
| Server game thread | 60 Hz cap | `NetServerMaxTickRate=60` set explicitly in `DefaultEngine.ini` |
| Net send to each client | 30 Hz baseline, 60 Hz for the owner's own actors and lines on their body | See `networking.md` |
| Client render | uncapped | Physics results interpolated for render |
| Substepping | off | The async fixed step is the step. Substepping and async tick are mutually exclusive in practice ([sub-stepping docs](https://dev.epicgames.com/documentation/en-us/unreal-engine/physics-sub-stepping-in-unreal-engine)) |

Why 60 Hz physics and not 120 Hz:

- The Haul ramp is 0.4 s, so 24 physics steps. Jack is 0.3 s, 18 steps. Overspin tells are 0.8 s. None of these need finer quantization.
- Dog projectiles move up to 160 m/s, which is 2.67 m per step. That is handled by a swept trace per step, not by rigid-body CCD, so the tick rate does not affect hit accuracy.
- Impact thresholds are on relative speed and contact impulse, both read from the solver, not from penetration depth.
- The server simulates up to six flight bodies, roughly 100 awake rigid bodies and fracturing geometry collections. 120 Hz halves the server headroom for no gameplay gain. Rocket League runs 120 Hz physics because ball contact is the whole sport; its network rate is 60 Hz ([community report](https://steamcommunity.com/app/252950/discussions/0/1693788202025800838)). KINETIC's contacts are slower and heavier.
- Revisit at 120 Hz only if the crush test (8 kN for 0.5 s) or Hang slip (0.5 m per 0.1 s) shows visible stepping in the prototype.

Project settings: `Tick Physics Async = true`, `Async Fixed Time Step Size = 0.0166667`, `Enable Physics Prediction = true`, `Max Supported Latency Prediction` = 250 ms (covers 150 ms RTT plus jitter; raise only if memory allows). The 5.8 Physics Settings page still labels async tick and the fixed step as experimental ([physics settings](https://dev.epicgames.com/documentation/en-us/unreal-engine/physics-settings-in-the-unreal-engine-project-settings)), while the networked physics system depends on them. That is a documentation inconsistency, not a reason to avoid it: Epic's own Mover examples require both settings ([Mover examples](https://dev.epicgames.com/documentation/en-us/unreal-engine/mover-examples-in-unreal-engine)).

Known caveat: async physics slows down when the game thread cannot keep up ([forum report](https://forums.unrealengine.com/t/async-physics-slows-down-at-low-game-thread-fps/2731575)). The server must hold 60 Hz on the game thread. Budget for it in `performance-budgets.md`.

## 5. Plugin and feature set

| Feature | Version / status (verified 2026-10-24) | Use in KINETIC |
|---|---|---|
| Unreal Engine | 5.8, released 2026-06-17, hotfixes through at least 5.8.4 ([forum](https://forums.unrealengine.com/t/unreal-engine-5-8-released/2729274)) | Base. 5.8 is reported by a third-party blog as the last planned UE5 release with UE6 Early Access targeted for end of 2027 ([State of Unreal 2026](https://www.unrealengine.com/news/state-of-unreal-2026-top-news-from-the-show)); treat that as unconfirmed |
| Chaos Physics, async tick | In engine | Everything |
| Networked Physics (prediction, Resimulation, Predictive Interpolation) | Roadmap card still titled "Beta" ([roadmap](https://portal.productboard.com/epicgames/1-unreal-engine-public-roadmap/c/1501-networked-physics-beta)); the API enum marks Resimulation "Work In Progress" ([API](https://dev.epicgames.com/documentation/unreal-engine/API/Runtime/Engine/EPhysicsReplicationMode)); the 5.8 overview has no label and says the system must be implemented in C++ ([overview](https://dev.epicgames.com/documentation/en-us/unreal-engine/networked-physics-overview)). At least one studio reports shipping on it in 5.8 ([Epic forum](https://forums.unrealengine.com/t/tutorial-networked-physics-fundamentals/2737552)) | Flight bodies, props |
| CharacterMovementComponent | Production | Grounded movement |
| Mover 2.0 / Chaos Mover | Experimental in 5.8 ([docs](https://dev.epicgames.com/documentation/en-us/unreal-engine/mover-in-unreal-engine)) | Not used; re-evaluate at 5.9 or UE6 |
| Chaos Destruction (GC, Fracture Mode, Fields) | In engine; Dataflow "Production-Ready" in 5.8 ([release notes](https://dev.epicgames.com/documentation/unreal-engine/unreal-engine-5-8-release-notes)) | All destructibles |
| Physics Control plugin | "ready for production use" in 5.8 ([release notes](https://dev.epicgames.com/documentation/unreal-engine/unreal-engine-5-8-release-notes)) | Ragdoll-with-pose cosmetic driver |
| Physics Constraint Component | In engine | Prototype only; replaced by custom line constraint |
| Cable Component | In engine; collision Experimental ([docs](https://dev.epicgames.com/documentation/en-us/unreal-engine/cable-components-in-unreal-engine)) | Slack-line cosmetic only |
| Chaos Vehicles | Plugin lives under `Engine/Plugins/Experimental` and wheel outputs are not replicated ([API](https://dev.epicgames.com/documentation/unreal-engine/API/Plugins/ChaosVehicles/FWheelsOutput)) | Not used. Haulers are kinematic splines |
| Iris replication | 5.8 release notes: "production-ready for licensees"; the Iris doc page still says Experimental ([docs](https://dev.epicgames.com/documentation/en-us/unreal-engine/iris-replication-system-in-unreal-engine)). Sources disagree | Enable on a branch in week 3; ship on legacy replication if any regressions |
| Replication Graph | Plugin; disabled on console builds because of splitscreen ([docs](https://dev.epicgames.com/documentation/en-us/unreal-engine/replication-graph-in-unreal-engine)) | Not needed at 6 to 8 players |
| Gameplay Ability System | In engine | Ability activation, cooldowns, tags, cues. Never forces |
| Niagara | Production since 4.25; Data Channels available ([docs](https://dev.epicgames.com/documentation/unreal-engine/data-channels-in-niagara-for-unreal-engine)) | Line ribbons, debris, tells |
| MetaSounds | 5.8 quick start labels it Beta ([quick start](https://dev.epicgames.com/documentation/en-us/unreal-engine/metasounds-quick-start)), widely shipped regardless | Line singing, snap, creaks |
| Lumen | Default GI; Lumen Lite is Beta in 5.8 | Yes |
| Nanite | Supports geometry collections ([docs](https://dev.epicgames.com/documentation/unreal-engine/nanite-virtualized-geometry-in-unreal-engine)) | Static and GC meshes |
| Virtual Shadow Maps | Default; "still actively being developed" ([docs](https://dev.epicgames.com/documentation/en-us/unreal-engine/virtual-shadow-maps-in-unreal-engine)) | Yes, with Nanite |
| World Partition | In engine | One persistent level per arena, streaming mostly off |
| Replay (DemoNetDriver) | In engine ([docs](https://dev.epicgames.com/documentation/en-us/unreal-engine/using-the-replay-system-in-unreal-engine)) | Victim-camera replay, LayCast |
| Chaos Visual Debugger | In engine, extended in 5.8 | Physics debugging |
| Online Services (Sessions) | Beta interface ([docs](https://dev.epicgames.com/documentation/unreal-engine/sessions-interface-in-unreal-engine)); EOS SDK 1.19.1.2 ships with 5.8 | Matchmaking and session registry |

## 6. Decision log

| # | Decision | Alternatives rejected | Why |
|---|---|---|---|
| D1 | Players are two bodies: a CMC capsule while grounded and one rigid "flight body" when line- or impact-driven. The ragdoll is cosmetic, driven toward the flight body by Physics Control | Full physics character under Chaos Mover; networked ragdoll | Chaos Mover is Experimental and adds input latency by design. A ragdoll of 15 bodies cannot be resimulated cheaply. One body resimulates and gives clean impact numbers |
| D2 | Custom line solver on the physics thread (sim callback). Haul as a capped force with the 0.4 s ramp; Jack as the Kick (a velocity-target drive to launch speed V with a force cap) followed by a push at T; Hang as an XPBD distance constraint with slip and break | PhysicsConstraintComponent drives; Physics Control; Cable Component | PhysicsConstraintComponent cannot model slip, its break event is reported unreliable ([forum](https://forums.unrealengine.com/t/5-0-2-chaos-bug-physics-constraint-broken-event-not-firing/594556)), and it cannot attach to a CMC capsule. Physics Control drives one body toward a target, not a pair. Cable Component is cosmetic Verlet and applies no force to bodies |
| D3 | 60 Hz fixed async physics | 30 Hz; 120 Hz | See section 4 |
| D4 | Haulers and all kinematics are spline/timeline actors evaluated from a replicated clock | Chaos Vehicles, replicated transforms | Determinism and schedule are the gameplay. The Yard arena note already requires this |
| D5 | Geometry collection breaks are server events; fragments simulate locally on clients; replication abandons below cluster level 1 | Full fragment replication; client-authoritative destruction | Bandwidth and determinism. Engine exposes `enable_abandon_after_level` for exactly this ([API](https://dev.epicgames.com/documentation/en-us/unreal-engine/python-api/class/GeometryCollectionComponent)) |
| D6 | GAS for ability activation, cooldowns, tags, cues only | Hand-rolled ability layer; GAS for forces too | GAS prediction covers activation, effects, tags and cues, not physics. It gives us the 0.25 s mode switch, recall timers, Staggered/Hoisted tags for free |
| D7 | Legacy replication at launch, Iris evaluated on a branch | Iris from day one | Sources disagree on Iris status; 6 to 8 players do not need it |
| D8 | Dedicated servers on a managed fleet (Edgegap or AWS GameLift), EOS for sessions, identity, anti-cheat | EOS P2P relay; listen servers | EOS does not host authoritative servers ([Edgegap/EOS post](https://edgegap.com/blog/can-epic-online-services-eos-relays-allow-for-dedicated-server-or-authoritative-server)). Physics games need one authority |
| D9 | Dog is a swept-trace projectile, not a rigid body | Rigid sphere with CCD | Deterministic, predictable, cheap; rewind-friendly |
| D10 | Perforce with Unreal Game Sync | Git LFS | See `project-structure.md` |

## 7. Risks, ranked

1. **The Kick under prediction.** The Kick (05 section 3.2) delivers the full relative speed V within 0.3 s, split by inverse mass. For a Climber self-dogged to a wall that is 90 kg × 18 m/s in 0.3 s, a 5.4 kN average force, far above the 400 N pull tension. The owner predicts it in Resimulation mode; a late or rejected seat means a 5.4 kN impulse was mispredicted. The Kick interlock (no Kick when either Dog is in another player's Catch Plate, 05 section 3.2) removes the worst case, a Kick against another predicted body. The handoff and the correction must not pop. Prototype week 1.
2. **Custom constraint under Resimulation.** Our line solver state must live in the networked physics history so it rewinds with the body. Epic's docs confirm resimulation only ticks the physics thread; a forum thread raises exactly this constraint question with no complete answer ([forum](https://forums.unrealengine.com/t/network-physics-component-how-to-do-for-complex-systems-with-constraints/2676595)). Prototype week 1.
3. **Kinematic-to-simulated handoff.** Grounded CMC capsule to flight body and back, predicted on the owner, must not pop. Prototype week 2.
4. **Heavy-vs-Light determinism across clients.** The 71:29 split must be identical on both ends under prediction. Fixed step helps; float order and sleep states can still differ. Prototype week 2.
5. **Destruction cost on the server.** Server fracturing geometry collections with no renderer still pays collision. Caps in `chaos-destruction.md`.
6. **Networked physics maturity.** Beta on the roadmap, C++ only, API still moving (5.7 to 5.8 added tick-offset helpers and Networked Actions). Pin the engine version per milestone.
7. **Hang lines solid to players.** A capsule-swept segment collider that can stagger and flip is custom code. Must not create physics explosions (the bible's own warning).

## 8. Prototype plan (weeks 1 to 4)

Goal: retire risks 1 to 4 with throwaway code in a graybox.

**Week 1: a line on a player.**
- One graybox arena, two players, dedicated server, 60 Hz async physics, Physics Prediction on.
- Flight body (capsule rigid body) with Resimulation; grounded CMC capsule with a `Dragged` custom movement mode.
- Line solver as a `TSimCallbackObject` with rewind support. Haul only, with the 0.4 s ramp and 25 m/s cap. Static Dog B.
- Exit test: a Haul on the owner's chest plate at 80 ms simulated RTT. No visible pop at the skid-to-drag transition at the class foot-hold. Reproduce the 05 section 4.3 reference case: Breaker's 1,000 N on a 130 kg Rigger from 15 m, feet leave the floor at 0.26 s, over the edge at 2.2 s, within 0.1 s on server and client. Then the self-dog Kick (Climber, 18 m/s) at 80 ms with no visible pop.

**Week 2: two bodies, determinism.**
- Player-to-player Haul and Jack. Breaker vs Climber. Log both clients' and the server's closure split per tick. Target: identical to 1 cm after 2 s at 0, 80 and 150 ms RTT.
- Add Hang as an XPBD distance constraint with Hang load slip. Pendulum from a static anchor.
- Exit test: 71:29 split reproduces on all three machines; Pendulum apex position matches within 5 cm.

**Week 3: the panic window.**
- Cutter with the 70° cone and plate reach. Server cut validation with the grace window from `networking.md`.
- Edge indicator, line audio parameter bridge, line emissive from force.
- Exit test: 20 trials per latency (30, 80, 150 ms) of a Breaker Haul on a Rigger toward an edge. Survival rate difference across latencies under 10 points.

**Week 4: impact and destruction smoke test.**
- Impact Monitor: Stagger, Flip, Stall from relative speed and impulse, including kinematic hits and a crush case.
- One container stack as a geometry collection with an anchor field and break-free, one scripted Shear.
- Exit test: zero false Stalls in 10 minutes of six-bot play; server frame under 10 ms with the stack falling.

Everything after week 4 is production code in the module layout from `project-structure.md`.
