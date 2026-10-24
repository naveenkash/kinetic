# THE ANCHOR: Events and Moving Parts

_Arena: The Anchor. File purpose: the Breath, the Pulse, and the Ring cycles second by second; the Scaffold Collapse; the Scrap sudden-death event; every tell with lead time; reset rules._
_Sources: docs/bible/03-arenas.md (The Anchor, map rules 4 and 5), docs/bible/05-core-mechanics.md v0.2.2 (2.3, 3.2 Reel and Kick interlock, 3.5, 3.6, 5.3, 5.4, 6.2, 6.2a, 6.3, 6.6, 6.7, 7.4, 9.1, 9.2, 11). Inventions are marked **PROPOSED**. Geometry authority: layout.md (Rim 0, Scaffold 6 to 38, Ring 42, Crown 60)._

Heights use the Rim datum (Rim deck Z = 0). Times are seconds elapsed from the Marshal's "live" unless marked as remaining.

## Force volumes on this map (05 section 6.6)

Force volumes act on players and loose objects only. Racked, stacked, bolted, chained, rated, and hanging objects are unaffected. Every profile below is authored on a 100 kg reference body; a body of mass m feels a_ref times 100 over m.

| Body | Mass (kg) | Factor | Foot-hold (m/s2) | Braced |
|---|---|---|---|---|
| Climber | 90 | 1.11 | 3 | |
| Scrapper | 110 | 0.91 | 4 | |
| Rigger, Doubler | 130 | 0.77 | 5 | Rigger 9 |
| Breaker | 220 | 0.45 | 5 | 20 |
| Propane cylinder (loose) | 45 | 2.22 | | |
| Clamp crate (loose) | 25 | 4.0 | | |
| Claim marker (runners) | 900 | 0.11 | | |

A body is airborne when its felt acceleration is at or above its foot-hold (inclusive). Heavy objects feel so little that they never lift and barely roll; Light objects feel so much that they hit the 50 m/s cap.

## The three cycles at a glance

| System | Period | Window | What it does | Who stays on their feet |
|---|---|---|---|---|
| The Updraft (the Breath) | 40 s | full 32 to 37 s | Lifts light harnesses and loose props from the inner half of every deck | Breaker at r over 17.7; Rigger and Doubler over 20.3; Scrapper over 21.5; Climber over 22.5; braced Breaker anywhere |
| The Pulse | 75 s from 0:55 (05 v0.2.2) | 0.4 s shove, 0.6 s inhale | A one-second storm: every unbraced player airborne, every loose prop thrown outward then dragged back | Braced Breaker anywhere; braced Rigger at r over 23.5; everyone on the Rim and crane decks |
| The Maintenance Ring | 120 s | continuous | Carries riders; severs every line it sweeps | Everyone, until the west void |

## The Breath: 40 s cycle, second by second

Upward reference acceleration a_ref(r, t). Core (r 12 to 16): peak 14 m/s2. Falls linearly to 0 at r 24, the Scaffold's outer rail. Above the Crown (Z over 60) the core also falls inward to 0 at r 8, so the Crown center is calm (**PROPOSED**). Vertical extent Z 0 to 70. Below the Rim datum the Well shaft is still air (**PROPOSED**).

| t (s) | Phase | Core a_ref | Felt at r 20: Climber / Scrapper / Rigger / Breaker | Tell | What happens |
|---|---|---|---|---|---|
| 0 to 26 | Calm | 0 | 0 | Hum steady | Nothing |
| 27 | Tell | 0 | 0 | Intake roar from the Well. Chalk dust and ribbon fragments lift 0.2 m. Banners rise. | 1 s lead per 05 section 11 |
| 28 | Building | 0 | 0 | Roar rising | Ramp begins |
| 29.5 | Building | 5.3 | 2.9 / 2.4 / 2.0 / 1.2 | | Loose Light props at the inner edge are already airborne |
| 29.6 | Building | 5.6 | 3.1 | | Climbers at r 20 leave the deck |
| 30.5 | Building | 8.8 | 4.9 / 4.0 / 3.4 / 2.0 | | Scrappers at r 20 leave the deck |
| 31.7 | Building | 13.0 | 7.2 / 5.9 / 5.0 / 2.9 | | Riggers and Doublers at r 20 leave the deck |
| 32 | Full | 14 | 7.8 / 6.4 / 5.4 / 3.2 | Roar steady, pitch up | Breakers at r 20 hold (3.2 under 5). Unbraced Breakers inside r 17.7 are airborne |
| 32 to 37 | Full | 14 | | | Net lift at the core (felt minus g): Climber +5.7, Scrapper +2.9, Rigger +1.0, Breaker -3.4 m/s2. A Climber in the core rises 71 m in 5 s, a Scrapper 36 m, a Rigger 12 m. A Breaker is off its feet and sinking: helpless, not rising |
| 37 | Dying | 14 | | Roar falling | Ramp down begins |
| 38.5 | Dying | 7 | | | Lifted bodies start to fall |
| 40 | Calm | 0 | 0 | Hum | Everything lifted is falling. Props land from 40 to 44 s |

