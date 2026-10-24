# THE YARD: Signature Plays

_Arena: The Yard (Collar Four Breaking Yard). File purpose: twelve plays unique to this arena, with positions, class, inputs in the style of 05 section 10, timing against the Haulers, the Gantry, and Shift Change, and the counter._
_Sources: docs/bible/03-arenas.md, docs/bible/05-core-mechanics.md. Anything not in those files is marked **PROPOSED**._

_Jack launches in this file: every Jack from a self-dog is the Kick (05 section 3.2), the 0.3 s capacitor launch to the class speed V of 18 / 14 / 12 / 14 / 10 m/s (Climber / Scrapper / Rigger / Doubler / Breaker), split by inverse mass between the two ends, followed by the motor push at T to class line length. Where an input below says Jack, read "Jack (the Kick at V)". Exception, 05 section 3.2 interlock: no Kick fires when either Dog is in another player's Catch Plate; a Jack against a player is the T push only._


Zone names and coordinates are from layout.md. Timetable values are from events-and-moving-parts.md: South trains nose-in at e 10 + 45k, Center at e 25 + 45k (from the east), North at e 40 + 45k. Klaxon 3 s before nose-in.

Class coverage: Breaker (1, 5, 6, 10, 12), Rigger (2, 7, 11), Climber (3, 8), Scrapper (4), Doubler (9, 10). Shift Change plays: 7, 8, 11. Times are Scrap times; add 165 s for Claim, 45 s for Haul, and 105 s for Free Scrap.

---

### 1. The Caboose, pure

**Where:** any trench lip. Best on the south lip of the Center trench at X +10 to +30, between the pit and grate 3, where an enemy crossing to the Haul Lane has 6 m of Floor behind them.
**Class:** any. Breaker is the canonical one because a braced Breaker takes none of the pull.
**Inputs:**
1. Hear the Center klaxon (two-tone). Brace.
2. Dog A: enemy's Catch Plate, back plate if they are facing the trench.
3. Dog B: the Hauler's load as its nose passes within 20 m.
4. Haul.
**Timing:** fire Dog B in the 1.2 s the load is passing in front of you. The line goes taut at 12 m/s relative; the enemy is dragged along the lip and into the trench inside 1.5 s.
**Why it works:** the Hauler is a kinematic at 12 m/s and the Stall against a kinematic is 9 m/s. The train does the scrapping; the line only introduces them.
**Counter:** cut inside 1.5 s (the Dog is in your back: 150 degree turn). Or Hang to the pit lip rail or a mesa face before you reach the lip; a Hang to static makes you the heavier end.

### 2. Trench Bridge

**Where:** North trench between Marker Flat and the North Dock, at X -40 (between grate 1 and the plank at -30).
**Class:** Rigger (two set lines, 12 s life, 12 kN Hang load).
**Inputs:**
1. Dog A: the near lip at 0.3 m above the Floor. Dog B: the far lip, 6 m across. Hang. Release: set line one, a footbridge.
2. Cross it. Set a second line 1 m east, parallel, as a second lane.
3. Stand on the far side, 8 m back, and wait.
**Timing:** set the bridge at the North train's e 40 + 45k minus 12 s, so it is a live crossing for 9 s and then the train arrives while the enemy is halfway. The train is in the west half of the trench 2 to 7 s after nose-in; at X -40 it arrives 3.3 s after nose-in.
**Why it works:** the load severs the line (05 section 3.5), the snap plays, and the enemy is in the trench with the load 0.3 s behind them at 12 m/s.
**Counter:** read the klaxon. Never step onto a line within 5 s of a nose-in in that trench. Cut the line yourself before crossing and take the grate.

### 3. Rail Ride

**Where:** NC2's top (-6, +21, 8) and the Gantry load passing at c 34 to 41 (X -12 to 0, trolley at about Y +7, load underside 27.3 m).
**Class:** Climber (50 m line, perch, mid-air Dogs).
**Inputs:**
1. At c 34, self-dog A. Dog B: the hoist cable just above the load, 24 m away (14 m lateral, 19 m up). Hang.
2. Step off the mesa's south edge. You swing under the load and settle beneath it, 27 m up, moving east at 1.6 m/s. Hold Haul inside the Hang to Reel in at 1.0 m/s if you want to stand on the load; hold Jack to pay out and hang lower.
3. At c 70 the Gantry stops over the pit and lowers. Before c 78 (the red lamps), release, self-dog, Dog B the load's underside, Jack (the Kick: 18 m/s for a Climber) away from the load at 45 degrees is about 33 m of travel, onto NC3's top or the East Pick Stack.
4. Arrive with a Hang to a ledge, not a floor: 18 m/s into steel is a Stall.
**Timing:** 44 s of air time over the center of the map, ending 8 s before the drop tell.
**Why it works:** nothing on the Floor can Dog you at 27 m except a Rigger with a clear angle, and from under the load you see every set line on Marker Flat lit.
**Counter:** a Breaker riding the bridge beam strips the load (play 12) and you go down with it. Or wait: a Climber who stays past c 78 rides the load into the pit.

