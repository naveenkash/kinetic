# Reserve Rigger

_In modes with respawn, your crew sends the next rigger down. The reserve is the same player, the same class, and a second livery. It makes the reserve fiction real for the price of a material swap._

## What a reserve is

A reserve rigger is the player's character after a respawn in Claim (8 s timer, 3 reserves per crew per match), Haul (8 s, unlimited), or Free Scrap (10 s, unlimited) per 05 §9.1. Scrap has no reserve. The reserve drops at the crew's spawn, or the alternate if an enemy is within 15 m (05 §9.3).

Rules:
- Same class mesh, same skeleton, same Catch Plates, same silhouette. README rule 9: a reserve is a surface treatment.
- Same Face head and voice, so the player's identity and bark set do not change (**PROPOSED**; the bible says only "the next rigger" and leaves the head open).
- The **reserve livery** cosmetic slot replaces the primary livery. Default reserve liveries are faction-authored and listed below. Players can set any owned livery as their reserve.
- A **reserve mark** is added on top of the livery: a white R and the reserve number (R1, R2, R3) painted on the right shoulder plate by the prep crew, 4 cm tall (**PROPOSED**). It is the one element that makes a reserve readable as a reserve from 20 m, which matters in Claim where reserve count is a resource.

## Faction default reserve variants

| Faction | Primary livery | Reserve livery | What differs |
|---|---|---|---|
| Lay Guild | Ochre spine frame, stamped plates, CLEAR tags | Plates bare steel, spine frame black `#141211`, fewer tally blocks | The reserve wears a Hand's plates: unpainted, lightly stamped. The Ledger number on the chest plate is the same. |
| HTR | Grey plates, orange fields, logo, name on the broadcast | Grey plates, orange fields, **a number instead of a name** on the back plate and broadcast | HTR's reserve riggers are numbered, not named (02). The number is the reserve index plus the crew's roster number: 04-2. The Face's name card is replaced by the number on the Scrap Card. |
| Ascendancy | Mirror plates, white wraps, altitude numerals | Mirror plates, **pale sky wraps** `#A9C7E8`, altitude numerals one tier lower | The reserve is read as a Pilgrim: the wraps are the sect's second color and the painted altitude is the player's second-highest perch (**PROPOSED**). |
| Hangtown Crews | Salvage plates, crew stencil on the back | Same plates, **neighborhood stencil** repeated on chest and both shoulders, and a hand-painted district name | Hangers decorate their reserves with the neighborhood's stencil (02). The reserve is louder than the primary. |
| The Fray | Black plates, neon tape, red-match count | Black plates, neon tape in the **other two** Fray colors, red-match count replaced by the book's current odds on the player, chalked | The Fray's reserve advertises the book. The odds are generated from the match state (**PROPOSED**). |

All five keep plate rim bands, Spool ring, and shoulder lamp in team tint. All five keep plate dimensions. None changes the Spool.

## Drop

The reserve uses the player's equipped Drop, shortened: the Drop animation plays from its 2 s mark so the reserve is on the floor within 4 s of the 8 s timer ending (**PROPOSED**). Lines are disabled until the feet touch. During the Drop the telemetry LED is off and the plate rim bands are dim; both come up on landing.

Face-specific Drops have a reserve variant note in each Face doc where the shortened version needs a different entry point.

## Behavior and animation

The reserve is the player. No NPC behavior. Animation needs beyond the class set:

| Clip | Notes |
|---|---|
| Reserve Drop entry | The class or Face Drop from its 2 s mark |
| Reserve idle | One idle variant per faction that reads as "the next one up": Guild checks the Spool strap, HTR checks the number on their own shoulder, Ascendancy touches the chest plate, Hangers tap the stencil, Fray looks at the odds (**PROPOSED**) |

## Audio

- The Marshal does not announce reserves.
- The Face voice pack adds one reserve-entry bark slot per Face (**PROPOSED**), played to the crew only. The bible's Face docs list it under "match start" where the Face has one.
- The Drop audio is the class Drop audio from its 2 s mark.

## Scrap Card

When a reserve scraps someone, the Scrap Card shows the reserve mark (R1 to R3) beside the Face name, or for HTR, the number alone.

## Technical

| Item | Spec |
|---|---|
| Implementation | A second `MI_Harness_<Class>_<Face>` instance set with `FactionPrimary`, `FactionSecondary`, `TapeColorA/B`, `TallyMask`, and a `ReserveIndex` int that drives the R decal and the HTR number. No mesh change. |
| Replication | `ReserveIndex` replicated with the player state; the material swap happens on the client at respawn. |
| Cost | One material instance per player per livery. Zero additional skeletal meshes. |
| Validation | Every reserve livery passes the 50 m silhouette test and the 15% plate coverage rule, as any livery does. |

## Reference search terms

racing team second driver livery, boxing corner man numbered bib, relay team second leg, football substitute warm-up bib, pit crew numbered helmets, HTR-style asset tag numbering.
