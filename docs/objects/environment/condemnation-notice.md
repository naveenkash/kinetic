# Condemnation Notice

One-line: environment, dressing, Light (0.05 kg paper on a 0.3 kg board where boarded), A4 (210 mm x 297 mm) Office notice or 400 mm x 600 mm HTR board, Hangtown (stapled to doors, wired to handrails, nailed to bridge bearers; about 120 per round), Ballast (HTR cut-sequence placards on racks and housings), the Yard (Office license notices on the control shack), the Anchor (contractor permits on the Scaffold).

## Role in play

Notices do no physics. They are the map's text layer and the narrative weight of Hangtown (03): the stakes are on the doors. A player who stops to read one learns which district HTR will cut if they win the Claim, whose house this is, and what the Office thinks of all of it. They also carry the only legible dates on the map, which is how players learn that the match they are in is a real Claim with a real consequence. Evidence objects (see `evidence-objects.md`) are sometimes hidden behind them.

| Harness | Relationship |
|---|---|
| Climber | Ascendants tear them down and leave the staples. An Ascendancy Claim win in Hangtown (not in the season rules; cosmetic) replaces notices with prayer cards. |
| Scrapper | Hangers write on them. Half the notices in the Hang carry a reply. |
| Rigger | Guild notices are the inspection tags beside the HTR boards: a different voice. |
| Doubler | Fray stickers over them: pirate stream QR-style glyphs, betting odds. |
| Breaker | HTR's boards are theirs. Reserve riggers' numbers appear on the boards as the cutting crew. |

## Appearance

Three authorities, three formats:

| Authority | Format | Color | Voice |
|---|---|---|---|
| Office of the Lay | A4, cream paper, yellow header band, black serif, a wax seal | #F2E8D0 paper, #F2C500 band, #141414 text | Dry, legal, numbered. "NOTICE UNDER THE ACCORD, ARTICLE 9: THIS STRUCTURE STANDS ON A CONTESTED CLAIM. OCCUPANCY DURING A CLAIM MATCH IS PROHIBITED. THE OFFICE REGRETS." |
| Halvard-Teague Reclamation | 400 x 600 mm corrugated board, white on orange, machine-printed, zip-tied | #E8541E ground, #F5F5F0 text | Corporate. "SCHEDULED FOR RECLAMATION. SECTOR 3-W. CUT WINDOW: CLAIM 62. HTR THANKS YOU FOR YOUR COOPERATION." A QR-like glyph. A reserve rigger's crew number as the responsible party. |
| Lay Guild | 100 x 150 mm ochre inspection tag, hand-written, wired | #B8752A, #141414 ink | Terse. "INSPECTED L.61. LOAD CABLE SOUND. SLACK CABLE REPLACE. LG 0441." |

| Part | Material | Base color | Roughness | Metallic |
|---|---|---|---|---|
| Office paper | cream bond, rain-cockled | #F2E8D0 | 0.9 | 0.0 |
| HTR board | corrugated plastic | #E8541E | 0.5 | 0.0 |
| Guild tag | card, ochre | #B8752A | 0.85 | 0.0 |
| Staples, zip ties, wire | steel, nylon | #8A8F93, #141414 | 0.5 | 0.6 |
| Replies | chalk, marker, paint | #F5F5F0, #141414, #2F6FAD | 0.8 | 0.0 |

Weathering: Office notices are reissued every Claim and layered: five seasons of them on the oldest doors, the top one crisp, the lower ones bleached and torn to the staple line. HTR boards are new and already scratched. Residents' replies on HTR boards: `NO`, `WE LIVE HERE`, a drawn hook, a tally of years. On Office notices the replies are politer and funnier (`THE OFFICE REGRETS` underlined, with `SO DO WE` beneath). Lighting: none; where a notice sits near a lantern it is readable at 2 m. All notice text is legible at 1 m in first person at 100 degrees FOV, which sets the minimum font size at 18 mm x-height on the A4 and 40 mm on the HTR board.

Read by tier: from the Upper Hang, orange rectangles on the houses on the HTR claim side only, which maps the dispute from above. From the Mid Hang, a notice on every door. From the Low Hang, HTR boards on the terrace posts.

## Variants

1. **Office A4, current season.** Crisp, sealed, dated this Claim.
2. **Office A4, stack.** Up to five layered, torn. Dressing for the oldest houses.
3. **HTR board, sector.** Orange, with the sector that goes if HTR wins. Placed only on the west district (the cut district).
4. **Guild tag.** On anchors and slack hitches; the Guild's assessment, which sometimes contradicts the Office's (a Guild tag reading `UNSAFE` on a house the Office cleared).
5. **Replied.** Any of the above with a resident's answer. Forty authored reply decals, never repeated within 30 m.
6. **Seasonal aftermath.** After an HTR Claim win: the cut district's remaining posts carry `RECLAIMED. CLAIM 62.` boards. After a Hanger win: the HTR boards are gone and the zip ties remain.

## Physics

- Static decal or a Light prop (boarded variant, 0.3 kg) held at 200 N (racked class: a zip tie). Any line pulls a board off its post; it travels at the Light speed cap (25 m/s) and delivers 7 N s, nothing. Lifetime once loose: 10 s. It is a Dog seat for exactly as long as it takes a Dog to pull it free, which makes a Dog aimed at a notice a Dog on the floor a moment later: the one way a notice plays.
- Paper variants are decals. Not Dog seats; the Dog seats in the door behind.
- Blast and fire: boards within a fire patch char (material state) and do not burn away.

## Interactions

- None mechanical beyond the Dog-pulls-board case. A notice's job is to be read in the four seconds between a Drop and Live, and in replays.
- The pirate feed (Fray cosmetic) overlays HTR boards with odds when a Doubler is in view: cosmetic, client-side.

## States

`Fresh`, `Weathered` (per-season layering), `Replied`, `Charred`, `Removed` (ties only).

## VFX and SFX hooks

`SFX_Paper_Flutter` in wind zones (rim, Upper Hang), `SFX_Board_ZipTie_Snap` on pull-off; `VFX_Paper_Edge_Flutter` (vertex animation on the top notice only).

## Technical

- Decals: one 4K atlas `T_Notice_Atlas` with 24 Office variants, 8 HTR, 12 Guild, 40 replies as a separate 2K alpha atlas layered by a second decal. Text is baked; localized builds swap the atlas.
- Boards: a 12-tri mesh, instanced, `Kinetic_Prop` on release.
- Placement rules: HTR boards on the cut district only; Office notices on every door; Guild tags on every anchor family object. A placement script enforces the 30 m reply spacing and the per-season layering count.
- Sockets on host meshes: `Notice_Door`, `Notice_Rail`, `Notice_Post`, `Tag_Anchor`.

## Reference search terms

eviction notice stapled door layered, condemned building placard, corrugated plastic site sign zip ties, scaffold inspection tag, favela demolition notice graffiti reply.