Where it lifts and where it raises, at full:

| Class | Off their feet at r under | Rising (felt over 9.81) at r under | Braced |
|---|---|---|---|
| Climber | 22.5 | 19.0 | |
| Scrapper | 21.5 | 17.8 | |
| Rigger, Doubler | 20.3 | 16.7 | Rigger braced: off feet under r 17.3, never rises |
| Breaker | 17.7 | never | Braced: holds anywhere |

03's "light harnesses get lifted" is now literal. The inner 3 m of every deck sends a Climber up and only unsettles a Breaker. The outer 1.5 m is safe for every class.

Loose props: a 45 kg cylinder feels 31 m/s2 at the core and 15 at r 20; a clamp crate feels 56. Everything loose inside r 22 is in the air by 30 s and comes down from 40 to 44 s, from heights of 20 to 70 m. A loose cylinder landing from 20 m detonates. Props that pass Z 70 despawn (**PROPOSED**).

### The Ascension, per class

Self-dog A, Dog B the deck under you, Jack (the Kick, which fires because Dog B is in the deck, not a player), aiming 10 degrees inward of vertical, from the inner edge of L4 or L5 (r 16). Net upward acceleration after the Kick is the push T over m, plus the felt Breath, minus g.

| Class | Rise with no Breath | From L5 at full Breath: net accel, time to the lip (22 m), speed at the lip | Verdict |
|---|---|---|---|
| Climber | 30 m | +10.2 m/s2, 1.0 s, 28 m/s | Works any time from L5 (lip at 9 m/s in calm, Perch). From L4 launch at 29.5 to 31 s during the build and arrive at 8 to 12 m/s. At full you overshoot to about Z 100 |
| Doubler | 27 m | +7.2, 1.2 s, 23 m/s | Works from L5 in calm (lip at 7 m/s, land on the Crown edge). At full, overshoot |
| Scrapper | 16 m | +6.5, 1.2 s, 22 m/s | Full Breath only. Push ends at 20 m of line; the Breath carries the rest |
| Rigger | 14 m | +5.6, 1.4 s, 20 m/s | Full Breath only |
| Breaker | 9 m | +1.1 until the push ends at 19 m, then -3.4; 2.0 s, 11 m/s | Full Breath only, launched at 32 to 35 s, and only just. No Perch: must arrest by Hang |

Arrest: pass the lip under 12 m/s and Perch (Climber) or mantle the lip with an inward lean (everyone else), or Dog the Crown surface, Hang, and Reel. A Hang engaged above the Crown holds you at that rod length; hold Haul to Reel down at 1 m/s, or release when under 7 m. Overshooters who Hang at the apex hang on a 30 to 40 m rod over the Crown for 30 s of Reel: a target the whole way. Any line that passes through the Ring's sweep (r 14 to 20 at Z 42) is severed when the Ring reaches it (05 section 3.5); a Jack line from L5 to a rising Climber is in that volume for its first 4 m.

Dogs fired across the full Breath drift upward about 1 m per 20 m of flight (**PROPOSED**). Aim low.

## The Pulse: 75 s period, 1.0 s long

