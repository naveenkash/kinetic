# KINETIC: Menus and Frontend

_System spec v0.2.2, revised to 05-core-mechanics v0.2 and the v0.2.1 and v0.2.2 rulings (CHANGELOG 2026-10-24). Implements bible 04 (Ledger, Grades, the Weight, Claims, Allegiance, Contracts, Manifest, Spool sidegrades, cosmetics) and 05 section 9 (lobby, match formats). Anything marked **PROPOSED** is new here._

## 1. Voice

Two voices own the frontend.

- **The Office of the Lay** writes everything administrative: the Ledger, the shop, Contracts headers, Manifest, bulletins. Dry. Procedural. Has outlived everyone who founded it. Never sells; it informs you that a thing is available.
- **LayCast** writes everything that moves: lobby, matchmaking, post-match, Claims map. Broadcast energy, sponsor kitsch, lower-thirds.

Faces speak in their own voice inside Contracts and the Apprenticeship only.

Rule: no exclamation marks in Office copy. One per screen, maximum, in LayCast copy.

## 2. Information architecture

```
Main (the Yard gate)
|- Play
|  |- Quick (Scrap, Claim, Haul, Free Scrap, casual)
|  |- Rated (Scrap, Claim; Grade 2+)
|  |- Apprenticeship (replayable)
|  `- Lobby -> Match -> Post-match summary
|- The Ledger (profile, codex, stats)
|  |- Record (Grade, Weight, Tonnage, career)
|  |- Entries (lore codex, evidence)
|  |- Rope Marks
|  `- Serial (your Cutter)
|- Harness (class select)
|  |- Face select
|  `- Spool loadout (Line, Dog, Cutter)
|- Allegiance
|  |- Contracts (3 per week)
|  `- Faction Ledger entries
|- Claims (season map of the Lay)
|  |- Standings
|  `- Evidence
|- The Manifest (seasonal pass)
|- Shop (Office Surplus)
|- Locker (cosmetics by slot)
`- Settings (video, audio, controls, accessibility)
```

Navigation: left rail with seven items. Back is always Escape or B. Any screen reaches the lobby in one input when a match is forming. The main menu background is the Guild training yard on Collar Four at the current time of day, with the player's harness standing in it.

## 3. The Ledger

One screen, three tabs. It looks like a worn logbook because it is one: paper texture, ruled lines, typewriter numerals, stamps. Office voice.

```
+-------------------------------------------------------------------+
| THE LEDGER                                 Guild No. 4471-R        |
| [Record] [Entries] [Rope Marks] [Serial]                           |
|                                                                    |
|  QUISPE-TRAINED            Grade 3  RIGGER       Allegiance: Guild |
|  Tonnage, career     41,230 t   Days up        61                  |
|  The Weight         -3,900 t    (Clear in approx. 9 matches)       |
|  Scrapped by: object 44%  Well 31%  world 18%  blast 5%  crush 2%  |
|  Scraps:       112    Spotter credits: 88   Lines cut: 1,204       |
|  Panic window survival:  52%  (median turn 1.3 s)                  |
|                                                                    |
|  Most moved: Ballast drive dog, 40 t, 1 time.                      |
|  Longest line held: 18.4 s (Hang, Yard Rails).                     |
+-------------------------------------------------------------------+
```

**Record**: Grade (faction name per 04 table: Greenhand, Hand, Rigger, Lead, Wright, Master Wright for Guild), Tonnage, the Weight with matches-to-Clear, elimination mix (mirrors the 05 section 12 tuning targets so players and designers read the same numbers), panic-window survival rate, per-class hours.

**Entries**: the codex. Short entries, most three sentences, unlocked by doing things. Filter by faction and by Claim. Evidence objects about the Question are listed under a separate heading, "Recovered," with the map and date. Locked entries show only their unlock condition in Office phrasing: "Entry available on moving a rated load."

**Rope Marks**: eleven named rigs, three tiers each (Tape, Band, Mark). Each row shows the tape swatch, count, and next threshold.

**Serial**: your Cutter. Serial number, date of the Cutter ceremony, warm-up and reach of the equipped model, and a single line of Office copy: "Issued. The Guild records the serial next to the name."

Copy samples:
- The Weight at zero: "Clear. The lease is paid. The harness is yours."
- Sponsored: "Weight carried by Kessl Harness Leasing. Patch required."

## 4. Harness select

Five harnesses in a row, in the training yard, standing on scales. Silhouette first; the silhouette tells the enemy how a line behaves with you. Selecting one steps it forward.

