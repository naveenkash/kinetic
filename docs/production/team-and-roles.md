# KINETIC — Team and Roles

_Minimum viable team for the roadmap, with the documents each role owns. A solo or very small team should read this as a list of hats, in priority order._

## Core (needed from Phase 0)

| Role | Count | Owns | First deliverable |
|---|---|---|---|
| Gameplay engineer (physics) | 1–2 | tech/spool-and-line-implementation.md, tech/player-physics.md, bible/05 numbers in engine | The Line constraint in three modes, week 2 |
| Network engineer | 1 | tech/networking.md, tech/architecture-overview.md | Two clients, one server, predicted Line, week 5 |
| Game designer | 1 | bible/05, systems/game-modes-and-match-flow.md, every signature-plays.md | The week-6 playtest protocol |
| Level designer | 1 | arenas/*/layout.md, prop-manifest.md, events-and-moving-parts.md | Phase 0 test arena, then the Yard greybox |
| Technical artist | 1 | tech/chaos-destruction.md, systems/vfx.md, the line material and audio parameter bridge | Line emissive vs force curve, week 4 |

## Added for Phase 1

| Role | Count | Owns |
|---|---|---|
| Environment artist | 2 | arenas/*/lighting-and-mood.md, objects/structures, objects/environment, objects/kinematics |
| Prop artist | 1–2 | objects/props-*, objects/volatiles, objects/heavy, objects/rated |
| Character artist | 1 | characters/harnesses, characters/faces, characters/equipment |
| Animator | 1 | systems/animation.md, characters/npcs |
| Sound designer | 1 | systems/audio.md |
| UI engineer/designer | 1 | systems/ui-hud.md, systems/menus-and-frontend.md |
| Writer | 0.5 | bible/01, 02, 04; characters/faces voice sections; systems/onboarding-script.md; evidence objects |
| Producer | 1 | production/* |

## Added for Phase 2–3

| Role | Count | Owns |
|---|---|---|
| Live/backend engineer | 1 | tech/backend-architecture.md: Claims season system, Ledger persistence, Contracts, matchmaking, economy (the Phase 1 slice lands before this hire if needed; see its section 19) |
| QA | 2 | production/playtest-plan.md execution, the signature-play verification videos |
| Additional environment artist | 1 | Parallel arena production |
| Voice director (contract) | | Face and Marshal recording per characters/faces |
| Composer (contract) | | systems/audio.md music stance |

## Decision owners

| Decision | Owner | Document |
|---|---|---|
| Any change to a number in bible/05 | Game designer, with physics engineer sign-off | bible/05 |
| Hero vs class | Game designer and creative lead | bible/04 open question 1 |
| Cutter cuts map cables beyond Hangtown | Game designer | bible/04 open question 4 |
| CMC vs Mover | Physics engineer, per research | tech/player-physics.md |
| Perforce vs Git LFS | Producer and tech lead | tech/project-structure.md |
| Lethality tone | Creative lead | bible/01, bible/04 open question 3 |

## Working agreements

- The bible is canonical. A document that disagrees with bible/05 is wrong until 05 is changed.
- Every arena hazard has a written job in its prop-manifest.md before it is placed.
- Every new system ships with its tells (bible/05 section 11) or it does not ship.
- Playtest twice a week from Phase 0 week 3 onward. Record every death. The question asked after every death is "what killed you," and the answer is logged verbatim.
