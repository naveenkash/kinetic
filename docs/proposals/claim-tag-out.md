# KINETIC: Claim with Tag-Out (concept)

_Concept proposal v0.2, 2026-10-25, written against 05-core-mechanics v0.2 and the v0.2.1 and v0.2.2 rulings. Everything in this document is **PROPOSED**. Nothing here is canon until bible 05 changes and the change is logged in the CHANGELOG. Where this document contradicts 05, section 11 lists the conflict. v0.1 used Weigh-In as the elimination; it is now an optional variant (section 13)._

## 1. The pitch

Two crews, one beacon each, one capture ends the match. You do not shoot, rope, or pull anyone. Every rigger wears a three-pip lockout code on their shoulders and back. Read it, call it, and the Office locks them out of the claim.

Your chest is blank. Facing someone is safe; turning away is how you die. So every fight is a dance: circling, faking, backing up against something to hide your back, and the only things to back up against in a breaking yard are walls next to drops.

**Why it fits Kinetic.**

- **Every weapon is a tool.** Lockout/tagout is real industrial safety procedure. A rigger whose code is called is locked out, the same way a machine is. Nobody brought a weapon.
- **The Catch Plates are already canon** (05 section 5.5): chest, back, left shoulder, right shoulder. 05 already calls the back "the kill plate". Tag-Out makes that literal.
- **The arena is still the killer's partner.** The safest stance (back to something) puts you beside an edge, a trench, or a Hauler lane.

**Inspiration.** Figure Me Out (Solo Groove Games, Steam app 5313470, announced for 2026-11-02): eliminate an opponent by typing the number on their forehead. Tag-Out keeps the reading race, the typing cost, and the information-denying gadgets. It flips the core rule: in Figure Me Out, looking at someone exposes you; in Tag-Out, looking at someone protects you, and the attack is getting around them.

## 2. The code

### 2.1 Plates and pips

| Plate | Shows | Faces |
|---|---|---|
| Left shoulder | Pip 1 of the code (1 to 4 dots) | Outward, to the rigger's left |
| Back | Pip 2 | Behind |
| Right shoulder | Pip 3 | Outward, to the rigger's right |
| Chest | Nothing. Crew tint only (05 section 11a) | Forward |

A code is three numbers from 1 to 4, written left, back, right: for example 3-1-4. That gives 64 codes.

**Why pips.** Dice-style dots read the same from any rotation (a rigger on a ladder or tumbling still reads), carry no color (colorblind-safe), and stay legible at about 12 px tall. Arrows and letters fail at least one of those.

### 2.2 Reading

- Each pip is readable from inside a **±55° cone** around the direction its plate faces.
- **Naked eye: 20 m.** Pips are sized so this holds at 1080p without zoom (section 10.4).
- **Sighting level** (every rigger carries one, the mode's binoculars): raise it in 0.4 s for ×3 zoom and **50 m** range. While it is up you move at crouch speed and the target sees a lens glint at your position.
- From directly behind you see the back pip. From behind and to one side (around 135°) you see the back and one shoulder. From the side you see one shoulder. From the front you see nothing.

A full read from one position is rare. Two teammates at two angles get it fast.

### 2.3 When the code changes

- **On every drop** (spawn and reserve).
- **At a tag locker** (section 2.5).
- Never on its own. A code you were shown stays true until the rigger re-stamps or dies.

### 2.4 The crew read board

Every pip any teammate has been shown lands on a shared board. Over each enemy the HUD shows what the crew knows, for example "3 · ? · 4", and how old the read is. A read goes grey the moment that rigger re-stamps or dies.

The board records only what the server actually showed your crew (section 10.1). It cannot be fooled or filled in by hand.

### 2.5 Tag lockers

Four lockers: one on each home pad, and two out on the map (section 9). Stand at one and hold Use for **3.0 s** without moving: the stamp slams and your code is re-rolled. Every rigger within 30 m hears the stamp. Being read is a problem you can fix, at the cost of three seconds standing still somewhere everyone knows about.

## 3. The Call

### 3.1 Making a call

1. **Enter.** Press Call to open the call strip and enter three pips: keys 1 to 4 on a keyboard, the d-pad on a controller (up 1, right 2, down 3, left 4). You can enter a code any time, from memory, before you even see the target.
2. **Aim.** Put the target's torso in the crosshair, within **40 m**, with line of sight. Press Call again to send.
3. **Lock.** Keep the target in view for **0.6 s**. The target's own harness ticks fast, and an edge indicator points them at you (section 8).
4. **Resolve.** If the lock holds and all three pips match, the target is **locked out**: a Stall (05 section 7.4) with the Scrap Card "tagged out". The Marshal reads the code aloud.

**The input cost is the same on every device.** Entering pips takes the left hand off WASD or the left thumb off the stick. For a moment you stop moving. That is the Figure Me Out typing cost, and it is why you enter codes from cover.

### 3.2 Outcomes

| Outcome | What happens | Cooldown before the caller can call again |
|---|---|---|
| **Clean call** (all three match, lock held) | Target is tagged out | 1.0 s |
| **Lost lock** (target breaks line of sight, enters dust, or the caller looks away during the 0.6 s) | Nothing. The target heard the tick and knows where you are | 1.5 s |
| **Bad call** (lock held, code wrong) | One of the caller's own pips (one the target's crew does not have yet, chosen at random) goes onto the target crew's board until the caller re-stamps. The caller's position is shown to the target's crew for 3 s | 2.0 s |

