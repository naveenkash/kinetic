# Marshal

_Office of the Lay referees. They watch from the overhead gallery and call the match. Their voice is the match announcer. They are bored, fair, and have seen everything._

## Appearance

Three Marshals per match in the gallery; one is the calling Marshal and the voice. Office uniform, not harness: they are never on the floor.

| Element | Spec |
|---|---|
| Height, build | Mixed. Three body variants, three head variants, randomized per match (**PROPOSED**). |
| Coat | Knee-length Office grey `#6B6E72` work coat, roughness 0.85. Office crest on the left breast, 6 cm. |
| Armband | Hi-vis yellow `#E6FF2A` on the left upper arm, MARSHAL in black. The only bright thing on them. |
| Headset | Single-ear boom mic, black. The calling Marshal's is lit with a small white LED when live. |
| Hands | A clipboard or a tablet in a rubber case; a thermos. |
| Face | Tired. Reading glasses on two of the three variants. No faction color anywhere. |

They are always 15 to 40 m from any player and are seen through gallery glass or over a rail. Detail budget goes to the silhouette and the armband.

## Behavior

- Stand or sit at the gallery rail. Track the nearest live line with the head. Lean forward on a Dog seat on a player. Lean back on a cut.
- The calling Marshal raises a hand on "Live" and drops it on a round end.
- On a Stall, one Marshal writes. On a Well, one Marshal looks down.
- On the Pulse countdown, all three put a hand on the rail.
- Never react to the audience, sponsors, or players' Rig Calls, with one exception: a rude Rig Call aimed at the gallery gets a slow look from one Marshal (**PROPOSED**).

## Animation needs

| Clip | Notes |
|---|---|
| Idle set | Stand at rail, sit, lean, drink, write. 6 to 10 s loops. |
| Track | Head and upper body aim toward a target, procedural. |
| Call live | Hand up, 1 s. |
| Call round | Hand down, 1 s. |
| Count | Fingers for the Pulse count, 10 to 1 (**PROPOSED**). |
| Hand on rail | Pulse brace, 0.5 s. |
| Look down | Well, 2 s. |
| Slow look | The rude Rig Call response, 3 s. |

## Audio

The calling Marshal is the match's voice. Delivery is flat, unhurried, slightly amplified, through gallery speakers with a short slapback. Lines are short. Never excited.

| Trigger | Line |
|---|---|
| Round start | "Live." |
| Stall | "Scrapped." Then the crew color: "Scrapped, amber." |
| Ring-out | "Down the Well." |
| Mid-match event, 5 s siren | "Shear in five." "Shift change." "Cable Day." "Scaffold." |
| Pulse | "Pulse in ten." Then "five, four, three, two." The last second is the shove. |
| Round end | "Round." Then the crew: "Round, sky." |
| Match end | "Called." |
| Claim marker in a Well | "Marker reset." |
| Stall under a rated load | "Rated." (said before "Scrapped," flat) |
| Forfeit (broken telemetry seal) | "Seal. Forfeit." |

A second voice set in a different register ships for variety; the Office has more than one Marshal. Gallery ambience: paper, a chair creak, a thermos lid.

## Technical

| Item | Spec |
|---|---|
| Mesh | One skeletal mesh, 12k tris LOD0, 4k LOD1, 1k LOD2. Three material variants. Shares `SK_Rigger` for animation reuse; no harness sockets populated. |
| Gallery | A non-dog-able volume (05 §3.1): Dogs bounce off the gallery glass with a tick. Players cannot enter it. |
| Replication | Marshal poses are driven by replicated match events, not simulated; clients animate locally. |
| Voice | Match-event-driven, server-authoritative order, 2D mix with a positional tail from the gallery. |

## Reference search terms

boxing judge at ringside, air traffic controller headset, railway signal box operator, 1970s sports official grey coat, tired court clerk, press box at a demolition derby.
