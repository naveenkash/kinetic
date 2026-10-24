# KINETIC: Project Structure

_Status: proposal v0.1, 2026-10-24. Engine: UE 5.8.x. The data-table schemas mirror `docs/bible/05-core-mechanics.md` sections 2.2, 3.4, 5.3 and 6.1 to 6.5; the bible stays the source of the values._

## 1. Module split

| Module | Type | Depends on | Contents |
|---|---|---|---|
| `KineticCore` | Runtime | Engine, Chaos, PhysicsCore, NetCore | Shared types (`FLineState`, tiers, thresholds), data-table row structs, the line solver (`FKineticLineSolver`), Impact Monitor, unit conversion, timeline evaluation. No gameplay actors. Unit-testable headless |
| `KineticGameplay` | Runtime | KineticCore, GameplayAbilities, GameplayTags, Niagara, MetasoundEngine, GeometryCollectionEngine, FieldSystemEngine, PhysicsControl | Characters, Spool, Dog, Line, Cutter, flight body, kinematic actors, force volumes, Destruction Director, Tell Scheduler, match modes, replay hooks |
| `KineticUI` | Runtime | KineticGameplay, UMG, CommonUI | HUD, edge indicator, Scrap Cards, Ledger, menus, LayCast overlay |
| `KineticEditor` | Editor | KineticGameplay, UnrealEd, EditorSubsystem, ComponentVisualizer | Line-debug visualizer, impact-threshold overlay, tell-timeline inspector, GC validator commandlet, table import |
| `KineticOnline` (Phase 1) | Runtime, client | KineticCore, HTTP, WebSockets, Json, JsonUtilities, OnlineSubsystemSteam | Client side of the Supabase backend (no official Unreal SDK; direct HTTP and Realtime WebSocket): login and tokens, crews, matchmaking and join tickets, profile, shop and Steam purchases ([backend-architecture.md](backend-architecture.md) section 24) |
| `KineticServer` (Phase 1) | Runtime, server-only | KineticGameplay, HTTP, Json | Server side of the backend: manifest fetch at boot, join-ticket verification in `PreLogin`, heartbeats, match report builder from the `KineticCore` event stream, replay and raw-event upload, fleet SDK adapters, metrics export. The client build never links it |

Build targets: `KineticEditor`, `KineticGame`, `KineticServer`, `KineticClient`. The server target compiles without Niagara and MetaSound runtime content.

## 2. Folder layout

```
Kinetic/
  Source/
    KineticCore/        Public/ Private/   (Types, Tables, Solver, Impact, Timeline, Units)
    KineticGameplay/    Public/ Private/   (Player, Spool, Line, Cutter, Kinematics, Volumes, Destruction, Match, Replay)
    KineticOnline/      Public/ Private/   (Session, Crew, Matchmaking, Profile, Store, BackendClient)
    KineticServer/      Public/ Private/   (Manifest, JoinTicket, Heartbeat, MatchReport, Upload, Fleet)
    KineticUI/
    KineticEditor/
  Content/
    Core/               Tables/ (DT_*), DataAssets/ (DA_*), Curves/, Tags/, Input/ (IA_, IMC_)
    Characters/
      Shared/           SK_Rigger_Base, PA_Rigger_Base, ABP_Rigger, AM_*, AS_*, BS_*
      Climber/ Scrapper/ Rigger/ Doubler/ Breaker/     (livery MIs, class DA overrides)
      Faces/            voice MS_, portrait T_
    Spool/              BP_Spool, BP_Dog_*, BP_Line, M_Line, NS_LineRibbon, MS_Line, MS_Snap, SM_Dog_*
    Cutter/             SM_Cutter_*, MS_Cutter, NS_CutSpark
    Objects/
      Light/ Medium/ Heavy/ Rated/ Volatile/        (SM_, BP_, PM_)
    Arenas/
      BLS_Ballast/      L_BLS, Destruction/ (GC_, FS_), Kinematics/ (BP_, SPL_), Timeline/ (DA_ArenaTimeline_BLS), Lighting/, Audio/, Materials/, Meshes/, Textures/
      YRD_Yard/
      HNG_Hangtown/
      ANC_Anchor/
      Shared/           SM_ and M_ reused across arenas, Well volumes, Hooks
    VFX/                NS_ shared (dust, sparks, gel), NE_ emitters, M_ particle
    Audio/              MS_ patches (MSP_), SC_ sound classes, SM_ submixes, Attenuation/, Marshal/
    UI/                 WBP_, T_UI_, fonts
    Spectator/          LayCast cameras, overlays
    Debug/              Graybox arenas, test props, automation maps (excluded from shipping)
  Config/
  Plugins/
    KineticFleet/       fleet provider adapter(s), optional
  Tools/
    p4/ typemap, triggers;  ci/ BuildGraph scripts;  horde/ config
```

