# KINETIC: Onboarding Script (The Apprenticeship)

_System spec v0.2.2, revised to 05-core-mechanics v0.2 and the v0.2.1 and v0.2.2 rulings (CHANGELOG 2026-10-24). Implements bible 04 (Onboarding: the Apprenticeship; Grades; the Cutter ceremony) with the numbers from 05 sections 3, 4, 7. Characters from 02. Anything marked **PROPOSED** is new here._

## 1. Constraints

- Under fifteen minutes total (04). Target 12:30.
- No cutscene longer than a Hauler cycle (04). No beat without player input for more than 20 s.
- Teodora Quispe (72, Wright, "Mother Line") runs it. She enforces the First Rule; she did not write it. Baskoro Hanif (29, Guild Lead, "Hitch") assists. Both are playable-set characters with a tutorial gesture set (animation.md section 11).
- Location: the Guild's training yard, a piece of Collar Four. Hull steel, a rail spur with a working hauler, a cutting line, a 6 m gap over a drip net, a dummy on a pole, an overhead beam.
- The player is a Greenhand on a Rigger harness with a loaner Cutter (same timings as Standard). Harness choice comes after the ceremony.
- Tone: dry, short, never quippy. Teodora does not raise her voice. Baskoro counts under his breath.
- The panic window is taught by fear, once, deliberately (04).
- Skippable after first completion. Replayable from the Ledger.

## 2. Beat sheet

| # | Beat | Target time | Teaches |
|---|---|---|---|
| 0 | Cold open | 0:30 | Who she is |
| 1 | Dog | 1:00 | Fire A and B, seat sounds |
| 2 | Haul | 1:15 | Haul, the ramp, mass |
| 3 | Jack | 1:15 | Jack, the Slingshot |
| 4 | Hang | 1:30 | Hang, set lines, the Clothesline |
| 5 | The Gallows | 1:30 | Two-step rig, plates, hoisted |
| 6 | Getting dragged, first | 1:00 | The panic window, failed |
| 7 | Getting dragged, second | 1:00 | The panic window, made |
| 8 | The Cutter ceremony | 0:45 | Grade 2 |
| 9 | First Claim Match, the Yard | 3:00 to 4:00 | Everything, against bots |

## 3. Script

Format: **PROMPT** is HUD text. **SUCCESS** and **FAIL** are conditions. Lines are final-draft candidates.

### Beat 0: Cold open (0:30)

Fade up on the player's hands. Spool on the right forearm, no Cutter in the left. The yard. Equatorial light. A hauler idles on the spur.

TEODORA (approaching, not hurrying): "Greenhand. You've got a Spool and no knife. That's the correct order."

She taps her own chest plate.

TEODORA: "Everything in this yard is heavier than you. Remember that and you'll live through the morning."

**PROMPT:** none. Movement enabled at 0:10. Players who run off get: TEODORA: "The beam's this way."

### Beat 1: Dog (1:00)

An overhead beam, 4 m up, with a painted target. A crate on the floor with a second target.

TEODORA: "Dog the beam. Then dog the crate. Two Dogs, one line."

**PROMPT:** "Fire Dog A at the beam" (crosshair shows the surface read). Then "Fire Dog B at the crate."

On the beam seat: TEODORA: "Steel. Hear it ring? You'll learn the floor by ear."

On B: the line exists, slack. TEODORA: "That's a line. It's not doing anything yet. Lines don't, until you tell them."

**SUCCESS:** A and B seated. **FAIL:** miss twice: BASKORO (off, from the hauler): "She's patient. The beam isn't going anywhere." Auto-recall after 4 s explains itself: TEODORA: "Four seconds between Dogs or the first comes home."

### Beat 2: Haul (1:15)

TEODORA: "Haul. Pull the crate to the mark."

**PROMPT:** "Select Haul" (chips light; the 0.25 s switch wipe plays). Then "Hold until the crate reaches the mark."

The crate (Medium, 120 kg) slides to the beam. Teodora points at the ramp on the tension arc.

