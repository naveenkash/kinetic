# KINETIC: Game Modes and Match Flow

_System spec v0.2.2, revised to 05-core-mechanics v0.2 and the v0.2.1 and v0.2.2 rulings (CHANGELOG 2026-10-24). Expands bible 05 section 9 (formats, round flow, spawns, Claim marker) and touches 04 (Claims as seasons, Grades, Allegiance) and 01 (the Match). Numbers in 05 are canonical. Anything marked **PROPOSED** is new here._

## 1. Formats (canonical, 05 section 9.1)

| Mode | Teams | Rounds | Round time | Reserve | Win |
|---|---|---|---|---|---|
| Scrap | 3v3 | best of 5 | 2:30 | none | Last crew standing |
| Claim | 3v3 | 1 | 8:00 | 8 s timer, 3 reserves per crew per match | First to 100 Claim points |
| Haul | 3v3 | 2, swap sides | 4:00 each | 8 s, unlimited | Deliver the 2,000 kg spool, or get it furthest |
| Free Scrap | 8 solo | 1 | 6:00 | 10 s, unlimited | Most scraps |

Common to all: two spawn zones per arena at opposite ends of the high or mid tier, 10 m across, never within 25 m of a Well edge (05 section 9.3). Reserve riggers drop at their crew's spawn, or the alternate if an enemy is within 15 m. The mid-match event fires with a 5 s siren at 1:15 remaining in a Scrap round, 4:00 remaining in Claim, 2:00 remaining in each Haul half, and 3:00 remaining in Free Scrap (05 section 9.2). Objects, rated-load consequences, and set lines reset between rounds only.

## 2. Scrap

**Setup.** Two crews of three. No reserves. First to three rounds. Sides swap spawns each round. Loadout changes allowed between rounds, during the reset.

**Round flow, second by second.**

| Clock | Event |
|---|---|
| Pre-round | 5 s reset: objects return to authored positions, set lines cleared, Marshal: "Crews on the claim." |
| 2:30 to 2:24 | Drop, 6 s. Crews enter via their Drop animations. Lines disabled. Cutter disabled. Movement enabled from 2:26 (**PROPOSED**: the last 2 s of the Drop let players orient) |
| 2:24 | Marshal: "Live." Everything enabled |
| 2:24 to 1:20 | Live |
| 1:20 | Siren, 5 s. Banner counts down. Marshal: "Event in five." |
| 1:15 | Mid-match event fires (Shear, Shift Change, Cable Day, Scaffold Collapse). The map is now more dangerous and stays that way for the round |
| 0:10 | Clock turns Caution, ticks each second |
| 0:00 | Time. Marshal: "Time. Count." |
| Round end | Marshal calls it. 4 s replay of the final scrap from the victim's camera. Pips update |

A round ends early the moment one crew has no riggers standing. Average target: 90 s (05 section 12).

**Scoring and ties.**

- Time-out: the crew with more riggers standing wins the round.
- Equal riggers at time-out: **sudden death**. The Marshal calls "Sudden death." and then the arena's own call (Full shear, Lockout, Second cable, Held breath). The arena's shrink event fires (section 2.1). The first Stall ends the round. Each arena's event defines its own duration and what happens if nobody falls.
- Match tie is impossible in best of 5.

### 2.1 Sudden-death arena shrink

05 section 9.1 defines sudden death as an arena-specific shrink event, defined in each arena's events document. This spec does not define those events. It references them and holds the common frame: a 5 s siren, the Marshal's call, the arena closing toward the Well, and the first Stall ending the round.

| Arena | Event | Defined in | Shape |
|---|---|---|---|
| Ballast | Full Shear | docs/arenas/ballast/events-and-moving-parts.md, section 4 | Bay decks shed in 2 m strips from the hull inward, the Hold floods, the Crown's outer strips go, the Breathing's amplitude doubles. Play is forced into the Spine and the Crown within 60 s |
| The Yard | Lockout | docs/arenas/the-yard/events-and-moving-parts.md, section 5 | Floodlights cut, two kinematic blast curtains close from X ±80 to X ±10 at 1.0 m/s over 70 s, Haulers run continuously. At T+90 the curtains close fully and the round is replayed |
| Hangtown | Second Cable | docs/arenas/hangtown/overview.md (events document pending) | The south-west main cable gives and the Low Hang drops away over 20 s |
| The Anchor | Held Breath | docs/arenas/the-anchor/events-and-moving-parts.md | The Breath locks at full, the Pulse fires every 15 s, the Office line contracts from r 60 to r 30 over 30 s. Anything outside the line is scrapped by the Marshal's call |

