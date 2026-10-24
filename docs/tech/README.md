# KINETIC Technical Documentation

_v0.1, 2026-10-24. Target engine: Unreal Engine 5.8.x. The gameplay bible in `docs/bible/` is canonical for every number; these documents describe how the engine delivers them._

## Index

| File | Covers |
|---|---|
| [architecture-overview.md](architecture-overview.md) | System map, authority model, tick model, plugin set, decision log, ranked risks, 4-week prototype plan |
| [spool-and-line-implementation.md](spool-and-line-implementation.md) | Dog projectile, the Line as a constraint object (Haul, Jack, Hang), drag model, ragdoll and GC attachment, Hang line collision, lifecycle, Doubler, overspin, replication, rendering and audio, class sketches |
| [player-physics.md](player-physics.md) | Two-body character, CMC vs Mover decision, state machine, impact detection, crush test, Stall sequence, ring-out, Brace, Perch, mantle, ledge grab, airborne prediction, anti-cheat |
| [chaos-destruction.md](chaos-destruction.md) | Geometry collection mapping per destructible, unit conversion, cluster strategy, anchor fields, scripted events, pooling, rated-load consequences, replication, Nanite, budgets, artist workflow |
| [kinematics-and-force-volumes.md](kinematics-and-force-volumes.md) | Time-synced kinematic actors, carried objects and the tilted hull, force volumes, tell scheduling |
| [networking.md](networking.md) | Dedicated server, tick rates, bandwidth arithmetic, priority, prediction per class, panic-window fairness, lag compensation, session flow with the KINETIC backend, regions, replays, cheat surfaces |
| [performance-budgets.md](performance-budgets.md) | Platform tiers, frame budgets, object caps, memory, World Partition, scalability, profiling plan |
| [project-structure.md](project-structure.md) | Modules, folders, naming, data tables, Blueprint vs C++, Perforce, CI, editor tools |
| [backend-architecture.md](backend-architecture.md) | KINETIC's backend on Supabase: schemas and RLS, data model, Steam login bridge, crews and Realtime channels, matchmaker and allocation, game-server endpoints, match reports, Ledger, economy, Contracts, Claims and the season flip, rate limiting, Redis policy, moderation, privacy, security, cost, Mac local dev, build order |
| [../../game/README.md](../../game/README.md) | The engine-free core of KineticCore: line solver, mass and impact rules, Cutter, data tables, the bible-table check, panic-window bench, and the findings that corrected the solver pseudocode |
| [../../backend/README.md](../../backend/README.md) | The backend code (Phases 1 to 3): Supabase migrations and Edge Functions, the worker (matchmaking, rewards, Claims, purchases, accounts), the fake game server, the admin console, the load simulator, and how to run and test it on a Mac |
| [runbooks.md](runbooks.md) | Backend operations: stuck and quarantined reports, worker leadership, the Claim flip, purchases and refunds, moderation, deletion and export, key rotation, the load test |
| [backend-architecture-diagram.md](backend-architecture-diagram.md) | One-page backend view: Mermaid system and match-flow diagrams, description and safety rules in ASD-STE100 Simplified Technical English |
| [tech-stack-plan.md](tech-stack-plan.md) | The whole stack around the engine: workstations, week 0 setup, backend and live services, telemetry, content tools, CI and release, platforms, legal, hosting cost, build order, new decisions |

## Key engine findings (verified 2026-10-24)