Radial horizontal force volume. Canon per 05 v0.2.2: every 75 s from 0:55, a 0.4 s outward shove, then a 0.6 s inhale toward the Stump. Reference magnitudes (arena-authored, **PROPOSED**): shove 16 m/s2 at r 16 and inward of it, falling linearly to 0 at r 44; inhale 8 m/s2 everywhere inside r 44; on the Crown 0 inside r 3 (the eye), rising to 16 at r 8 (**PROPOSED**). Vertical extent Z -15 to 70.

| t (s) | Event | Tell |
|---|---|---|
| -5.0 | Marshal: "five" | Hum pitch begins to climb. Dust sheets off the Stump face. |
| -4 to -1 | "four, three, two, one" | Each count, the Stump's faint glow pulses brighter. |
| 0.0 to 0.4 | Shove | Bass thud. Every unbraced player inside r 40 leaves their feet. Every loose prop inside r 44 lurches outward. |
| 0.4 | Whip-crack | |
| 0.4 to 1.0 | Inhale | Reverse whoosh. Everything airborne is dragged back toward the Stump. |
| 1.0 | Done | Hum drops back to base pitch. Bodies land. |

Felt accelerations and displacement for a player standing at the Scaffold's inner edge (a_ref 16 then 8):

| Class | Shove felt | Speed after 0.4 s | Inhale felt | Speed at 1.0 s | Total travel | Outcome |
|---|---|---|---|---|---|---|
| Climber | 17.8 | 7.1 m/s out | 8.9 | 1.8 m/s out | 4.1 m outward | Lands mid-deck, no stagger |
| Scrapper | 14.5 | 5.8 | 7.3 | 1.4 | 3.3 m | Same |
| Rigger, Doubler | 12.3 | 4.9 | 6.2 | 1.2 | 2.8 m | Same |
| Breaker, unbraced | 7.3 | 2.9 | 3.6 | 0.7 | 1.7 m | Same |
| Breaker, braced | 7.3 under 20 | 0 | | 0 | 0 | Never moves |
| Rigger, braced | 12.3 over 9 | airborne | | | as unbraced | Holds only at r over 23.5 |

A player at the outer rail is pinned to it for 0.4 s, then inhaled: a Climber comes off the rail at 5.3 m/s inward and lands about 3 m in, at r 21. Nobody standing on a deck is thrown into the Well by the Pulse, from anywhere. Nobody standing on a deck reaches the outer rail from inside r 19. The Pulse is a one-second loss of footing for everyone unbraced, and one free second for whoever is braced or Hung.

Who holds, by radius (felt at or above foot-hold is airborne):

| r (m) | a_ref shove | Unbraced holds | Braced holds |
|---|---|---|---|
| 16 to 24 (Scaffold) | 16 to 11.4 | Nobody (Breaker feels 5.2 at the rail, inclusive threshold) | Breaker anywhere; Rigger at r over 23.5 |
| 14 to 20 (Ring) | 16 to 13.7 | Nobody | Breaker |
| 24 to 40 (gap, gangways) | 11.4 to 2.3 | Breaker beyond r 24.8; Rigger, Doubler beyond 32.6; Scrapper beyond 36.3; Climber beyond 39.3 | Both beyond r 24.8 |
| 40 to 60 (Rim, crane decks) | 2.3 to 0 | Everyone | Both |
| Crown eye, r 0 to 3 | 0 | Everyone | Both |
| Crown r 3 to 8 | 0 to 16 | Climber to r 3.8; Scrapper 4.4; Rigger, Doubler 5.0; Breaker 6.4 | Rigger to 6.7; Breaker anywhere |
| Crown r 8 to 10 | 16 | Nobody | Breaker |

**Who goes into the Well, and from where:**

| Victim | Where | Why |
|---|---|---|
| Anyone airborne in or over the Well mouth at the inhale | Ascending, Pendulum swinging, hoisted, Reeling up to the lip, or mid-Slingshot past the face | The inhale drags them into the Stump face at up to 5 m/s (stagger). A Climber Perches; anyone else falls unless already Hung. |
| Anyone on a rod pivoted at a braced Breaker at the inner edge | The Held Line (signature-plays.md) | The shove swings them out, the inhale swings them back over the mouth. |
| Climbers and Scrappers standing within 1.5 m of the Crown lip (r 8.5 to 10) | Crown rim zone | They reach the 1 m lip at 6 to 7 m/s and flip over it (**PROPOSED**: the lip flips a ragdoll arriving at or above 6 m/s, as a Hang line does). Riggers and Breakers arrive under 5 m/s and are caught by it. |
| Anyone hoisted by a Reel whose anchor is over the mouth | Gallows from the boom tips | The inhale swings the hung body into the face; the hoister cuts. |

