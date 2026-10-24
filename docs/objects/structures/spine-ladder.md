# Spine Ladder

One-line: structure, static world with destructible cage panels, infinite mass (ladder) and Light (cage panels, 14 kg), rungs 0.3 m apart on a 0.5 m wide stile, in 6 m runs with rest platforms, Ballast Spine: the dorsal ladder run from the Spine Foot (Z 0, the Hold floor) unbroken to the Spine head and Crown deck (Z 44), plus three partial runs on the other shaft walls between drive dog ledges (**PROPOSED**). Datum and heights per `arenas/ballast/layout.md`; the Spine is the forward 72 m of the shaft, of which Z 0 to 44 is climbable.

## Role in play

The ladders are the honest route (03, rule 3): the way a Breaker reaches the Crown without a Slingshot. Everyone else Slingshots the shaft. A player on a ladder is a slow target with a predictable path and their back plate facing the shaft, which makes the Spine the kill plate's natural habitat. The ladders also run past the drive dog housings (see `rated/drive-dog-piston.md`), so a Breaker climbing is also a Breaker arriving at a rated load. In Haul mode the counterbalanced ratchet rail that carries the spool up the Spine (arena-unique; see `arenas/ballast`) runs the shaft wall between the north and east ladders, and its rest platforms are shared with the north ladder's.

| Harness | Relationship |
|---|---|
| Climber | Never. Perches on the rest platforms between Slingshots. |
| Scrapper | Uses the bottom 6 m to get above the Hold water. Recalls Dogs and lines up the shaft instead of climbing. |
| Rigger | Hangs a Clothesline across a ladder at a rest platform. A climber (small c) hits it at 2 m/s, under the 3 m/s stagger, so it is a wall, not a trap. |
| Doubler | Lines a climbing enemy's back plate from a catwalk and snaps. |
| Breaker | The user. 44 m at 1.6 m/s (05 section 2.3: ladders 2.0 m/s, Breaker 1.6) is 28 s plus rests; from the Spine collar at Z 26 it is 18 m, 11 s. The Breathing swings the Crown end 4.6 m while they climb. |

## Appearance

Silhouette: a caged steel ladder bolted to the shaft wall, with the cage hoops at 0.7 m spacing and a 1.2 m x 1.2 m rest grating every 6 m. The cage is broken away in places. Between ladders, the shaft walls carry the drive dog housings, pipework, and the climber's original light fittings.

| Part | Material | Base color | Roughness | Metallic |
|---|---|---|---|---|
| Stiles and rungs | steel, original Consortium grey, worn bright on rungs | #7C8488 / #B3B6B8 | 0.45 | 0.9 |
| Cage hoops | flat bar, rust | #7A3B1E | 0.75 | 0.6 |
| Rest grating | bar grating | #5E6366 | 0.6 | 0.9 |
| Wall behind | hull inner plating, seep-streaked | #4B5055 with #C9C3B6 mineral | 0.7 | 0.5 |
| Emergency strip | original photoluminescent strip, still faintly green | #7BD389 emissive 0.2 | 0.6 | 0.0 |

Weathering: the lower 12 m sits in the Hold's damp and is scaled white. The upper runs are wind-dried and dusty. Rungs are polished by sixty years of boots: HTR's cutters use these ladders daily. Markings: deck numbers stenciled every 6 m (`D-3`, `D-2`) in the climber's font, with HTR's survey elevations in orange beside them, and Guild chalk crosses on rungs that the Guild says are cracked. The photoluminescent strip along each stile is original and makes the ladders faintly visible in the Spine's dark, which is intentional: the honest route should be findable without a light.

Lighting: Office LED strips on the rest platforms (4000 K), aimed up the shaft so a climbing silhouette is lit from below.

Read by tier: from the Crown, four green threads converging into the dark. From the Bays, a ladder at the shaft lip. From the Hold, a ladder rising out of the water into the light.

## Variants

1. **Standard run.** 6 m, caged, rest platform at top. The dorsal run has landings at the Hold Ledge (Z 6), the Mouth Plate (Z 16), the Spine collar (Z 26), and the Spine head (Z 44).
2. **Broken-cage run.** Hoops gone on the shaft side. Nothing stops a Jack from taking a climber off the ladder. Used on the runs beside the drive dog ledges at Z 10 and Z 26.
3. **Foot run.** Bottom 2 m in the Wash where the Spine Foot's Belly face is torn open. Climbing out of water at 1.0 m/s for the first 2 m.
4. **Head run.** Top 6 m, open to sky, cage removed by HTR. Wind audio.

## Physics