```
+-------------------------------------------------------------------+
| HARNESS                                                            |
|   Climber   Scrapper   [RIGGER]   Doubler   Breaker                |
|    90 kg     110 kg     130 kg     130 kg    220 kg                |
|                                                                    |
|  RIGGER  "Journeyman's Hang"                                       |
|  Line 40 m . Pull 600 N . Hang 12 kN . Jack 12 m/s . Dogs 120 m/s  |
|  Two set lines, 12 s. Brace. Cutter warm-up 0.17 s.                |
|  Run 6.2 m/s  Sprint 7.5  Mantle 2.2 m                             |
|                                                                    |
|  Office note: harness output is standardized under the Accord.     |
|  Inspected before every sanctioned match.                          |
|                                      [Face]  [Spool]  [Confirm]    |
+-------------------------------------------------------------------+
```

All numbers come from 05 section 2.2 and 3.4 and are displayed, not hidden. Each harness is labeled with its weight class per 05 v0.2.1: Light (Climber), Standard (Scrapper, Rigger, Doubler), Heavy (Breaker). Bar comparisons use the 02 relative scale. The Breaker's panel adds: "1,000 N. Moves rated loads." The Doubler's adds: "Two lines, 480 N each. Overspin to 1,200 N. Snaps at 1.8 s."

## 5. Face select

Two Faces per faction, shown as LayCast pre-match portraits with their broadcast name, age, and their one quote from 02. A Face is identity: voice pack, Drop, bark set, Scrap Card template. Selecting a Face from a faction you have not declared Allegiance to is allowed; the Face addresses you by Grade only once you have declared.

Copy: Teodora Quispe, "Mother Line." Wright. _"The building does the killing. You just introduce them."_ Voice pack: Guild Contracts, tier 2.

## 6. Spool loadout

Three slots: Line, Dog, Cutter. Tables from 04 displayed as tradeoffs, both halves always visible. Locked items show their Grade or Contract gate. The baseline is marked "Standard. Competitive."

```
+-------------------------------------------------------------------+
| SPOOL                                        Rigger                |
|  LINE    [Standard] Longline  Hawser  Quickline  Patient  Hair-tr. |
|           Longline: +30% length (52 m) / -20% pull (480 N)         |
|  DOG     [Standard] Barb  Mag  Spike                               |
|           Spike: higher load on concrete, wood, rock. Slides on    |
|           metal. Next map: the Yard (steel). Office advises.       |
|  CUTTER  [Standard] Hot  Long                                      |
|           Hot: warm-up 0.12 s, reach 1.1 m.                        |
|                                                                    |
|  Resulting: Line 52 m . 480 N . Hang 12 kN . Cutter 0.17 s / 1.5 m |
+-------------------------------------------------------------------+
```

The resulting line computes the equipped Line against class tuning and shows the final numbers. **PROPOSED**: Line sidegrades modify pull tension and length only; Hang load is a filament rating (05 section 3.2) and is unchanged by them. **PROPOSED**: when a map is known (lobby), the Dog row shows the map's dominant material and a one-line Office advisory. Loadout can be changed in the lobby and between rounds, never mid-round.

## 7. Allegiance and Contracts

Top: five faction plates. Declaring opens that faction's Contracts, three per week, written in the faction's voice (samples in 04). Switching mid-Claim keeps rewards, resets Contract progress. The confirm dialog is Office voice: "Allegiance recorded. Contract progress resets. The Guild remembers."

```
+-------------------------------------------------------------------+
| ALLEGIANCE: THE LAY GUILD                 Claim 14: Hangtown       |
|  CONTRACTS, this week                                              |
|  [ ] Hang a line that someone else uses. Twenty times.    13/20    |
|  [x] Cut a line on a teammate's back. Five times.          5/5     |
|  [ ] Move a rated load on Ballast.                          0/1    |
|  Pays: Guild tape (ochre/black), Quispe voice pack II, Entry 41.   |
|                                                                    |
|  Standing contribution this week: +9 (see Claims)                  |
+-------------------------------------------------------------------+
```

Faction Ledger entries unlocked by Contracts appear in the Ledger under that faction's heading.

## 8. Claims (season map)

LayCast voice. A long horizontal scroll of the Lay, 3,000 km, west to east: Collar One in the sea, the Cordillera, the Basin, the Anchor. Each season's contested section is lit. Faction standings as five stacked bars, with the per-capita index (see game-modes-and-match-flow.md section 8) and days remaining.

```
+-------------------------------------------------------------------+
| CLAIM 14 . HANGTOWN . 23 days                       LayCast        |
|  ~~~[C1]~~~~~~ Cordillera ^^^^ [C3 HANGTOWN] ^^ Basin [C4] ... [A] |
|  Guild    |||||||||||||||||  52.1                                  |
|  Hangers  ||||||||||||||     47.8                                  |
|  HTR      ||||||||||||       44.0                                  |
|  Ascend.  ||||||||           31.2                                  |
|  Fray     |||||||            29.9                                  |
|  If HTR holds: next season's Hang loses the east district.         |
|  Evidence this Claim: recovered by 12% of riggers. [Where?]        |
+-------------------------------------------------------------------+
```

Consequence copy is stated plainly per 04 (a Hanger win keeps it standing; an HTR win removes a district). Tapping a past Claim shows what happened to the map. The evidence object is listed with its map, never its location.