**Loose props are the Pulse's real weapon.** A loose cylinder at r 18 feels 36 m/s2: 14 m/s outward in 0.4 s, 2.8 m traveled, then 18 m/s2 of inhale for 0.6 s leaves it at 3.5 m/s outward after 8 m: it reaches the outer rail or a board stack at over 8 m/s and detonates. Light props (clamp crates, tool bags, planks) feel 50 to 130 m/s2 and hit the 50 m/s cap: a 25 kg crate at 25 m/s is 625 N s, a flip for whoever is standing outboard of it. Rule for players: **Hang in, face the Stump, and have nothing loose inboard of you.** The outer rail is where the deck's tools land. Line tension under the shove is mass times felt acceleration: 1.6 kN on a Climber, 1.6 kN on a Breaker, under every class Hang load. A Rigger's set lines (12 s) hold a teammate through it.

The Claim marker, 900 kg on 0.05 runners, feels 1.8 m/s2 at most against 0.5 of friction: it creeps a few centimeters if outside the eye and does not leave the Crown.

Claim coincidence: the Pulse at 355 s lands inside the full Breath (352 to 357 s). The Marshal calls it "full exhale" (**PROPOSED**).

## The Maintenance Ring: 120 s

docs/objects/kinematics/maintenance-ring.md. Z 42, r 14 to 20. Clockwise seen from above, 3 degrees per second, 0.89 m/s at mid-width. At round start the drop ladder is at azimuth 0 (N). It passes the ladder bays at 45, 135, 225, 315 degrees at 15, 45, 75, 105 s, with a bell at each (**PROPOSED**). After the Collapse it crosses the west void from azimuth 225 to 315, which is 75 to 105 s in a Scrap round and again every 120 s.

It is kinematic (05 section 6.7) and it **severs any line it passes through** (05 section 3.5): the line snaps, both Dogs release. That includes a Reel climber's rod from L5 to the Crown lip (it crosses r 15.3 at Z 42), a Gallows from a boom tip to anyone on L5, a Guillotine set line hung from a boom tip, and the first 4 m of an Ascension's Jack line. The Ring reaches a given azimuth once per 120 s; a 22 s Reel climb from L5 has a one-in-five chance of meeting it unless timed. Lines attached to the Ring itself are not severed: Hang to it and ride; Hang an enemy to it and they ride at 0.9 m/s until the deck runs out. A braced Breaker is the only rider the Pulse does not lift.

## Scrap round timeline (150 s)

| Elapsed | Remaining | Event |
|---|---|---|
| 0 | 2:30 | Live. Ring ladder at N. Breath phase 0. |
| 27 to 40 | | Breath 1. Full 32 to 37. |
| 50 to 56 | | Pulse 1 countdown, shove, inhale. |
| 67 to 80 | | Breath 2. Full 72 to 77. |
| 70 | 1:20 | Collapse siren. |
| 75 | 1:15 | Scaffold Collapse. Ring crosses the void 75 to 105. |
| 107 to 120 | | Breath 3. Full 112 to 117. |
| 125 to 131 | | Pulse 2. |
| 147 | 0:03 | Breath 4 tell. Round ends before it builds. |
| 150 | 0:00 | Time. Tie: Held Breath. |

Claim (480 s): Pulses at 55, 130, 205, 280, 355, 430. Collapse at 240 s (4:00 remaining per 05 section 9.2), a calm Breath phase. Breath full windows every 40 s from 32 s. Free Scrap (360 s): Collapse at 180 s (3:00 remaining).

## Mid-match event: Scaffold Collapse

Fires with a 5 s siren per 05 section 9.2. The west quadrant (azimuth 225 to 315) fails outward.

