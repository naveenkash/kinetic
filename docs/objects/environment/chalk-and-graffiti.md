# Chalk and Graffiti

One-line: environment, decal system, no mass, marks from 50 mm tallies to 6 m murals, all arenas: Hangtown densest (residents), Ballast and the Yard (Guild and HTR working marks), the Anchor (Ascendancy altitudes and prayer lines). Five hands, each with its own medium.

## Role in play

Marks are the second text layer after notices (see `condemnation-notice.md`) and the first one players actually use in a match. The Guild's chalk is functional: fall-axis arrows on racks, `TWO BOLTS` under a live column, a Hauler timetable at a trench crossing. HTR's spray is the cut sequence, which tells a Breaker what is live. The Ascendancy's altitudes mark perches. The Hangers' murals mark home. The Fray's stickers mark the camera angles their pirate feed uses. A player who learns to read the five hands reads the map.

| Hand | Medium | Where | What it says |
|---|---|---|---|
| Lay Guild | white or ochre chalk, lumber crayon | on structure, at hand height, beside the thing it describes | Surveys. Arrows. `SEATED`, `FRICTION ONLY`, `9 s` (Mei-Lin's estimates, copied by Guild crews who hate that she is right). Ledger numbers. |
| HTR | orange spray through stencils, orange survey paint rings | on anything scheduled | `CUT 02 SEQ A`, sector codes, elevations, a date. |
| Ascendancy | silver paint pen, white chalk | at the top of things, on perches, on the Stump | Altitudes (`+41.2`), a short upward arrow, lines of a litany one word per perch so it reads only by climbing. |
| Hangers | house paint, brushed; children's chalk; charcoal | on their own walls, on the market ring, on the rim compound | Murals (the first cable, the grandmother, a hook), hopscotch, a tally of years, names. Replies to notices. |
| Fray | vinyl stickers, neon marker | on camera lines, on HTR boards, on anchors they use | Odds (`3:1 KITE`), red-match counts, a glyph that marks a pirate camera's framing. |

## Appearance

| Hand | Base color | Roughness | Notes |
|---|---|---|---|
| Guild chalk | #F5F5F0, #B8752A | 0.95 | Dusty edge, smeared where read by a thumb. |
| HTR spray | #E8541E | 0.6 | Hard stencil edge with over-spray halo. Never the same orange as slack-cable tape (#FF8A00); the two are 20 degrees of hue apart and HTR orange never carries a white thread. |
| Ascendancy pen | #C9CED3 (silver) | 0.3 metallic 0.6 | Thin, exact, in a trained hand. |
| Hanger paint | #2F6FAD, #C8452B, #F4B860, #3E7A5B | 0.8 | Brushed, drips, two coats where it mattered. |
| Fray vinyl | #39FF14, #FF2A9D on #141414 | 0.4 | Printed, peeling at one corner. |

Weathering: chalk washes off in a season; the Guild re-chalks before every match, so Guild marks are always fresh and are therefore trustworthy. HTR spray is permanent and is layered: last season's `CUT 02` under this season's. Ascendancy altitudes are the oldest marks on the Anchor, some from L.20, and the paint is going. Hanger murals are repainted after every Claim the Hangers win, which is the one place the map visibly celebrates. Fray stickers are new every match. Lighting: none; marks are lit by their surfaces. Chalk gains a 10% emissive under line light so Guild marks read in dark sectors (**PROPOSED**; a readability aid, not a glow).

Read by tier: from far, murals and HTR stencils only (both are sized to read at 30 m). From mid, Guild arrows (0.5 m) read at 10 m. From close, tallies, names, and altitudes.

## Variants (authored sets)

1. **Guild working set.** 60 decals: arrows (4 directions, 3 sizes), words (`SEATED`, `LIVE`, `DEAD`, `NOT YET`, `TWO BOLTS`, `DON'T`, `FALL LINE`, `STAND HERE`), numbers 0 to 99, timetables (one per trench crossing in the Yard, matching the real schedule).
2. **HTR cut set.** 30 decals: `CUT 01` to `CUT 12`, `SEQ A` to `SEQ D`, sector codes, survey rings (three diameters), `PROPERTY DISPUTED`, `UNIT ON LOAN`.
3. **Ascendancy set.** 40 decals: altitudes in 0.1 m steps authored to the actual height of the perch they are on (a build-time script checks this), arrows, the litany (24 words).
4. **Hanger set.** 8 murals (2 m to 6 m), 30 small marks, 40 notice replies (shared with `condemnation-notice.md`).
5. **Fray set.** 20 stickers: odds for each Face by name, red-match tallies, four camera glyphs.
6. **Seasonal set.** After each Claim: the winning faction's tally mark is added to a wall in each arena (the Office permits one; it is the only cross-season graffiti).

## Physics

None. Decals. Marks on destructible surfaces fracture with the surface and are not re-projected onto fragments (fragments are short-lived). Marks on a dented container stretch with the morph.

## Interactions

- Guild marks are truthful: a rack's chalk arrow is its authored fall axis; a `9 s` beside a column is within 1 s of the real consequence timeline; a trench timetable matches the Hauler schedule. This is a design rule, not a hope. If the authored data changes, the decal set is regenerated from it.
- HTR marks are truthful about what is live this round: `CUT` numbers appear only on live rated loads. Dead loads carry last season's faded number under a painted-out strike.
- Ascendancy altitudes are truthful to 0.1 m.
- Hanger and Fray marks are expressive, not informative, with one exception: a Fray camera glyph marks a spot where a pirate-feed replay camera sits, and in the replay system that camera is real.

## States

`Fresh`, `Season-weathered` (chalk gone, spray faded 30%, paint chipped), `Struck` (HTR number painted out), `Fractured` (gone with its surface).

## VFX and SFX hooks

None at runtime. `SFX_Chalk_Write` is an apprenticeship-only cue when Teodora marks a rack in the tutorial.

## Technical

- Deferred decals, one 4K atlas per hand (five atlases), plus a 2K shared normal and roughness set for chalk dust, spray halo, and paint drips. Decal budget per sector: 64 visible.
- Guild working set is data-driven: a build step reads each rated load's authored fall axis, consequence lead time, and each Hauler schedule, and writes the decal parameters (arrow rotation, number). Ascendancy altitudes are written from perch socket heights the same way.
- Sockets on host meshes: `Mark_Guild`, `Mark_HTR`, `Mark_Asc`, `Mark_Hanger`, `Mark_Fray`, each with a size class. Placement script enforces hand-by-arena density: Hangtown 60% Hanger, the Yard 50% Guild, Ballast 50% HTR, the Anchor 40% Ascendancy.
- Localization: Guild words are stencil-English in the fiction (the Guild's working language is written as the game's language); murals carry no text that needs translation.

## Reference search terms

lumber crayon survey marks steel, demolition spray paint cut marks, scaffold chalk marks ironworker, favela mural painted house, climber summit altitude graffiti, peeling vinyl sticker industrial.