1. UE 5.8 shipped 2026-06-17 with hotfixes through at least 5.8.4. Epic's 5.8 notes mark Iris, MegaLights and Dataflow production-ready and the Physics Control plugin "ready for production use". Mover is still Experimental.
2. Networked Physics (Resimulation and Predictive Interpolation) exists and is documented for 5.8, but the roadmap card is still "Beta", the API enum marks Resimulation "Work In Progress", and Epic says it must be implemented in C++. One studio reports shipping on it.
3. Chaos Mover (physics-thread movement) adds input latency by design and does not synchronize with Network Prediction actors. CMC remains the production choice for grounded movement.
4. Geometry collections expose per-level damage thresholds, two damage models, damage propagation factors, and replication fields including abandon-after-level. There is no official multiplayer destruction guide; the damage-strain formula is undocumented and must be calibrated.
5. Anchor Fields, External and Internal Strain, Sleep, Kill and Force Dynamic Switch fields are documented and are the right tools for stacks, scaffolds and scripted collapses.
6. PhysicsConstraintComponent has linear limits, soft constraints, breakable thresholds, plasticity and position/velocity drives, but community reports say its break event is unreliable; Cable Component is cosmetic and applies no force; Physics Control drives bodies toward targets. None of them is a line between two arbitrary bodies with slip, so the line is a custom physics-thread constraint.
7. Chaos Vehicles is a wheeled suspension sim still under the Experimental plugin folder with wheel outputs not replicated. Haulers are kinematic splines.
8. Async fixed-step physics is required by networked physics and Chaos Mover, yet the Physics Settings page still labels it experimental. The docs disagree with themselves; the feature is in use.
9. Online services are KINETIC's own backend on Supabase ([backend-architecture.md](backend-architecture.md)); Steam is used only for login tickets, overlay invites and payments. Hosting goes to Edgegap, AWS GameLift or Agones; Hathora and Unity Multiplay are reported closed in 2026. Bandwidth, not CPU, is the server cost driver.
10. Lumen, Nanite and VSM have been positioned for 60 fps on current consoles since 5.1; VSM is "still actively being developed" and wants Nanite geometry; Lumen Lite is Beta in 5.8. MetaSounds is labelled Beta on its 5.8 quick start despite wide use.

## Contradictions found in the bible

These need an answer from design before the solver is tuned. Listed in order of engineering impact.

Resolved by the 2026-10-24 bible v0.2 and v0.2.1 changelogs and no longer open: the tension vs panic-window scale (pull tensions are now 400 to 1,000 N and the 4.3 table reproduces), the braced Breaker rule (20 m/s² × 220 kg = 4,400 N is static to every line), the Climber Jack bonus (now the 18 m/s launch speed V in 3.4), the overspin stored impulse (now 1,500 N·s × F/1,200 N), the Gantry load classification (now a kinematic rated attachment), break-free at equality (all thresholds are inclusive), the Jack launch force vs pull tension (the launch phase is now the Kick, a capacitor discharge separate from motor pull), team formats (launch is 3v3; 3v3v3 is post-launch), and the Scrapper small-prop bonus (Light plus Medium under 100 kg, on Haul and the post-Kick push only).

1. **Hang as rod or rope (05 sections 3.2, 8).** "Rigid rod of length L₀" implies two-sided; "neither can leave the other's radius" and pendulum use imply a rope that goes slack under compression. Implemented as rope by default with a per-line rod flag; confirm the default.
2. **Player-on-player impulse (05 section 7.2).** Uses `m_other × v_rel` per party, which is not a physical collision impulse (that would use reduced mass). Kept as a design convention; the Impact Monitor computes it explicitly rather than reading the solver impulse for player-player contacts.
3. **Airborne definition (05 section 5.1).** "Airborne" is defined as the simulated ragdoll state, but ordinary jumps are also airborne. Implemented as: own-power jump and fall stay kinematic (CMC Falling); line-, impact- and force-volume-driven flight is simulated. Confirm.
4. **Hanging house release and cable rating (03 vs 05 section 6.2).** The Hangtown engine note frees a house when its "strain threshold is hit"; 05 now says only a Breaker's active pull, a Doubler at overspin, or a cut slack cable drops one, and that Hang lines never test break-free. Implemented per 05; the arena note should drop the strain-threshold wording.
5. **Ballast Haul route (CHANGELOG v0.2.1 vs the ratchet-rail ruling).** The v0.2.1 changelog says Ballast's Haul route is the Bays deck, not the Spine; the later ruling defines a counterbalanced ratchet rail up the Spine for the Haul sled. The tech docs implement the Spine rail (kinematics doc section 1.1a). One of the two bible statements needs to go.
6. **Arena plays that swing houses with lines (03).** Hangtown's "Swing" and "Eviction" describe Pendulum-ing a house into a crowd; 05 section 6.3 says a line cannot swing a Heavy hanging object dangerously on its own and swings come from gravity. The arena text needs a pass to phrase these as cut-cable and Sway-pumping plays.
7. **Force volumes vs the arena notes (05 section 6.6 v0.2.2 vs 03).** Volumes now act on players and loose objects only, with a constant force of `100 × a_ref` N per body. The Ballast note's "water in the Hold shifts and shoves whatever's standing in it" and the Anchor note's "Hang loads swing" in the Updraft still read as if volumes move hanging and placed objects; they do not until something frees them. Arena text needs a pass.