| T | What happens |
|---|---|
| -5.0 | Siren. The W quadrant's outer edge drops 0.3 m on all five levels. Planks slide outward. Dust from the four W clamps. |
| -3.0 | Sag 0.6 m. Loose props on W decks slide to the outer rail. The NW ladder bay's cage rattles. |
| -1.0 | The W clamps groan (rated-load tell, 1.0 s). Sag 1.0 m. |
| 0.0 | Clamps shear L5 first, then L4, L3, L2, L1 at 0.2 s intervals (**PROPOSED**). The quadrant rotates outward about its footing. |
| +1.5 | Decks pitch past 40 degrees. Every player on a W deck is sliding or airborne. Dogs seated in W sections stay seated (the sections are the largest fragments). |
| +3.0 | The upper levels hit the Rim west arc (r 40 to 48). Sections fracture at 800 N s into Light boards, Light clamps, and one Medium frame piece each. The W gangway (plank bridge, 600 N s) is crushed. |
| +5.0 | The lower levels hit the bowl floor. The NW ladder bay is gone. |
| +10 | Debris settles. Fragments despawn over 10 s except the authored pile. |

After, zone by zone:

| Zone | After the Collapse |
|---|---|
| Scaffold W | A cliff face: bare Stump with 20 clamp stubs. Three surviving ledges, 3 by 2 m: Z 14 at azimuth 240, Z 30 at 270, Z 38 at 300 (**PROPOSED**). Perches for Climbers, nothing else. |
| Scaffold N, E, S | Unchanged, but each now has an open end at the west seams: a 38 m drop with no rail at azimuths 225 and 315. The SW ladder bay (225) is the cliff's edge. Overboard along the deck toward an open end needs no rail speed at all. |
| Ladder bays | NW (315) destroyed. Three remain. |
| The Ring | Unchanged. For 30 s of every revolution its outer edge is over a 42 m void. |
| Gangways | W buried. Three remain. |
| Rim W | r 40 to 48, azimuth 240 to 300, under 1 to 3 m of scaffold debris. Surface counts as scrap heap (x2.0, **PROPOSED**). The W alternate spawn moves to (-50, -20, 0). |
| Bowl floor W | Debris, same surface rule. |
| The Breath | The west 90 degrees of the column has nothing in it. A body lifted there rises clean. |
| The Pulse | Unchanged in profile. A loose prop thrown outward at an open seam goes into the void instead of the rail. |
| Crown, Rim N E S, cranes | Unchanged. The Crown now sees the whole west Rim. |

Ring-out share will rise after the Collapse. 05 section 12 caps ring-outs at 35%; if the west void pushes the map over, add rail at the open seams, not floor.

## Player-triggered structural changes

Rated loads move for a Breaker's 1,000 N alone, a Doubler at overspin, or any lines summing to 1,000 N on one axis: a Rigger and a Scrapper Hauling together, or two Riggers (05 v0.2.2).

| Trigger | Who | Tell | Consequence |
|---|---|---|---|
| Haul a maintenance clamp (rated) | Breaker; Doubler overspin; a crew summing 1,000 N | Groan, dust, planks shiver, 1.0 s, then 0.17 to 0.27 m/s2 of visible creep for several seconds | That quadrant's deck at that level drops onto the level below. Sections fracture. Players on it fall one level (8 m onto planks: a flip, since planks need 14.4 m/s to Stall) or further if the next level is already gone. |
| Hit a west-clamp charge at 6 m/s (L1 W, L3 W) | Anyone, with a thrown prop or a body. Dogs seat in a charge without triggering it. | Hiss, 0.3 s, then blast 5 m, 2,500 N s | Same as a clamp pull at that level, plus the blast: Stall inside 2.6 m, flip inside 3.8 m. The Collapse still fires on whatever is left. |
| Haul a crane slab (rated) | Breaker; Doubler overspin; a crew summing 1,000 N | Brake squeal, hum stop, 1.0 s | The slab free-falls down the Well mouth. Passes L5 at 1.9 s, Rim datum at 3.3 s, kill volume at 3.8 s. Anything Hung to it follows. Resets between rounds only. |
| Hit a tower-leg charge at 6 m/s | Anyone | Hiss 0.3 s, blast, then a 2 s lean with lattice groan and flickering floods (**PROPOSED**) | The tower falls toward the Stump, top landing on L1's outer rail. The lit sector (NE or SW) goes dark. The fallen lattice is a 31 m ramp from the Rim to L1 for anyone. |
| Jack the gas manifold over 600 N | Doubler (800 N), Breaker (1,000 N); the Kick fires (Dog B is in geometry) | Hiss 0.3 s | 6 m jet at 15 m/s for 3 s along the footbridge, outward. Anyone on the footbridge is pushed onto L5 S or into the Well mouth. |
| Break a gangway (600 N s) | A falling Heavy, a blast, or a Collapse section | Planks crack | The gangway drops into the gap. Three or fewer remain. |