### 3.3 The gamble

| Pips known | Chance a call is right |
|---|---|
| 0 | 1 in 64 |
| 1 | 1 in 16 |
| 2 | 1 in 4 |
| 3 | Certain |

"I have two of three, do I risk it?" is the decision the mode is built on. A bad call is never fatal, but it costs you a pip and your position, and the next bad call costs another.

### 3.4 Why the lock exists

05 pillar 5 says every kill can be escaped by a player who reacts in time, and pillar 2 says every lethal thing telegraphs. The 0.6 s lock is this mode's panic window: on the first tick, turn to face the caller, break line of sight, or step into dust, and the call fails. Turning to face them also shows them nothing, which is the point. Target lock-escape rate at launch: about 40% (section 12).

## 4. Turning people

Face to face, nobody can read anyone, so the mode needs ways to make people turn.

- **Cutter shoulder hit.** 05 section 4.4 already makes a Cutter swing on a Catch Plate stagger the target for 0.3 s. Tag-Out adds a **70° spin** away from the hit, so the plate opposite the hit swings toward the hitter's side. Hit a left shoulder and the back turns toward your teammate on your left.
- **Noise.** Throw any Light-tier prop (05 section 6.1: bolts, cans, tools). Riggers turn toward sounds. So do players.
- **The arena.** When a Hauler klaxon sounds or the Gantry swings in, everyone looks at it, and everyone turns their back on something else. Crews that play with the timetable get free reads.
- **Pressure.** Two riggers on one target. The target can only face one of them.

## 5. Claim

### 5.1 Format

| Item | Value |
|---|---|
| Teams | 4v4 (inside the 8-player budget that Free Scrap already uses). 3v3 fallback |
| Rounds | 1 |
| Regulation | 8:00 |
| Overtime | Up to 3:00 (section 5.6) |
| Reserve | Unlimited. Timer 6 s for the first 3:00 elapsed, 9 s until 6:00, 12 s until the end of regulation, 15 s in overtime |
| Win | First crew to plant the enemy beacon on its own home pad |

### 5.2 Beacons and bases

Each crew's **beacon** sits on a stand at the center of its **home pad**, a 4 m circle on its spawn mesa (section 9). A beacon is a 25 kg salvage claim marker with a polished reflector on top and a flag panel. It is Light tier for physics (05 section 6.1), but it can only be moved by carrying or throwing it.

- **Grab.** An enemy rigger touches the stand and holds Use for 0.5 s. The Marshal calls it. Both crews get a klaxon.
- **Your own beacon** cannot be picked up by your own crew.

### 5.3 The carrier

- **The flag panel shows the carrier's back pip at twice the size, readable at 40 m by the naked eye.** Running home means showing your back to everyone chasing you. Escorts have to physically cover the carrier.
- **Slower:** sprint −10%, ladder climb −25% (1.5 m/s, Breaker 1.2 m/s). Jump and mantle as normal.
- **No gadgets** and no re-stamping while carrying.
- The reflector flashes every 3 s and is visible from anywhere in the Yard with a line of sight to the sky above it.
- **Throw:** 1.0 s wind-up, about 12 m on a flat arc. A pass to a teammate is fast, and also the easiest way to lose the beacon over an edge.

Picking up the beacon does not change your code. A crew that has already read your runner will tag them out on the way home, so the best runner is one nobody has read yet.

### 5.4 Dropped beacons