## 9. The Manifest

A salvage shipping manifest: crates stenciled with contents, two columns, Free and Sponsored. Crates tick off as earned. Contents are cosmetics and Ledger entries only (04). No Line, Dog, or Cutter ever appears here, and the header says so: "Sidegrades are not shipped. See the Office."

```
+-------------------------------------------------------------------+
| MANIFEST . CLAIM 14                  Tonnage to next crate: 820 t  |
|  FREE                          SPONSORED (Kessl)                   |
|  [x] Tape, Hangtown stencil    [x] Livery, Kessl Ochre             |
|  [x] Entry 38                  [ ] Drop: "Off the Lift"            |
|  [ ] Rig Call: rude 2          [ ] Scrap Card: Hanger             |
|  [ ] Reserve livery, Vilca     [ ] Tallies, prep crew set 3        |
+-------------------------------------------------------------------+
```

## 10. Shop: Office Surplus

Office voice throughout. No urgency, no timers with flashing, no "deals." Items are listed as surplus available for Tonnage credit or sponsor credit. The Visibility Clause is printed on every Tape item: "Glow color is fixed to crew assignment under the Visibility Clause. Pattern only."

Copy samples:
- Header: "OFFICE SURPLUS. Items listed are cosmetic. Harness output is standardized. Nothing here moves a load."
- Livery: "Plate paint. Silhouette unchanged. Weight class remains legible at fifty meters, as required."
- Cutter cosmetic: "Handle and blade pattern. The snap is universal and is not for sale."
- Sponsorship: "Kessl Harness Leasing will carry your Weight. You will wear the patch. The Office takes no position."
- Sold-out state does not exist. Rotation note: "Stock rotates with the Claim."

## 11. Locker

Cosmetic slots from 04 in one grid: Livery, Tape, Dogs, Cutter, Rig Calls, Drops, Scrap Cards, Tallies (earned, display only), Voice, Reserve livery. Preview is in the training yard with the line live, so Tape is seen under glow. Tallies cannot be removed; the Office painted them.

## 12. Match lobby

LayCast voice. Three crew slots per side, class silhouette, Face, Grade, Rope Mark tape. Map card with the mid-match event named and the moving part named ("Ballast. The Breathing, 90 s. The Shear at 1:15." (Claim lobbies read "at 4:00"; Haul, "at 2:00 each half")). Loadout button. Pre-match LayCast segment, 30 s, skippable, where the Faces talk (04 lore delivery).

```
+-------------------------------------------------------------------+
| LAYCAST . SCRAP . BALLAST                    Live in 0:24          |
|  AMBER                              SKY                            |
|  Rigger  Hitch       Lead G4   |  Breaker  Wreckingball  Dir. G5   |
|  Climber Swift       Asc. G4   |  Breaker  No. 7         Op.  G2   |
|  [Rigger YOU         Rgr  G3]  |  Doubler  Snap          Earn G3   |
|  Suggested role: Spotter. Two Breakers opposite: expect rated      |
|  loads. Spike Dogs seat poorly on hull steel.                      |
|  [Spool] [Ready]                       Stall-Gel by Nurri          |
+-------------------------------------------------------------------+
```

**PROPOSED**: a role suggestion line (Lead, Hand, Spotter) based on crew composition. Advisory only. Titles are traditional, not enforced (01).

## 13. Post-match summary

LayCast voice, then Office voice. Two pages.

Page 1, LayCast: result, round pips, the final replay thumbnail, three "clips" (Marshal-flagged moments: longest hoist, biggest load, closest cut).

Page 2, Office: the Tonnage breakdown. Every number must reconcile with the Ledger.

```
+-------------------------------------------------------------------+
| TONNAGE . this match                                               |
|  Mass moved under your lines      3,120 t.m   ->   1,560 t         |
|    of which rated load              40 t x 300 m       (Ballast)  |
|  Scraps                               3    x 120  ->     360 t     |
|  Spotter credit                       2    x 180  ->     360 t     |
|    (your set line, Swift's scrap; your hung load, Hitch's scrap)   |
|  Ring-outs, height paid           1 x 38 m x 10  ->     380 t      |
|  Assists                              1    x  60  ->      60 t     |
|  Match total                                           2,720 t     |
|  Against the Weight                                   -2,720 t     |
|  The Weight, remaining                                 1,180 t     |
|  Contracts advanced: Guild I (13 -> 17)                            |
|  Grade progress: Rigger -> Lead   62%                              |
+-------------------------------------------------------------------+
```

Rates shown (120 per scrap, 180 for Spotter credit, 10 per meter of ring-out height, 1 t per 2 t.m moved) are **PROPOSED** placeholders for economy tuning. The structure is canonical per 04: Spotter credit is the larger share, ring-outs pay by height, mass times distance is the base, and a mover should out-earn a cutter.

A "Report" button on both pages is Office voice: "Conduct reports are read. Outcomes are not published."
