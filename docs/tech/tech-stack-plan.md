# KINETIC: Tech Stack and Implementation Plan

_Status: proposal v0.1, 2026-10-24. Engine: UE 5.8.x. The engine-side decisions in [architecture-overview.md](architecture-overview.md) and its sibling documents are adopted here as written and are not re-argued. This document covers the whole stack around them: workstations, tools, online services, backend, telemetry, build and release, platforms, legal, cost, and the order in which to stand each piece up. Vendor prices and terms marked **CHECK** were read on the date above and must be confirmed before money is spent._

## 0. Three facts that shape everything else

1. **Team size decides the tooling.** The roadmap assumes five core people in Phase 0 and about fifteen by Phase 1 ([team-and-roles.md](../production/team-and-roles.md)). Every choice below gives a small-team default (1 to 3 people) and a studio default (5 or more). If you are solo, read section 12 before buying anything.
2. **Mac-only until Phase 1, then add Windows.** The only machine today is a Mac. UE 5.8 runs natively on Apple Silicon, and Phase 0 is greybox physics and networking, which is platform-independent: Chaos, the Line solver, CMC, networked physics, multi-process PIE with Network Emulation, and a Mac dedicated server for local tests all work there. What a Mac cannot do: build or test the Windows client (there is no Mac-to-Windows cross-compile), profile DX12, Nanite and VSM on PC hardware, test a Windows client anti-cheat, run the min-spec PC readability test, or host console SDKs. The official Linux cross-compile toolchain is for Windows hosts, so the Linux server is built on a Linux machine instead. Windows is about 95% of the Steam audience, so a Windows build has to exist before the first external playtest. Section 2.1 gives the path.
3. **A dedicated server needs a source-built engine.** The launcher build of Unreal cannot compile a `Server` target. Week 0 is getting Unreal Engine source access on GitHub, cloning the engine at the pinned 5.8 hotfix tag, and building it. Plan for 150 to 250 GB of disk for engine, intermediates and DDC, and an hour or more for the first full compile.

## 1. The stack at a glance

| Layer | Choice | Where it is specified | Small-team alternative |
|---|---|---|---|
| Engine | Unreal Engine 5.8.x, source build, hotfix pinned per milestone | architecture-overview section 5 | Same. No alternative |
| Languages | C++20 for anything on the physics thread, replication, thresholds, server code; Blueprint for cosmetic wiring and arena scripting; Python for editor automation and CI checks | project-structure section 5 | Same |
| IDE | JetBrains Rider for Unreal, or Visual Studio 2022 with Visual Assist | new | Rider is free for non-commercial use (**CHECK** the licence once the game sells) |
| Physics | Chaos, async fixed step at 60 Hz, custom physics-thread Line solver, Networked Physics (Resimulation for the owner, Predictive Interpolation for the rest) | architecture-overview sections 3, 4; spool-and-line-implementation | Same |
| Movement | CharacterMovementComponent when grounded, one rigid flight body when line- or impact-driven; Mover not used | player-physics | Same |
| Abilities | Gameplay Ability System for activation, cooldowns, tags and cues only | architecture-overview D6 | Same |
| Destruction | Geometry collections, Fracture Mode, Chaos Fields, Dataflow | chaos-destruction | Same |
| Kinematics, force volumes | Timeline and spline actors evaluated from a replicated clock | kinematics-and-force-volumes | Same |
| Input, UI | Enhanced Input; UMG with CommonUI | project-structure section 1 | Same |
| Audio | MetaSounds, in-engine mixing | systems/audio sections 13, 14 | Same. Wwise or FMOD only if a dedicated sound designer prefers them; the line-singing system is already designed around MetaSound parameters |
| VFX | Niagara, Data Channels | systems/vfx | Same |
| Replication | Legacy replication at launch, Iris on a branch | architecture-overview D7 | Same |
| Platform services (PC) | Steam only: login ticket, overlay invites, real-money payments | backend-architecture sections 8, 9, 13 | Same |
| Server hosting | Managed fleet: Edgegap, AWS GameLift or Agones, chosen on a two-week RTT beacon test | networking sections 8, 9 | Edgegap pay-as-you-go until a beacon test justifies otherwise |
| Backend: accounts, crews, matchmaker, allocation, Ledger, economy, Contracts, Claims, moderation | Supabase (Postgres, Auth, Realtime, Storage, Edge Functions, Cron, Queues) plus a small TypeScript worker for the matchmaker and report processing; no Redis at launch | [backend-architecture.md](backend-architecture.md) | Same; Supabase Free for dev and staging |
| Voice | Games voice vendor with an Unreal plugin, or self-hosted LiveKit | backend-architecture section 16 | Vendor |
| Client anti-cheat | Server authority first; a commercial client anti-cheat evaluated in Phase 3 | backend-architecture section 16 | Server authority only |
| Telemetry, logs, crashes | **Not yet specified. See section 6** | this document | Hosted log store plus Sentry |
| Source control | Perforce P4 with Unreal Game Sync | project-structure section 6 | P4 free tier (5 users, 20 workspaces) |
| CI and build farm | Horde, Unreal Build Accelerator, BuildGraph | project-structure section 7 | One build PC running BuildGraph nightly; Horde when a second engineer joins |
| Content tools | See section 7 | art-pipeline | See section 7 |