- If the carrier Stalls, the beacon drops as a physics object where they fell. If they fell off a tier, it lands on the tier below. Chasing a beacon down through the Stacks is meant to happen.
- If a beacon enters a kill volume (the Press pit below −6 m), it goes back to its stand.
- If a dropped beacon is left untouched for 30 s, it goes back to its stand.
- The owning crew returns a dropped beacon by holding Use on it for **2.0 s**. If the returner is tagged out during the channel, the return fails. The attacking crew picks it up again with the normal 0.5 s grab.

### 5.5 The plant

The carrier enters their own home pad and holds Use for **1.0 s** to clip the beacon in. If the carrier Stalls during the 1.0 s, the plant fails and the beacon drops. A completed plant ends the match at once.

**Your own beacon does not need to be home** to plant. Classic capture the flag requires it, but with only one capture this would create stalemates where both beacons are out and both crews turtle. Section 11 lists this as an open question.

### 5.6 Match flow

| Clock | Event |
|---|---|
| Pre-match | Drop, 6 s (05 section 9.2). Call disabled. Beacons on their stands |
| 8:00 | Marshal: "Live." Everything enabled |
| 4:00 remaining | The arena's mid-match event fires with the 5 s siren (05 section 9.2; the Yard's Shift Change) |
| 0:00 | If nobody has planted: **overtime** |
| Overtime | The arena's sudden-death event runs (the Yard's Lockout). A carrier's full code goes onto the enemy crew's board the moment they pick up the beacon. Tag lockers shut. Carriers can still escape a call by breaking the lock, but they can no longer hide their code |
| End of overtime | The crew whose carrier brought the enemy beacon closest to its own home pad at any point in the match wins (closest distance reached, measured to the pad edge along a walkable route). If neither crew ever lifted the enemy beacon, the match is a draw in casual and a loss for both in Rated |

### 5.7 Roles

05 does not change the classes. The mode gives them jobs on its own:

| Class | Natural job | Why |
|---|---|---|
| Climber | Runner | Fastest, best routes to the Rails, hard to stay behind |
| Scrapper | Turner | The quickest Cutter (05 section 4.1, −0.05 s warm-up), so the best at spinning people for a teammate's read |
| Rigger, Doubler | Reader, escort | Steady at call range. With a Rigger and a Doubler in one crew, the enemy also has to keep track of which is which |
| Breaker | Anchor on the home pad | Slow, so they spend the match with their back to the home pad's parapet, guarding the stand and the locker |

## 6. Hiding your code

| Tool | Where | What it does |
|---|---|---|
| **Your back to something** | Anywhere | Free and always available. A pip against a wall, heap, or mesa face cannot be read. Most of those surfaces are next to a drop |
| **Grease rag** | Loadout gadget, 2 per drop | Smears one plate for 12 s. That pip cannot be read. 1.0 s to apply, both hands |
| **Dust bag** | Loadout gadget, 2 per drop | Cut it open (Cutter, 05 section 4.1 timings). A 6 m cloud for 8 s. Pips cannot be read through it, and calls cannot lock through it |
| **Sun glare** | Fixed sun, 35° elevation | A polished plate flares when the sun's reflection points at the viewer. That pip cannot be read from that direction. Separately, a call cannot lock on a target within 12° of the sun in the caller's view |
| **Tarp curtains** | Authored on the Yard's Canyon gaps and grate bridges | Cut the lashing and a tarp drops across the gap, blocking sightlines until someone cuts through it (one Cutter swing) |
| **Heat haze** | Over hot steel: the press frame, the Hauler exhausts | Pips beyond 12 m cannot be read, even through the sighting level |
| **Crane shadow** | Under the Gantry and its load | The beacon reflector does not flash in shadow. Carriers route under the Gantry |

**Loadout.** One gadget slot: grease rags or dust bags. Every rigger also carries the sighting level and the Cutter.

## 7. Daylight

The mode is designed for daytime arenas. There is no darkness system and no night lighting. Information is denied by the sun, by surfaces, and by things on the site (section 6).

The sun's azimuth is perpendicular to the line between the two home pads (from the north-northwest in the Yard), so neither base always has the sun at its back. Individual lanes do.

## 8. Tells (05 section 11 format)

