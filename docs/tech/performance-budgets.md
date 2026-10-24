# KINETIC: Performance Budgets

_Status: proposal v0.1, 2026-10-24. Engine: UE 5.8.x. Caps here are the numbers the other documents refer to; change them here first._

## 1. Platform targets and assumptions

| Tier | Hardware (assumed) | Target | Rendering path |
|---|---|---|---|
| PC minimum | 6-core CPU (Ryzen 5 2600 / i5-8400), 16 GB RAM, GTX 1660 Super / RX 5600 XT 6 GB, DX12 with Shader Model 6 and current drivers (required by Nanite and VSM: [Nanite](https://dev.epicgames.com/documentation/unreal-engine/nanite-virtualized-geometry-in-unreal-engine), [VSM](https://dev.epicgames.com/documentation/en-us/unreal-engine/virtual-shadow-maps-in-unreal-engine)) | 1080p, 60 fps, Low | Lumen software ray tracing, global tracing; Lumen Lite (Beta in 5.8) evaluated as the Low setting |
| PC recommended | Ryzen 5 5600 / i5-12400, 16 GB, RTX 3060 Ti / RX 6700 XT 8 GB | 1440p, 60 fps, High; or 1080p 120 fps | Lumen software detail tracing |
| PC competitive | Ryzen 7 7800X3D class, RTX 4070 or better | 1080p or 1440p, 240 fps cap | Lumen hardware RT optional |
| PlayStation 5, Xbox Series X | Current generation | 1440p internal, TSR to 4K, 60 fps | Lumen software, VSM, Nanite. Epic positioned Lumen, Nanite and VSM for 60 fps on these consoles from 5.1 onward ([80.lv on 5.1](https://80.lv/articles/unreal-engine-5-1-has-been-released/)) |
| Xbox Series S | Current generation, 10 GB shared | 1080p internal, 60 fps | Lumen Lite or software global tracing; VSM at reduced page pool |
| Nintendo Switch 2 | Stretch goal | 1080p docked 30 fps, or 60 fps with Lumen Lite | 5.8 release notes cite Lumen Lite for 60 fps on Switch 2 ([5.8 blog, via summary](https://ludusengine.com/blog/unreal-engine-5-8-whats-new)). Not committed |
| Dedicated server | 2 vCPU, 2 GB per match process, Linux | 60 Hz game and physics | No renderer |

Physics runs at 60 Hz on every tier. Frame rate never changes physics. Competitive settings change only rendering.

## 2. Frame budgets (60 fps, 16.67 ms)

Client, PC recommended and consoles:

| Thread | Budget (ms) | Breakdown |
|---|---|---|
| Game thread | 9.0 | Gameplay and Spool 1.5; CMC and animation 2.5; Physics Control ragdoll update 0.5; Niagara game-thread 0.8; audio parameter bridge 0.2; networking 0.8; UI 0.7; tells and timeline 0.2; misc 1.8 |
| Physics thread (async, parallel) | 8.0 | Must finish inside one 16.67 ms step with headroom for resimulation bursts. Rigid bodies 3.0; GCs 3.0; line solver and force volumes 0.5; collision events and Impact Monitor 0.5; resimulation reserve 1.0 |
| Render thread | 9.0 | Nanite culling and draw submission 3.0; VSM 2.0; Niagara render 1.0; UI 0.5; rest 2.5 |
| GPU | 15.0 | Nanite base pass 3.5; VSM 2.5; Lumen GI and reflections 4.0 (2.0 with Lumen Lite); Niagara 1.0; lines and translucency 0.5; TSR and post 2.5; UI 0.5; reserve 0.5 |
| Audio thread | 3.0 | MetaSound graph, 64 voices max, 16 reserved for lines |

Server, 2 vCPU:

| Thread | Budget (ms) | Notes |
|---|---|---|
| Game thread | 10.0 | Must hold 60 Hz: async physics slows when the game thread stalls ([forum report](https://forums.unrealengine.com/t/async-physics-slows-down-at-low-game-thread-fps/2731575)). Networking 3.0; gameplay 3.0; replication serialization 2.0; replay recording 1.0; reserve 1.0 |
| Physics thread | 12.0 | Same simulation as clients minus cosmetic bodies. Destruction 3.0 of this |

A resimulation on a client replays up to `Max Supported Latency Prediction` worth of steps (15 steps at 250 ms) in one frame. At 0.5 ms per replayed step for the resimulated set (own flight body, own lines, nearby props) that is 7.5 ms, which is why the physics thread keeps a 1.0 ms steady-state reserve and why only the owner's objects are in Resimulation mode. Measure this in week 1.

## 3. Object caps

| Cap | Server | Client | Enforced by |
|---|---|---|---|
| Awake non-GC rigid bodies | 96 | 128 (plus cosmetic) | Destruction Director sleeps oldest Light props |
| Active (fracturing or awake) geometry collections | 12 | 12 | Director queues further breaks |
| Awake GC fragments | 400 | 400 | Kill field on oldest Light fragments |
| Live lines | 16 | 16 | Rules: 6 to 9 players, Doubler has 2 |
| Set lines | 16 | 16 | Rules |
| Flight bodies | 9 | 9 | One per player |
| Visual ragdolls | 0 | 9 | Client only |
| Kinematic actors | 24 | 24 | Per arena |
| Force volumes active | 8 | 8 | Per arena |
| Niagara systems active | 0 | 150, of which ribbons (lines) 32 | Pool with eviction |
| Niagara particles | 0 | 200,000 GPU | Scalability |
| MetaSound voices | 0 | 64 | Concurrency groups |
| Replicated actors | 400 | | Budget check in CI |
| Pooled Light props | 150 (Yard), 60 (others) | same | Pre-spawned |

## 4. Memory

| Pool | PC recommended | PC minimum | Consoles (PS5 / XSX) | Series S |
|---|---|---|---|---|
| Total budget | 10 GB system, 8 GB VRAM | 8 GB system, 6 GB VRAM | 10 GB combined | 7 GB combined |
| Textures (streamed) | 3.0 GB | 2.0 GB | 3.0 GB | 1.8 GB |
| Nanite and static meshes | 1.5 GB | 1.2 GB | 1.5 GB | 1.0 GB |
| GC rest collections and collision | 300 MB | 300 MB | 300 MB | 300 MB |
| Skeletal and animation | 400 MB | 400 MB | 400 MB | 300 MB |
| Audio | 400 MB | 300 MB | 400 MB | 250 MB |
| Niagara and VFX | 200 MB | 150 MB | 200 MB | 120 MB |
| Lumen and VSM page pools | 600 MB VRAM | 400 MB | 600 MB | 350 MB |
| Replay ring (10 s local) | 64 MB | 64 MB | 64 MB | 64 MB |
| Networked physics history | 32 MB | 32 MB | 32 MB | 32 MB |
| Reserve | remainder | | | |

Server process: 2 GB, of which GC rest collections 300 MB, physics 200 MB, replication 100 MB, replay 128 MB.

## 5. Streaming and World Partition

- One World Partition persistent level per arena with One File Per Actor so level designers and artists do not block each other ([World Partition](https://dev.epicgames.com/documentation/en-us/unreal-engine/world-partition-in-unreal-engine)).
- Arenas are under 400 m across, so runtime streaming is effectively off: a single 2D runtime grid with a cell size of 256 m and a loading range covering the whole arena. Everything is resident from the loading screen. Loading-time budget 12 s on console from the main menu.
- Data Layers: `DL_Base`, `DL_Destruction` (GCs and fields), `DL_Event_<Name>` (Shear, Cable Day, Collapse, Shift Change props), `DL_LightSector_<N>` (Anchor towers), `DL_Spectator` (LayCast cameras), `DL_Debug`.
- HLOD only for vistas: the Rucu Gorge, the ravine below Hangtown, the Basin horizon.
- The server loads the same level with rendering components stripped; `DL_Spectator` and `DL_Debug` excluded.

## 6. Scalability

| Group | Low | Medium | High | Epic |
|---|---|---|---|---|
| Global illumination | Lumen Lite or software global tracing | Lumen software, low final gather | Lumen software, detail tracing | Lumen hardware RT where available |
| Shadows | VSM, small page pool | VSM | VSM | VSM, high resolution |
| Nanite | on | on | on | on |
| Effects | Niagara particle cap 50k; Level 3 GC fragments hidden (client cosmetic only) | 100k | 200k | 200k |
| Resolution | TSR 50 percent | TSR 67 | TSR 75 | native |
| Post | minimal | | | |

Never scaled: physics rate, line emissive and line audio (the Visibility Clause), tells, HUD edge indicator, fragment counts at or above Level 1. The Low preset must pass the readability test in the darkened Anchor sector.

## 7. Profiling plan

Captures use Unreal Insights ([docs](https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-insights-in-unreal-engine)) and the Chaos Visual Debugger.

| When | Capture | What we look for |
|---|---|---|
| Week 1 prototype | Client `-trace=cpu,frame,counters,bookmark` at 80 ms RTT; CVD recording of the line-on-player scene | Resimulation cost per step; handoff hitches; line solver time |
| Week 2 | Server and both clients, same trace plus `net` | Determinism divergence correlated with frame spikes |
| Week 4 and every milestone | 6-bot, 5-minute match per arena on server (`-trace=cpu,frame,net,counters`) and on PC minimum, PS5, Series S | Thread budgets above; peak during the mid-match event |
| Every arena content drop | Memory Insights at level load on Series S | Pool budgets in section 4 |
| Weekly automated | Headless server soak, 30 minutes, 8 bots, Free Scrap | Game-thread 99th percentile under 12 ms; awake-body cap never exceeded; no replicated-actor growth |
| Before each playtest | Networking Insights on a real 6-player match | Bytes per actor class vs `networking.md` section 3; priority starvation of lines |
| Destruction tuning | CVD capture per GC calibration drop | Threshold crossings, cluster breaks, contact impulses |

Bookmarks are emitted at round start, mid-match event, each Stall and round end so captures can be sliced by match phase. `stat unit`, `stat chaos` and `stat net` are shown in all internal builds. Any merge that moves a budget line above its number by more than 10 percent in the weekly soak is reverted first and discussed second.