## 2. Workstations and test hardware

| Machine | Spec | Why |
|---|---|---|
| Engineering workstation (one per engineer) | 16-core desktop CPU, 64 GB RAM (128 GB if you run the editor, a server and three PIE clients at once), RTX 4070 Ti or better with 12 GB or more, two 2 TB NVMe drives (engine and project on one, DDC on the other), Windows 11 | Source engine builds, multi-process PIE, Insights captures. The Phase 0 tests run a server and two clients on one machine |
| Content workstation | Same CPU class, 64 GB, RTX 4070 class | Nanite, Lumen, fracture authoring, bakes |
| Minimum-spec test PC | GTX 1660 Super or RX 5600 XT, 6-core CPU, 16 GB | The Low preset and the readability test in the darkened Anchor sector must pass on this tier ([performance-budgets.md](performance-budgets.md) section 6) |
| Second machine on the LAN | Any | Real two-machine client tests without a cloud bill |
| Shared cache (studio) | Unreal Zen Storage server for a shared DDC | Saves every artist a full shader compile |
| Console dev kits | Only after platform approval (section 9) | Phase 3 |

### 2.1 Starting from a Mac only

| When | Machine | Used for |
|---|---|---|
| Week 0 to Phase 0 week 4 | The Mac. Apple Silicon Pro or Max class, 32 GB RAM minimum (64 GB if you run the editor, a server and two clients together), 1 TB free or a fast external NVMe for engine source and DDC. Xcode installed for the toolchain; Rider for Mac as the IDE | Source engine build, all Phase 0 prototype work, Mac dedicated server for local multi-process tests |
| From Phase 0 week 5 | A rented Linux VM in the cloud, a few dollars a day while running | Builds the `KineticServer` Linux target natively and the container image for the fleet; doubles as the CI build agent. A Linux ARM64 server on ARM instances is an alternative if the fleet provider supports it |
| From Phase 0 week 5, occasional | A rented Windows GPU VM by the hour (cloud instances with an NVIDIA GPU, reached over Parsec or a similar remote desktop) | First Windows client builds, DX12 smoke tests, Windows bugs |
| Before Phase 1 week 7 (before any external playtest) | A Windows desktop at the engineering spec above, plus the min-spec test PC | Windows client builds and profiling, client anti-cheat testing, readability on min-spec, all artist-facing work. The Mac stays as a second development machine and the Mac build target |

Do not try Windows in a virtual machine on the Mac. Windows on ARM in Parallels runs x64 UE editors through emulation with limited GPU support, which is too slow and too unrepresentative for this project.

