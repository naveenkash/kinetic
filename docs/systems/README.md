# KINETIC: System Design Specifications

_Index v0.2.2, revised to 05-core-mechanics v0.2 and the v0.2.1 and v0.2.2 rulings (CHANGELOG 2026-10-24). These documents implement the world bible in `docs/bible/`. Bible 05 (Core Mechanics) is canonical for every number. Where a system doc invents something, it is marked **PROPOSED**. Where the bible is ambiguous or contradicts itself, the doc picks one reading and the conflict is listed below._

## Files

| File | One line |
|---|---|
| [ui-hud.md](ui-hud.md) | The in-match HUD as LayCast's broadcast package: every element, position, state, the line-on-body edge indicator, Scrap Cards, Rig Calls, team tints, accessibility. |
| [menus-and-frontend.md](menus-and-frontend.md) | The Ledger, Harness and Face select, Spool loadout, Allegiance and Contracts, Claims map, Manifest, Office Surplus shop, lobby, post-match Tonnage breakdown, with wireframes and copy. |
| [audio.md](audio.md) | The line singing system (pitch map in newtons), Dogs, Cutter, Stall, Hooks, Marshal callouts, klaxons, groans, footsteps, beds, music stance, mix priorities, MetaSounds, loudness, broadcast layer. |
| [vfx.md](vfx.md) | Niagara systems with triggers, parameters, lifetimes, tints, LOD and budgets, under the "lines are the light" rule. |
| [animation.md](animation.md) | Locomotion per class, Spool and Cutter frame timings, drag skid, ragdoll-with-pose bone split, hoisted, flipped, stagger, the three cut-self-free turns, NPC sets, rigging and retarget plan. |
| [camera-and-feel.md](camera-and-feel.md) | First and third person rules, FOV, camera control per line state, the panic-window turn on mouse and stick, shake table, victim replay, spectator cameras, sickness mitigations, default bindings. |
| [game-modes-and-match-flow.md](game-modes-and-match-flow.md) | Scrap, Claim, Haul, Free Scrap in full, sudden-death shrink per arena, matchmaking by Grade, crew formation, Rated vs casual, Claim-season standing math, forfeit and leaver rules, bot backfill. |
| [onboarding-script.md](onboarding-script.md) | The Apprenticeship as a shootable script with Teodora Quispe and Baskoro Hanif, the deliberate failed panic window, the Cutter ceremony, the first bot match, and the 20 Hand tips. |

## Cross-reference: bible 05 sections to system docs

| 05 section | Topic | Implemented in |
|---|---|---|
| 1 | Design pillars | All (readability contract in ui-hud 1, audio 1, vfx 1) |
| 2.1 | Body, camera, FOV | camera-and-feel 1; animation 12 |
| 2.2 | Harness classes (speeds, jump, mantle, air control) | animation 2; menus-and-frontend 4 |
| 2.3 | Movement verbs (slide, perch, brace, ledge grab, airborne firing) | animation 2, 6, 9; camera-and-feel 2, 8 |
| 2.4 | Falling | camera-and-feel 2, 7; vfx 2.3 |
| 3.1 | Firing Dogs, self-dog, slack window, mode selection, release, recall | ui-hud 4.1, 4.2; animation 3; camera-and-feel 8; audio 3 |
| 3.2 | Haul, Jack (the Kick and its interlock), Hang, Reel | ui-hud 4.2, 4.3; animation 3; audio 2, 2.7, 2.8; vfx 2.1; camera-and-feel 2, 8; onboarding beats 3, 5 |
| 3.3 | Force resolution | menus-and-frontend 4 (display); game-modes 4 (spool) |
| 3.4 | Class tuning (length, tension, Hang load, Dog speed, switch, recall, set lines) | audio 2.1; animation 3; ui-hud 4.2, 4.5; menus-and-frontend 4, 6 |
| 3.5 | Lines as objects, stagger and flip on Hang lines, kinematics sever lines | ui-hud 4.1, 4.6, 4.8; animation 7; audio 2.6; vfx 2.1 |
| 3.6 | Set lines, early recall by the owner (the Guillotine drop) | ui-hud 4.5; audio 2.4; vfx 2.1 |
| 3.7 | Overspin | ui-hud 4.4; audio 2.3; vfx 2.1; animation 3 |
| 3.8 | Two active lines | ui-hud 4.1, 4.2; audio 2.2; camera-and-feel 8 |
| 4.1 | Cutter draw and cut timings | animation 4; ui-hud 4.7; audio 4 |
| 4.2 | Cutting a line, the 70° cone, the edge indicator | ui-hud 4.6; animation 8; camera-and-feel 2, 3 |
| 4.3 | The panic window by the numbers | camera-and-feel 3; onboarding-script beats 6, 7 |
| 4.4 | Cutter as melee | animation 4; audio 4; vfx 2.2 |
| 4.5 | Cutting map cables | ui-hud 4.7; animation 4; audio 4; onboarding tip 19 |
| 5.1 | Grounded vs airborne, drag | animation 5, 6; camera-and-feel 2; vfx 2.3 |
| 5.2 | Hoisted | ui-hud 4.8; animation 7; camera-and-feel 2 |
| 5.3 | Foot-hold | animation 5 |
| 5.4 | Brace | animation 9; vfx 2.3; camera-and-feel 8 |
| 5.5 | Catch Plates | ui-hud 4.6; animation 8; audio 3.3 |
| 6.1 to 6.3, 6.1a, 6.2a | Mass tiers, Heavy objects on rollers and skids, break-free, rated loads, multiple lines on one object | vfx 2.4; audio 8.3; menus-and-frontend 13; game-modes 3, 4; ui-hud 4.11, 4.12 |
| 6.4 | Volatiles | vfx 2.5; audio 12 |
| 6.5 | Destructibles | vfx 2.4 |
| 6.6 | Force volumes | vfx 2.5; audio 8.1 |
| 6.7 | Kinematics | vfx 2.6; audio 8.1 |
| 7.1 to 7.3 | Thresholds, mass on both ends, surfaces | ui-hud 4.16 (card readouts); camera-and-feel 4 |
| 7.4 | The Stall sequence | ui-hud 4.16; audio 5; vfx 2.3; animation 7 |
| 7.5 | Ring-out | ui-hud 4.16; audio 6; vfx 2.3; camera-and-feel 6 |
| 8 | Interaction matrix | onboarding-script (taught by task) |
| 9.1 | Formats | game-modes 1 to 5 |
| 9.2 | Round flow, Drop, siren, victim replay | game-modes 2; ui-hud 4.13; camera-and-feel 5; animation 9 |
| 9.3 | Spawns | game-modes 1 |
| 9.4 | Claim marker | ui-hud 4.11; game-modes 3 |
| 10 | Named rigs | ui-hud 4.15 (kill feed); menus-and-frontend 3 (Rope Marks); onboarding beat 5 |
| 11 | Tells | audio 12 (mix priorities); vfx 1, 2 (systems marked T); ui-hud 4.6, 4.13 |
| 12 | Tuning intent | menus-and-frontend 3, 13; game-modes 2, 9 |