## Decisions to confirm (engineering lead, one page)

| # | Decision | Default in these docs | Needed by |
|---|---|---|---|
| 1 | The Kick as a velocity-target drive (reach V in 0.3 s, force cap 1.5 × m_eff × V / 0.3) followed by a motor push at T; Scrapper bonus on Haul and push only | As stated | Week 1 |
| 2 | Character movement: CMC plus networked flight body, not Mover | CMC | Week 1 |
| 3 | Custom physics-thread line solver, not PhysicsConstraintComponent | Custom | Week 1 |
| 4 | Physics tick 60 Hz fixed async; revisit 120 Hz only on evidence | 60 Hz | Week 1 |
| 5 | Hang default: rope (slack under compression) with rod flag | Rope | Week 2 |
| 5a | Reel input: holding the Haul or Jack mode input past 0.2 s while in Hang is Reel; a shorter press is a mode switch | 0.2 s hold threshold | Week 2 |
| 5b | Kinematic sever test at four sub-samples per physics step; not client-predicted | As stated | Week 4 |
| 6 | Flight body is one rigid capsule; ragdoll is cosmetic via Physics Control | Yes | Week 2 |
| 7 | GAS for activation, cooldowns, tags, cues only | Yes | Week 3 |
| 8 | Legacy replication at launch; Iris on a branch | Legacy | Week 3 |
| 9 | Grace window for line-caused Stalls: `min(150 ms, one-way latency + 48 ms)`; cuts judged at the client's frame | As stated | Week 3 |
| 10 | Catch Plate and line rewind cap 150 ms | 150 ms | Week 3 |
| 11 | GC replication abandons below cluster level 1; fragments local | Level 1 | Week 4 |
| 12 | Damage threshold calibration multiplier per material from drop tests, not the material-strength model | User-defined model | Week 4 |
| 13 | Fleet provider: Edgegap vs AWS GameLift vs Agones, chosen on a two-week RTT beacon test and egress pricing | Undecided | Pre-alpha |
| 14 | Launch regions (8 listed) and 150 ms matchmaking cap | As listed | Pre-alpha |
| 15 | Perforce with UGS and Horde | Perforce | Now |
| 16 | Engine pin: 5.8 latest hotfix; upgrades gated by a green nightly | 5.8.x | Now |
| 17 | Server SKU 2 vCPU / 2 GB per match | As stated | Alpha |
| 18 | Switch 2 as a stretch platform using Lumen Lite | Not committed | Alpha |
| 19 | Console scalability floor: Series S at 1080p60 with Lumen Lite or software global tracing | As stated | Alpha |
| 20 | Victim-camera replay from local memory-streamer recording plus replicated victim camera rotation | As stated | Beta |

## Research citations

All accessed 2026-10-24. Epic documentation pages render for UE 5.8 unless noted. Several Epic pages (Physics Control overview, GeometryCollectionComponent C++ reference, Mover vs CMC comparison, 5.8 and 5.1 news posts) returned only a table of contents or an HTTP error during this review; where that happened the nearest fetchable page or a clearly labelled secondary source is cited instead.

**Engine releases**
- UE 5.8 release notes: https://dev.epicgames.com/documentation/unreal-engine/unreal-engine-5-8-release-notes
- UE 5.8 released (forum, 2026-06-17, hotfix list): https://forums.unrealengine.com/t/unreal-engine-5-8-released/2729274
- 5.8.2 hotfix (NetPhysics TickOffset fix): https://forums.unrealengine.com/t/5-8-2-hotfix-released/2746335
- UE 5.7 released (forum): https://forums.unrealengine.com/t/unreal-engine-5-7-released/2673913
- State of Unreal 2026 (UE6 Early Access target end of 2027): https://www.unrealengine.com/news/state-of-unreal-2026-top-news-from-the-show
- Third-party 5.8 summary (claims 5.8 is the last planned UE5 release; unconfirmed): https://ludusengine.com/blog/unreal-engine-5-8-whats-new
- UE 5.1 release coverage (Lumen, Nanite, VSM for 60 fps on consoles): https://80.lv/articles/unreal-engine-5-1-has-been-released/ and https://www.unrealengine.com/en-US/blog/unreal-engine-5-1-is-now-available

