# Floodlight Mast

One-line: environment, static world with a destructible head, Heavy head (640 kg) on an infinite-mass mast, 30 m lattice mast (Hangtown rims, 4 per rim), 24 m dock masts in the Yard (four, at X plus or minus 52, Y plus or minus 40, each with a 4 m x 4 m platform carrying three floods and a ladder from the dock), and two roof heads on the Yard control shack (roof Z 14), 3 to 8 lamps per head at 4000 K, up to 120,000 lm per head. Geometry per `arenas/the-yard/layout.md`. The Office's light.

## Role in play

Masts are the Office's lighting rig and the reason arenas are lit the way they are (03, rule 8: a singing line is always the brightest thing in frame). They do not move and most of them cannot be reached. The Yard shack masts fall with the control shack (see `rated/structural-column.md`) and darken the Rails' east sector, which is the Yard's "what breaks" entry for lights. A dark sector is a sector where lines are the only illumination, which is the point.

| Harness | Relationship |
|---|---|
| Climber | Mast platforms are the Yard's mid-high perches (24 m, outside the Gantry's span, with half the Yard in view each) and the Hangtown rim heads the highest (30 m above the deck). |
| Scrapper | A broken lamp head drops 8 Light lamp housings (6 kg each): junk. |
| Rigger | A Hang from a mast platform over a Yard dock is a 24 m Gallows, hoisted by Reel at 1 m/s. |
| Doubler | Snapback off a mast to clear the control shack roof. |
| Breaker | Drops the shack masts via the column. Cannot reach the others. |

## Appearance

Silhouette: a triangular lattice mast on a flanged base, a ladder up one face, a square lamp head with lamps in two rows angled down, a cable tray down the mast to a yellow Office junction box. Hangtown rims: tall and guyed. Yard: short, bolted to the shack roof and the Rails legs.

| Part | Material | Base color | Roughness | Metallic |
|---|---|---|---|---|
| Lattice | galvanized angle, Office grey | #6E7378 | 0.55 | 0.8 |
| Lamp housings | die-cast aluminium, black | #1A1A1C | 0.4 | 0.6 |
| Lamp lenses | tempered glass, lit | emissive, 4000 K | 0.1 | 0.0 |
| Junction box | Office yellow | #F2C500 | 0.6 | 0.6 |
| Guys (Hangtown) | 16 mm wire, not cuttable, untaped | #6E7378 | 0.4 | 0.9 |

Weathering: the Office's gear is the best maintained on any map. Galvanizing clean, lamps matched, a Hook-serviced ladder. Insects at the lamps (a particle system). Markings: `OFFICE OF THE LAY  LIGHTING UNIT 14  VISIBILITY CLAUSE COMPLIANT` on the junction box, an Office seal, a LayCast sticker (`LIT FOR BROADCAST`). Lighting: each head is an actual light source in the lighting build: a spot cluster at 4000 K, 120,000 lm, aimed per arena. Key rule: intensity is tuned so that a line at full force reads at least 2 stops brighter than any lit surface in frame. Masts never point at the camera's typical angles; they rake across.

Read by tier: from the top tier, the heads at eye level or below, blinding if looked at (an authored bloom clamp keeps them readable). From the mid tier, a rake of light with hard shadows. From the low tier, light through grating.

## Variants

1. **Rim mast (Hangtown).** 30 m, guyed, 8 lamps. Aimed down onto the Upper Hang. Indestructible.
2. **Shack roof heads (the Yard).** Two 3 m stubs with 3 lamps each on the control shack roof (Z 14). Fall with the shack (see `rated/structural-column.md`). Light the west third of the Rails and the North Dock below.
3. **Dock mast (the Yard).** 24 m, a 4 m x 4 m platform with three floods and a dock ladder (2.0 m/s, Breaker 1.6). Lights the trenches and the Floor. Indestructible; its lamps are not (300 N s each). The Lockout sudden-death event cuts all four.
4. **Portable (apprenticeship, Ballast Hold).** 3 m tripod, 2 lamps, Medium (60 kg), a prop. Any line moves it; its light moves with it, which the tutorial uses to teach the Visibility Clause.

## Physics

- Mast: static world, infinite mass. Dogs seat on the lattice and the head. Perch surface at the head.
- Head (shack mast): a Heavy body (640 kg) parented to the shack; it falls with the shack on the column consequence, not on its own. Lamps are 8 Light housings (6 kg) that detach on impact (300 N s each from a fall) and are 10 s props.
- Lamp breakage: any impact over 300 N s on a housing kills that lamp (glass VFX, light off). A propane blast within 4 m kills every lamp on a head. A Hauled 90 kg grate piece at 13 m/s into a leg mast head kills it. This is the deliberate way to darken a Yard trench without a Breaker: Scrappers do it.
- Light sector toggle: each head owns a lighting sector tag. Head dead (all lamps off, or head fallen) sets `Lighting.Sector.<name> = Off`. Lines, Dogs, and set lines render at full emissive regardless; the Hangtown lanterns and Ballast work lamps are not in any mast's sector and stay on.
- Guys (Hangtown): untaped, uncuttable, static. Dog seats.

## Interactions

- **Haul** self to a head: a perch 30 m up in Hangtown, 24 m in the Yard.
- **Jack** off a head: the Slingshot from the highest fixed point on the map.
- **Hang** from a head to an enemy, then hold Haul: the Gallows, hoisted by Reel at 1 m/s under the class Hang load (05 section 3.2). From a Yard dock mast the hoist carries them up past the platform and leaves them hanging 24 m over the dock they were standing on; only a Breaker's Haul lifts a Climber outright (11 m/s²).
- Chain: a shack falling with its roof heads puts 6 lamps out and darkens the west third of the Rails and the North Dock; the only light there is whatever lines are singing.

## States

| State | Visual |
|---|---|
| Lit | All lamps on, insects, a faint hum. |
| Partial | Some lamps out, broken glass under the head. |
| Dark | All lamps out; the junction box's standby LED stays red so the mast is still findable. |
| Fallen (shack heads) | Stubs in the shack ruin on the North Dock; lamps scattered. |

## VFX and SFX hooks

`SFX_Floodlight_Hum_Loop`, `SFX_Lamp_Pop` (per lamp), `SFX_Mast_Lattice_Ring` on impact; `VFX_Lamp_Glass_Burst`, `VFX_Insects_Lamp`, `VFX_Light_Shaft_Volumetric` (budgeted: two shafts per arena in the dustiest sectors), `Lighting.Sector` toggle.

## Technical

- LOD0 4,500 tris (30 m) or 3,600 (24 m with platform), LOD1 half, LOD2 lattice as a masked card, LOD3 a pole and a bright quad.
- Textures: `T_Office_Lattice_Trim` 2K, `T_Floodlight_Head` 2K, `T_Office_Decal_Atlas` 2K.
- Pivot at the base flange. Sockets: `Head`, `Lamp_0..7`, `Perch`, `Ladder_Top`, `Junction`, `Guy_0..2`.
- Lighting implementation: one spot light component per lamp at LOD0 within 60 m, merged to one per head beyond; shadow casting from the head's merged light only. Sector tags on the lighting manager; dead-lamp state is replicated as a bitmask per head.
- Collision: `Kinetic_WorldStatic` mast and platform; `Kinetic_Heavy` head when falling with the shack; `Kinetic_Prop` lamps. Mast platforms are moving-platform-free static perches with `Perch` sockets at the rail.

## Reference search terms

stadium floodlight lattice mast, port floodlight tower guyed, high mast lighting rail yard, die-cast LED flood housing, temporary site lighting tripod.
