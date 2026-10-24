# THE ANCHOR: Overview

_Arena: The Anchor. File purpose: story, stakes, footprint, tiers, modes, and the one-sentence thesis._
_Sources: docs/bible/03-arenas.md (The Anchor), docs/bible/05-core-mechanics.md (sections 2, 3, 6, 7, 9, 11). Inventions are marked **PROPOSED**._

## Thesis

**One sentence:** The Anchor is a map where the ground itself breathes and shoves on two timers, so the only safe place is the one everybody wants, twenty meters across, sixty meters up.

## Story

The Meridian Anchor is the artificial island on the equator where the Strand began. The socket that held the cable to the planet is a kilometer-wide bowl of concrete and Weave. At its center stands the Stump: the last three hundred meters of Strand still rooted in the seabed, still under residual tension, humming at a frequency riggers claim they can feel in their teeth. Under it is the Well, the shaft where the root goes down into the sea floor.

The Accord says whoever holds the Anchor Claim holds the right to re-raise the Strand from this socket. Three Second Strand projects want it. The Ascendancy believes standing on the Crown is prayer. HTR believes the Stump is the most valuable billet on Earth. The Guild believes nobody should be up there until someone has read the harmonics logs. The Hangers and the Fray fight it for the purse and the footage. Nobody has held the Anchor two seasons running.

A contractor scaffold wraps the Stump five rings deep. It was put up to strip the Stump's cladding, and it is half done: the upper face is bare Weave, the lower face is still plated. The cutting crew's torches, racks, and charges are exactly where they left them at the shift horn. Rim cranes were hoisting ballast slabs to the Crown for a survey rig when the Office called the match. The slabs still hang over the Well.

## Location in the Lay

The west end of the Lay, offshore. The island is reached by the Office's ferry. The bowl's far rim is a kilometer away and is vista only. The playable arena is a 120 m diameter disc centered on the Stump: the bowl is modeled as vista and the socket's inner Rim at radius 40 to 60 m is the playable outer ring. Spawns are on the Rim's two crane decks (05 section 9.3).

## Who contests the claim

The seasonal Anchor Claim is open to all five factions. Per docs/bible/04-progression.md, the winner gets its colors on the Rim cranes and its Face on the Crown's broadcast board. Nothing else changes, and nobody has kept it twice. The default dressing before any season result is Office grey with LayCast livery.

## What the player should feel

Exposure. Every tier is a ledge over a drop that does not end. The Stump hums under everything, the hum rises before the Pulse, and it drops an octave when someone goes down the Well. On the Scaffold you are inside a cage of planks with a 4 m Well mouth at your heels. On the Crown you are on a table in the wind with the whole bowl watching. The Breath lifts light harnesses off the inner edge; the Pulse is a one-second storm that takes everyone unbraced off their feet and throws the deck's loose tools outward. The Anchor is the map where a rigger counts timers aloud, and where the Ascension clip comes from.

## Height resolution

03-arenas.md puts the Crown 300 m up. Map rule 3 requires every tier to be reachable in one Slingshot. Ruled canon in 05 v0.2.2 (this layout.md is the geometry authority): the Stump is 300 m tall measured from its root at the bottom of the Well. The Rim deck of the socket sits 240 m up that height. The Crown is therefore 60 m above the Rim. Both facts hold: the Stump is three hundred meters of Strand, and the Crown is one Slingshot above the top of the Scaffold.

## Footprint and tiers

| Measure | Value |
|---|---|
| Footprint (X by Y) | 120 m by 120 m (disc, radius 60 m) |
| Lowest playable floor | Bowl floor, Z = -8 m (datum: Rim deck = 0 m) |
| Highest playable floor | Crown, Z = 60 m |
| Total playable height | 68 m |
| Well kill volume | Cylinder radius 16 m, below Z = -15 m |

Tier heights. The first column is the datum used in every Anchor file (Rim deck = 0 m). The second column is the height above the lowest playable floor as the brief asks.

| Tier | Z (Rim datum) | Above lowest floor | Role |
|---|---|---|---|
| Bowl floor | -8 m | 0 m | Dead-end pit between Rim and Scaffold |
| Rim | 0 m | 8 m | Low tier. Open, sees everything |
| Crane decks (spawns) | 10 m | 18 m | Spawn platforms on the two rim cranes |
| Scaffold L1 to L5 | 6, 14, 22, 30, 38 m | 14 to 46 m | Mid tier. Main fighting volume |
| Maintenance Ring | 42 m | 50 m | Rotating gantry, top of mid tier |
| Crown | 60 m | 68 m | High tier. Claim marker |

## Modes and player count

| Mode | Supported | Notes |
|---|---|---|
| Claim | Yes, primary | Marker authored at the Crown center (0, 0, 60). Per 03. |
| Scrap | Yes, primary | Best of 5, 2:30 rounds per 05 section 9.1. Sudden death: "Held Breath" (**PROPOSED**, see events-and-moving-parts.md). |
| Haul | No | There is no truck route and the 2,000 kg spool would spend the match in the Well. |
| Free Scrap | Possible | 8 solo fits the Rim plus Scaffold. Not a launch target. |

Player count: 3v3. 3v3v3 is possible on the Rim's three-fold symmetry (crane N, crane S, gallery E) but is not the launch target.

## Round-length expectations

- Scrap: 05 section 12 targets 90 s per round between even crews. The Anchor should run long, 100 to 120 s, because the Pulse (every 75 s from 0:55, 05 v0.2.2) takes every unbraced player off their feet for a second and the Scaffold Collapse at 1:15 remaining forces a reset of every engagement. Rounds that reach the second Pulse (130 s elapsed) should be rare and decisive.
- Claim: a full 8:00. Expect the marker to change hands at every Pulse (every 75 s) and Breath peak (every 40 s). A crew that holds the Crown through two consecutive Pulses has effectively won.

## Dominant class and best modes

Climber first: the Ascension puts a Climber on the Crown from L4 or L5 in one move. Rigger second: two set lines are two anchors, and an anchored rigger on the Pulse is the deadliest thing on the Scaffold. Breakers are slow here and, unbraced, leave their feet at 5 m/s2 like a Rigger (05 section 5.3); braced, a Breaker is the only thing the Pulse does not move. Best modes: Claim, then Scrap.