| System | Audio | Visual | Lead |
|---|---|---|---|
| You are being called | Fast tick from your own harness, directional | Edge indicator toward the caller | 0.6 s (the lock) |
| Bad call | Buzz at the caller's position | The caller's position for 3 s, to the target's crew | 0 s |
| Sighting level raised at you | None | Lens glint at its position | 0 s |
| Cutter spin | Plate clang | The stagger and turn | 0 s |
| Re-stamp | Stamp slam, 30 m audible | The locker light turns from green to red for 3 s | 3.0 s (the channel) |
| Beacon grabbed | Klaxon for both crews, Marshal call | The reflector starts flashing | 0 s |
| Beacon dropped | Marshal call | A pillar on the HUD over its position | 0 s |
| Plant in progress | Rising clip tone, audible across the pad | Ring fills on the pad | 1.0 s |
| Return in progress | Rising tone at the beacon | Ring fills on the beacon | 2.0 s |

## 9. In the Yard

| Item | Position | Note |
|---|---|---|
| Home pad A and locker | Spawn A, West Mesa top (−68, −39, 8) | The carrier's final climb is the east-face rungs at (−56, −39), 8 m, with their back to the Haul Lane |
| Home pad B and locker | Spawn B, East Mesa top (+68, +39, 8) | The west-face rungs. The carrier's last approach crosses grate 4 at X +70 |
| Map lockers | NC2 top (−6, +21, 8) and SC4 top (+28, −21, 8) | Point-symmetric about the origin, on exposed mesa tops so a re-stamp is a real risk |
| Tarp curtains | Every Canyon gap 10 m or narrower, and all four grate bridges | New props |
| Sun | Azimuth from the north-northwest, perpendicular to the A to B diagonal, 35° elevation | Replaces the night setup in lighting-and-mood.md |
| Claim marker (old mode) | Removed from Marker Flat | Marker Flat stays as the central fight ground and the scrap heaps stay |

**Back-to-the-wall geometry.** The Yard already has what the mode needs: mesa faces at the edge of every Canyon gap, the 1.2 m parapet on West Mesa, the press frame beside the pit, trench walls along every Hauler lane. Each is safe from behind and dangerous from somewhere else. The grey box should check that every zone has at least one such spot, and that none is safe from everything.

**Asymmetry warning.** The spawns are point-symmetric about the origin but the Press pit is not: Spawn B is 71 m (walkable) from the pit and Spawn A is 140 m. In a one-round mode a side advantage decides matches. The grey box has to log capture rate by side, and move the pads if one side wins more than 55% of matches.

**Traversal without the Spool.** The Yard's high-tier routes (Climber Slingshots to the Rails, Scrapper hops onto mesas) assume the Spool. Without it, every vertical route is a ladder or rungs. The grey box has to confirm every zone is still reachable and that the Rails are not a dead end.

## 10. Making it work in practice

### 10.1 Codes never leave the server unless they could be seen

The biggest risk to any read-to-kill game is a cheat that reads every code from memory. So a client is never sent a code. It is sent a pip only while that client could actually see it:

- inside the plate's ±55° cone,
- in range (20 m, or 50 m with the sighting level raised),
- with a clear line of sight to the plate,
- not blocked by dust, grease, heat haze, or glare from that viewpoint.

Otherwise the client draws a scuffed, blank plate. This is the same approach as fog of war in tactical shooters.

- **Cost.** 8 players × 7 others × 3 plates at 10 Hz is about 1,700 line traces a second. That is trivial for the server.
- **Pop-in.** Reveal a little early (cone +10°, range +5 m) so a pip is already drawn by the time it is on screen and readable.
- **The read board** is filled by the server from exactly these reveals, so it is always honest.

### 10.2 Catching cheaters for free

The server knows which pips each player was ever shown. For each call it can count how many of the target's three pips the caller had been shown since the target last re-stamped. A clean call with zero pips shown should happen 1 in 64 times, and with one pip 1 in 16. Accounts far above those rates are reading memory or auto-typing. Flag them for review.

### 10.3 Fair calls over latency

- Validate the send with the same lag compensation as any hit registration: rewind to the caller's view time, at most 150 ms.
- Track the 0.6 s lock on the server, with a 100 ms grace for a target flickering at the edge of an occluder.
- A call is one small event and a 0.6 s window. That is far less demanding over latency than the continuous line forces it replaces, and it retires the roadmap's hardest risk (identical heavy-versus-light pulls on both clients).

### 10.4 Readability on every screen

- A pip must be at least 12 px tall at 20 m on a 1080p screen at the default 100° FOV. Size the plates to that, not to taste.
- Pips are white on a dark plate with a thin dark outline, so they read on bright daylight backgrounds.
- At higher FOVs the sighting level is the answer, not bigger pips.

### 10.5 Where the code lives

