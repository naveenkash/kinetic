# Catwalk Section

One-line: structure, destructible, Heavy tier, 360 kg, 4.0 m long x 1.2 m wide x 1.1 m handrail height, Ballast: the port-starboard cross-catwalk (4 m wide) through the Mouth Plate at Z 16 on the Lower Bay decks, the 2 m Upper Gallery catwalks along both bays' dorsal walls at Z 26, and the Hold Ledge grating at Z 6, per `arenas/ballast/layout.md` (Hold floor Z 0).

## Role in play

Catwalks are the Bays' connective tissue. Most Ballast fights start on one. They are narrow, so a Clothesline closes one in a single line. They cross the Spine shaft, which puts them in the fall line of a drive dog piston (see `rated/drive-dog-piston.md`). The Shear takes the port-side catwalks with the bay wall at 1:15.

Named rigs: the Clothesline, the Caboose onto a sliding container, Overboard into the Spine shaft, the Draw (a Hang across the shaft that catches a swinging Climber).

| Harness | Relationship |
|---|---|
| Climber | Crosses by Slingshot above the rail. Perches on the underside stringers. |
| Scrapper | Fast lines between handrails. Grate fragments become 90 kg projectiles. |
| Rigger | Two Clotheslines on one catwalk makes it impassable for 12 s. |
| Doubler | A full-overspin snap delivers 1,500 N s to whoever is on the catwalk: a Stall. It does not break the section (2,500 N s). |
| Breaker | The only harness that can be certain of crossing without being pulled off: a braced foot-hold of 20 m/s² holds against every line in the game. |

## Appearance

Silhouette: open steel grating on two C-channel stringers, with a tube handrail on both sides and a mid-rail. Every 4 m a hanger rod rises to the deck above or a stanchion drops to a rack.

| Part | Material | Base color | Roughness | Metallic |
|---|---|---|---|---|
| Grating | serrated steel bar grating, 30 mm | #5E6366 | 0.6 | 0.9 |
| Stringers | C-channel, climber-grey original paint | #7C8488 | 0.5 | 0.8 |
| Handrail | steel tube, Office yellow where repainted | #F2C500 / #7C8488 | 0.55 | 0.8 |
| Hangers | threaded rod, rust | #7A3B1E | 0.8 | 0.6 |

Weathering: the climber's original paint survives on undersides. Topsides are worn to bare metal in a 0.6 m walking stripe. Seep water drips through the grating onto the Hold, so stalactites of mineral scale hang off the underside. Stencils: `TENACITY  DECK 3  CROSSOVER` in Consortium sans-serif, and HTR's demolition sequence numbers (`CUT 14`) in orange spray on sections scheduled to go. Office prep crews have zip-tied LED strip lights to the underside of every catwalk so that a player below can read the silhouette; the strips are cool white, 4000 K, 300 lm per meter.

Read by tier: from the Crown, two bright parallel strips crossing the dark shaft. From the Bays, a railing at eye level and footsteps on grating. From the Hold, lit rectangles overhead with water falling through them.

## Variants

1. **Bay crossover.** 4 m straight section between racks. Hangers to the deck above.
2. **Mouth Plate.** An 8 m x 8 m grated platform at Z 16 spanning the Spine shaft, carrying the cross-catwalk, the Claim marker's authored position, and the Haul spool's start sled, clustered from four sections; fractures at 2,500 N s like any catwalk. The piston's fall line.
3. **Condemned.** Grating partly removed, orange mesh on the rail, one stringer visibly cut. Fractures at 1,200 N s (**PROPOSED**, half the standard). Used on the Shear side.
4. **Stair run.** A 4 m section at 35 degrees with treads. The port and starboard stairs from the Lower Bay decks (Z 16) to the Upper Galleries (Z 26), one switchback each. Same threshold.

## Physics

