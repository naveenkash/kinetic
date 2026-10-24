# KINETIC: Networking

_Status: proposal v0.1, 2026-10-24. Engine: UE 5.8.x. Player counts and timings from `docs/bible/05-core-mechanics.md` sections 3, 4 and 9._

## 1. Model

Dedicated server, authoritative over all physics outcomes. Clients send inputs, predict their own character, Dog, line and flight body, and interpolate everything else. No listen servers, no peer-to-peer, no relays.

Player counts: 6 for Scrap, Claim and Haul (3v3); 8 for Free Scrap. 3v3v3 (9 players) is post-launch per the bible v0.2.1 rulings. Budgets below are computed for 6 and 8; 9 is noted where it changes a number.

## 2. Tick rates

| Clock | Value | Config |
|---|---|---|
| Server game thread | 60 Hz | `NetServerMaxTickRate=60` in `[/Script/OnlineSubsystemUtils.IpNetDriver]`. Set it explicitly; sample configs show both 30 and 60 as defaults in the wild ([config reference](https://ikrima.dev/ue4guide/networking/configuration/)) |
| Physics (server and clients) | 60 Hz fixed, async | `Tick Physics Async`, `Async Fixed Time Step Size = 0.0166667` |
| Client input send | 60 Hz | CMC moves and networked physics inputs piggyback the same packet |
| Net update, owner's actors and lines on their body | 60 Hz | `NetUpdateFrequency=60` |
| Net update, other players and tier-1 bodies | 30 Hz | `NetUpdateFrequency=30` |
| Net update, tier-2 bodies | 10 Hz | |
| Net update, tier-3 bodies and sleeping | 3 Hz, dormant when asleep | `DORM_DormantAll` on sleep |

Comparison points: Valorant 128, Overwatch 2 and Rainbow Six Siege 64, Halo Infinite and Rocket League 60, Fortnite 30, Apex Legends 20 ([tick rate table](https://diamondlobby.com/server-tick-rates/)). Rocket League simulates physics at 120 Hz behind its 60 Hz send rate ([community report](https://steamcommunity.com/app/252950/discussions/0/1693788202025800838)). 60/60 puts KINETIC at the physics-game norm without Rocket League's ball-contact precision needs.

## 3. Bandwidth estimate

Payload sizes assume quantized vectors (1 cm, 16-bit per axis within arena bounds) and compressed quaternions. Each replicated physics body costs about 48 bytes per update (position 6, rotation 6, linear velocity 6, angular velocity 6, server frame and flags 8, actor header and property bits 16). A kinematic-character update costs about 32 bytes. A line force update costs 5 bytes.

Per-client downstream, worst case (96 awake rigid bodies, 12 active geometry collections, all lines live):

| Stream | Count | Bytes | Rate (Hz) | Bytes/s |
|---|---|---|---|---|
| Other players' bodies (airborne worst case) | 5 | 48 | 60 | 14,400 |
| Own body corrections | 1 | 48 | 60 | 2,880 |
| Live and set lines (force byte over 0 to 6 kN) | 16 | 5 | 30 | 2,400 |
| Tier-1 bodies (within 15 m of any player, or on a line) | 24 | 48 | 30 | 34,560 |
| Tier-2 bodies | 40 | 48 | 10 | 19,200 |
| Tier-3 bodies | 32 | 48 | 3 | 4,608 |
| GC level-1 clusters, nearest 36 | 36 | 48 | 10 | 17,280 |
| GC level-1 clusters, remaining 36 | 36 | 48 | 2 | 3,456 |
| Events (seat, cut, break, tells), average | | | | 1,500 |
| UDP/IP headers | 60 packets | 28 | 60 | 1,680 |
| **Total worst case** | | | | **≈ 102,000 B/s ≈ 0.82 Mbit/s** |

Typical case (30 awake bodies, 3 active GCs, 2 players airborne): about 45,000 B/s, or 0.36 Mbit/s. With 8 players add about 6,000 B/s worst case per client for the extra bodies and lines; with 9, about 9,000 B/s.

Upstream per client: inputs at 60 Hz, roughly 24 bytes (move, camera, Spool and Cutter bits, physics input), plus headers: about 3,200 B/s, or 26 kbit/s.

Server egress: 6 clients at 102 kB/s is 612 kB/s (4.9 Mbit/s) worst case per match; 8 clients at 108 kB/s is 864 kB/s (6.9 Mbit/s). A match slot that is busy all month at the typical 45 kB/s per client moves about 700 GB; at the worst case about 1.6 TB. At Edgegap's published $0.10 per GB (Q3 2026, vendor figure, [comparison page](https://edgegap.com/comparison/edgegap-vs-aws-gamelift)) that is $70 to $160 per always-busy match slot per month, which makes bandwidth, not CPU, the cost to watch. Secondary sources report AWS GameLift dropped egress charges on generation-6 instances in June 2026 ([summary](https://gameye.com/cost-comparison/)); confirm on AWS pricing before choosing.

Settings: `MaxClientRate=100000`, `MaxInternetClientRate=100000`, `ConfiguredInternetSpeed=100000` (bytes per second). When a connection saturates, the priority system below sheds tier-2 and tier-3 updates first; lines and players are never shed.

## 4. Relevancy

At 6 to 9 players in arenas under 400 m across, everything is relevant to everyone. `bAlwaysRelevant` is off, but the cull distance is set beyond arena bounds. Replication Graph is unnecessary at this scale and is disabled on console builds anyway ([docs](https://dev.epicgames.com/documentation/en-us/unreal-engine/replication-graph-in-unreal-engine)). Iris is "production-ready for licensees" per the 5.8 release notes but still labelled Experimental on its own doc page ([Iris docs](https://dev.epicgames.com/documentation/en-us/unreal-engine/iris-replication-system-in-unreal-engine)); we evaluate it on a branch in week 3 and ship on the legacy driver unless it is clean.

## 5. Priority

`NetPriority` per actor, evaluated per connection:

| Actor | Priority | Rule |
|---|---|---|
| A line with a Dog in the viewer's Catch Plate | 10 | Top. Always. The panic window depends on it |
| The viewer's own body and flight body | 10 | |
| Any line the viewer owns | 8 | |
| Other players | 6 | |
| Bodies attached to any live line | 5 | |
| Bodies within 15 m of the viewer | 4 | |
| Bodies in the viewer's frustum | 3 | |
| GC level-1 clusters awake | 2 | |
| Everything else awake | 1 | |
| Sleeping bodies | dormant | wake on server wake |

## 6. Prediction and reconciliation per class

| Class | Owner | Others | Reconciliation |
|---|---|---|---|
| Grounded character | CMC client prediction | Interpolated | CMC saved moves and server correction |
| Flight body | Physics replication mode **Resimulation** (client runs about half RTT ahead, caches history for at least one RTT, rewinds on mismatch) | **Predictive Interpolation** | As documented in the 5.8 overview ([networked physics](https://dev.epicgames.com/documentation/en-us/unreal-engine/networked-physics-overview)); sim-proxy mode set with the `Sim Proxy Rep Mode` override on the settings data asset ([Epic forum](https://forums.unrealengine.com/t/tutorial-networked-physics-fundamentals/2737552)) |
| Dog projectile | Predicted sweep | Replicated spawn and seat event | Snap to server seat |
| Line | Predicted from local inputs, including the Reel rest length from the owner's held input; solver state in physics history | Replicated state; Reel rest length at 30 Hz while reeling | Owner's line rewinds with the owner's body |
| Kinematic sever and set-line recall | Server events | Reliable multicast | Not predicted |
| Light and Medium props | Predictive Interpolation when within 15 m or on a line; Default mode otherwise | Same | Engine |
| Heavy and Rated | Predictive Interpolation | Same | Engine; break-free is a server event |
| GC | Server break events; local fragment sim below level 1 | Same | None below level 1 by design |
| Kinematics, force volumes, tells | Evaluated from the replicated clock | Same | Clock resync |

`Max Supported Latency Prediction` is set to 250 ms. A client whose RTT exceeds 300 ms is moved to Predictive Interpolation for its own flight body and warned; above 400 ms it is removed from matchmaking pools for the region.

## 7. Latency targets and the panic window

Targets: median RTT under 50 ms, 95th percentile under 120 ms, hard matchmaking cap 150 ms to the chosen region.

The panic window is 1.0 to 2.0 s from Dog seat to the point of no return. Latency eats into it from both ends: the seat tell arrives one-way-latency late, and the cut arrives one-way-latency late. At 150 ms RTT that is 150 ms lost, up to 15 percent of the shortest window. The server gives it back:

### 7.1 Server-side cut validation with a grace window

1. The client draws the Cutter. `ServerCutterDraw(ClientPhysFrame)` starts the warm-up timer on the server at the client's frame, bounded by the rewind cap.
2. The client swings. `ServerCutSwing(ClientPhysFrame, CameraRotation)`.
3. The server keeps a 20-frame (333 ms) history of every line segment and every player's plate transforms. It rewinds to `ClientPhysFrame`, capped at 150 ms before present (9 frames), and tests reach and the 70° cone against the line segment at that frame. Camera rotation rate between the last two client frames is clamped at 3,000 degrees per second.
4. If valid, the cut is applied now, and it is timestamped at `ClientPhysFrame`.
5. **Grace window G**: any line-caused Stall is held as *pending* for `G = min(150 ms, one_way_latency + 33 ms + 15 ms)` before it is confirmed. `one_way_latency` is the server's smoothed estimate for the victim's connection. If a valid cut arrives with a timestamp earlier than the Stall frame while the Stall is pending, the Stall is cancelled, the line is removed, and the body continues from its present state (the server does not rewind physics; the player is alive, near the thing that nearly killed them).
6. The harness-lock animation (0.2 s, 05 section 7.4) is longer than G, so a cancelled pending Stall is invisible to the victim and reads as a last-instant cut to everyone else.
7. Going over an edge is the point of no return and is not reversible by a cut; the Well volume is 0.5 s of fall below the lowest floor, so there is no race to adjudicate there.

What this buys: a player at 150 ms who reacts at the same wall-clock moment as a player at 30 ms gets the same outcome, because the cut is judged at the frame they swung, not the frame it arrived. What it costs: killers see a confirmed Stall up to 150 ms late. The Scrap Card already waits for the Marshal call, so nothing visible is delayed.

### 7.2 Lag compensation for Dog hits

The Catch Plate rewind is the same 20-frame buffer, capped at 150 ms (see `spool-and-line-implementation.md` section 2.3). Unreal does not ship a general rewind for hit registration; this is project code. It is cheap: 9 players × 4 plates × 20 frames of 28 bytes is 20 KB.

### 7.3 Validation matrix

Automated test at 30, 80 and 150 ms simulated RTT with 20 ms jitter, 20 trials each of the 05 section 4.3 scenario with a scripted victim reaction at 1.1 s after seat. Pass: identical survival outcome in all three latency buckets, and the server-confirmed Stall arrives at the killer no later than seat time plus the scenario time plus G.

## 8. Matchmaking and session flow

KINETIC runs its own backend for identity, crews, matchmaking, allocation and results; no platform-vendor online services are used. The full design is [backend-architecture.md](backend-architecture.md). The in-match side of the flow:

1. **Identity**: the client exchanges a Steam web API ticket for a Supabase Auth session through the `auth-steam` function (backend-architecture section 8).
2. **Crews and matchmaking**: crews of one to three queue with mode, region ping table and rating; the KINETIC matchmaker pairs teams under the 150 ms cap (backend-architecture section 10).
3. **Allocation**: the matchmaker asks the fleet (Edgegap, AWS GameLift, or Agones on our own Kubernetes) for a server in the chosen region through one adapter interface. Hathora is reported to have shut down in May 2026 and Unity Multiplay ended direct support in March 2026 ([secondary source](https://tech-insider.org/gameye-vs-edgegap-vs-aws-gamelift-2026/)); both are off the list pending confirmation.
4. **Manifest**: the dedicated server boots, fetches the match manifest (roster, validated loadouts, arena variant, rules) with a credential bound to that match, and reports ready.
5. **Join**: clients travel to the connect string with a backend-signed join ticket. The server verifies the signature locally in `PreLogin` and checks the roster; no backend round trip on join. Reconnects fetch a fresh ticket.
6. **End**: the server posts one match report of facts to the backend, uploads the replay and raw events, and is returned to the fleet.

## 9. Region strategy

- Launch regions: NA East, NA West, EU West, EU Central, South America East, Japan, Singapore, Australia East. Edgegap advertises 615 locations across 17 providers; GameLift is quoted at 21 AWS regions or 35 locations with Local Zones depending on the source ([comparison](https://edgegap.com/comparison/edgegap-vs-aws-gamelift), [summary](https://gameye.com/cost-comparison/)). Pick the provider by measured 95th percentile RTT from a two-week beacon test, not by location count.
- Clients ping beacons in all regions at login and every 10 minutes. The matchmaker places a match in the region that minimizes the maximum RTT across the six players, subject to the 150 ms cap. Crews that cannot satisfy the cap together are offered a longer queue, never a worse match.
- One match per server process; 2 vCPU and 2 GB per process as the starting SKU (see `performance-budgets.md`).

## 10. Replay recording

- The server records every match with the DemoNetDriver (`bHandleDedicatedServerReplays = true`), to a memory streamer ring for the in-match uses and to an HTTP streamer for LayCast ([replay docs](https://dev.epicgames.com/documentation/en-us/unreal-engine/using-the-replay-system-in-unreal-engine), [streamers](https://dev.epicgames.com/documentation/en-us/unreal-engine/demonetdriver-and-streamers-in-unreal-engine)).
- **Victim-camera replay** (4 s at round end, 05 section 9.2): each client also records the last 10 s locally with the memory streamer. The victim's camera rotation is replicated to all clients at 30 Hz as a compressed quaternion (about 720 B/s for six players, included in the estimate above). At round end each client plays its local replay from the Stall frame minus 4 s, with the camera attached to the victim's head socket and driven by the replicated rotation. No extra download.
- **LayCast spectator**: a spectator client joins as a spectator connection with its own priority table (all players equal, all lines high), plus the HTTP-streamed server replay with a 30 s broadcast delay for the production desk.

## 11. Cheat surfaces

| Surface | Mitigation |
|---|---|
| Client-authored position | Clients send inputs only. CMC moves validated against class speeds; flight body is physics-replicated, never client-set |
| Fake cut timestamps | Rewind capped at 150 ms; camera turn rate clamped; warm-up tracked on the server from the draw request |
| Fake Dog hits | Server resweeps against rewound plates; client seat is cosmetic |
| Speed or tension hacks | Tension and ramps are server data; clients predict with the same table but the server's line state wins |
| Wallhack on lines | Lines are the game's intended readable; nothing to hide. Enemy positions are needed for interpolation; the arenas are small. Accept |
| Physics misprediction farming | Per-player correction statistics; flag systematic favourable corrections |
| Server-side bots or DoS | Fleet provider DDoS protection; backend-signed join tickets checked against the roster; connection rate limits |
| Memory tampering | Server authority limits what tampering can change; commercial client anti-cheat evaluated in Phase 3 (backend-architecture section 16) |