All four are marked **PROPOSED** in their own documents. Common rules this spec adds (**PROPOSED**): the event begins the instant the Marshal's count is equal; no reserves are granted in sudden death in any mode; set lines placed before time-out persist; Tonnage earned in sudden death counts.

### 2.2 Rulings the implementation needed (PROPOSED)

The engine-free match flow ([game/Source/KineticCore/Public/Kinetic/ScrapMatch.h](../../game/Source/KineticCore/Public/Kinetic/ScrapMatch.h)) had to settle these; design should confirm or change them.

- A round's seconds run from the start of the Drop.
- Two crews down in the same instant, or sudden death running out with nobody fallen, replays the round. It does not count toward the score, and sides still swap.
- In sudden death the round ends the moment one crew has fewer riggers standing, so a withdrawal counts like a fall.
- A withdrawn rigger is not standing, and a crew with nobody standing loses the round at once, even at the start of the Drop.
- In casual, the backfill bot takes over the rigger where it stands, and a leaver who returns within the window takes it straight back. In Rated, a withdrawn leaver who returns rejoins at the next reset; dropping again before then cancels the rejoin. Past the window the seat stays with the bot (casual) or empty (Rated) for the match. Whatever a backfill bot does earns nothing for the leaver.
- Idle time counts only while the rigger can move; 60 s idle is a disconnect, and moving again within the reconnect window is the return.
- A forfeit needs 2 yes votes, or every connected human if fewer than 2 remain; votes clear at each reset and on a disconnect. A forfeit during a round logs that round as forfeited without scoring it.
- Once a crew has its third round the match is decided; nothing during the final replay (a vote, a leaver) changes it.
- Both crews losing their last humans within the same 10 s is a no-contest: no winner, no forfeit. So are three replayed rounds in a row, so a match where nobody falls still ends.

## 3. Claim

**Setup.** One 8:00 round. A 4 m radius salvage marker (Weave billet on a pallet, Heavy tier, movable by lines, 05 section 9.4) at an authored position: Ballast's Bays, the Yard's Floor, Hangtown's market ring, the Anchor's Crown. Three reserves per crew per match, 8 s reserve timer.

**Flow.**

| Clock | Event |
|---|---|
| 8:00 to 7:54 | Drop |
| 7:54 | Live. Marker is neutral |
| Continuous | 1 point per second to a crew with at least one rigger inside the 4 m ring and no enemy inside. Contested (both inside): no points. Empty: no points |
| 4:05 | Siren, 5 s. Marshal: "Event in five." |
| 4:00 | Mid-match event fires (05 section 9.2: 4:00 remaining in Claim) |
| First to 100 | Marshal: "Claim's settled." Match ends immediately |
| 0:00 | Time. Higher points wins |

**Marker rules.** Moving the marker is legal. It sits on polished runners (friction 0.05, 05 section 6.1a): a Rigger, Doubler, or Breaker moves it, slowly. A Climber or Scrapper alone cannot start it; two of them pulling together exceed the threshold because pull forces add (05 section 6.2a). A crew that Hauls it toward their spawn is making a choice: it still has to be held. If the marker enters a Well it resets to its authored position after 2 s. A marker in motion still scores. A marker hanging in the air on a Hang line still scores for whoever is inside the 4 m sphere around it (**PROPOSED**: the ring is a sphere when the marker is off the ground).

**Reserves.** A scrapped rigger with a reserve available drops after 8 s. A crew with all three riggers scrapped and no reserves left loses immediately. Reserves do not refresh.

