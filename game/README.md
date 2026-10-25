# KINETIC game project

The Unreal 5.8 project (`Kinetic.uproject`, laid out per [docs/tech/project-structure.md](../docs/tech/project-structure.md)). At its centre is `KineticCore`, the rules of the game in standard C++20 with no engine types. The same sources build three ways:
- as an Unreal module;
- headless, tested with any C++20 compiler;
- for the browser client ([web/](../web)).

## The Unreal project

| Path | What it is |
|---|---|
| `Kinetic.uproject`, `Source/*.Target.cs` | The project, with the editor and game targets. The dedicated server target needs a source build of the engine, because the Launcher build has no server ([standalone-server-option.md](../docs/tech/standalone-server-option.md)) |
| `Source/KineticCore/KineticCore.Build.cs` | `KineticCore` as a module. It uses no PCH and no unity build, has exceptions on (a bad table throws `FTableError`), and force-includes `Private/Unreal/KineticExports.h`, which exports the library's symbols to other modules without per-function `_API` macros. `Private/Unreal/` is the only engine code in the module, and only Unreal builds it |
| `Source/KineticGameplay/` | The primary game module. It holds:<br>- `UKineticRulesSubsystem`, which loads the rules tables once per engine from `Content/Core/Tables` (a packaged build stages them as loose files, `Config/DefaultGame.ini`).<br>- `AKineticCharacter`, the harness character. A 1.9 m first-person capsule moving by its class's row of `DT_HarnessClass`: run, sprint, jump height, air control, crouch speed and mass. It mantles to the class's mantle height and climbs `AKineticLadder` volumes at the class's ladder speed.<br>- `UKineticSpoolWorld`, every Spool line in the level, solved once per step by KineticCore's rules in the browser client's order: Dogs sweep the level and seat (a Catch Plate, a prop, the world); lines engage, switch mode after the class delay and time out; a trial solve decides break-free; Haul and Jack forces go to props as impulses at the seat and to players by footing (braced, skid, airborne, hoisted); Hang and Set lines are solved together over every body and go back to Chaos and character movement as velocities. The Cutter swings through it. The project runs at a fixed 60 Hz, the rules' step.<br>- Hits: the character records every blocking hit while the velocity that made it is intact (landings, `UKineticMovementComponent`'s impacts, props striking it with Chaos's impulse), and each step the Spool world runs KineticCore's Impact Monitor on them. A stagger or flip stuns; a Stall takes the player out, their lines go, and in this mode they are back at Spawn A after 3 s. A fall below -6 m (the Well, or off the edge) is a ring-out. Hang and Set lines are solid: walking into one stops you, and at 3 m/s it staggers you, at 6 m/s it flips you over. Surfaces are actor tags (`Surface.Scrap`, `Surface.Wood`).<br>- `AKineticProp`, a generated mesh simulated by Chaos with its spec's mass and placement: loose props simulate, racked and stacked ones wait for an active pull that meets their break-free threshold. `AKineticLineRenderer` draws the lines by mode and the Dogs in flight.<br>- `AKineticGameMode`, which spawns the class `?Class=` names at Spawn A, and `AKineticHUD`.<br>- `Private/Tests/`, the `Kinetic.*` automation tests. `Kinetic.Core.*` runs KineticCore inside the engine; `Kinetic.Yard.Play` plays the Yard in PIE and checks the spawn, the run speed and the jump height. `Kinetic.Yard.Spool` plays the Spool on West Mesa: a Haul brings a chair in until the Dogs meet, a Hang holds the player at its length, the released Hang is a set line the Cutter cuts, and a Jack Kicks the player up at the class's launch speed. `Kinetic.Yard.Hits` checks a 5.9 m/s landing staggers, a 20 m fall Stalls against the world, the player is back at Spawn A after the respawn, and the Well is a ring-out |
| `Tools/unreal/build_yard_greybox.py` | Builds `L_YRD_Greybox`, the level the project opens on, from [the Yard's layout](../docs/arenas/the-yard/layout.md) and prop manifest. It has:<br>- the floor islands, trenches and pit;<br>- the mesas with their ledges and rungs;<br>- the grate and plank crossings, the press, the portals, rail walks, gantry and masts;<br>- every ladder as a climbable volume;<br>- the generated props where the manifest puts them, and grey placeholders for objects not built yet;<br>- the spawns.<br>Where the layout's numbers collide, it resolves them and logs it ([layout.md, Grey-box findings](../docs/arenas/the-yard/layout.md#grey-box-findings-proposed)) |
| `Tools/unreal/import_props.py` | Imports the generated props from `art/out` ([art/README.md](../art/README.md)). It builds the master materials (`M_KineticProp`, and `M_KineticProp_Masked` for weld-mesh cards), the static meshes with their sockets and UCX collision, the textures (the normal map's green is flipped, since Blender bakes OpenGL) and one material instance per mesh. It fails if a socket is missing or moved, or a mesh has no collision, and writes `Saved/Kinetic/import_report.json` |

Content (`.uasset`, `.umap`) is not committed. Like `art/out`, it is generated by scripts and rebuilt with them.

```bash
make editor
```

```bash
make unreal-test
```

```bash
make import-props
```

```bash
make yard-greybox
```

To play the grey-box, open the project in the editor (it opens on the Yard) and press Play. You walk it as a Rigger, with the browser client's controls: left click fires Dog A then Dog B (or swings the drawn Cutter), E puts the next Dog in your own chest plate, 1 2 3 pick Haul, Jack and Hang, R releases (a Hang becomes a set line), right mouse held draws the Cutter, and C held braces. For another class, run it standalone with the class in the URL:

```bash
"/Users/Shared/Epic Games/UE_5.8/Engine/Binaries/Mac/UnrealEditor.app/Contents/MacOS/UnrealEditor" "$PWD/Kinetic.uproject" "/Game/Arenas/YRD_Yard/L_YRD_Greybox?Class=Breaker" -game -windowed -ResX=1600 -ResY=900
```

`make editor` builds the editor target with Xcode 15.2 to 27.9 (the range in the engine's `Config/Apple/Apple_SDK.json`). `make unreal-test` runs the automation tests headless. `make import-props` imports `art/out`, and `make yard-greybox` then builds the level. `UE_ROOT` overrides the engine location, which defaults to the Launcher's `/Users/Shared/Epic Games/UE_5.8`. The editor's log is in `~/Library/Logs/Unreal Engine/KineticEditor/`.

## The engine-free core

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
| `Tests/KineticCore/` | 218 tests, including section 7.3 of networking.md (the latency validation matrix and the grace window), bible/05 section 4.3 (the panic window), the 71:29 split, the Kick, Overboard and its counter, the Gallows, Anchor Drop, shared Hang loads, chains, 200 random line networks, swing loads (a Bola included), slip and Reel, every 7.1 impulse example, crush, and bit-identical determinism |
| `Tools/report/SampleMatch.cpp`, `Tools/ci/check_report.py` | Play a scripted Scrap match and check its report and raw events as the backend will read them (`api.srv_store_report`) |
| `Tools/determinism/DiffRecordings.cpp` | `build/diff-recordings A B [tolerance_m]`: where two determinism recordings first diverge, for the week 2 and week 5 "record positions, diff" tests |
| `Tools/bench/PanicBench.cpp` | Prints, for every attacker and victim class, when a Haul from 15 m lifts the victim off their feet and when it has dragged them 15 m |

The code follows Unreal's naming (`F` structs, `E` enums, `b` booleans) inside a `Kinetic` namespace and uses no engine types, so it builds as the `KineticCore` module unchanged; `FVec3` maps onto `FVector`. Everything is SI; `Units.h` converts to centimeters at the engine boundary.

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