### 4. Pit Overboard

**Where:** the press approach, east lip, X +52 to +60, Y -4 to +4. The one lip with no rail.
**Class:** Scrapper (110 kg, instant recall, fastest Dogs).
**Inputs:**
1. Put the East Pick Stack's west face at your back (static behind you).
2. Self-dog A. Dog B: the enemy's chest plate as they come off grate 4 or the East Mesa rungs toward the marker, 9 to 12 m from the lip.
3. Jack. No Kick fires with a Dog in a Catch Plate (05 section 3.2 interlock): this is the 400 N motor push, ramped over 0.3 s.
**Timing:** any time. Best during the Center train's approach, when the enemy is watching the trench instead of you.
**Why it works:** 400 N on a 130 kg Rigger is 3.1 m/s², under their 5 m/s² foot-hold, so they skid backward on planted feet and cross 9 m in about 2.4 s; the stack face behind you means you take none of it. On a 90 kg Climber it is 4.4 m/s², over their 3 m/s² foot-hold: airborne, 9 m in 2 s. Both windows sit at the long end of the 05 section 4.3 target, and the play works because they are watching the trench, not you. The pit below -6 m is the Well.
**Counter:** Hang to the pit lip rail (the north or south one) or to the Floor behind you. Cut: the Dog is in your chest, already in view, 0.35 s with a standard Cutter.

### 5. The Plow

**Where:** R2 at (X +30 to +42, Y +19 to +25) on Marker Flat, 2 m from the North trench lip.
**Class:** Breaker (1,000 N, rated), or two Riggers pulling the same edge on the same axis (1,200 N), or a Doubler at overspin.
**Inputs:**
1. Brace at (+36, +14), facing north. Dog A: R2's near edge. Dog B: self. Haul.
2. 1.0 s groan. R2 slides off its blocks (authored, 4 s) north into the North trench and drops in flat.
3. Release. Step back.
**Timing:** start the Haul 8 s before a North nose-in (e 40 + 45k minus 8). R2 is in the trench 5 s later; the train reaches X +36 9.7 s after nose-in, so the margin is 12 s. Enough to walk away. Not enough for the enemy on the North Dock to work out what the wall is for.
**Why it works:** the train carries R2 ahead of it at 12 m/s as a 12 m wall whose ends overhang both lips by 3 m. Anyone within 3 m of either lip from X +36 to the east plenum is hit by 4,500 kg at 12 m/s.
**Counter:** stand further than 3 m from a lip when a plow is running. Watch for the groan and the tape fluttering on R2.

### 6. Shack Drop

**Where:** the column at (-62, +35), North Dock, with Spawn A's crew holding the Shack Perch.
**Class:** Breaker, or two Riggers on the same column and axis (1,200 N), or a Doubler at overspin.
**Inputs:**
1. Brace at (-62, +44), between the column and the perimeter wall. Dog A: the column at 2 m. Dog B: self. Haul.
2. 1.0 s groan, dust from the shack floor.
3. The shack drops 10.5 m onto X -66 to -58, Y +34 to +44. Hold the brace; the Breaker is outside the footprint by 2 m.
**Timing:** when at least one enemy is on the shack roof or at the ladder landing at 10.5 m. Confirm with the roof floods: a player on the roof blocks one.
**Why it works:** a 10.5 m fall is 14.3 m/s against the Floor; the Stall threshold is 12. The west third of the Rails goes dark, and a Climber crew that lives on the Rails now lives in the dark.
**Counter:** a Climber on the roof hears the groan and Slingshots off (Jack, the Kick: 18 m/s; 1.0 s is enough). A Rigger Hangs to the west leg, not the shack. Keep a Scrapper on the dock to Cutter-swing the Breaker's chest plate: it unseats the self-dog and the Haul ends.

### 7. Magnet Drop (Shift Change)

