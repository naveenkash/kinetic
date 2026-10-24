# KINETIC — Roadmap and Milestones

_Assumes a small-to-mid team (see team-and-roles.md). Durations are working weeks. Every milestone has an exit test that a person can run, not a feeling._

## Phase 0: Risk prototype (weeks 1–6)

Goal: prove the one thing nobody has shipped. A tether that is a spring, a negative spring, and a rigid rod between arbitrary bodies including a networked player, and a panic window that is fair at 80 ms.

Greybox only. One flat test arena with a 20 m drop, three container stacks, one rail hauler on a loop, a propane rack. Two harness classes (Breaker and Climber, the extremes of the mass table in bible/05 section 2.2). Standard Line, Standard Dog, Standard Cutter. No art.

| Week | Build | Exit test |
|---|---|---|
| 1–2 | Dog projectile, Line constraint in all three modes against static world and loose Medium objects. Local only. | Haul a barrel to you. Jack off a wall for a measured launch distance. Hang a barrel from a beam and swing it. |
| 3 | Line on a kinematic character: the drag model (05 section 5.1), foot-hold, transition to airborne ragdoll-with-pose, landing and stagger. | A Breaker Hauls a Climber off a ledge; the Climber's transition from skid to airborne is readable and does not pop. |
| 4 | Cutter: warm-up, cone test, cut, snap impulse. The edge indicator. | A player dragged by a back-plate Dog can turn, find, and cut in under 1.5 s on 50% of attempts at 0 ms latency. |
| 5 | Dedicated server, two clients, prediction for own Line and own body. 80 ms simulated latency. | Same test at 80 ms. Survival rate within 10 points of the 0 ms rate. Heavy-vs-Light pull resolves identically on both clients (record positions, diff). |
| 6 | Volatiles, Stall thresholds, ring-out, Scrap Card, round reset. Three greybox playtests of 3v3. | Players can describe why they died every time. Zero "what killed me" reports in the final session. |

**Go/no-go at week 6.** If the panic window cannot be made fair over 80 ms, the design changes (longer Haul ramp, server-side grace) before any art is commissioned.

## Phase 1: Vertical slice (weeks 7–20)

Goal: one arena at shipping quality, all five harnesses, Scrap mode, and the onboarding, so that a stranger can sit down, learn the game in 15 minutes, and have one match they want to talk about.

Arena: **the Yard**. It has the most conventional layout and the clearest kinematic (Haulers), which makes it the best teacher and the safest first art target. Ballast's tilting hull is a harder engineering problem and goes second.

| Weeks | Build |
|---|---|
| 7–9 | All five harnesses with class tuning from 05 section 3.4. Brace, Perch, Doubler's two lines and overspin, Scrapper recall, Rigger's two set lines, Breaker rated loads on test geometry. |
| 9–12 | The Yard greybox to final layout per arenas/the-yard/layout.md. Haulers, Gantry, Shift Change, Shear Press Well. Full prop manifest in greybox. Daily 3v3 playtests from week 10. |
| 12–16 | Art pass on the Yard: structures, kinematics, environment dressing, lighting per arenas/the-yard/lighting-and-mood.md. Hero props first (shipping container, billet crate, rail hauler, gantry crane, propane cylinder, fuel drum). |
| 12–18 | Harness models (five), first-person arms, shared humanoid rig and the full animation set in systems/animation.md. Two Faces (Teodora Quispe and Rook Adeyemi) at full fidelity. |
| 14–18 | HUD per systems/ui-hud.md. Audio: line singing system, Dog seat tells, Cutter, Marshal callouts, Yard ambience. VFX: line, snap, blasts, fracture. |
| 16–20 | Apprenticeship per systems/onboarding-script.md. Bot crew. Scrap mode complete with replay. Front end minimum: Harness select, Face select, lobby. |

**Exit test.** Twenty external players, no instructions beyond the Apprenticeship. Median first-match panic-window survival between 35% and 65% (05 section 12). At least 70% say they would play again. Elimination mix within 10 points of the 05 section 12 targets.

## Phase 2: Content build (weeks 21–44)

| Weeks | Build |
|---|---|
| 21–28 | Ballast (tilting hull is the engineering risk: schedule 2 weeks of kinematics work before art). Claim mode. Remaining eight Faces at full fidelity. Lines, Dogs, Cutters sidegrades per bible/04. |
| 29–36 | Hangtown (dynamic-house budget per arenas/hangtown/prop-manifest.md; slack-cable cutting; Cable Day). Haul mode. The Ledger, Allegiance and Contracts, Tonnage economy. |
| 37–44 | The Anchor (updraft, Pulse, Ring, Scaffold Collapse). Free Scrap. Rope Marks. Cosmetics pipeline and the Manifest. Claims season system with map-change variants (Hangtown cut district, Ballast stabilized variants). |

Each arena follows the same internal order: greybox → two weeks of daily playtests → art → lighting → audio → signature-play verification (every play in that arena's signature-plays.md must be performable by a tester on video).

## Phase 3: Hardening (weeks 45–56)

- Closed beta on two regions. Server tick, bandwidth, and relevancy tuning per tech/networking.md against real latency distributions.
- Performance to budget on every target in tech/performance-budgets.md.
- Anti-cheat integration and server-side cut validation audit.
- Accessibility pass per systems/ui-hud.md.
- Localization of all Marshal and Face lines (the voice scripts are in characters/faces/).
- Season 1 Claim content and the first evidence object (objects/environment/evidence-objects.md).

**Exit test.** Four arenas, four modes, 1,000-concurrent-player beta weekend with crash-free rate above 99.5% and a median server frame under the physics budget.

## Phase 4: Launch and live (week 57 onward)

- Season cadence: 10 weeks per Claim. Each Claim ships one map variant driven by the previous Claim's result, one evidence object, five faction Contract sets, and one Manifest.
- Backlog arenas (bible/03 Backlog: the Smelt, the Drowned Collar, the Climb) are candidates for Claims 3, 5, and 7.

## What is explicitly not in scope for v1

- Hero abilities beyond harness class tuning (bible/04 open question 1).
- Crew sizes other than three.
- Any form of health, armor, or damage-over-time.
- User-generated arenas. The authored-consequence model makes this a large tooling investment; revisit after launch.
