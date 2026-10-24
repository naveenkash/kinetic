# KINETIC browser client (prototype)

A playable browser version of KINETIC, built before Unreal is set up. The game's rules are KineticCore itself, compiled to WebAssembly: the line solver (Haul, Jack, the Kick), the Hang solver, footing and brace, break-free, the Impact Monitor, the Cutter's reach-and-cone test, the bots' decisions and the Scrap match flow. Around them, `sim/` supplies a grey-box world, doing the job the Unreal integration layer will do later. `client/` draws it with three.js.

You play the Yard (grey-box) as one of the five harnesses, with two bot crewmates, against a bot crew of three, best of five.

It is a prototype for feel and for the open design questions (docs/production/open-items.md 7c, for instance). It is not the shipping game. The one-server-for-every-client option in [docs/tech/standalone-server-option.md](../docs/tech/standalone-server-option.md) would make this the first client of that server.

## Run it

You need Emscripten for the WebAssembly (`brew install emscripten`) and Node 22.

```bash
make -C web dev
```

Then open http://localhost:5173. `make -C web client` writes a static build to `web/client/dist`, which you can host anywhere.

Two links skip the menu:
- `?class=breaker` starts as a Breaker (any harness name works).
- `?demo` lets a bot play your slot while the camera follows it; add `&seed=3` to replay a match.

## Controls

| Input | Action |
|---|---|
| Mouse | Look (click the view to capture the mouse, Esc to release it) |
| W A S D, Shift, Space | Move, sprint, jump |
| Left click | Dog A, then Dog B; a third shot releases and fires a new A |
| E | Next Dog into your own chest plate (Slingshot, Pendulum) |
| 1 2 3 | Mode: Haul, Jack, Hang, before or while the line is live |
| R | Release: a Hang line becomes a set line |
| Right mouse (hold) | Draw the Cutter; left click swings once it is warm |
| C (hold) | Brace (Rigger, Breaker) |
| H | Controls |

A Dog in your plate shows as a red arrow at the edge of the screen. Turn until the line is in front of you, draw, and swing.

## What is here

| Path | What it is |
|---|---|
| `sim/World.h` | The world: grey-box solids, box bodies, walking and air control, Dogs in flight and their seats, lines (Haul and Jack through `StepLine`, Hang and Set through `SolveHangLines`), the Cutter, collisions judged by the Impact Monitor, ring-out |
| `sim/Arena.cpp` | The Yard in grey-box: two yards split by a trench with two bridges, containers and a doorway, pylons at the trench and past the edges, crates |
| `sim/Brains.h` | Bot controllers: the cut reflex (`FBotDefense`) and rig choice (`ChooseRig`) from KineticCore's `Bots.h`, turned into button presses |
| `sim/Session.h` | One match: the world, the bots and `FScrapMatch` (rounds, the clock, best of 5, side swaps), and the JSON the client draws |
| `sim/Bindings.cpp` | The Emscripten entry point |
| `sim/Tests/WebTest.cpp` | Native tests (`make -C web test`) |
| `client/` | Vite, TypeScript and three.js: the view, the HUD, input, and `smoke.mjs`, which loads the built WebAssembly in Node and plays a minute |

## Prototype simplifications (PROPOSED, prototype only)

- **Bodies:** boxes that never rotate. Players are 0.8 × 0.8 × 1.8 m. Collisions push bodies apart by inverse mass.
- **Impacts:** only a body in flight is judged, meaning a player off their feet or a crate moving faster than 2 m/s. A walking player's bumps are not hits, as the character movement in the engine will treat them.
- **Lines:** a line acts at the body's centre and is drawn from, and cut at, the Catch Plates. A Dog that reaches the body anywhere seats in the plate nearest the impact (05 5.5).
- **Line on a grounded player:** a Haul or Jack overrides their movement (05 5.1). When it takes hold the feet plant, keeping only the motion along the pull. Air control needs input, only adds along it up to the run speed, and is off while hoisted or under a line.
- **Hang and set lines are solid to players (05 3.5).** Walking into one stops you; over 3 m/s it staggers you, for the line owner's class time; over 6 m/s it flips you over it, unless a solid is in the way, which makes it a stagger. A player already staggered is blocked but not hit again. A line within 0.3 m of the floor is stepped over. They are not solid to crates.
- **Placed crates** (a stack) stay put until a line breaks them free or the crate under them goes. Loose crates slide and stop by friction.
- **Not modelled:**
  - overspin and the Doubler's second line;
  - crouching or sliding under a line;
  - the Cutter's melee unseat (05 4.4);
  - kinematics (Haulers, the Gantry) and the mid-match event;
  - the dark tick pattern on enemy lines (05 11a): team is carried by colour only.
- **Bots:** they walk toward the nearest enemy, cross the trench by a bridge, and never step off a drop. They never Slingshot toward one. They aim with a lead, plus up to 0.3 m of error.
- **Pacing:** an all-bot best of five takes 6 to 15 minutes. Rounds often run to the clock, and most Stalls are ring-outs, with some wall impacts. Tuning the pace is open.
- **The idle rules** (game-modes 8) apply to you: any input counts, including looking around.
