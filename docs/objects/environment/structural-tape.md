# Structural Tape

One-line: environment, marking system, no mass, two tapes: Office rated-load tape (100 mm, yellow and black chevrons at 45 degrees) and Office slack-cable tape (50 mm, orange with a white reflective thread), applied by the Office's prep crew before every match, all arenas. A third family, harness and line tape (faction and Rope Mark wraps), is owned by the character team and listed here only so the three never collide.

## Role in play

Tape is the readability contract made physical (03, rule 7: rated loads are marked; 05 section 6.1: Rated loads are marked with yellow-and-black tape; 05 section 4.5: slack cables marked with orange tape). The rule is simple and absolute: yellow-and-black means a Breaker can move it and something authored will happen; orange means any Cutter can cut it in 0.6 s and something authored will happen; anything untaped does neither. Players must be able to read what will and won't come down from across an arena, and tape is how.

| Harness | Relationship |
|---|---|
| Climber | Reads yellow-and-black as "a Breaker will be here." Reads orange from above as the map of what can fall. |
| Scrapper | Orange is theirs. The fastest Cutter in the league and the most orange on the map. |
| Rigger | Guild ochre-and-black is the Guild's harness tape and is deliberately not a chevron, so it never reads as Office yellow-and-black at distance. |
| Doubler | Fray neon tape on lines; the Doubler is the only harness whose tape is louder than the Office's, and it is on their body, never on structure. |
| Breaker | Yellow-and-black is a to-do list. HTR orange (#E8541E) is their livery and is kept 20 degrees of hue from slack-cable orange (#FF8A00), with no white thread, on different objects. |

## Appearance

| Tape | Width | Pattern | Colors | Finish | Where |
|---|---|---|---|---|---|
| Rated-load tape | 100 mm | chevrons at 45 degrees, 200 mm pitch, black leading edge pointing toward the load's fall axis | #F2C500 / #141414 | matte vinyl, roughness 0.7, non-emissive | A continuous band around the live rated load at hand height (1.0 to 2.0 m), plus a cross on the exact Haul target socket. |
| Slack-cable tape | 50 mm | solid, with one 3 mm white reflective thread spiraling at 100 mm pitch | #FF8A00 / #F5F5F0 thread | matte, thread retroreflective | 0.3 m wraps at both ends of every slack cable, 0.1 m from each hitch. |
| Office edge stripe | 150 mm | solid | #F2C500 | matte | Deck edges over a Well, moving-part paths (kinematic paths take the redder path stripe instead; see below). |
| Office white band | 200 mm | solid | #F5F5F0 retroreflective | | At 1.0 m on every container in Ballast so the Avalanche reads on camera. |
| Kinematic path stripe | 300 mm | solid | #FF6A13 (a redder orange, no thread) | matte | The L5 inner toe-board under the Maintenance Ring's drop ladder sweep, the Lift counterweight notch, the Gantry's end-truck path on the rail walks. Kinematic, not cuttable: hence no thread and a different width. |

Weathering: tape is new every match and is the cleanest material in any arena. Where it crosses a dirty surface the surface is cleaner under it (the prep crew wipes before taping). The chevron direction is meaningful: the Guild taught the Office to point the black edge toward the fall axis. Torn tape is a state (see below), not a dressing choice; a live rated load never starts a round with torn tape.

Lighting: no tape is emissive. The reflective thread on orange tape and the retroreflective white band catch line light and flood light. Rule: tape is never brighter than a line, and is never placed on a line. Team tints (05 section 11a: Amber #E69F00, Sky #56B4E9, Rose #CC79A7, Bone #E8E6E0) are emissive and belong to lines, plates, HUD, and Dog trails only; Office yellow (#F2C500) is 15 degrees of hue from Amber and non-emissive, and the two never share an object. The Corda sponsor banner is the one place Amber appears off a line, and it is kept out of the Hangtown Mid Hang (see `sponsor-banners.md`).

Read by tier: from the top tier, yellow-and-black bands mark the two to four live rated loads per arena like pins on a map; orange marks the cuttable set. From mid, the chevron direction reads at 10 m. From close, the cross on the Haul target.

## Variants

1. **Live rated tape.** Band plus cross. Only on live rated loads this round. Dead structural objects carry no tape (a column that is structural but not live this round is untaped, with last season's tape residue as a ghost band in the dirt).
2. **Slack tape, cuttable.** On every slack cable. Four decoys per Hangtown round (laundry lines) keep the rule honest: orange always cuts; it does not always matter.
3. **Kinematic path stripe.** Redder, wider, no thread. On the floor, never wrapped.
4. **Harness and line tape (character team).** Guild ochre #B8752A and black, straight bands; Ascendancy silver; Hanger mixed; Fray neon #39FF14 and #FF2A9D; HTR none (machined plates). Rope Mark tape is a colored wrap on the Line near the Spool (04). None of these are chevrons and none are Office yellow or slack orange.
5. **Season residue.** Faded ghost bands on objects that were live in past seasons. Dressing; it tells veterans what was once rated.

## Physics

None. Decals and a thin mesh band for the wraps. The tape's only mechanical role is as the visual half of two systems: the live-rated-load authored consequence (README) and the slack-cable cut (05 section 4.5). Both systems read their state from the load, not from the tape; the tape is generated from the state.

## Interactions

- A Dog aimed at the tape cross on a rated load seats on the Haul target socket (a 0.3 m hint radius). The cross is the aim point.
- The chevrons point along the authored fall axis; a Breaker pulling within 45 degrees of that axis commits the load, and a pull against the chevrons shudders and does not. The tape is the instruction.
- Torn tape: at commit, the band splits at the Haul target with a tape-tear sound. This is the first visible frame of the lurch tell (0.2 s, 05 section 11).

## States

| State | Visual |
|---|---|
| Fresh | Clean band, crisp chevrons, cross. |
| Stressed | Band flutters, cross stretches (vertex animation) while a line is on the load under threshold. |
| Torn | Band split at the target, ends curling, from the commit onward. |
| Residue | Faded ghost band, no cross. Dead or past-season objects. |

## VFX and SFX hooks

`SFX_Tape_Tear` (commit), `SFX_Tape_Flutter` (stressed, quiet); `VFX_Tape_Cross_Stretch`, `VFX_Tape_Split`. Slack tape: `VFX_Tape_Proximity_Glow` (thread catches light 20% stronger within 5 m of a player, **PROPOSED**).

## Technical

- Decals for bands on flat surfaces; a 20-tri spline-swept band mesh for wraps on cables and posts. One 1K atlas `T_Office_Tape` (chevron, orange with thread mask, edge stripe, white band, path stripe) and a 512 `T_Tape_Residue`.
- Generation: a build-time pass reads every object's `bLiveThisRound` (rated loads), `Cuttable` (slack cables), kinematic path splines, and Well edges, and places tape from sockets `Tape_Band`, `Tape_Cross`, `Tape_Low`, `Tape_High`, `Path_Stripe`. Tape is never hand-placed. The live-load selection per round re-runs the pass at round reset.
- Chevron direction is set from the load's `FallAxis` socket rotation.
- Validation: an editor check flags any object with tape and no consequence, any consequence with no tape, any orange within 2 m of HTR livery meshes, any Office yellow within 2 m of a line spawn socket, and any kinematic path without a stripe.

## Reference search terms

hazard chevron tape 45 degree black yellow, orange barricade tape reflective thread, crane load marking tape, floor marking tape forklift path, scaffold tag wrap, race track kerb paint.