## Sudden death: Held Breath (PROPOSED)

Scrap time-out with equal riggers. The Marshal calls "held breath". 5 s siren.

1. The Breath locks at full (14 m/s2 reference at the core, 0 at r 24) and does not cycle.
2. The Pulse fires every 15 s with a 3 s count.
3. A cordon, the Office line, contracts from r 60 to r 24 at 1 m/s over 36 s and stops at the Scaffold's outer rail. Anyone outside it is scrapped by the Marshal's call. Rendered as a wall of LayCast light.
4. Loose props carried past Z 70 despawn; the decks are clean of loose props within 15 s.

Where anyone can stand once the cordon closes: the Crown eye (three capsules); the outer deck strip (Breaker beyond r 17.7, Rigger and Doubler beyond 20.3, Scrapper beyond 21.5, Climber beyond 22.5); the Ring for a braced Breaker only (a Climber on the Ring at r 17 feels 13.6 m/s2 and rises). Every 15 s the Pulse takes the unbraced off their feet for one second. A Climber can Ascend at will, and the Crown eye is where the match ends. First crew to lose a rigger loses. No time cap; it ends inside 90 s in practice.

## Tells (05 section 11 minimums)

| System | Audio | Visual | Lead |
|---|---|---|---|
| Breath | Intake roar from the Well, pitch rising | Dust, chalk, ribbon fragments lift; banners rise | 1 s to building, 5 s to full |
| Pulse | Marshal count from five; hum climbs | Dust sheets off the Stump; glow pulses per count | 5 s |
| Inhale | Reverse whoosh | Dust reverses | Follows the shove (0.4 s) |
| Ring | Motor drone, bell at each seam | Shadow sweeps the deck | 10 s shadow, continuous |
| Ring severing a line | The snap | Line flash | 0 s; the shadow is the warning |
| Ring drop ladder | Chain rattle | | 3 s |
| Clamp pull | Structural groan, then creep | Dust, planks shiver | 1.0 s, then several seconds of creep |
| Slab release | Brake squeal, hoist hum stops | Hoist line jitters | 1.0 s |
| Charge armed | Hiss | Venting | 0.3 s |
| Tower fall | Lattice groan | Lean, floods flicker | 2 s after the blast |
| Manifold vent | Hiss | Vapor | 0.3 s |
| Scaffold Collapse | Siren | Sag, outer edges dropping, dust | 5 s |
| Well | The Stump's hum drops an octave around the faller (05 v0.2.2). No Hooks (03, 05 section 7.4) | Glow | 0.5 s |
| Reel | Ratchet click, 1 per meter | Rod shortens | continuous |
| Held Breath | Marshal call, siren | Office line appears | 5 s |

## Reset rules

- Scrap, between rounds (05 section 9.2 step 5): everything. The west quadrant is rebuilt. Towers stand. Charges re-arm. Slabs re-hang. Clamps reseat. Props return to authored positions. Set lines clear. Cycles restart at phase 0: Breath at 0, Ring ladder at N, Pulse timer at 0. Prop pools do not replenish within a round.
- Claim, single round: no reset. Fallen rated loads stay down (05 section 6.3). The marker returns to (0, 0, 60) 3 s after entering the Well (**PROPOSED** delay). Props that enter the kill volume despawn. Props lifted past Z 70 despawn (**PROPOSED**).
- Scrapped bodies follow 05 section 7.4 everywhere except the Well: a body that goes down the Well is not retrieved by a Hook. It falls out of sight and the Marshal's call is the only closure.