**Where:** Marker Flat, the marker at (0, +12) and the Shift Change drop point at (-15, +9).
**Class:** Rigger or Breaker. The marker (900 kg planning mass, 05 section 6.1a) sits on polished runners at friction 0.05, about 440 N to start: a Rigger's 600 N moves it at 0.18 m/s², a Breaker's 1,000 N at 0.62 m/s², and a 400 N line does not start it.
**Inputs:**
1. Rigger: at e 66, Dog A: the marker pallet. Dog B: NC1's east face at (-24, +18). Haul. At 0.18 m/s² the marker has slid 11 m west by e 78, to about (-11, +11). Breaker: start at e 72; at 0.62 m/s² it covers the same 11 m in 6 s.
2. Release at e 78. Back off 8 m east.
3. The siren ends at e 80. The load falls on (-15, +9), a 12 m by 6 m footprint, X -21 to -9. The marker is inside it.
**Timing:** 12 s of pull for a Rigger, 6 s for a Breaker, release 2 s before the drop.
**Why it works:** an enemy standing on the marker to contest it is under 2,800 kg at 23 m/s. The marker itself is shoved east and keeps scoring from wherever it lands (05 section 9.4: moving the marker is legal; it resets only if it enters a Well).
**Counter:** Jack the marker back east from NC2's face (the Kick gives 900 kg about 1.5 m/s, 2 m of slide on the runners, and the push adds the rest). Or hold the marker from its east edge, 4 m from center, which is outside the footprint by 1 m, and accept the debris shove (a_ref 12 m/s² for 0.5 s, scaled by 100 / m: a stagger, not a Stall).

### 8. Press Feed (Shift Change)

**Where:** the East Pick Stack (+62, +14, 5.4) and the pit.
**Class:** Climber.
**Inputs:**
1. At e 90, perch on the East Pick Stack's live top section. The Gantry picks it at e 94 (magnet thump).
2. Ride up (5 s) and west (5 s) toward the pit.
3. At e 104, over the press approach at 30 m, Dog A: an enemy on the east lip. Dog B: the load you are standing on. Hang. Jump clear: self-dog is not involved; you are a passenger.
4. At e 107 the red lamps; at e 109 the load drops into the pit with the enemy tethered to it. The Anchor Drop, with the Gantry pushing.
**Timing:** 15 s window from pick to drop. The Hang must go on by e 106 to leave the enemy their 2 s panic window plus the fall.
**Why it works:** a Hang from a player to a falling Heavy object drags the player with it (05 section 8, Player to Heavy, Hang). The load falls 15.8 m into the Well.
**Counter:** cut. The line comes from above and in front: the chest or a shoulder plate, a 60 to 90 degree turn at most. 2 s is a long window. Riggers should also never stand on the press approach between e 100 and e 110.

### 9. Double Caboose

**Where:** the South trench lip from the Haul Lane side, X -30 to -10, during Haul mode when both enemy pullers are on the spool.
**Class:** Doubler (two active lines, 480 N each).
**Inputs:**
1. South klaxon (low horn). Dog A: puller one's plate. Dog B: the Hauler load as it passes. Haul.
2. Immediately, Dog C: puller two's plate. Dog D: the same load. Haul.
3. Hold overspin on line one for 1.0 s (octave jump), release before 1.8 s to keep the elevated tension.
**Timing:** both lines inside the 1.2 s the load passes. Scrappers do this faster; Doublers do it twice.
**Why it works:** 480 N on a 130 kg Rigger is 3.7 m/s², under their 5 m/s² foot-hold, so they skid toward the lip instead of flying, which looks survivable. The lip is 12 m from the Haul corridor; they reach it in 2.5 s at 9 m/s and the train has not finished passing. Two pullers off the spool is the pull lost.
**Counter:** the panic window is 2.5 s, longer than usual. Cut. The Doubler is standing still with two lines singing and is the brightest thing on the Haul Lane: the third enemy Hauls them into the trench with their own train.

### 10. Billet Avalanche on the Haul Lane