Phase 0 rendering on the Mac goes through Metal. Lumen and Nanite run on Apple Silicon, but their cost there tells you nothing about the PC and console budgets, so no performance decision is made from Mac numbers. Physics and network numbers (server frame time, physics time, bandwidth) are valid on any platform.

Latency testing uses UE's Network Emulation profiles in Phase 0 (custom profiles at 30, 80 and 150 ms, with jitter and loss) and a real server in a distant cloud region from Phase 0 week 5. Simulated latency alone does not reproduce jitter or bursty loss.

## 3. Week 0 setup checklist

1. Mac set up with Xcode at the version the UE 5.8 release notes list, and Rider. Backups configured before the first commit.
2. Unreal Engine source access on GitHub; engine cloned at the 5.8 hotfix tag; `Setup.command`, `GenerateProjectFiles.command`, Development Editor built for Mac. (On a Windows machine the same steps use the `.bat` files.)
3. `KineticServer` builds for Mac and runs locally with two clients. The Linux server target is added on the Linux build VM in Phase 0 week 5 (section 2.1).
4. Perforce server running (section 8), typemap from `Tools/p4/`, engine in its own stream, project stream created, exclusive checkout on binary assets.
5. Project created with the four modules from [project-structure.md](project-structure.md) section 1 and the build targets `KineticEditor`, `KineticGame`, `KineticServer`, `KineticClient`.
6. Project settings from [architecture-overview.md](architecture-overview.md) section 4: async physics on, fixed step 0.0166667 s, physics prediction on, max supported latency prediction 250 ms, `NetServerMaxTickRate=60`.
7. Plugins enabled: Gameplay Abilities, Physics Control, Geometry Collection and Fields, Dataflow, Niagara, MetaSounds, CommonUI, Enhanced Input, Online Subsystem Steam, HTTP and WebSockets, Chaos Visual Debugger.
8. Network Emulation profiles saved in editor preferences for 30, 80 and 150 ms.
9. Test harness: an Automation Spec target for `KineticCore` (headless solver and threshold tests) and a Gauntlet config that launches one server and two clients with emulated latency and collects logs. Every Phase 0 exit test should eventually run from this harness unattended.
10. CI job that compiles all targets and runs the `KineticCore` tests on every submit. Until the Unreal targets exist, [.github/workflows/ci.yml](../../.github/workflows/ci.yml) runs the engine-free core (`make -C game all-checks`) and the backend's checks and database tests on every push.
11. The bible-table check (`Tools/ci/check_tables.py`, project-structure section 4) wired into CI from the first data table.

## 4. Engine-side architecture: where each piece lives

The engine design is complete in `docs/tech/`. This is the map, not a restatement.

| System | Document | The decision that matters most |
|---|---|---|
| Authority, tick model, plugin set, risks, 4-week prototype | [architecture-overview.md](architecture-overview.md) | Server owns every outcome; physics 60 Hz async on every machine |
| Spool, Dog, Line in Haul, Jack (Kick plus push), Hang | [spool-and-line-implementation.md](spool-and-line-implementation.md) | The Line is a custom physics-thread constraint whose state lives in the networked physics history |
| Two-body player, drag, Stall, ring-out | [player-physics.md](player-physics.md) | CMC capsule plus one flight body; ragdoll is cosmetic |
| Destruction | [chaos-destruction.md](chaos-destruction.md) | Breaks are server events; fragments are local; damage thresholds calibrated by drop test |
| Kinematics, force volumes, tells | [kinematics-and-force-volumes.md](kinematics-and-force-volumes.md) | Everything scheduled evaluates from one replicated clock |
| Netcode, bandwidth, panic-window fairness | [networking.md](networking.md) | Grace window for cuts; 150 ms rewind cap; bandwidth is the server cost driver |
| Performance | [performance-budgets.md](performance-budgets.md) | Server game thread must hold 60 Hz or async physics slows |
| Modules, naming, data tables, Blueprint policy, editor tools | [project-structure.md](project-structure.md) | Blueprint never applies a force or decides a threshold |