- Tier: Heavy (360 kg). Fragments per 05 section 6.5: four Medium grate pieces (90 kg each) plus Light handrail tube (10 s lifetime).
- Anchoring: bolted to stringers and hangers. Static to lines: a Haul on the rail sings and nothing moves. The section only moves after fracture.
- Fracture threshold: 2,500 N s. Worked: a 2,000 kg container sliding on the lean at 1.25 m/s delivers 2,500 N s. A Breaker falling onto it from 12 m (15.3 m/s) is 3,370 N s and goes through. A propane blast cannot do it alone (1,500 N s at center). A drive dog piston at any speed does it.
- A grate piece (90 kg, Medium under 100 kg, so it gets the Scrapper's 30% bonus) Hauled by a Scrapper: 400 N / 90 kg = 4.4 m/s², about 13 m/s at the end of a 20 m line, 1,200 N s: a Stall on anyone it hits, with no margin. A Breaker's 1,000 N gets it to 13 m/s in 6 m.
- Players on a fracturing section go airborne with the piece. From an Upper Gallery (Z 26) to the Lower deck is 10 m, 14 m/s, scrapped. From the Mouth Plate (Z 16) down the shaft to the Spine Foot is 16 m, 17.7 m/s, scrapped.
- On the Breathing, the catwalk is a child of the hull root and tilts with it. Loose props on grating slide above 3 degrees.

## Interactions

- **Haul** a player to a handrail: a grapple. Haul from a catwalk to a sliding container: you leave with it.
- **Jack** off a stringer underside: a Slingshot upward through the shaft. Jack between a player and the rail behind you: Overboard into the shaft.
- **Hang** rail to rail across the shaft: a bridge for players (solid), not for objects. A piston falls through it.
- Chain: a container Avalanched onto B-Low fractures it, and the grate pieces fall onto B-Low's opposite number.
- The Shear: the port Upper Gallery's outer edge and the port Lower deck's outer 4 m strip are parented to the shed wall panel and leave with it; the panel is kinematic for its fall and severs every line it passes through (05 section 3.5).

## States

| State | Visual |
|---|---|
| Intact | Grating flat, rail straight, LED strip lit. |
| Stressed | Shear siren only: strip flickers, hanger rods hum, dust from bolts. 5 s. |
| Failing | Stringer buckles at the impact point, grating tears with a shriek, strip goes out. 0.4 s. |
| Failed | Stub ends with bent hangers. Grate pieces below. |

## VFX and SFX hooks

- `SFX_Catwalk_Footstep_Grate` per class mass (Breaker rings it).
- `SFX_Catwalk_Tear`, `VFX_Catwalk_Sparks`, `VFX_Catwalk_Dust` on fracture.
- `VFX_Catwalk_Strip_Flicker` driven by the stressed state.
- Water drip particles under every section, budget 8 emitters per Bay.

## Technical

- LOD0 2,400 tris, LOD1 1,200, LOD2 400, LOD3 card. Grating is a masked material, not geometry, at LOD1 and below.
- Textures: `T_Catwalk_Trim` 2K, `T_Grating_Tile` 1K masked, decal atlas shared with Ballast signage.
- Pivot at the section's start end, deck height. Sockets: `Rail_L_0..3`, `Rail_R_0..3` (1 m spacing, Dog seat hints), `Hanger_A`, `Hanger_B`, `Strip`.
- Chaos: geometry collection per section. Level 0 = section (anchored to hull root via anchor field), level 1 = four grate pieces plus stringer halves, level 2 = rail tube. Damage threshold at level 0 from 2,500 N s. Condemned variant halves the threshold. Parent to `BP_BallastHull` so the Breathing moves it.
- Collision: `Kinetic_WorldStatic` intact, `Kinetic_Prop` fragments. Mark the Mouth Plate with `Tag.PistonFallLine` so the upper drive dog's consequence can pre-fracture it on schedule.

## Reference search terms

industrial bar grating catwalk, ship cargo hold crossover walkway, serrated grating, cable tray handrail, mineral scale drip stalactite industrial.