**Networked physics and movement**
- Networked Physics overview (5.8): https://dev.epicgames.com/documentation/en-us/unreal-engine/networked-physics-overview
- Roadmap: Networked Physics (Beta): https://portal.productboard.com/epicgames/1-unreal-engine-public-roadmap/c/1501-networked-physics-beta
- Roadmap: Asynchronous physics simulation: https://portal.productboard.com/epicgames/1-unreal-engine-public-roadmap/c/381-asynchronous-physics-simulation
- EPhysicsReplicationMode (Resimulation "Work In Progress"): https://dev.epicgames.com/documentation/unreal-engine/API/Runtime/Engine/EPhysicsReplicationMode
- Devtricks, Networked Physics and UNetworkPhysicsComponent in 5.8 (updated 2026-09-23): https://vorixo.github.io/devtricks/phys-prediction-use/
- Epic forum, Networked Physics Fundamentals (MBobbo, 2026-07-22): https://forums.unrealengine.com/t/tutorial-networked-physics-fundamentals/2737552
- Epic forum, Networked Physics Pawn tutorial: https://forums.unrealengine.com/t/tutorial-networked-physics-pawn-tutorial/2657596
- Epic forum, network physics with constraints: https://forums.unrealengine.com/t/network-physics-component-how-to-do-for-complex-systems-with-constraints/2676595
- Mover in Unreal Engine (Experimental): https://dev.epicgames.com/documentation/en-us/unreal-engine/mover-in-unreal-engine
- Mover Examples (limitations): https://dev.epicgames.com/documentation/en-us/unreal-engine/mover-examples-in-unreal-engine
- Roadmap: Character Mover 2.0 (Experimental): https://portal.productboard.com/epicgames/1-unreal-engine-public-roadmap/c/1281-character-mover-2-0-experimental-
- StraySpark, Mover 2.0 vs CMC decision guide (secondary, 2026): https://www.strayspark.studio/blog/mover-2-0-vs-character-movement-component-ue5-7-2026
- Physics settings reference (async tick labelled experimental): https://dev.epicgames.com/documentation/en-us/unreal-engine/physics-settings-in-the-unreal-engine-project-settings
- Physics sub-stepping: https://dev.epicgames.com/documentation/en-us/unreal-engine/physics-sub-stepping-in-unreal-engine
- Async physics slows at low game-thread fps (forum): https://forums.unrealengine.com/t/async-physics-slows-down-at-low-game-thread-fps/2731575

**Chaos Destruction**
- Chaos Fields user guide: https://dev.epicgames.com/documentation/en-us/unreal-engine/chaos-fields-user-guide-in-unreal-engine
- GeometryCollectionComponent (Python API, property list): https://dev.epicgames.com/documentation/en-us/unreal-engine/python-api/class/GeometryCollectionComponent
- DamageModelTypeEnum: https://dev.epicgames.com/documentation/en-us/unreal-engine/python-api/class/DamageModelTypeEnum
- GeometryCollectionDamagePropagationData: https://dev.epicgames.com/documentation/en-us/unreal-engine/python-api/class/GeometryCollectionDamagePropagationData
- Fracturing Geometry Collections user guide: https://dev.epicgames.com/documentation/en-us/unreal-engine/fracturing-geometry-collections-user-guide
- Cluster Geometry Collections user guide: https://dev.epicgames.com/documentation/unreal-engine/cluster-geometry-collections-user-guide-in-unreal-engine
- Fracture Mode selection tools: https://dev.epicgames.com/documentation/en-us/unreal-engine/fracture-mode-selection-tools-user-guide
- Destruction quick start: https://dev.epicgames.com/documentation/unreal-engine/destruction-quick-start
- Physics in Unreal Engine (overview): https://dev.epicgames.com/documentation/en-us/unreal-engine/physics-in-unreal-engine
- Forum: field responses, documentation lacking: https://forums.unrealengine.com/t/managing-geometry-collections-with-chaos-physics-field-responses-and-controlled-destruction/2557942
- Forum: UE5 Chaos replication issues: https://forums.unrealengine.com/t/ue5-chaos-replication/2740297
- Chaos Visual Debugger: https://dev.epicgames.com/documentation/en-us/unreal-engine/chaos-visual-debugger-in-unreal-engine

