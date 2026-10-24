# Well Kill Volume

One-line: environment, kill volume, no mass, one per arena at the arena's floor (05 section 7.5): Ballast's gorge beyond the torn nose cone, the Yard's Shear Press pit floor, Hangtown's ravine below the Low Hang, the Anchor's shaft under the Stump. Entering it is a Stall with the "down the Well" Scrap Card.

## Role in play

The Well is where the conversation ends (03, rule 6). It is the ring-out, 25% of all eliminations by target (05 section 12), and the destination of Overboard, the Anchor Drop, the Caboose off an edge, and every Pulse. It pays Tonnage by height fallen, most at the Anchor (04). Its tell is the Hooks spinning up 0.5 s before entry, except at the Anchor, where Hooks do not go and the sound is different (03, 05 section 7.4).

| Harness | Relationship |
|---|---|
| Climber | Lives above it. A Climber's 50 m line is the only one that reaches a wall from the middle of the Hangtown ravine or the Anchor's shaft. |
| Scrapper | Overboard is theirs, and it is a T push, not a Kick: the Kick never fires when a Dog is in a player's plate (05 section 3.2 interlock). A Scrapper's 400 N on a Breaker is 1.8 m/s², a skid under the Breaker's 5 m/s² foot-hold, 2 m in 1.5 s toward the edge, with a wall behind the Scrapper so the push goes one way. The victim has the whole skid to cut. |
| Rigger | Hangs to the floor before going over (05 section 10, Overboard counter) and is the harness most often found hanging under an edge waiting for the set line to expire. |
| Doubler | The Snapback (deliberately snapping a line to launch yourself) over an edge is a ring-out on purpose; the Fray does it to end a lost round with a clip. |
| Breaker | Braced, a Breaker goes nowhere. Airborne, a Breaker is a cargo drop with a heartbeat (02). The Well is a Breaker's one fear. |

## Geometry per arena

| Arena | Well | Lowest playable floor | Kill volume top (PROPOSED) | Fall time from floor | Hook pad |
|---|---|---|---|---|---|
| Ballast | The gorge, through the torn nose cone and off the port Bay edge after the Shear | The Hold deck at the nose lip | 12 m below the nose lip, a volume filling the gorge; 12 m below the port Bay deck edge after the Shear | 1.56 s from the lip at rest | Under the gallery at the Crown; Hooks descend the Spine and exit the nose |
| The Yard | The Shear Press pit (X +38 to +52, Y -7 to +7) | The trench floors at Z -4 are the lowest playable floor; the pit lip is Z 0 and the pit floor Z -10 | Below Z -6 (entered at 6 m of fall from the lip); the ram's top of stroke is -6.5 | 1.1 s | Hatch in the pit's north wall at Z -5 |
| Hangtown | The ravine | The Low Hang terraces and nets at minus 24 m (Upper Hang is 0) | Minus 45 m, a plane across the whole ravine, 21 m below the Low Hang; the mist layer begins at minus 36 m | 2.07 s from the Low Hang | West rim beside the headframe, and the east rim compound |
| The Anchor | The shaft under the Stump: the annulus r 12 to 16 from the Crown lip (Z 60) down | The bowl floor at Z -8 (the Scaffold's footing); L1 at Z 6 | Cylinder r 16, Z below -15 (7 m under the bowl floor's inner edge, 21 m under L1) | 2.07 s from L1 | None. No Hooks. |

Fall speeds at entry: Ballast 15.3 m/s, the Yard 10.8, Hangtown 20.3, the Anchor 20.3 from L1 (34 m/s from the Crown). All but the Yard are over the 12 m/s Stall against a hard surface anyway; the volume's job is to end the fall cleanly, in the dark, before any surface.

## Tells

| Arena | Audio (0.5 s before entry) | Visual |
|---|---|---|
| Ballast | `SFX_Hook_Rotor_Spinup` from the Spine above, Doppler falling away | The gorge's work-light string far below, then gel. |
| The Yard | Hook spin-up from the south pad; the press beacons visible from inside the pit | The ram face above you. |
| Hangtown | Hook spin-up from the rims (two sources, far); the mist closing | Lantern light through mist, then none. |
| The Anchor | No Hooks. The Stump's hum drops an octave (canon, CHANGELOG v0.2.2), heard by everyone and loudest in the faller's mix, with a wind-shear tone rising over the last 0.5 s. The Office retrieves by other means and does not discuss them. The Marshal calls it. | The Stump's glow brightens around the faller, the shaft's blue work-lights pass, then the Weave root's iridescence, then dark. |

The 0.5 s lead per 05 section 11 is measured to the volume, not to the lowest floor. A player passing the lowest playable floor is the trigger (05 section 7.5): Hooks start spinning when the player's capsule drops below the floor's plane at any point not over a surface, so the lead is 0.5 s for the Yard (where the pit is shallow) and longer elsewhere by the difference in fall time; the spin-up loop plays until entry.