Other bible sources: 01 (LayCast as HUD, tone, the Office) in ui-hud and menus-and-frontend; 02 (Faces, kits, named rigs) in menus-and-frontend and onboarding-script; 03 (arena events, kinematics, rule 8) in vfx, audio, game-modes; 04 (progression, cosmetics, onboarding) in menus-and-frontend, game-modes 7, onboarding-script.

## Contradictions and ambiguities found in the bible

The first pass of these documents (v0.1) found thirteen contradictions or ambiguities in the bible. Items 1 through 8 and 11 through 13 were resolved in 05-core-mechanics v0.2 on 2026-10-24 (see `docs/bible/CHANGELOG.md`): pull tension rescaled to newtons (400 / 400 / 600 / 800 / 1,000 N, overspin 1,200 N) with new foot-holds (3 / 4 / 5 / 5 / 5 m/s² after the v0.2.1 ruling that set the Breaker's unbraced foot-hold to 5; braced 9 / 20) and a Jack launch speed per class; slide 0.7 m and crouch 1.0 m; per-class mode switch and recall; mid-match events at 1:15 Scrap, 4:00 Claim, 2:00 per Haul half; sudden death as an arena-specific shrink defined per arena; third person casual-only at 90° FOV; stagger keeps look and Cutter live; airborne firing for all with the Climber keeping momentum; Cutter heat time; no Hook retrieval from the Anchor's Well; and the Apprenticeship ending in Claim mode. The v0.2.1 follow-up rulings of the same day are also applied: the Kick (Jack's capacitor discharge, with its own sound, housing flare, and HUD state), Heavy objects moving only on rollers or skids (05 section 6.1a; spool on rollers, marker on runners, water tanks fixed), house hangers rated 1,000 N with swings from gravity only, canonical team tints in 05 section 11a, Teodora Quispe at 72, 3v3 at launch, scrapped bodies at class mass, and Ballast's Haul route up the Spine on a counterbalanced ratchet rail. The v0.2.2 play-preserving rulings are applied as well: Reel (the Hang winch at 1 m/s; the Gallows hoists by Reel), the Kick interlock on any player-target Jack, the Guillotine dropping by recalling or cutting your own set line with early recall for owners, kinematics severing lines they pass through, the Free Scrap event at 3:00, and the Anchor Well's ring-out tell as the Stump's hum dropping an octave. The system docs are at v0.2.2.

Two notes remain, neither a contradiction:

9. **Rigger Hang stagger (02 vs 05).** 02 says anything that runs into a Rigger's Hang line "staggers longer"; 05 section 3.5 gives the number, 1.0 s versus 0.6 s. Consistent; 02 could quote it.
10. **Reserve fiction vs Scrap (01 vs 05).** 01 describes reserves generally; 05 gives Scrap no reserves. Lobby copy and Marshal callouts must not promise a reserve in Scrap. Handled in game-modes-and-match-flow.md and audio.md.
