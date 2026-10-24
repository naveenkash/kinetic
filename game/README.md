# KINETIC game project

The Unreal project will live here (`Kinetic.uproject`, per [docs/tech/project-structure.md](../docs/tech/project-structure.md)). Until Unreal 5.8 is installed, this holds the engine-free core of Phase 0: the parts of `KineticCore` that do not need the engine, built and tested with any C++20 compiler.

## What is here

| Path | What it is |
|---|---|
| `Content/Core/Tables/*.csv` | The data tables (`DT_HarnessClass`, `DT_CutterVariant`, `DT_ImpactThresholds`, `DT_SurfaceModifiers`, `DT_BreakFree`, `DT_MassTiers`, `DT_Volatiles`, `DT_SpoolGlobals`) in SI units, in the CSV format Unreal imports as data tables |
| `Tools/ci/check_tables.py` | Fails the build if any table value differs from [bible/05](../docs/bible/05-core-mechanics.md), or is not a finite number; 231 values today |
| `Source/KineticCore/Public/Kinetic/` | The library: `LineSolver` (Haul, the Kick and Jack push), `HangSolver` (every Hang and Set line solved together, with slip, Reel and the floor), `MassRules` (footing, brace, break-free, rated commit, tiers, force volumes), `Spool` (overspin, Doubler split, snap and cut impulses), `Impact` (stagger, flip, Stall, the Impact Monitor, crush, blasts), `Cutter` (warm-up and the cone and reach cut test), `Tables`, `Units` |
| `Source/KineticCore/Public/Kinetic/` (match layer) | `MatchEvents` (the event schema, written as JSON Lines: Dog seats, mode engagements, cuts, Stalls, hits, snaps, rated commits, per-round time and mass moved, round ends, connections, server health), `Credit` (elimination, Spotter and assist credit, and the Scrap Card), `ScrapMatch` (the Scrap round clock and calls, best of 5, side swaps, sudden death, forfeit, leaver, idle and backfill rules), `MatchReport` (facts and the report the backend stores), `Determinism` (per-tick recordings and the first divergent tick), `Json`, `Sha256` |
| `Source/KineticCore/Public/Kinetic/Kinematics.h`, `ForceVolumes.h`, `Tells.h` | Kinematics as a function of match time (Hauler timetables with Shift Change, the Gantry cycle, the Ring, the Breathing hull), the Haul sled's ratchet, the client's server-time estimate, the swept test by which a kinematic severs lines; force volume profiles (Updraft, Pulse, water rush, gas jet, fire patch) with 05 6.6's mass scaling and cap; tell minimum leads, the `Tell.LeadTimes` check, the client tell scheduler, the server's lead gates, and mid-match event timing ([kinematics-and-force-volumes.md](../docs/tech/kinematics-and-force-volumes.md) section 4) |
| `Source/KineticCore/Public/Kinetic/NetCodec.h`, `NetPriority.h`, `Rewind.h` | Replication encodings (1 cm positions, smallest-three rotations, the 32-byte body and 5-byte line), per-viewer priority, rates and tiers, the per-connection scheduler that sheds tier-3 and then tier-2 bodies but never lines or players, the 20-frame rewind history, server-side cut validation (camera turn clamp, warm-up, swing cycle, reach and cone from the rewound eye), the Dog-hit re-sweep against rewound plates, the Stall grace window, the latency policy and correction statistics ([networking.md](../docs/tech/networking.md) section 12) |
| `Source/KineticCore/Public/Kinetic/Bots.h` | What the bot behavior trees decide: the cut reflex on lines in the bot's own plates (backfill bots between 1.0 and 1.6 s, the tutorial's Probationers at 1.4 s), a backfill bot's next rig from the base kit, what bots never do, and the tutorial crew's schedule ([game-modes-and-match-flow.md](../docs/systems/game-modes-and-match-flow.md) section 9.1) |
| `Source/KineticCore/Public/Kinetic/PlayerState.h` | The player's state machine (Grounded, Dragged, Airborne, Hoisted, Flipped, Staggered, Perched, LedgeHang, Stalled), which body is simulated and when to hand off, stagger and brace timers, what each state allows, airborne Dog fire, the Stall sequence and the Hooks cue |
| `Source/KineticCore/Public/Kinetic/ReferenceSim.h` | A small point-body simulator used to check the solver against the bible's worked examples and to run the panic-window bench. Production physics stays in Chaos |
| `Source/KineticCore/Tests/` | 218 tests, including section 7.3 of networking.md (the latency validation matrix and the grace window), bible/05 section 4.3 (the panic window), the 71:29 split, the Kick, Overboard and its counter, the Gallows, Anchor Drop, shared Hang loads, chains, 200 random line networks, swing loads (a Bola included), slip and Reel, every 7.1 impulse example, crush, and bit-identical determinism |
| `Tools/report/SampleMatch.cpp`, `Tools/ci/check_report.py` | Play a scripted Scrap match and check its report and raw events as the backend will read them (`api.srv_store_report`) |
| `Tools/determinism/DiffRecordings.cpp` | `build/diff-recordings A B [tolerance_m]`: where two determinism recordings first diverge, for the week 2 and week 5 "record positions, diff" tests |
| `Tools/bench/PanicBench.cpp` | Prints, for every attacker and victim class, when a Haul from 15 m lifts the victim off their feet and when it has dragged them 15 m |