TEODORA: "It takes a moment to come on. Four tenths of a second. That moment is the only mercy in the trade."

Then: a second crate, Heavy, 1,800 kg, in a rack. **PROMPT:** "Haul the big one."

The crate is in a wheeled cradle (rollers, 05 section 6.1a). The line sings, the crate creaks, lurches free of the cradle (break-free 200 N, 05 section 6.2), and rolls at about 0.3 m/s² (600 N on 1,800 kg). TEODORA: "Six hundred newtons on two tons. It'll come, slowly. Mass is destiny. Yours too."

**SUCCESS:** first crate at mark. Second crate moves 1 m. **FAIL:** none; the beat waits.

### Beat 3: Jack (1:15)

A cart on rails, pointed at the cutting line.

TEODORA: "Jack. Push the cart down to the cut."

**PROMPT:** "Dog the cart, dog the wall behind you, select Jack."

The Kick cracks and the cart rolls.

TEODORA: "Hear that crack? That's the Kick. The Spool dumps everything it has in a third of a second, then it's a motor shoving. Jack is a kick and a shove. Haul is a pull that doesn't stop. Learn the two sounds."

TEODORA: "One more thing about the Kick. It won't fire with a Dog in a person. Accord interlock, every sanctioned Spool. Jack against a rigger is a shove, not a kick. They get their window. You make sure the shove ends somewhere."

Then Teodora walks to the 6 m gap over the drip net.

TEODORA: "Now yourself. Dog your own plate. Dog the wall behind you. Jack. You're the cart."