**One scheduling conflict to settle before week 1.** [architecture-overview.md](architecture-overview.md) section 8 puts a dedicated server, Resimulation and a Line on a networked player in week 1. [roadmap-and-milestones.md](../production/roadmap-and-milestones.md) Phase 0 builds the Line locally in weeks 1 to 4 and adds the server in week 5. Follow the architecture order. Risks 1 and 2 in [risks.md](../production/risks.md) are both network risks, and a Line solver written without rewind support has to be rewritten to get it. Keep the roadmap's content (barrel, wall, beam, Cutter, volatiles) but run all of it on a server with two clients from the first week.

## 5. Backend and live services

KINETIC runs its own backend on Supabase; no platform-vendor online services are used. The full design is [backend-architecture.md](backend-architecture.md). In one paragraph: Supabase provides Postgres, Auth (sessions, staff MFA, bans), the Data API with row-level security, Realtime (crew and per-player channels), Storage (replays), Edge Functions (Steam login, purchases, game-server endpoints), Cron and Queues. A small TypeScript worker runs the matchmaker loop, fleet allocation and match-report processing. Redis is not used at launch; rate limiting is a Postgres limiter called by every write. Steam proves identity, handles overlay invites and takes real-money payments. Game servers fetch a match manifest at boot and post one report of facts at the end; the backend turns facts into Tonnage and rewards, so the economy can be tuned without a game patch.

The rules that matter most, from that document:

- Only the dedicated server reports progress, with a credential bound to one match. Reports are validated and processed once.
- One event schema, defined in `KineticCore` in Phase 0, feeds telemetry, progression and the post-match screen.
- Money and progress are append-only transaction rows.
- Map variants for Claims ship in the build and are switched by a flag; the season flip is a reviewed runbook.
- The whole loop runs on a Mac with the Supabase CLI and a local fleet adapter that launches the Mac server build.

## 6. Telemetry, logs and crashes

| Stream | Source | Destination | Needed from |
|---|---|---|---|
| Gameplay events ([backend-architecture.md](backend-architecture.md) section 12.1 facts, from the `KineticCore` event schema) | Dedicated server, batched over HTTPS at round end | A queryable event store: Axiom (already connected to your tools), ClickHouse, or BigQuery | Phase 0 week 3, the first playtest |
| Server health | Server: frame time, physics time, awake bodies, active lines, bytes sent per actor class, per second | Same store, or Prometheus and Grafana via OpenTelemetry | Phase 0 |
| Client performance | Client: frame time percentiles per thread, hitch counts, scalability preset, hardware | Event store, sampled | Phase 1 |
| Crashes | UE crash reporter redirected to Sentry or Backtrace; symbols uploaded by CI per build | Crash service | Phase 1, the first external build |
| Product analytics | Apprenticeship completion steps, queue time, match completion, day-1 and day-7 retention | Event store | Phase 1 external test |
| Playtest qualitative | The post-death "what killed you" answer, logged verbatim | Event store with match and Stall IDs | Phase 0 week 3 |

Dashboards to build first: the Phase 0 exit tests as live charts (survival rate per latency band, closure-split divergence per tick, false-Stall count), then the Phase 1 elimination mix against the bible 05 section 12 targets.

## 7. Content pipeline and tools

Scale: 73 objects, 5 harnesses, 10 Faces, 5 NPC types and 4 arenas ([asset-master-list.md](../production/asset-master-list.md)).