The code follows Unreal's naming (`F` structs, `E` enums, `b` booleans) inside a `Kinetic` namespace and uses no engine types, so it moves into the `KineticCore` module unchanged; `FVec3` maps onto `FVector`. Everything is SI; `Units.h` converts to centimeters at the engine boundary.

## Build and test on a Mac

```bash
cd game
```

```bash
make all-checks
```

`make test`, `make test-asan`, `make bench`, `make check` and `make report` run the parts separately. CI runs `make all-checks` on every push ([.github/workflows/ci.yml](../.github/workflows/ci.yml)), along with the backend's format, lint, type, unit and database tests.

## Findings while implementing

These change the design documents; they are recorded in [docs/tech/spool-and-line-implementation.md](../docs/tech/spool-and-line-implementation.md) section 9 and [docs/tech/player-physics.md](../docs/tech/player-physics.md) section 4.

1. The Kick's velocity target must be taken at the end of each step. Taken at the start (as the design pseudocode had it), a fixed 60 Hz step never reaches V inside the 0.3 s window.
2. Haul and Jack must use each endpoint's free inverse mass, and footing, brace and break-free must be decided from that step's force. As designed, a Jack could not start a placed object, vent the manifold or break a brace, and Overboard applied no force.
3. Hang lines must be solved together as one constraint solve with impulses capped at the Hang load. As designed, two lines did not share a load, a swing read half its load, slip depended on the tick rate, a slipping line could push, and a Hang line never acted on a grounded player (the Gallows, Anchor Drop and the Overboard counter did nothing).
4. Slip works as a clutch whose friction is the Hang load. The bible's fixed slip rate cannot be applied: from rest it needs the line to push, and as a cap it needs an infinitely strong line (open item 7a). A line that slips past the end of its filament lets go.
5. The Kick counts toward objects' break-free at T, or a Climber's Kick would start a bolted object and a Lift counterweight (open item 7b).
6. bible/05 disagrees with itself on the slip rate: section 3.2 says 0.5 m per 0.1 s (5 m/s), the tells table in section 11 says 0.3 s per 0.5 m (1.7 m/s). The tables keep section 3.2.
7. Inclusive thresholds need a floating-point tolerance: a 1,000 N pull measures 999.9999999 N on about a third of axes.
8. Hits against the world are judged on speed only; an object bearing down on a player is classified less the weight the player carries (PROPOSED). The crush test sums contact points per body.