## Physics

- A trigger volume. Entry sets the Stall sequence (05 section 7.4) with the Well Scrap Card. Lines on the player release. The body is not retrieved from the Anchor's Well; elsewhere a Hook descends to the body's last position above the volume (authored: the Hook plays its retrieval animation at the volume's top plane, not inside it).
- Objects: props entering the volume despawn (pooled) or are destroyed (Heavy, Rated). The Claim marker and the Haul spool reset to their authored positions (05 section 9.4). Evidence objects reset at round end. Rated-load fragments and the drive dog piston never reach a Well (their consequences stop them above it). The Lift counterweight and a pulled anchor pin's cable socket do.
- Ledge grab: automatic within 0.5 m of a ledge while falling at under 8 m/s (05 section 2.3). Every Well edge in the game has a grabbable lip except the Yard pit's west lip (0.1 m, authored: the loading face) and the Anchor's inner Scaffold edge during the Pulse's inhale (the toe-board is 0.2 m and the inhale pulls the player away from it).
- Tonnage: ring-out pays by height fallen from the victim's last grounded position to the volume. The Anchor pays most because the shaft is authored as 300 m deep for the purpose of the payout even though the volume is 10 m down (04: "the Anchor's Well pays most"). **PROPOSED** payout heights: Ballast 60 m (gorge floor), the Yard 10 m, Hangtown 400 m (the ravine floor), the Anchor 300 m.

## Interactions

- **Haul** a falling teammate from an edge: a Breaker's 1,000 N on a 90 kg Climber is 11 m/s², against gravity's 9.8: a net 1.3 m/s² upward. They stop falling and rise slowly. It works, barely, and only for a Breaker on a Climber; everyone else slows the fall and extends the panic window by 0.5 to 1.0 s, which is sometimes enough for the faller to Hang to the wall.
- **Hang** to the floor before going over (05 section 10): the line holds the class Hang load (a player's weight is 0.9 to 2.2 kN, under every Hang load), and the player swings under the edge. They climb back by Reel (hold Haul inside Hang: the rod shortens at 1 m/s under the Hang load, 05 section 3.2), which takes as many seconds as they have meters of line out, in full view.
- **Jack** off the Well's wall on the way down (Climber): a Slingshot back to a ledge. With 18 m/s and a 50 m line, a Climber in the Hangtown ravine can Dog the nearest cable at 50 m and Haul; a Breaker with 20 m and 10 m/s cannot, which is the whole class contract in one fall.
- Chain: the Yard's press removes anything that reaches the pit floor at the next ram contact; the Lift counterweight into the Hangtown ravine breaks through the Low Hang terrace on its way (an authored breakaway, not a volume effect).

## States

Static. `Armed` always. The volume's top plane moves in Ballast after the Shear (the port Bay edge gains its own volume) and in Hangtown's cut-district season (the Low Hang terraces under the cut cables are gone, so the lowest playable floor rises to the Mid Hang in that quarter and the Hook trigger plane with it).

## VFX and SFX hooks

`SFX_Hook_Rotor_Spinup` (spatialized from the pad, with Doppler on the falling listener), `SFX_Hook_Retrieve`, `SFX_Stump_Hum_Octave_Drop` (canon), `SFX_Well_Wind_Shear`; `VFX_Stall_Gel` (the Stall sequence's own), `VFX_Hook_Descent`, `VFX_Mist_Layer` (Hangtown), `VFX_Shaft_Worklights` (the Anchor).

## Technical

- `BP_WellVolume`: a `Kinetic_KillVolume` overlap with an arena-specific shape (box for the Yard pit, a plane-topped box for Hangtown, a gorge-shaped hull for Ballast, an annular cylinder for the Anchor). A second trigger, `LowestFloorPlane`, fires the Hook spin-up and the Scrap Card pre-roll.
- Hook actors: one per pad, kinematic flight on a spline from the pad to the retrieval point and back; no collision with players (05 section 3.1, Dogs do not seat in Hooks). Hooks are the one kinematic exempted from line severing (**PROPOSED**; otherwise every retrieval would cut the lines above the Well). Budget: two Hooks airborne per arena.
- Payout height and Scrap Card variant per arena in a data asset. The Anchor's variant suppresses the Hook flight and plays the alternate audio.
- Collision: the volume is overlap-only; the arena floor beneath it (where any exists) is `Kinetic_WorldStatic` and unreachable.

## Reference search terms

mine shaft looking down lights, gorge at night from above, mist layer in ravine, hydraulic press pit floor, recovery drone rotor spin up, deep shaft wind sound.