| Discipline | Tools | Notes |
|---|---|---|
| Modeling | Blender (free) or Maya | Build at spec dimensions in meters; export greyboxes on day one ([art-pipeline.md](../production/art-pipeline.md) section 2) |
| Sculpt, bake | ZBrush or Blender sculpt; Marmoset Toolbag or Substance Painter bakers | Hero props and harnesses only |
| Texturing | Substance 3D Painter and Designer | Trim sheets from the shared material library |
| Fracture | UE Fracture Mode and Dataflow in engine; Houdini optional for procedural pre-fracture | Validated by the GC validator commandlet |
| Characters | Shared humanoid rig across five harnesses; evaluate MetaHuman for the ten Faces' heads against the character art direction | MetaHuman saves months of head modeling and facial rigging if the look fits |
| Animation | UE Control Rig and Sequencer; inertial or video-based motion capture for the base set | Physics Control drives the cosmetic ragdoll |
| Audio | Reaper or another DAW, field recordings of steel, cable and winches, MetaSounds | Loudness targets in systems/audio section 14 |
| Voice | Recording studio and voice director (contract), UE Localization Dashboard plus a translation management service | Every Marshal and Face line is localized in Phase 3 |
| UI | Figma for layouts, UMG and CommonUI in engine | One token file drives Unreal, web and Figma ([systems/ui-design-system.md](../systems/ui-design-system.md)) |
| Design data | Bible markdown parsed into data tables by CI | The bible stays the source of every number |

## 8. Source control, CI and build distribution

**Source control.** Perforce P4 as decided in [project-structure.md](project-structure.md) section 6. The free tier is 5 users and 20 workspaces; above that the cloud plan was listed at $39 per user per month (**CHECK**). Host it on a small cloud VM with nightly checkpoint and depot backups to object storage, or on a local machine with off-site backups. A one-person project can run Git with LFS and file locking instead, but switches to Perforce before the first artist joins.

**The docs repository.** The design package is in Git today. The CI table check needs to read the bible. Either move `docs/` into the depot as `//Kinetic/main/docs` and keep the Git repository read-only, or mirror Git into Perforce on every commit. Pick one before the first data table exists.

**CI.**

| Trigger | Job | Small team | Studio |
|---|---|---|---|
| Every submit | Compile all targets, `KineticCore` tests, bible-table check | Build PC, BuildGraph | Horde with UBA |
| Nightly | Cook Windows client and Linux server; build the server container image and push it to the fleet registry; 8-bot headless soak; validators | Build PC | Horde |
| Weekly | Min-spec readability screenshots; console cooks once kits arrive | Manual | Horde |
| Release | Symbol upload to the crash service; Steam depot upload to a private branch | Script | Horde |

**Server deployment path.** Linux server build, Docker image, registry, new application version on the fleet, matchmaker pointed at it. Automate this by Phase 0 week 5 so real-latency tests use the same path as launch.

**Getting builds to people.** Internal: UGS precompiled binaries for artists. External playtests: Steam Playtest or a private Steam branch with keys, from Phase 1.

## 9. Platforms, stores and certification

| Item | Plan |
|---|---|
| Client form | A native desktop (and later console) game client. Not a web app: UE5 has no browser export, and Pixel Streaming from the cloud adds a video round trip on top of the network one, which breaks the panic window and costs a GPU per player. Web is for companion pieces only: the marketing site, a read-only Ledger and Claims season map, and internal admin tools |
| Launch platform | PC first, on Steam |
| Unreal royalty | 5% of gross revenue above the first $1M lifetime per product, under the standard Unreal Engine EULA (**CHECK** against the EULA in force) |
| Store fees | Steam takes 30% at first, stepping down at revenue thresholds (**CHECK**) |
| Crossplay and cross-progression | The KINETIC account links platform identities (backend-architecture section 8); everything keys on the KINETIC account ID; platform policies on cross-purchase and cross-currency apply per console |
| Consoles | Apply to PlayStation Partners, ID@Xbox and the Nintendo developer portal well before Phase 3. After approval, the Unreal Engine console platform code is released to the approved studio. Budget dev kits, a TRC/XR/Lotcheck pass per platform, and two to three months for a first certification |
| Console requirements that touch design | Account linking flows, platform entitlements for paid items, achievements, parental-control limits on chat and purchases, suspend and resume mid-match, platform UI for invites and crews |
| Age rating | IARC for digital storefronts; ESRB and PEGI submissions where a console requires them (**CHECK** per platform). The Stall and lethality tone (bible/04 open question 3) affects the rating |

## 10. Legal, privacy and compliance