Codes, reveal sets, call resolution, bad calls, and re-stamps are pure rules. They belong in KineticCore, beside the line solver: engine-free, unit-tested on a Mac without Unreal. The match layer's event schema gets four events (Revealed, Called, TaggedOut, Restamped), and the determinism recorder logs every call so disputed tag-outs can be replayed.

## 11. What changes in canon

| Canon | Change | Status |
|---|---|---|
| 05 pillar 1: "If a thing hurts a player, it is because mass met velocity." | A clean call Stalls without an impact | **Conflict.** Lore covers it (a lockout, not an injury). Alternative: a clean call freezes the target's harness rigid for 2 s, and the arena does the killing. Slower, truer to 05, less adrenaline |
| 05 section 3 (the Spool) | Not used in this mode. The fire input becomes Call | Open: keep the Spool for traversal only (self-dog moves, no lines on players) |
| 05 section 4.4, Cutter melee | Adds a 70° spin to the 0.3 s stagger | |
| 05 section 5.5, Catch Plates | Plates carry pips. Dogs no longer seat in them in this mode | |
| 05 section 9.1, Claim row | Replaced by this mode | **Name clash.** If the old marker Claim stays, this mode needs another name (Lockout, Tag-Out) |
| 05 section 9.4, Claim marker | Unused in this mode | |
| 05 section 11 | Adds the tells in section 8 | |
| docs/arenas/the-yard/overview.md and lighting-and-mood.md: night shift, overcast, sodium glow | Daylight, fixed sun | The game Yard is lit for night today (game/Tools/unreal/build_yard_greybox.py) |

## 12. Tuning intent

- A regulation match averages 6 to 9 minutes. About 70% end with a plant before overtime.
- Elimination mix: about half by Call, half by the arena (falls, Haulers, Gantry, the press). If Calls pass 70%, the arena has stopped mattering: shorten call range. If they drop under 30%, reading is too hard: widen the pip cones.
- About 60% of clean calls are made with all three pips known. If gambles on two pips pass 50% of calls, bad calls are too cheap.
- Lock-escape rate about 40% at launch, tuned by the lock time (0.6 s), as 05 tunes the panic window by the Haul ramp.
- A carrier survives from grab to plant about 25% of the time. Most captures should take two or three attempts and at least one pass.
- Re-stamps: roughly once per life for a median player. If nobody re-stamps, bad calls cost too little. If everyone re-stamps constantly, the lockers are too safe.

## 13. Variant: Weigh-In

v0.1's elimination, kept as a hardcore variant or a separate mode for players who like deduction. Instead of pips, each rigger's secret is 0 to 20 kg of kit on top of their public class base. You eliminate them by calling their exact total weight (±1 kg), which you work out from physics: footstep pitch, needle gauges at midspan of every bridge, ladder creaks, truck weighbridges with public displays, and a strain gauge. Picking up the 25 kg beacon re-rolls the carrier's kit weight. Everything in sections 3.1 to 3.4 and 5 to 10 carries over with weight in place of pips.

It is slower and more analytical than Tag-Out, which is why it is no longer the main elimination.

## 14. Open questions

1. **Stall on a clean call, or freeze and let the arena kill?** Section 11, first row.
2. **Must your own beacon be home to plant?** No is better against stalemates. Yes is classic and rewards defense.
3. **4v4 or 3v3?** 4v4 gives a runner, a turner, a reader, and an anchor. 3v3 is canon and cheaper.
4. **Three pips or four?** Three from 1 to 4 is 64 codes. Adding a chest pip would make facing unsafe and remove the mode's central rule, so a fourth pip would have to go somewhere else (a helmet top, readable only from above, would make the Rails a reading platform).
5. **Should the Spool stay as traversal?** It would restore the Yard's vertical routes but bring back part of the netcode risk this pivot removes.

## 15. Grey-box plan

The question to answer first: are circling, faking, and backing up against walls tense with no art at all? Build the smallest test that answers it.

1. **One Canyon gap, 1v1.** Plates, pips, the call strip, the lock, bad calls, the Cutter spin. No beacons, no gadgets. If the duel is just two people staring at each other, add a second turning tool before going further.
2. **2v2 in the Yard.** Add the read board, the sighting level, and the tag lockers. Measure call range, lock-escape rate, gamble rate, and elimination mix.
3. **4v4 Claim.** Add beacons, regulation only. Measure match length, capture rate by side, carrier survival.
4. **Daylight tools.** Glare, dust, grease, tarps, heat haze. Check that each gets used at least once per match, and that none is mandatory.

Steps 1 and 2 need only the server-side visibility gate (section 10.1), the call event, and Stall from the existing KineticCore match layer.