**Where:** the billet stacks at (-60, -20) and (-50, -20), 8 m south of the Haul corridor, as the enemy's spool pull approaches Truck A.
**Class:** Breaker, Rigger, or Doubler (over the 500 N stack break-free: 1,000, 600, 800 N). Not Climber, not Scrapper (400 N).
**Inputs:**
1. Brace at (-55, -25), south of the stacks. Dog A: the bottom crate of the (-50, -20) stack, on its north face. Dog B: self. Haul.
2. 0.2 s shudder, then the 1.5 s lean (**PROPOSED** settle time), then the bottom crate rolls north out of the stack on its roller pallet (0.02 to start, 0.005 rolling) and the two above it fall across the corridor.
**Timing:** begin the Haul when the spool is at X -40 and rolling west at 1 to 2 m/s; the crates land across Y -16 to -10 about 3 s later, in the corridor.
**Why it works:** a 900 kg billet crate needs only 1.4 m/s to deliver 1,200 N s to a player. The bottom crate arrives rolling at 2 to 3 m/s; the two above fall 1.2 and 2.4 m and arrive at 4.9 and 6.9 m/s. A puller hit by any of the three is scrapped, and the spool stops against the pile.
**Counter:** don't pull past stacks on a Breaker's map; pull along Y -9, the corridor's north edge, out of the fall line. A Scrapper on the pull cannot Jack the top crate away first (the Kick is tested against the 500 N stack break-free like any active pull, and a Scrapper's 400 N will not start it); the correct answer is the Cutter on the Breaker's line.

### 11. Lane Lock (Shift Change)

**Where:** grate 2 over the North trench (X -10, Y +30, deck at 3.5 m), the only crossing between Marker Flat and the North Dock for 40 m either way after the plank bridges are gone.
**Class:** Rigger (two set lines, 12 s).
**Inputs:**
1. At the siren (e 75), stand on the grate's south landing. Dog A: the handrail post at the south stair head, 1.2 m up. Dog B: the opposite post. Hang. Release.
2. Walk 8 m across. Set the second line across the north stair head at 1.2 m. Release.
3. Drop off the grate's west side onto Marker Flat (3.5 m, 8.3 m/s, safe) and wait at the stack of R1.
**Timing:** both lines live from e 80 to e 92. The Haulers now run every 7.5 s somewhere and the North trench runs at e 98, 120.5, 143. Nobody crosses at Floor level; everybody takes the grate.
**Why it works:** a player sprinting up a 35 degree stair hits the line at over 6 m/s and is flipped (05 section 3.5), over the rail and off the grate: 7.5 m to the rail bed, 12.1 m/s, a Stall, with the North train due.
**Counter:** crouch at every stair head during Shift Change: a crouching player passes under a 1.2 m line. Cut it. Or do not cross; the grate is a lane now, and lanes are where Riggers live.

### 12. Crane Operator

**Where:** the Gantry bridge beam at 36 m, beside the trolley, during the loaded traverse over Marker Flat (c 10 to 70).
**Class:** Breaker (the load is a rated attachment, 1,000 N break-free, 05 section 6.7).
**Inputs:**
1. Cage ladder on the leg at (-60, +30) to 36 m (22.5 s from the Floor at the Breaker's 1.6 m/s). Walk the north walkway to X -50 and step onto the beam while it is parked for the pick (c 0 to 10).
2. Ride east. Brace on the beam beside the trolley, Y +8, with the load 6 m below you.
3. When the load is over the target (an enemy on the marker at c 40 to 45, X -2 to +6), Dog A: the load's top face. Dog B: self. Haul.
4. 1.0 s structural groan, then the magnet lets go and the load falls 27 m. Release your line as it goes.
**Timing:** c 40 to 45 for the marker. Any time over Marker Flat for a target of opportunity. The Gantry runs the rest of its cycle empty.
**Why it works:** 2,800 kg from 27 m arrives at 23 m/s. The only tell is a 1.0 s groan from directly above, where nobody on the Floor is looking. The Breaker is the only harness that can be 10 m from the load at all, because the only place that close is the beam.
**Counter:** watch the beam: a Breaker riding the Gantry is a silhouette against the sodium cloud. Stay off the load's shadow, which the magnet lamps draw on the Floor the whole traverse. On the beam, a Cutter swing to the Breaker's chest plate unseats the self-dog and ends the Haul.

---

## Notes for tuning

- Plays 1, 2, 5, and 9 all depend on the timetable being learnable. If telemetry shows the Hauler panic window (Dog seat to trench entry) under 1.0 s anywhere, move the Haul corridor and the Floor cover 2 m further from the lips before touching the Haul ramp.
- Play 7 depends on the Shift Change drop point being authored and identical every round. It is. The surprise is for the first week only; after that it is a position everyone fights over at e 70, which is the intent.
- If the pit exceeds 25% of eliminations, extend the pit lip rail to the east lip and leave only a 4 m gap at the press approach.