**Constraints and cables**
- Physics Constraint reference (5.8): https://dev.epicgames.com/documentation/en-us/unreal-engine/physics-constraint-reference-in-unreal-engine
- Forum: constraint broken event not firing: https://forums.unrealengine.com/t/5-0-2-chaos-bug-physics-constraint-broken-event-not-firing/594556
- UPhysicsControlComponent API: https://dev.epicgames.com/documentation/unreal-engine/API/Plugins/PhysicsControl/UPhysicsControlComponent
- Cable Component: https://dev.epicgames.com/documentation/en-us/unreal-engine/cable-components-in-unreal-engine

**Vehicles**
- Chaos Vehicles: https://dev.epicgames.com/documentation/unreal-engine/chaos-vehicles
- FWheelsOutput (wheel outputs not replicated; plugin under Experimental): https://dev.epicgames.com/documentation/unreal-engine/API/Plugins/ChaosVehicles/FWheelsOutput
- Chaos Modular Vehicles: https://dev.epicgames.com/documentation/unreal-engine/chaos-modular-vehicles

**Rendering, audio, VFX, world**
- Nanite: https://dev.epicgames.com/documentation/unreal-engine/nanite-virtualized-geometry-in-unreal-engine
- Virtual Shadow Maps: https://dev.epicgames.com/documentation/en-us/unreal-engine/virtual-shadow-maps-in-unreal-engine
- Lumen: https://dev.epicgames.com/documentation/en-us/unreal-engine/lumen-global-illumination-and-reflections-in-unreal-engine
- World Partition: https://dev.epicgames.com/documentation/en-us/unreal-engine/world-partition-in-unreal-engine
- MetaSounds: https://dev.epicgames.com/documentation/en-us/unreal-engine/metasounds-in-unreal-engine
- MetaSounds quick start (Set Float Parameter, Execute Trigger Parameter; labelled Beta): https://dev.epicgames.com/documentation/en-us/unreal-engine/metasounds-quick-start
- Niagara Data Channels: https://dev.epicgames.com/documentation/unreal-engine/data-channels-in-niagara-for-unreal-engine
- Gameplay Ability System: https://dev.epicgames.com/documentation/en-us/unreal-engine/gameplay-ability-system-for-unreal-engine
- GAS prediction key (what is and is not predicted; older docs): https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Plugins/GameplayAbilities/FPredictionKey

**Networking, replay, profiling**
- Iris replication system (doc page says Experimental): https://dev.epicgames.com/documentation/en-us/unreal-engine/iris-replication-system-in-unreal-engine
- Replication Graph: https://dev.epicgames.com/documentation/en-us/unreal-engine/replication-graph-in-unreal-engine
- Replay system: https://dev.epicgames.com/documentation/en-us/unreal-engine/using-the-replay-system-in-unreal-engine
- DemoNetDriver and streamers: https://dev.epicgames.com/documentation/en-us/unreal-engine/demonetdriver-and-streamers-in-unreal-engine
- Unreal Insights: https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-insights-in-unreal-engine
- Networking config keys (secondary): https://ikrima.dev/ue4guide/networking/configuration/
- Server tick rates by game (secondary): https://diamondlobby.com/server-tick-rates/
- Rocket League 120 Hz physics, 60 Hz network (community): https://steamcommunity.com/app/252950/discussions/0/1693788202025800838

**Online services and hosting**
- Edgegap vs AWS GameLift (vendor pricing and regions): https://edgegap.com/comparison/edgegap-vs-aws-gamelift
- Edgegap vs Agones: https://edgegap.com/comparison/edgegap-vs-agones
- Gameye cost comparison (vendor; GameLift egress note): https://gameye.com/cost-comparison/
- Hosting comparison incl. Hathora and Multiplay closures (secondary): https://tech-insider.org/gameye-vs-edgegap-vs-aws-gamelift-2026/

**Source control and CI**
- Using Perforce as source control: https://dev.epicgames.com/documentation/en-us/unreal-engine/using-perforce-as-source-control-for-unreal-engine
- Horde: https://dev.epicgames.com/documentation/en-us/unreal-engine/horde-in-unreal-engine
