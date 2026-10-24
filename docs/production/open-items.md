# KINETIC — Open Items After the v0.2.2 Reconciliation

_Everything the writers flagged that the bible did not settle, consolidated from the contradiction logs in characters/README.md, objects/README-props.md, objects/README-structures.md, arenas/README.md, systems/README.md, and tech/README.md. Each folder's own log keeps the full history; this is the list that still needs a human decision. Dated 2026-10-24._

## Needs a design decision

| # | Item | Where it lives | Default in the docs today |
|---|---|---|---|
| 1 | **Force-volume cap: 2× or 3× a_ref.** At 2×, every body under 50 kg feels the same gust, so the Updraft and the manifold jet no longer sort a chair from a cylinder by speed. 3× (ceiling 33 kg) restores the sort without changing any Medium figure. | 05 section 6.6; objects/README-props item 55 | 2× |
| 2 | **Reel hold threshold.** The same Haul and Jack buttons reel in Hang context after a hold; the hold length (0.2 s proposed) decides how often a player who wanted a mode switch gets a Reel instead. | systems/camera-and-feel; tech/README decision 5a | 0.2 s |
| 3 | **Team Amber versus warning tape.** Amber #E69F00 is within 16° of hue of slack-cable orange and rated yellow-and-black tape. | arenas/README section 4; objects/environment/structural-tape | Keep Amber; structural-tape fixes hue gap and never-on-the-same-object rule; verify in the Phase 1 readability session |
| 4 | **Office gallery position in the Yard.** The Yard layout proposes the control shack as the gallery; office-gallery.md keeps a separate gallery at the portal leg head. | arenas/the-yard/layout; objects/environment/office-gallery | Layout is the geometry authority, so the shack, unless the level designer prefers the portal head |
| 5 | **Hero versus class.** Still the largest open design question; everything is written class-first. | bible/04 open question 1 | Class-based, Faces as identity |
| 6 | **Lethality tone.** The Stall fiction supports either a survivable or a grim reading. | bible/04 open question 3 | Written to hold either |
| 7 | **Cutter on map cables beyond Hangtown.** | bible/04 open question 4 | Hangtown only, any Cutter |
| 7a | **Hang slip rate and behavior.** bible/05 section 3.2 says 0.5 m per 0.1 s (5 m/s); the tells table in section 11 says 0.3 s per 0.5 m (1.7 m/s). Also, neither rate can be applied as written: paying out at a fixed rate from rest needs the line to push the load, and capping a falling load at that rate needs an infinitely strong line. The solver treats slip as a clutch whose friction is the Hang load, so a load just over the limit creeps and a heavy one falls fast. Design should say whether that reads right, or what the rate should mean. Found while implementing the line solver. | bible/05 sections 3.2 and 11; tech/spool-and-line section 9 | Clutch at the Hang load; the 5 m/s (section 3.2) stays in the data tables, unused by the solver |
| 7b | **The Kick and break-free.** bible/05 6.2 counts the Kick toward break-free, but 6.2 and 6.7 also say which classes can start a bolted object, a stacked container or a Lift counterweight. At full Kick force a Climber starts all three. | bible/05 sections 6.2 and 6.7 | The Kick counts at T for object thresholds; the brace sees the full Kick |
| 7c | **How a victim stops after a cut, and bot survival.** Game-modes 9 says a bot cut 1.0 to 1.6 s after the seat puts bot survival near the 50% launch target (05 section 12). Every bot cut lands before the edge on the bench (a Haul from 15 m reaches it at 1.85 s at the soonest). But 05 does not say how a cut victim stops, and they are moving fast: a Climber Hauled by a Breaker and cut at 1.6 s is 11 m along at 15.5 m/s and needs about 30 m/s² to stop short. If a cut victim slows at their own foot-hold, bot survival is 73% in 05 4.3's case (Breaker on Rigger), ranges from 0% to 100% by pairing, and averages about 85%. Design should say what stops a cut victim; the bot window follows from that. Found while implementing the bots. | systems/game-modes-and-match-flow 9 and 9.1; bible/05 sections 4.3, 5.3 and 12 | 1.0 to 1.6 s as written; no stopping rule |
| 7d | **Which tutorial crew.** characters/npcs/bot-crew.md has three Breakers that cut at 1.4 s and never move rated loads. onboarding-script.md step 9 has one Breaker, one Rigger and one Scrapper cutting at 1.0 to 1.6 s, a bot Breaker Hauling a hull section (a rated load), and a Guild bot Climber on the player's crew. | characters/npcs/bot-crew.md; systems/onboarding-script.md step 9; game-modes 9.1 | The code follows bot-crew.md; the Guild Climber uses the backfill profile |

## Needs an engine answer before art is committed

| # | Item | Where |
|---|---|---|
| 8 | CharacterMovementComponent versus Mover for the grounded body, with the flight-body handoff. | tech/player-physics; tech/README decision table |
| 9 | Chaos geometry-collection damage units versus the N·s thresholds in 05 section 6.5 (a ×100 mapping is a hypothesis pending a drop-test calibration). | tech/chaos-destruction |
| 10 | Networked physics prediction maturity in UE 5.8 for the Line on a player over 80 ms; the Phase 0 week-5 test decides. | tech/architecture-overview; production/roadmap |
| 11 | Dedicated-server fleet provider and regions. | tech/networking |

## Housekeeping

| # | Item | Action |
|---|---|---|
| 12 | scrapped-body.md is Medium tier but filed under objects/heavy; crane-counterweight.md and lift-counterweight.md are rated attachments filed under objects/heavy. | Move when the arena manifests' paths are next touched, or leave and let the README index carry the tier. |
| 13 | Structures docs for Hangtown (walkway heights, Well plane, market ring) were written before arenas/hangtown/layout.md was final and were aligned only in passing. | One alignment pass against the layout before Hangtown greybox. |
| 14 | The bible's prose files (01 to 04) still use the em dash in headings and some sentences; every downstream document avoids it by writing rule. | Cosmetic. Normalize if a style rule is adopted. |
| 15 | 05 section 6.1 lists hull sections on the Yard floor as rated; the Yard splits 2,800 kg Heavy plates from 4,500 kg rated uncut sections. | Accepted as written in the arena; 05 wording could name both. |

## Settled this cycle, for the record

Tension rescale to newtons; Jack as the Kick plus push; Kick interlock on Catch Plates; Reel within Hang; Hang never tests break-free; inclusive thresholds; rated loads as a 0.6 s commit trigger with an authored timeline, committed by a Breaker, an overspinning Doubler, or a crew sum; Heavy objects movable only on rollers or skids; Avalanche stacks on roller pallets; hanging houses on rated hangers with gravity-only swings; kinematics sever lines (Hooks exempt); force volumes on free bodies only, mass-scaled and capped; Guillotine by recall or cut; team tints; Breaker foot-hold 5; character ages and Collar count; arena geometry (Ballast forward 72 m, Anchor Rim 240 m and Crown 60 m above, Yard Rails 36 m, Pulse every 75 s as shove then inhale); Ballast Haul on the Spine ratchet rail. The full sequence is in bible/CHANGELOG.md.