- Ladder: static world, infinite mass. Dogs seat in stiles, rungs, and cage.
- Cage panels: Light, 14 kg each, held by 200 N (racked). Any Haul peels one off. Fragments of nothing; they are props.
- Ladder movement (05 section 2.3, canon): climb speed 2.0 m/s for all classes, 1.6 m/s for the Breaker. Spool usable while climbing; Cutter usable one-handed. Climbing players are grounded for line purposes (drag applies: a line above their foot-hold pulls them off the ladder into the shaft). Players on a ladder cannot brace.
- A player pulled off the dorsal ladder falls to the Spine Foot's crushed-cargo plug at Z 0, or out of the Foot's torn Belly face into the Wash. From Z 44: 29.4 m/s, scrapped (the Wash is 0.6 m deep, no multiplier). The Sump (1.2 m, x3.0, Stall at 36 m/s) is 4 m from the Well rent and is the map's only safe long drop; it is not under the Spine. A fall from the Spine head that is steered by a Jack toward the Belly can reach it.
- The Breathing: the shaft pitches with the hull (theta = 70 + 6 sin(2 pi t / 90) degrees). At Z 44 the ladder moves 4.6 m laterally over 45 s. Ladder speed is unaffected; the climber's world position swings. A Hang line from a climbing Breaker to a Bay gallery changes length by up to 2 m per half-cycle; one that crosses the Mouth Plate's drive chains' arc is severed by nothing (chains are props), but one that crosses the Shear panel's path at 1:15 is severed by it.
- Rest platforms: 1.2 m square grating. A player standing on one is a player with three open sides.

## Interactions

- **Haul** a climbing enemy from the catwalk opposite: they come off the ladder if T/m exceeds their foot-hold. A Breaker's 1,000 N takes every class off (11 m/s² on a Climber, 7.7 on a Rigger); across the 6 m shaft a Climber arrives at 11.5 m/s, flipped against the far wall, and the fall to the Hold does the rest. A Rigger's 600 N on a Breaker (2.7 m/s², under their 5 m/s² foot-hold) only makes them skid on the rungs.
- **Jack** off a stile: a Slingshot up the shaft. From the Spine collar (Z 26) a Climber or Doubler reaches the Crown deck in one Kick (18 m of rise); a Rigger needs two.
- **Hang** from a rest platform to yourself and step off: a Pendulum across the shaft that the Breathing lengthens. A hanging player climbs by Reel (hold Haul inside Hang, 1 m/s): the slow way up the Spine for a Scrapper who missed the ladder.
- Chain: a drive dog falling past a ladder takes its cage panels with it (14 kg at 22 m/s is 310 N s, a stagger, on anyone below) and anyone on the ladder within 1 m of the piston's 1.6 m diameter path.

## States

| State | Visual |
|---|---|
| Intact | Cage complete, strip glowing. |
| Stressed | Piston groan: dust and scale fall from the rungs above, strip flickers (power coupling on the same wall). |
| Failing | Cage hoops shear off in sequence ahead of a falling piston, 0.1 s per hoop. |
| Failed | Stiles bent out into the shaft, hoops gone, rungs intact. Still climbable. The ladder never fails as a route. |

## VFX and SFX hooks

- `SFX_Ladder_Rung_Grip` per class (Breaker is heavier, slower cadence), `SFX_Cage_Rattle` on Breathing peaks.
- `VFX_Scale_Fall` and `SFX_Hoop_Shear` during a piston pass.
- `SFX_Shaft_Wind` layered by height.

## Technical

- LOD0 900 tris per 6 m run, LOD1 400, LOD2 120, LOD3 two emissive lines (the strip).
- Textures: `T_Ballast_Steel_Trim` 2K shared, `T_Photolum_Strip` 256 emissive, `T_Ballast_Decal_Atlas` 2K.
- Pivot at the run's bottom rung center. Sockets: `Rung_0..19`, `Rest_Top`, `Cage_0..8` (panel spawn), `Strip`.
- Implementation: climbable volume per run with the ladder movement mode (05 section 2.3). Cage panels as pooled Light props with a per-panel anchor constraint at 200 N. The piston consequence pre-clears panels along its path on its own timeline.
- Parented to `BP_BallastHull`. Collision: `Kinetic_WorldStatic` (ladder), `Kinetic_Prop` (panels). The climbable volume is `Kinetic_Interact`.

## Reference search terms

caged fixed ladder industrial shaft, ship engine room ladder photoluminescent strip, elevator shaft maintenance ladder, mineral scale on steel ladder, demolition access ladder survey marks.