Arena prefixes: `BLS`, `YRD`, `HNG`, `ANC`.

## 3. Asset naming

| Prefix | Asset | Example |
|---|---|---|
| `BP_` | Blueprint actor or component | `BP_Hauler` |
| `ABP_` | Animation Blueprint | `ABP_Rigger` |
| `AM_` `AS_` `BS_` | Montage, sequence, blend space | `AM_Cutter_Swing` |
| `SK_` `SM_` | Skeletal mesh, static mesh | `SM_YRD_Billet_A` |
| `PA_` | Physics asset | `PA_Rigger_Base` |
| `PM_` | Physical material | `PM_Steel`, `PM_ScrapHeap` |
| `GC_` | Geometry collection | `GC_BLS_ContainerStack_03` |
| `FS_` | Field system actor or asset | `FS_ANC_Scaffold_R2Q1_Anchor` |
| `SPL_` | Spline path asset or actor | `SPL_YRD_Hauler_T2` |
| `M_` `MI_` `MF_` `MPC_` | Material, instance, function, parameter collection | `M_Line`, `MI_Line_Guild` |
| `T_` | Texture, suffix `_D _N _ORM _E` | `T_Container_01_D` |
| `NS_` `NE_` | Niagara system, emitter | `NS_LineRibbon` |
| `MS_` `MSP_` | MetaSound source, patch | `MS_Line`, `MSP_Pitch_FromForce` |
| `S_` `SC_` `SMX_` `ATT_` | Sound wave, class, submix, attenuation | `S_Seat_Clank_01` |
| `DT_` | Data table | `DT_HarnessClass` |
| `DA_` | Data asset | `DA_ArenaTimeline_YRD` |
| `CUR_` | Curve | `CUR_HaulRamp` |
| `IA_` `IMC_` | Input action, mapping context | `IA_FireDog` |
| `GA_` `GE_` `GC_Cue_` | Gameplay ability, effect, cue | `GA_Brace`, `GE_Staggered`, `GC_Cue_Seat` |
| `WBP_` | Widget Blueprint | `WBP_EdgeIndicator` |
| `L_` | Level | `L_YRD`, `L_Debug_Graybox01` |
| `DL_` | Data layer | `DL_Event_Shear` |

Gameplay cue prefix `GC_Cue_` avoids a clash with `GC_` for geometry collections. Arena-specific assets carry the arena code after the type prefix.

## 4. Data tables and assets

Values are entered in SI (m, s, kg, N, N·s). `KineticCore` converts to engine units at load. A CI check fails the build if a row value differs from the bible table it mirrors; the bible markdown is parsed by `Tools/ci/check_tables.py`.

### 4.1 `DT_HarnessClass` (05 sections 2.2, 3.4, 5.3, 5.4, 4.1)