**PROMPT:** "Self-dog (secondary). Dog the wall behind you. Jack." The Slingshot (the Kick at the Rigger's launch speed, 12 m/s, 05 section 3.4) clears the gap with room; the net catches a mistimed one (×2.0 threshold multiplier, no harm).

BASKORO (watching): "One."

TEODORA: "He counts. Ignore him."

**SUCCESS:** player lands on the far side. **FAIL:** lands in the net: TEODORA: "The net's there because everyone does that. Again." No limit.

### Beat 4: Hang (1:30)

A doorway in a hull section. Baskoro stands on the far side.

TEODORA: "Hang. Line across the door, chest height. Then let it go. It stays. Twelve seconds, on a Guild harness."

**PROMPT:** "Dog one side of the door. Dog the other at 1.2 m. Select Hang. Release." The set-line pip appears and drains.

TEODORA: "Baskoro. Walk into it."

BASKORO: "Every time." He sprints into the line. Staggered 1.0 s (Rigger's line). Then he crouches under a second one the player sets.

TEODORA: "Crouch under, slide under, or cut it. Those are the three answers. Everything in this trade has three answers."

**SUCCESS:** a set line placed and Baskoro staggered. **FAIL:** line set above 1.9 m or below 0.7 m: BASKORO walks under or steps over: "Height, Greenhand."

### Beat 5: The Gallows (1:30)

The dummy on a pole, under the overhead beam.

TEODORA: "The Gallows. Beam first. Then the dummy's plate. Hang. Then Haul."

**PROMPT:** "Dog the beam. Dog the dummy's chest plate. Select Hang. Hold Haul to Reel." The dummy rises at a meter a second, the ratchet clicking.

TEODORA: "Hang: they're tethered. They can't walk off. Then hold Haul and the Spool winds in. A meter a second. Slow, strong, and it takes anything up to what the line can hang. Nobody gets hauled into the air except by a Breaker. The rest of us reel. That's the Gallows. Hitch."

BASKORO (walking in, turning his back on purpose): "Go on."

**PROMPT:** "Dog the beam. Dog Baskoro's back plate." Baskoro rises on the Reel, a meter a second, the ratchet clicking at his back plate. He draws his Cutter, turns in the harness, finds the line, cuts. The snap. He drops 2 m and lands fine.

BASKORO: "Back plate. You're learning from the right woman."

TEODORA: "Chest, you see it. Shoulders, you turn a little. Back, you turn all the way round. Back's the one that kills people."

**SUCCESS:** Baskoro hoisted. **FAIL:** Dog seats in another plate: BASKORO: "That's a shoulder. Try the back." He waits.

### Beat 6: Getting dragged, first time (1:00)

The hauler on the spur begins its departure cycle: klaxon, then lights. Teodora stands behind the player.

TEODORA: "Face the hauler. Don't turn round."

A Dog seats in the player's **back plate** (seat clank, edge indicator at the bottom edge and one side). The other Dog is on the hauler. Haul engages.

TEODORA (level): "Find it."

The configuration is built to fail: the hauler is 6 m away and already rolling. The Breaker-strength rig on the hauler pulls at 1,000 N; the player's feet leave the floor at 0.26 s (05 section 4.3 reference case) and the point of no return is about 1.0 s after seat, and the player has been given no instruction about the Cutter's existence beyond what they saw Baskoro do. The player is dragged, lifted, and hits the hauler's frame. Stall threshold against a kinematic: 9 m/s. The Stall fires: lock, gel, the Scrap Card (victim view, with "You turned 40 of 150 degrees").

Black for one second. The gel clears.

TEODORA: "That was your Stall. Nurri's gel, the harness lock, a Hook. In a sanctioned match it's a bad afternoon."

Beat.

TEODORA: "In a red match there's no gel. The line was dark and you'd have been under the wheels with your hand on a knife you never found. That's L.3. That's why the Guild writes the serial down."

**Branch (PROPOSED):** if the player cuts in time anyway (possible but unlikely), TEODORA: "Huh." Then: "Most don't, the first time. Hear the rest anyway." She gives the red-match line, and beat 7 plays as a confirmation at a longer window.

**SUCCESS:** none required. **FAIL:** is the design.

### Beat 7: Getting dragged, second time (1:00)

The hauler resets. Teodora hands the player the loaner Cutter, hilt first.

TEODORA: "Now you know where the knife is. Left hand. It takes a fifth of a second to come hot. The swing is fast. Finding the line is the slow part. You look at it, you cut it. You can't cut what you can't see."

**PROMPT:** "When the line seats: turn to the indicator until it vanishes. Then Cutter."

The hauler departs from 15 m. Dog seats in the back plate. Window: about 2.0 s to the point of no return. The edge indicator shows. When the line enters the cone the indicator vanishes and the Cutter glyph rings.

On the cut: the snap. The player skids to a stop.

TEODORA: "That's the sound. You'll hear it across a claim and know somebody lived."

BASKORO: "Or didn't."

TEODORA: "Or didn't."

**SUCCESS:** cut before the point of no return. **FAIL:** Stall again: TEODORA: "Again. Turn first, then draw. Not the other way." The hauler resets. On the third fail the window extends to 2.5 s (**PROPOSED**) and the prompt adds an arrow on the floor. No fourth failure state; it extends until success.

### Beat 8: The Cutter ceremony (0:45)

The yard's office. A ledger on a table, an actual book. Baskoro has stopped counting.

TEODORA: "The loaner goes back in the drawer."

She sets a Cutter on the book. Standard. Handle wrapped in ochre and black tape.

TEODORA: "This one's yours. Nobody lends it. Nobody borrows it. The serial goes next to your name, and the name stays in the book after you don't."

The player picks it up (hold interact, 1.5 s, two-handed take, 90 f). The Ledger screen opens for the first time: Grade 1 Greenhand becomes Grade 2 Hand. The Serial tab writes itself: a number, today's date, "Issued."

TEODORA: "Hand. Not Greenhand. Don't make me regret the ink."

BASKORO: "Three. She likes you."

**SUCCESS:** Cutter taken. No fail.

### Beat 9: First Claim Match, the Yard (3:00 to 4:00)

Harness select opens, all five, with Teodora's advisory: "Rigger's what you learned on. The others are your business." Then the Yard, Claim mode, shortened: first to 40 points (**PROPOSED**), against a bot crew in HTR orange (one Breaker, one Rigger, one Scrapper). Baskoro and a Guild bot Climber are the player's crew. LayCast package live for the first time. The Marshal calls it.

Bots play honestly and cut at 1.0 to 1.6 s. The Hauler schedule runs. Shift Change fires at 4:00 remaining (Claim timing, 05 section 9.2).

Scripted assists, each fires once, only if the trigger occurs:

- Player dogged in the back by a bot: BASKORO: "Line on you. Back plate."
- Player near a trench when the klaxon sounds: BASKORO: "Klaxon. Off the rails."
- A bot Breaker Hauls a hull section: BASKORO: "Rated load. That's a Breaker. We don't pull those."
- Player holds the marker 10 s: BASKORO: "Holding. Stay in the ring."

On the win: Marshal: "Claim's settled." TEODORA (on comms, first time she is heard remotely): "Go to work." Post-match summary, with the Weight shown for the first time.

On a loss: the match still ends with the ceremony intact. TEODORA: "You'll lose more. Keep the knife sharp."

## 4. The Hand: post-tutorial tips

One-line contextual tips, Office voice, shown once each at (50, 70) for 4 s, maximum 20 total, never more than one per 30 s, all disabled after Grade 3 or by option. Each is tied to a trigger that fires only when the player has not yet done the thing.

| # | Trigger | Tip |
|---|---|---|
| 1 | First match on Ballast, 0:10 after Live | "The hull leans every ninety seconds. Loose containers lean with it." |
| 2 | First Hauler klaxon within 20 m | "Klaxon, then lights, then train. Three seconds." |
| 3 | First time standing under a Gantry load | "The magnet drops its load on a cycle. Once is the number of times you stand under it." |
| 4 | First rated load in crosshair, non-Breaker | "Yellow and black tape: rated. Only a Breaker's line moves it." |
| 5 | Dog in a Catch Plate, a line on body for 2 s, not cut | "Turn until the indicator vanishes. Then the knife." |
| 6 | Line in chest plate, not cut in 1.5 s | "A chest line is already in front of you. Cut." |
| 7 | Staggered by a Hang line | "A set line stops a sprinter. Crouch under lines above one meter." |
| 8 | Player enters a 3v3 round with a set line unused for 3 rounds (Rigger) | "You carry two set lines for twelve seconds. Doorways are free." |
| 9 | First overspin heard within 20 m | "That octave is a Fray line overspun. It snaps in under a second. Be elsewhere." |
| 10 | First propane cylinder in a rack targeted | "Racked cylinders need two hundred newtons to break free. Any harness has it." |
| 11 | Player Hauls a Heavy object for 3 s with no movement | "Two tons at six hundred newtons is slow. Heavy things fall. They do not fly." |
| 12 | First time hoisted | "Hoisted. You can still fire and cut. You cannot walk. On your own line, hold Haul to climb." |
| 13 | Teammate has a line on their back for 1 s | "Your Cutter frees a teammate. Hit the plate, not the rigger." |
| 14 | First ring-out death | "Over an edge above eight meters a second, there is no ledge grab. Hang to something." |
| 15 | First Scrap Card as a Spotter credit | "Your set line, their scrap. Spotter pays the larger share." |
| 16 | First time within 5 m of a dim line (set line) | "Dim line, low hum: a set line. It brightens as you approach. Someone wants you to walk through it." |
| 17 | First Jack with a Dog in an enemy's plate | "Jack against a person is a shove, not a kick. The interlock. Put an edge or something hard behind them." |
| 18 | Brace available, never used, 2 matches (Rigger, Breaker) | "Crouch still for a third of a second. Braced, you are the anchor." |
| 19 | First Hangtown match, orange-taped cable in crosshair | "Orange tape: slack cable. Any Cutter cuts it in six tenths. Whatever hangs from it, drops." |
| 20 | First Claim marker moved by an enemy line | "The marker rides on runners. A Rigger's line or heavier moves it, slowly. It still scores wherever it is." |

Tips are written to the Ledger's Entries tab as "Hand's notes" once shown, so a player can reread them.