| Area | What to do |
|---|---|
| Company and IP | Form a legal entity before contractors start. Every contract includes IP assignment, including voice talent and composer |
| Trademark | "KINETIC" is a common word. Run a trademark search in games and software classes before spending on branding |
| Privacy | GDPR, UK GDPR and CCPA: privacy policy, data processing agreements with every vendor (fleet, cloud host, voice, telemetry, crash), player data export and deletion that reaches all of them, telemetry consent where required |
| Minors | Expect a Teen or PEGI 12 to 16 rating. Age-gate at account creation. If under-13 players are allowed in any region, COPPA and equivalent rules apply: parental consent, chat off by default, no personalized ads. The simplest path is not allowing them |
| Voice and text chat | US CVAA accessibility duties apply to in-game communication. The UK Online Safety Act and the EU Digital Services Act add reporting and moderation duties for user-to-user chat. Rig Calls (bible/04 cosmetics) as the primary comms with crew-only voice keeps this small |
| Monetization | No randomized paid items, which the design already guarantees, keeps you clear of loot-box regulation. Show real-money prices next to sponsor credit. The Office Surplus shop copy (menus-and-frontend section 10) already avoids urgency and countdowns, which regulators now look for |
| Third-party licences | UE EULA; backend open-source dependencies; voice vendor; fonts; sound libraries; any marketplace assets. Keep a licence register from week 0 |
| Accessibility | Accessibility pass is in Phase 3 ([roadmap](../production/roadmap-and-milestones.md)); platform accessibility guidelines and CVAA make parts of it mandatory, so design the remapping, subtitle and colour-safe systems in Phase 1 |

## 11. Cost model

### 11.1 Hosting per match

From [networking.md](networking.md) section 3 and Edgegap's January 2025 published rates ($0.00115 per vCPU-minute, $0.10 per GB egress; **CHECK**, rates may have changed):

| Item | Per match-hour (6 players, 2 vCPU) |
|---|---|
| Compute: 2 vCPU × 60 min × $0.00115 | $0.14 |
| Egress, typical (45 kB/s per client): about 0.97 GB | $0.10 |
| Egress, worst case (102 kB/s per client): about 2.2 GB | $0.22 |
| **Total** | **$0.24 to $0.36 per match-hour, or $0.04 to $0.06 per player-hour** |

At 1,000 concurrent players, about 167 concurrent matches cost roughly $40 to $60 per hour while that concurrency holds. Every byte cut from the bandwidth table in networking section 3 saves money in proportion, which makes quantization and priority tuning a cost task as well as a quality task. Bare-metal fleet plans advertise no egress fees and would change this arithmetic; get a quote before Phase 3.

### 11.2 Spend by phase (small team, excluding salaries)

| Phase | Main costs |
|---|---|
| Week 0 | Nothing beyond the Mac; an external NVMe drive if internal storage is short |
| Phase 0 | Perforce host VM and backup storage; occasional cloud servers for real-latency tests (a few dollars per session); from week 5 a Linux build VM and hourly Windows GPU VM time; IDE licence once commercial |
| Phase 1 | Windows workstation and a min-spec test PC (bought only if the week 6 go/no-go passes); DCC subscriptions per artist; fleet hosting for external playtests; crash and telemetry services (free or low tiers cover playtest volume); Steam app fee per title; voice recording for two Faces |
| Phase 1 to 2 | Backend: Supabase Free for dev and staging, Pro ($25 a month) for production once data must persist, plus a few dollars for the worker host; about $60 to $250 a month at 1,000 concurrent players (backend-architecture section 22); remaining voice recording; localization vendor quotes |
| Phase 3 | Beta-weekend hosting at 1,000 concurrent players (section 11.1); console dev kits and certification; voice service; client anti-cheat licence if chosen |
| Live | Hosting scales with players; store fees and royalty scale with revenue; customer support tooling; moderation |

## 12. Scope for a small team

