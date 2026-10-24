# KINETIC — Playtest Plan

_What is tested, when, how, and what number means pass. Tuning targets come from bible/05 section 12._

## Cadence

- Phase 0: twice weekly from week 3. Internal, 6 players.
- Phase 1: daily from week 10 on the Yard greybox. Internal, 6 players. External 20-player test at week 20.
- Phase 2: daily per arena in production. External monthly.
- Phase 3: closed beta weekends.

## Instrumentation (built in Phase 0, never removed)

Every match logs:

- Every Dog seat: shooter, target type, plate if a player, class of both.
- Every mode engagement: mode, tension, duration, mass on each end, outcome (released, cut, snapped, completed).
- Every cut: who, whose line, time from seat to cut, plate, camera turn angle required.
- Every Stall: cause (object impact with object id and impulse, world impact with speed, crush, blast, ring-out), killer, named rig if detected, the 4 s of victim camera.
- Every stagger and flip.
- Per-class floor time, airborne time, hoisted time.
- Tonnage breakdown per player.
- Server frame time, physics time, dynamic body count, active line count, per second.

## The post-death question

After every death in a moderated session, the moderator asks "what killed you?" and logs the answer verbatim with the actual logged cause. A mismatch is a readability bug, filed against the tell table in bible/05 section 11.

## Pass criteria by phase

### Phase 0 exit (week 6)

| Metric | Pass |
|---|---|
| Panic-window survival at 0 ms, median player, back plate | 40–60% |
| Same at 80 ms | within 10 points of 0 ms |
| Client position divergence, Heavy-vs-Light pull | under 10 cm at line end |
| "Don't know what killed me" | 0 in final session |
| Objects over 60 m/s | 0 per match |

### Phase 1 exit (week 20, external)

| Metric | Pass |
|---|---|
| Apprenticeship completion | over 90% |
| Apprenticeship duration | under 15 min median |
| First-match panic-window survival | 35–65% |
| Elimination mix | within 10 points of 40/25/20/10/5 (object/ring-out/world/blast/crush) |
| Scrap round length | 60–120 s median |
| Two-step rig frequency (Guillotine, Anchor Drop, Caboose) | about 1 per round per crew |
| Class pick spread | no class under 10% or over 35% |
| Class win rate | every class 45–55% |
| "Would play again" | over 70% |

### Per-arena acceptance (Phase 2)

- Every play in the arena's signature-plays.md performed on video.
- Ring-out share under 35% (bible/05 section 12), or floor is added.
- Every tell in events-and-moving-parts.md confirmed at its lead time with a stopwatch.
- Line readability confirmed in every zone at 4 kN and 10 kN under the arena's lighting.
- Dynamic body count never exceeds the prop manifest's cap.

## Session formats

- **Mechanics session** (Phase 0): scripted scenarios. "Breaker Hauls Climber from behind at 15 m from a drop" repeated 20 times per participant, survival logged.
- **Free play** (all phases): 3v3 Scrap, best of 5, then interviews.
- **Signature-play verification** (per arena): QA attempts every documented play; films success; logs required attempts.
- **Readability session**: participants watch 20 logged death replays and name the cause; under 90% correct means a tell is missing.
- **New-player session** (Phase 1 onward): Apprenticeship then three matches, no coaching, think-aloud.

## Interview script (after free play)

1. What killed you most often? (compare to log)
2. Which class felt strongest? Weakest? Why?
3. Did you ever die without knowing why? When?
4. Did you ever cut yourself free? How did it feel?
5. Name one trap you set that worked and one that didn't.
6. What did you want to do that the game wouldn't let you?
7. Would you play again? What would you tell a friend?