**Ties.** Equal points at 0:00: sudden death. The ring shrinks to 2 m over 5 s (Marshal: "Marker's closing."). The first full uncontested second wins. Up to 60 s; still tied, the crew with more time holding the marker during the match wins (**PROPOSED**).

## 4. Haul

**Setup.** Two rounds of 4:00, sides swapped. A 2,000 kg Weave spool (Heavy tier) at the center of an authored route. Each crew has a truck at its end. Arenas: the Yard (Floor route between trenches), Ballast (the Spine, on a counterbalanced ratchet rail: the spool rides the rail up the shaft and the rail's ratchet holds the height gained, so a crew that stops pulling does not lose it, **PROPOSED** detail on the canonical route). Hangtown and the Anchor do not host Haul (03). Reserves unlimited, 8 s.

**Flow.** Drop and Live as above. The mid-match event fires at 2:00 remaining in each half, siren at 2:05 (05 section 9.2). The spool moves only under lines. It sits on rollers (friction 0.02, 05 section 6.1a) and needs about 400 N to start, so any class can move it alone, at 0.2 to 0.5 m/s² per line. Pull forces from three lines add as vectors (05 section 6.2a), so a crew pulling together moves it three times faster than one rigger. Hanging it from the Gantry needs 19.6 kN, two Riggers' Hang loads together. Haul is a crew mode because a crew is faster, not because one rigger cannot start it. It is delivered when its center enters the truck's volume. Delivery ends the round and records the time. At 0:00 the spool's progress is recorded as signed distance along the route from center, positive toward the delivering crew's truck.

**Scoring.** After two rounds: a crew that delivered beats one that did not. Both delivered: faster delivery time. Neither: greater progress distance. Equal: a 1:00 sudden-death round with the spool at center and the Haulers (Yard) on Shift Change schedule (**PROPOSED**).

**Spool rules.** The spool is physical. It can fall into a trench, be knocked off the Spine's ratchet rail, ride a Hauler, or be hung from the Gantry. If it enters a Well it resets to the last route checkpoint it passed (checkpoints every 20 m, **PROPOSED**). Players can be scrapped by it (a 2,000 kg object needs 0.6 m/s for 1,200 N·s).

## 5. Free Scrap

Eight solo riggers, 6:00, 10 s reserve, unlimited. The mid-match event fires at 3:00 remaining, siren at 3:05 (05 section 9.2). Most scraps wins. Tie: fewest times scrapped, then sudden death until the next scrap. Tints: the player is Amber, everyone else Sky (relative tints only; absolute tints are disabled because eight tints would not be colorblind-safe). Scoreboard shows the top three. Free Scrap is casual only and the place to learn a new harness.

## 6. Matchmaking

### 6.1 Pools by Grade

Grade (04) gates sidegrades and cosmetics, never stats, so Grade pools are about experience and manners, not power.

| Pool | Grades | Notes |
|---|---|---|
| Apprentice | 1 (Greenhand) | Loaner Cutter. Bots may fill. Scrap and Free Scrap only. Leaves this pool at the Cutter ceremony |
| Hand | 2 to 3 | All casual modes |
| Journeyman | 4 to 6 | All casual modes |
| Rated | 2 and up, by Office Rating | See 6.3 |

Casual pools widen to the adjacent pool after 90 s in queue, and to all pools after 180 s.

### 6.2 Crew formation

- **3-stack**: queue as a crew. Matched against other stacks first, then mixed crews after 60 s.
- **Duo plus one**: the third slot is filled from solo queue.
- **Solo queue**: three solos. The lobby shows a **role suggestion** (Lead, Hand, Spotter) per player based on their Ledger: the player with the most set-line Spotter credit is suggested Spotter; the most scraps, Lead; the remainder, Hand. Titles are traditional, not enforced (01). Harness suggestions follow: a solo crew of three Climbers gets an Office note, not a block.

### 6.3 Rated versus casual

| | Casual | Rated |
|---|---|---|
| Modes | All four | Scrap, Claim |
| Entry | Any Grade | Grade 2 and up (post Cutter ceremony) |
| Rating | None | Office Rating, hidden number, shown as a band on the Ledger (**PROPOSED**: six bands named for the Office's inspection stamps: Provisional, Inspected, Certified, Rated, Sanctioned, Championship) |
| Tints | Relative or absolute | Relative or absolute |
| Leaver | Bot backfill | No backfill; see section 9 |
| Loadout | Changeable between rounds | Locked at the first "Live," changeable only during resets |
| Assisted turn | Allowed | Off |
| Arena pool | All | All four, with the Anchor weighted toward higher bands |

## 7. Claim-season contribution

Every match feeds the player's declared faction's standing for the current Claim (04). The math has to prevent the largest faction winning by population alone, and reward playing rather than farming.

**Per match, per player, Standing points P:**

| Result | P |
|---|---|
| Win, casual | 3 |
| Loss, casual | 1 |
| Win, Rated | 4.5 (×1.5) |
| Loss, Rated | 1.5 |
| Free Scrap top 3 | 2; otherwise 1 |
| Contract completed | +2 each |
| Evidence object recovered | +5, once per player per Claim |
| Forfeit or leave | 0, and the match does not count |

Weekly cap per player: 60 P (**PROPOSED**). Matches past the cap still pay Tonnage, never Standing.

**Faction standing S_f**, recomputed hourly:

S_f = (sum of P over the faction's players this Claim) / max(A_f, 0.6 × mean(A))

where A_f is the number of the faction's players who earned at least 1 P this Claim, and mean(A) is the mean across the five factions. A small faction is divided by at least 60% of the average population, so it cannot win by being tiny either. The Claims map (menus-and-frontend.md section 8) shows S_f to one decimal.

End of Claim: the highest S_f holds the Claim, and the map consequence from 04 is applied to the next season's version of that arena. Ties: the faction with more Rated wins.

## 8. Forfeit, leaver, and idle rules

- **Forfeit**: a crew may forfeit by vote, 2 of 3. The Marshal logs it: "Crew's forfeited. Logged." A forfeit in Rated costs Rating as a loss plus 20%. In casual it costs nothing but writes a Ledger line.
- **Leaver**: a player who disconnects has 60 s to return in casual, 90 s in Rated. Their rigger stands idle (and is a valid target) for 10 s, then is withdrawn; in reserve modes they rejoin as a reserve.
- **Casual**: after the 10 s, a bot backfills (section 9).
- **Rated**: no backfill. The short crew may forfeit without penalty after 60 s. The leaver takes the full loss plus a Ledger entry, "Walked off a claim," visible on their Record for the Claim. Three in a Claim: Rated locked for 7 days.
- **Idle**: 20 s without input, Marshal: "Office notes the rigger standing still." 60 s: treated as a leaver.
- **Dodge**: declining a formed Rated lobby twice in an hour is a 15 minute queue lock.

## 9. Bot backfill

Bots use the Apprenticeship's bot crew (onboarding-script.md), the same behavior trees that wear HTR orange in the tutorial. In matches they wear Office Bone livery, numbered, with a Bone tint on their names. The Marshal calls them in: "Office crew in."

Bot rules:

- A bot inherits the leaver's harness and loadout.
- Bots play the base kit honestly: Slingshot, Haul to an anchor, Clothesline at doorways, Caboose on Haulers. They do not overspin, do not use map cables in Hangtown, and never Haul a teammate.
- Bots cut lines on their own body between 1.0 and 1.6 s after seat, uniformly random, which puts their panic-window survival near the 50% launch target (05 section 12).
- Bots never earn Standing for anyone. Tonnage earned by a bot's Spotter credit goes to the human it set up.
- A crew of three bots cannot exist in a live match; if a crew loses all humans, the match ends as a forfeit by that crew.

### 9.1 Implementation (engine-free)

`game/Source/KineticCore/Public/Kinetic/Bots.h` holds the decisions the bot behavior trees call; the trees move, aim and press. It is tested by `game/Tests/KineticCore/BotTest.cpp`.

| Piece | What it does |
|---|---|
| `FBotRandom` | Each bot's random draws come from the match seed and its slot, so a match with bots replays exactly. |
| `FBotDefense` | The cut reflex for lines seated in the bot's own plates. Backfill bots cut between 1.0 and 1.6 s after the seat, uniformly at random. The tutorial's Probationers cut at 1.4 s. The cut is the swing request, which the server judges at that frame (networking 7.1). The draw starts one class warm-up earlier, so the warm-up ends at the swing. A bot has one Cutter, so a second seated line waits for the swing and the recovery (0.45 s together, from the Cutter's table). A swing that does not end the line, because it was aimed off or refused by the server, is tried again after each recovery until the line ends. A line cut by someone else is dropped from the plan. |
| `ChooseRig` | A backfill bot's next rig from the base kit, in this order: Caboose, Haul to an anchor over an edge, Clothesline at a doorway, Haul to a wall, then Slingshot. **Caboose:** Dog A goes into an enemy within 10 m of the stretch of track a Hauler is about to run, fired 1.5 s or less before the train reaches the point beside them (never after it has passed them). Dog B goes into the train there (05 10). **Haul to an anchor:** the anchor must be at least 4 m from the enemy, with the bot at least 2 m off the drag path. **Clothesline:** an enemy within 8 m heading for the doorway; the bot Hangs, then releases the line into a set line. **Slingshot:** only when every enemy is out of line range, off a wall at least 120° behind the bot. Targets are only standing enemies, never teammates. A bot with a live line, or a Dog in its own plate, starts nothing. |
| `BotMayUse` | What bots never do: Haul a teammate or use Hangtown's map cables. The tutorial crew also never Jacks or moves rated loads. Bots never overspin: they release the mode input on engagement. |
| `FProbationerCrew` | The tutorial crew's schedule from characters/npcs/bot-crew.md. A Probationer on a line (the player, the stack or the Hauler) keeps it until the game reports it gone, and fires nothing else meanwhile. A Probationer who is down gets no orders, and the lead skips it. **Fire and Haul:** the lead fires at 15 m after a 1 s wind-up, braces 0.3 s on a seat, then Hauls. **When cut:** the lead gives a 0.5 s look and a 0.9 s recall, does not fire again for 6 s, and the lead passes to the next Probationer. **Avalanche:** P-02 makes one after a 2 s wind-up once the player stands within 6 m of a stack. **Caboose:** P-03 makes one when a klaxon sounds with the player within 10 m of the trench. **Scrap cap:** after the second scrap of the player, Dogs aim 1 m off, and an Avalanche wind-up under way is called off. **Otherwise:** they hold the marker when on it, and walk the route when not. |

Choices this section did not make (**PROPOSED**):

- ChooseRig's ranges and order.
- The scrap cap also withdraws the Avalanche and the Caboose, since neither can miss by 1 m.
- A missed Dog is recalled and fired again.
- After each cut the lead passes from P-01 to P-02 to P-03 in turn.
- Callsigns: O-01 upward for Office bots in matches, P-01 to P-03 in the tutorial.
- The tutorial crew follows bot-crew.md: three Breakers that cut at 1.4 s and never move rated loads. onboarding-script.md step 9 describes a different crew: one Breaker, one Rigger and one Scrapper cutting at 1.0 to 1.6 s, a bot Breaker Hauling a hull section, and a Guild bot Climber on the player's side. The Guild Climber uses the backfill profile. The rest is open-items 7d.

**Survival depends on how a victim stops after a cut** (open-items 7c). The cut window above puts a bot's swing at 1.0 to 1.6 s. On 05 4.3's bench (a Haul from 15 m, every attacker and victim class), the victim reaches the edge at 1.85 s at the soonest (Breaker on Climber), so every bot cut lands before the edge. But a cut ends the pull, not the slide: that Climber, cut at 1.6 s, is 11 m along at 15.5 m/s. Nothing in 05 says how fast a cut victim stops. If they slow at their own foot-hold (05 5.3), the backfill window gives 73% survival in 05 4.3's own case (a Breaker on a Rigger). That ranges from 0% (a Breaker on a Climber) to 100% (any Climber or Scrapper Haul), about 85% across all 25 pairings. The tests pin these figures.