- The roadmap is 56 weeks to a 1,000-player beta with about fifteen people. Phase 0 (six weeks, engineering and greybox only) is realistic for one or two engineers, possibly a few weeks longer. Everything after it needs people or cuts.
- **Build only the thin backend slice before the Phase 1 exit test passes.** Phase 0 needs none (local `dev` login, a JSON profile). Phase 1 needs login, crews, matchmaking and allocation so strangers can play (backend-architecture section 26). The Ledger, economy, Contracts and Claims wait until the panic window is proven fair; the meta-game is worthless without it.
- **The cut line already exists.** [risks.md](../production/risks.md) item 11 allows three arenas and six Faces. For a small team, a fair test target is one arena (the Yard), two or three harnesses (Breaker, Climber, Rigger), Scrap mode, bot crews, and a Steam Playtest.
- **Queue fragmentation is a real launch risk.** A free-to-play 3v3 game with four modes and eight regions splits a small population sixty ways. Launch with two or three regions and one or two playlists, backfill with the bot crew ([characters/npcs/bot-crew.md](../characters/npcs/bot-crew.md)), and add regions when queue times allow.

## 13. When each part of the stack gets built

| Phase | Engine and gameplay | Services and infrastructure |
|---|---|---|
| Week 0 | Source engine on the Mac, project skeleton, modules, settings | Perforce, CI compile job, Mac server target, Network Emulation profiles, Gauntlet harness |
| Phase 0, weeks 1 to 6 | The architecture-overview prototype order: Line on a networked player in week 1, two bodies and determinism in week 2, panic window in week 3, impact and destruction in week 4, then the roadmap's weeks 5 and 6 content | Event schema (backend-architecture section 12.1); determinism recorder; telemetry to an event store; server container pipeline to the fleet by week 5 for the real-latency test |
| Phase 1, weeks 7 to 20 | Five harnesses, the Yard, HUD, audio, VFX, Apprenticeship, bots | Backend Phase 1 slice: Steam login, crews, matchmaker, fleet allocation, join tickets, match reports stored; crash reporting; Steam Playtest; the region beacon test (move it here from pre-alpha so the fleet choice is made on data) |
| Phase 2, weeks 21 to 44 | Ballast, Hangtown, the Anchor, other modes | Backend: Ledger, inventory, economy, Contracts, Claims, ratings, admin console; cosmetics pipeline; localization infrastructure; remote config |
| Phase 3, weeks 45 to 56 | Performance to budget on every platform | Steam purchases, voice, moderation tools, client anti-cheat decision; 1,000-player load test; console ports and certification; privacy, age-gate and chat compliance; season-flip runbook rehearsed |
| Phase 4 | Season content | Live-ops admin tools, player support, status page, moderation workflow |

## 14. New decisions to make

These are in addition to the twenty engineering decisions in [README.md](README.md).

| # | Decision | Default here | Needed by |
|---|---|---|---|
| S1 | Team size and who wears which hat | Read team-and-roles as a list of hats | Now |
| S2 | When to buy a Windows workstation | Mac only through Phase 0; rented Linux and Windows VMs from week 5; a Windows desktop and min-spec PC before Phase 1 | Phase 0 week 6 go/no-go |
| S3 | Phase 0 order: network from week 1 (architecture-overview) or local first (roadmap) | Network from week 1 | Week 0 |
| S4 | Where `docs/` lives once Perforce exists | Move into the depot | Before the first data table |
| S5 | Backend | KINETIC's own (backend-architecture.md), on Supabase, with its own decisions B1 to B13 | Phase 1 start |
| S6 | Telemetry store and crash service | A hosted event store plus Sentry | Phase 0 week 3 |
| S7 | Business model: free-to-play with Sponsorship and the Manifest as the paid tracks, as bible/04 implies | Free-to-play | Phase 1 |
| S8 | Launch platforms | PC only on Steam, consoles after | Phase 1 |
| S9 | Voice chat scope | Crew-only voice, Rig Calls as primary comms | Phase 1 |
| S10 | Launch regions and playlists | Two or three regions, one or two playlists | Phase 3 |
| S11 | Trademark search on the name | Run it | Before any branding spend |
