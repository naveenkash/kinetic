# KINETIC: One Game Server for Every Client (Option)

_Status: **PROPOSED**, not decided. Raised 2026-10-25. The baseline in networking.md and tech-stack-plan.md (an Unreal dedicated server running Chaos) stands until this is decided._

## The idea

Run matches on one engine-free server program built on `KineticCore`, instead of on an Unreal dedicated server. Clients send inputs and draw what the server reports. The Unreal client and the browser client (`web/`) both talk to the same server, so browser and Unreal players could share a match.

## What already supports it

- **The game's rules:** `KineticCore` holds them with no engine. That covers lines, Haul, Jack and Hang, the Cutter, impacts and Stalls, match flow and bots.
- **Most of the server's networking:**
  - the encodings, priority, tiers and the bandwidth scheduler;
  - the rewind history, cut validation, the Dog re-sweep and the Stall grace window (networking.md section 12).
- **The browser client:** it runs the same `KineticCore`, compiled to WebAssembly. It is the first client that could connect to such a server.

## What it changes

| Area | Unreal server (baseline) | Standalone server |
|---|---|---|
| Physics authority | Chaos, inside the Unreal server | An engine-free physics library (Jolt is the first candidate) on the server. Clients use the same library for anything gameplay-relevant, so their predictions match; Chaos becomes cosmetic (debris, cloth) |
| Replication and prediction | Unreal's replication, CMC prediction, networked physics | Project netcode on both sides: snapshots, client prediction and reconciliation, interpolation |
| Transport | Unreal's UDP net driver | UDP for native clients, plus WebTransport or WebRTC for browsers, which cannot open raw UDP |
| Destruction | Chaos geometry collections, replicated at cluster level 1 | Server-side fracture as scripted break states; detailed fracture stays cosmetic on clients |
| Server cost | About 2 vCPU and 2 GB per match (decision 17) | Expected to be lower, since there is no engine; unmeasured |
| Cross-play | Unreal clients only | Any client that speaks the protocol |

## Risks to test before deciding

1. **Heavy physics:** can Jolt carry the game's physics at 60 Hz within budget on the server? That means stacks, hull sections, the Gantry load and the Breathing hull.
2. **Prediction parity:** the Unreal client's Jolt predictions must match the server closely enough. The two prediction-sensitive cases are the flight body and the line on the player.
3. **Netcode cost:** how much custom netcode it takes to match Unreal's (snapshot compression, prediction and interpolation), against the time saved by not fighting Unreal's networked physics.
4. **Browser transport:** WebTransport support across the target browsers, with a WebSocket fallback, and what it does to latency.

## Proposed path

Build the browser client first, single player against bots (`web/`). Then, if the decision goes ahead, add a minimal standalone server that runs one match for two browser clients. Measure risks 1 to 4 on it before any Unreal client work depends on the choice.
