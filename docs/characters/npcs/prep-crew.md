# Prep Crew

_The Office's staging crews. They set every arena as a demolition site, not a battlefield. Charges go where a cutting crew would put them. Fuel sits where the cranes need it. They also paint the tallies._

## Appearance

| Element | Spec (**PROPOSED** unless from the bible) |
|---|---|
| Count | Four per arena, visible during the Drop phase and between rounds. Gone at "Live." |
| Build | Mixed, four body variants, four heads. |
| Vest | Office hi-vis, yellow `#E6FF2A` with grey `#6B6E72` reflective bands, PREP on the back |
| Under | Grey coveralls, knee pads, work gloves |
| Hard hat | White with the Office crest and a Nurri sticker. Two of four have headlamps. |
| Kit | Tape rolls (yellow-black structural, orange slack-cable), a stencil brush and a white paint pot, a spray can, a charge case, a clipboard |
| No harness | No Catch Plates, no Spool. They are the only people on the floor who cannot be dogged. |

## Behavior

During the Drop phase and between rounds they finish the arena the way it was found:

| Task | Where | Visible result |
|---|---|---|
| Place demolition charges | On structural elements per the arena's authored list | A 12 kg charge with a white tag, a wire run |
| Tape rated loads | Columns, drive dogs, counterweights, clamps | Yellow-and-black structural tape (03, rule 7) |
| Tape slack cables | Hangtown only | Orange tape and visible fraying |
| Stage volatiles | Propane in racks, drums near cranes | Racks, fuel stacks |
| Stage Heavy loads | Anything Heavy that is meant to move: the Claim marker on polished runners, the Haul spool on rollers, containers on their rack rollers | Rollers, skids, greased runners under the load (05 §6.1a). A Heavy object on a bare floor is scenery and the prep crew leaves it that way. |
| Place Stall Gel pads | Rare, authored spots | A 2 x 2 m Nurri pad |
| Paint tallies | On riggers in the spawn zone, before the Drop | White strokes on plates (see `equipment/harness-shared-components.md`) |
| Repaint HTR plates | HTR plates are new every match | Fresh plates carry the rigger's career tallies before the Drop |
| Remove unofficial tape bands | Any Line with a band for a rig the Office does not recognize | A strip of tape peeled off on camera |
| Inspect salvaged plates | Scrapper harnesses mostly | A 1 cm white inspection dot in a plate corner |
| Reset between rounds | Everything | Objects back to authored positions; set lines cleared |

They do not react to players. If a player stands where a prep crew member needs to be, the member waits.

## Animation needs

| Clip | Notes |
|---|---|
| Walk with kit | Carrying a case, a tape roll, a paint pot |
| Kneel and place | Charge placement, 3 s |
| Tape wrap | Around a column, 4 s loop |
| Stencil paint | Short strokes on a plate at chest height, 2 s per stroke group |
| Peel tape | 1.5 s |
| Inspect | Look at a plate, press a dot, 2 s |
| Clipboard | Idle write |
| Clear the floor | At "Live" minus 2 s, walk briskly to the nearest exit volume and despawn |

Tally painting on a player uses the player's `S_Tally_01` to `S_Tally_08` sockets as targets; the stroke appears in `TallyMask` at the brush's contact.

## Audio

| Event | Sound |
|---|---|
| Tape | A long tape rip, then a slap |
| Spray | A can rattle, a short hiss |
| Stencil brush | Soft dabs |
| Charge case | Latches, a wire reel |
| Radio | Office radio chatter, low, unintelligible, with a squelch tail |
| Clear | A whistle from the gallery, then footsteps leaving |

The prep crew is the arena's "someone was just here" layer. Their sounds in the Drop phase set the demolition-site tone before anything moves.

## Technical

| Item | Spec |
|---|---|
| Mesh | Shares `SK_Rigger` (no harness sockets populated). 10k tris LOD0, 3k LOD1. One 2K set with four color masks. |
| Spawning | Spawned at Drop, despawned at Live minus 2 s through exit volumes. Spawned again at round reset for 4 s. |
| Collision | Capsule only, no Catch Plates, no `PM_CatchPlate`. Dogs do not seat in them; they bounce. |
| Interaction | None with players. Scripted sequences per arena from the arena's prep list. |
| Tally painting | Cosmetic, client-side, driven by the player's Ledger delta since their last match. |

## Reference search terms

demolition prep crew placing charges, stage crew blackout gear, road crew hi-vis in dawn light, aircraft ground crew stenciling, scaffolding inspection tags, film set grip crew.