| Column | Type | Climber | Scrapper | Rigger | Doubler | Breaker |
|---|---|---|---|---|---|---|
| `ClassId` | FName | Climber | Scrapper | Rigger | Doubler | Breaker |
| `MassKg` | float | 90 | 110 | 130 | 130 | 220 |
| `RunSpeed` | float | 7.5 | 7.0 | 6.2 | 6.2 | 5.0 |
| `SprintSpeed` | float | 9.0 | 8.5 | 7.5 | 7.5 | 6.0 |
| `JumpHeight` | float | 1.4 | 1.3 | 1.1 | 1.1 | 0.8 |
| `MantleHeight` | float | 2.6 | 2.4 | 2.2 | 2.2 | 1.8 |
| `AirControl` | float | 0.8 | 0.6 | 0.4 | 0.4 | 0.2 |
| `CrouchSpeed` | float | 3.5 | 3.2 | 2.8 | 2.8 | 2.2 |
| `bCanBrace` | bool | false | false | true | false | true |
| `bCanPerch` | bool | true | false | false | false | false |
| `FootHold` | float | 3 | 4 | 5 | 5 | 5 |
| `BracedFootHold` | float | 0 | 0 | 9 | 0 | 20 |
| `LineLength` | float | 50 | 20 | 40 | 30 | 20 |
| `TensionN` | float | 400 | 400 | 600 | 800 | 1000 |
| `OverspinTensionN` | float | 0 | 0 | 0 | 1200 | 0 |
| `KickLaunchSpeedV` | float | 18 | 14 | 12 | 14 | 10 |
| `HangLoadN` | float | 4000 | 4000 | 12000 | 8000 | 10000 |
| `DogSpeed` | float | 140 | 160 | 120 | 120 | 90 |
| `ModeSwitch` | float | 0.20 | 0.15 | 0.25 | 0.25 | 0.35 |
| `Recall` | float | 0.4 | 0 | 0.6 | 0.6 | 0.9 |
| `SetLines` | int32 | 1 | 1 | 2 | 1 | 1 |
| `SetLineLife` | float | 8 | 8 | 12 | 8 | 8 |
| `ActiveLines` | int32 | 1 | 1 | 1 | 2 | 1 |
| `HangStaggerSeconds` | float | 0.6 | 0.6 | 1.0 | 0.6 | 0.6 |
| `CutterWarmupOffset` | float | 0 | -0.05 | -0.03 | 0 | +0.10 |
| `SmallPropSpeedMul` (Haul acceleration and post-Kick push on Light and Medium under 100 kg; never the Kick's V) | float | 1.0 | 1.3 | 1.0 | 1.0 | 1.0 |
| `AirborneFireSpeedKeep` | float | 1.0 | 0.85 | 0.85 | 0.85 | 0.85 |
| `LadderSpeed` | float | 2.0 | 2.0 | 2.0 | 2.0 | 1.6 |

Shared constants live in `DA_SpoolGlobals`: `HaulRamp 0.4`, `KickTime 0.3`, `KickForceCapMul 1.5`, `bKickInterlockOnPlates true`, `ReelRate 1.0 m/s`, `ReelHoldThreshold 0.2 s`, `KinematicSeverSubsteps 4`, `HardVelocityCap 50`, `HaulCloseCap 25`, `JackSeparateCap 30`, `SnapImpulseBase 1500 N·s at 1200 N`, `CutRecoil 150 N·s per 1000 N`, `SmallPropBonusMassCap 100 kg`, `ManifoldJackTrigger 600 N`, `DogsMeet 0.5`, `DogRadius 0.08`, `SlackTimeout 4`, `SlipPerStep 0.5 m per 0.1 s`, `OverspinOctave 1.0`, `OverspinSnap 1.8`, `OverspinLaunch 10`, `OverspinStagger 0.8`, `OverspinCooldown 3`, `DoublerSplit 0.6`, `DoublerRestore 0.3`, `SetLineFade 0.5`, `SetLineProximity 5`.

### 4.2 `DT_CutterVariant` (05 section 4.1)

| Column | Type | Standard | Hot | Long |
|---|---|---|---|---|
| `Warmup` | float | 0.20 | 0.12 | 0.26 |
| `Swing` | float | 0.15 | 0.15 | 0.15 |
| `Recovery` | float | 0.30 | 0.30 | 0.30 |
| `Reach` | float | 1.5 | 1.1 | 2.0 |
| `ConeDeg` | float | 70 | 70 | 70 |

### 4.3 `DT_LineVariant`, `DT_DogVariant` (04 progression)

Modifiers as multipliers and offsets: `LengthMul`, `TensionMul`, `DogSpeedMul`, `RetetherMul`, `SetLifeMul`, `JackMul`, `OverspinWindowOffset`, `SnapRecoilMul`; Dogs: `PlateSeatAnyAngle`, `MetalLoadMul`, `MineralLoadMul`, `SeatsInMetal`, `SeatsInMineral`.

### 4.4 `DT_ObjectPhysics` (05 sections 6.1 to 6.5)

| Column | Type | Notes |
|---|---|---|
| `ObjectId` | FName | |
| `Tier` | enum `Light, Medium, Heavy, Rated` | Light under 30 kg, Medium 30 to 300, Heavy 300 to 3,000, Rated over 3,000 or structural |
| `MassKg` | float | |
| `Placement` | enum `Loose, Rack, Stack, Bolted, Rated` | |
| `BreakFreeN` | float | 0, 200, 500, 800, 1000 by placement; overridable. Compared with `>=` against the vector sum of active pull (Haul, Jack push, Kick). Hang never tests it |
| `PlacementFriction` | float | Effective floor friction for movable Heavies: rollers 0.02, skids or runners 0.05, hanging 0, bare floor 0.3 or more. The placement validator flags any Heavy above 0.05 as immovable (6.1a) |
| `bFixedScenery` | bool | True for Heavies on a bare floor and for water tanks; no line moves them |
| `bForceVolumeAffected` | bool | Derived: true only for loose objects (break-free 0). Force volumes skip everything else (6.6) |
| `bRated` | bool | Only a summed line force at or above 1,000 N moves it |
| `bKinematicAttachment` | bool | Kinematic on a carrier's path until released by script or by 1,000 N of line force (Gantry load, Lift counterweight) |
| `RatedConsequence` | FName | Row in `DT_RatedLoads` |
| `FractureThresholdNs` | float | 0 if non-fracturing |
| `FractureEngineThreshold` | float | Calibrated engine value (see `chaos-destruction.md` section 2) |
| `GeometryCollection` | TSoftObjectPtr | |
| `FragmentTierByLevel` | array of enum | |
| `LightFragmentLifetime` | float | 10 |
| `VolatileType` | enum `None, Propane, FuelDrum, DemoCharge, GasManifold` | |
| `VolatileTriggerSpeed` | float | 8, 10, 6, 0 |
| `VolatileTriggerJackN` | float | 0, 0, 0, 600 |
| `bDogSeatTriggers` | bool | false for all; charges are moved, not shot |
| `bFixedWorldGeometry` | bool | true for the manifold |
| `UnderwaterImpulseMul` | float | 0.5, no fire patch under more than 1 m of water |
| `BlastRadius` | float | 4, 3, 5, 0 |
| `BlastImpulseNs` | float | 1500, 1200, 2500, 0 |
| `BlastStallRadius` | float | 2 (propane) |
| `BlastFlipRadius` | float | 4 (propane) |
| `FirePatchRadius` | float | 3 (fuel drum) |
| `FirePatchSeconds` | float | 6 |
| `JetLength`, `JetSpeed`, `JetSeconds` | float | 6, 15, 3 for the manifold |
| `PhysicalMaterial` | TSoftObjectPtr | For the surface modifier table |
| `ForceVolumeOnBreak` | TSoftClassPtr | Water tank |

### 4.5 `DT_ImpactThresholds` (05 section 7.1) and `DT_SurfaceModifiers` (7.3)

| Row | `Stagger` | `Flip` | `Stall` | Measure |
|---|---|---|---|---|
| `PlayerVsStatic` | 5 | 8 | 12 | m/s |
| `PlayerVsKinematic` | 4 | 6 | 9 | m/s |
| `ObjectVsPlayer` | 300 | 600 | 1200 | N·s |
| `PlayerVsPlayerFloor` | 5 | 8 | 14 | m/s |
| `Crush` | | | 8000 N for 0.5 s | N, s |
| `HangLineHit` | 3 | 6 | | m/s |
| `LedgeGrabMax` | | | 8 | m/s |

Surface multipliers (apply to stagger, flip and Stall): Hard 1.0, Wood 1.2, Scrap/Laundry/Nets 2.0, DeepWater 3.0, StallGel 4.0.

### 4.5a `DA_ForceVolume_<Name>`

`ARef` (m/s², the acceleration of a 100 kg reference body; the volume applies a constant force of `100 × ARef` N per affected body), `Shape`, `FalloffCurve`, `Phases` (array of `Seconds`, `ARefScale`, `DirectionMode` for the Pulse's shove and inhale), `Period`, `FirstFireAt`, `LeadSeconds` (minimum 1.0), `AffectsPlayers true`, `AffectsLooseObjectsOnly true`.

### 4.6 `DA_ArenaTimeline_<Arena>` and `DT_RatedLoads`

Timeline event struct is in `kinematics-and-force-volumes.md` section 3.1. `DT_RatedLoads`: `LoadActor`, `ConsequenceField`, `LeadSeconds` (minimum 1.0), `ResetBetweenRounds` (always true).

### 4.7 `DT_MatchMode` (05 section 9.1)

`Mode`, `TeamCount`, `TeamSize`, `Rounds`, `RoundSeconds`, `ReserveSeconds`, `ReservesPerCrew`, `WinCondition`, `MidEventAtRemaining` (75 s Scrap, 240 s Claim, 120 s per Haul half, 180 s Free Scrap), `MidEventSiren` (5 s), `DropSeconds` (6), `EndReplaySeconds` (4), `SuddenDeathEvent` (arena-specific shrink event, FName).

## 5. Blueprint vs C++ policy

| In C++ | In Blueprint |
|---|---|
| Anything on the physics thread (line solver, impact, force volumes, kinematic pose evaluation) | Cosmetic responses to events (VFX, audio one-shots, UI animation) |
| Replication, RPCs, prediction, history | Arena-specific wiring: which field fires on which event, which light sector toggles |
| Thresholds, state machines, Stall, ring-out | Level scripting through the timeline asset, not Level Blueprint |
| GAS abilities' core logic (`GA_` base classes) | `GA_` child tuning and cue selection |
| Data-table row structs and loaders | Data entry |
| Anything that must run on the server build | HUD layout |

Blueprint never applies a force, changes a body's simulation state, or decides a threshold. Blueprint-callable wrappers exist for all of these so designers can prototype in the `Debug/` folder; prototypes graduate to C++ before they ship.

## 6. Source control: Perforce with Unreal Game Sync

Recommendation: Perforce Helix Core, not Git LFS.

- Unreal's binary `.uasset` and `.umap` files cannot be merged; the editor integrates exclusive checkout with Perforce, and the P4 typemap controls read-only handling ([Perforce in UE](https://dev.epicgames.com/documentation/en-us/unreal-engine/using-perforce-as-source-control-for-unreal-engine)). Git LFS file locking exists but is not integrated into the editor's check-out flow to the same degree, and large binary histories grow Git repositories painfully.
- Unreal Game Sync and Horde (build health, precompiled binaries for artists, CI) are built around Perforce ([Horde](https://dev.epicgames.com/documentation/en-us/unreal-engine/horde-in-unreal-engine)).
- Epic recommends a case-insensitive Perforce server to avoid tool issues ([Perforce in UE](https://dev.epicgames.com/documentation/en-us/unreal-engine/using-perforce-as-source-control-for-unreal-engine)).
- Cost: Perforce licensing above the free tier and a server to run. Accept it; the alternative is lost art.

Streams: `//Kinetic/main`, `//Kinetic/dev-<team>` child streams, `//Kinetic/release-<milestone>`. Engine source in `//Kinetic/engine` as a virtual stream pinned to the 5.8 hotfix in use, with project patches as a separate changelist series so an engine upgrade is a rebase, not an archaeology dig. Docs (`docs/`) are text and live in the same depot so the CI table check can read the bible.

## 7. Build and CI

- Horde for CI, remote execution (Unreal Build Accelerator) and UGS metadata. BuildGraph scripts in `Tools/ci/`.
- Per-changelist: compile all targets; run `KineticCore` unit tests headless (solver, impact classification, table parse); run the bible-table check.
- Nightly: cook Windows client, Linux server; 8-bot headless soak per arena; Insights capture compared with `performance-budgets.md`; GC and placement validators across `Content/Arenas`.
- Weekly: console cooks; PC minimum readability screenshot test in the darkened Anchor sector (line emissive luminance above a fixed threshold).
- Engine version pinned per milestone. Hotfix uptake requires a green nightly on a branch first, since networked physics APIs moved between 5.7 and 5.8.

## 8. Editor tooling to build

| Tool | Module | What it shows |
|---|---|---|
| **Line-debug visualizer** | `KineticEditor` (component visualizer) plus a runtime `showflag` | Each line as a colored segment: mode color, thickness by applied force, L and L0 labels, slip accumulation, which endpoint is treated as invMass 0 and why (static, kinematic, braced, break-free, rated), per-endpoint force vectors. Server and client predicted lines overlaid in PIE with the divergence in cm |
| **Impact-threshold overlay** | `KineticGameplay` debug draw, toggled by `kinetic.ImpactOverlay 1` | Per player: current state, last 10 contacts with relative speed, impulse, classification, surface multiplier, crush timer bar, pending Stall and grace countdown. Color bands at 5/8/12 and 300/600/1200 |
| **Tell-timeline inspector** | `KineticEditor` (asset editor for `DA_ArenaTimeline`) | A time ruler per arena with cue and payload markers, lead-time bars, violations of the section 11 minimums in red, a scrub that moves kinematics in the viewport to the chosen time, and a light-sector preview that reports line emissive luminance |
| **GC validator** | `KineticEditor` commandlet `Kinetic.ValidateGC` | Thresholds vs `DT_ObjectPhysics`, leaf convex vertex counts, removal flags, abandon level, naming |
| **Placement validator** | `KineticEditor` commandlet `Kinetic.ValidatePlacements`, also a map-check warning | Every Heavy in the level: `PlacementFriction` above 0.05 and not `bFixedScenery` is an error (immovable at these tensions, 6.1a); break-free and friction must match the authored placement; the Claim marker and Haul spool must be on runners or rollers. Spawn zones: the navmesh path length from any point in the zone to the nearest Well edge must be at least 25 m (05 section 9.3 measures spawn distance along walkable routes, not straight-line); arenas where every tier overhangs the Well spawn on the Rim crane decks |
| **Determinism recorder** | `KineticCore` | Records per-tick body states on server and clients with a shared match seed; the editor diff view shows the first divergent tick |
| **Panic-window bench** | `Debug/` map plus commandlet | Runs the 05 section 4.3 scenario per class at scripted latencies and prints seat-to-edge time and survival outcome |
