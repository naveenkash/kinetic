# KINETIC

_Multiplayer physics-trap arena for Unreal Engine 5. No guns. One cable, one knife, and everything the building was already going to do._

Sixty-one years ago the only space elevator ever built snapped and fell across the equator. The people who take the wreck apart call themselves riggers. When two crews claim the same piece of it, they settle it the way the trade taught them: with mass, tension, and gravity. Four billion people watch.

## What is here

A complete pre-production design package: about 380,000 words across 165 documents, one file per object, character, arena, and system, written so that each can be handed to the person building it. The full list with word counts is in [docs/production/asset-master-list.md](docs/production/asset-master-list.md).

| Folder | What it holds | Start with |
|---|---|---|
| [docs/bible/](docs/bible/) | Canon. World, factions, arenas, progression, and the core mechanics spec with every number. Everything else defers to it. | [05-core-mechanics.md](docs/bible/05-core-mechanics.md), then [01](docs/bible/01-world-and-lore.md) to [04](docs/bible/04-progression.md) |
| [docs/characters/](docs/characters/) | Five harness classes, ten Faces, equipment (Spool, Line, Dogs, Cutter, Catch Plates), NPCs. Visual sheets, animation lists, voice, budgets. | [characters/README.md](docs/characters/README.md) |
| [docs/objects/](docs/objects/) | 73 objects in eight categories: light and medium props, volatiles, heavy loads, structures, rated loads, kinematics, environment. Appearance, physics, interactions, states, VFX hooks, Chaos setup, budgets. | [README-props.md](docs/objects/README-props.md), [README-structures.md](docs/objects/README-structures.md) |
| [docs/arenas/](docs/arenas/) | Four arenas, six documents each: overview, zone-by-zone layout with plans, prop manifest, events and moving parts, lighting and mood, signature plays. | [arenas/README.md](docs/arenas/README.md) |
| [docs/systems/](docs/systems/) | HUD and front end, audio, VFX, animation, camera and controls, game modes, onboarding script. | [systems/README.md](docs/systems/README.md) |
| [docs/tech/](docs/tech/) | UE5 architecture with verified citations: Spool and Line implementation, player physics, Chaos destruction, kinematics, networking, budgets, project structure, the backend, and the full tech stack plan. | [tech/README.md](docs/tech/README.md) |
| [game/](game/) | The game project. Today: the engine-free core of `KineticCore` (line solver, mass and impact rules, Cutter, data tables, the bible-table CI check, panic-window bench) and the match layer (event schema, credit, the Scrap match flow, the match report, the determinism recorder), C++20, tested on a Mac without Unreal and in CI. | [game/README.md](game/README.md) |
| [backend/](backend/) | Phase 1 backend code on Supabase: schema, Edge Functions, matchmaker worker, fake game server, tests. Runs on a Mac with Docker. | [backend/README.md](backend/README.md) |
| [docs/production/](docs/production/) | Roadmap, team, art pipeline, risks, playtest plan, master asset list. | [roadmap-and-milestones.md](docs/production/roadmap-and-milestones.md) |

## The three sentences that hold it together

- **Every weapon is a tool.** The Smart Cable is a rigger's Spool. The Thermal Knife is the safety Cutter every rigger carries. Nothing in the arena was brought there to hurt anyone.
- **Every arena is a demolition site.** The hazards are the job. The map is being taken apart, and the players take it apart, season by season.
- **Mass is destiny.** Five harnesses, five masses. What a line does to you depends on what you weigh and what you are tied to, and everyone can see both.

## How canon works

1. [docs/bible/05-core-mechanics.md](docs/bible/05-core-mechanics.md) is the source of every number. A document that disagrees with it is wrong until 05 changes.
2. Changes to 05 are logged in [docs/bible/CHANGELOG.md](docs/bible/CHANGELOG.md) with the date. Downstream documents carry a version header saying which bible version they were last reconciled to.
3. Anything a writer had to invent is marked **PROPOSED** inline. Everything else is canon. Each folder's README keeps a contradictions log with the status of each item.
4. Physics is stated in SI units (meters, seconds, kilograms, newtons). Unreal units are centimeters.

## Reading order by role

- **Designer:** bible 05, then 02 and 03, then arenas/README, then systems/game-modes-and-match-flow.
- **Engineer:** bible 05, then tech/architecture-overview, tech/spool-and-line-implementation, tech/player-physics.
- **Environment artist:** bible 03, arenas/README, your arena's six files, then every object the manifest references, then production/art-pipeline.
- **Character artist:** characters/README, the five harness docs, then the Faces.
- **Audio, VFX, UI:** bible 05 sections 3, 4, 7, 11, then your systems document.
- **Producer:** production/roadmap-and-milestones, team-and-roles, risks, playtest-plan.

## Status

World bible v0.2.2, 2026-10-24. Pre-production. No engine work has started. The first six weeks of the roadmap exist to retire the two risks that decide whether the game works at all: a fair panic window over real latency, and identical heavy-versus-light pulls on both clients.
