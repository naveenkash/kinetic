# Market Platform

One-line: structure, cable-hung ring with destructible stalls, Heavy tier as an assembly (each of six deck segments 2,400 kg; stalls Medium), 18 m outer diameter, 4 m wide annular deck with a 10 m open center, Hangtown Mid Hang center: the Claim marker's authored position and the Cable Day event site.

## Role in play

The market ring is Hangtown's one open floor and its Claim point. It hangs from four main load cables by eight hangers and is braced to six houses by swing bridges. In Claim mode the marker sits on the north segment. Everything about the platform is built so that the Cable Day event can drop its south half to a 40 degree slope and leave it hanging by its remaining lines; after that, the Claim marker slides, stalls slide, and the ring becomes a ramp into the Well.

| Harness | Relationship |
|---|---|
| Climber | Perches on the hanger cables above the deck, drops onto the marker, leaves by Slingshot off a stall post. |
| Scrapper | Six stalls of Light and Medium ammunition. The Kiss with two propane cylinders from the food stall is the market's standard opener. |
| Rigger | The open center is a 10 m hole: a Hang across it is a tripwire nobody sees against the sky. |
| Doubler | Two lines to two stalls, Scatter into both approach bridges. |
| Breaker | Cannot move the platform (hangers are load cables, not slack). Can brace on the high side after Cable Day and own the slope. |

## Appearance

Silhouette: a flat timber ring with a steel ring-beam under it, eight hanger cables rising to the main cables like tent lines, six timber stalls with corrugated roofs around the outer edge, a rope rail around the inner hole. Lanterns and string lights everywhere.

| Part | Material | Base color | Roughness | Metallic |
|---|---|---|---|---|
| Ring-beam | welded hull plate box section | #4B5055 | 0.5 | 0.8 |
| Deck | mixed timber, painted in neighborhood colors | #8C7355 with #2F6FAD and #C8452B bands | 0.85 | 0.0 |
| Stall frames | timber and scaffold tube | #8C7355 / #8A8F93 | 0.8 | 0.3 |
| Stall roofs | corrugated steel, rust and paint | #7A3B1E / #3E7A5B | 0.7 | 0.6 |
| Hanger cables | 36 mm wire rope | #6E7378 | 0.4 | 0.9 |
| Lanterns | paper and plastic, lit | #F4B860 emissive | 0.9 | 0.0 |

Weathering: the deck is swept. It is the most cared-for surface in the game, and it shows: paint touched up, boards replaced in mismatched colors. Stall counters are polished by elbows. Markings: the blue hook stencil of Vilca's Hang on the ring-beam, chalk price boards, a painted compass rose at the center hole's edge, LayCast's seasonal Claim marker decal (Office yellow circle, 4 m radius, repainted every season). Condemnation notices on the south stalls only; the north stalls have torn them off. Lighting: lanterns at 2700 K, 60 lm each, 48 of them, plus the Office's two floodlights aimed from the Upper Hang.

Read by tier: from the Upper Hang, a lit ring with a dark hole. From the Mid Hang, a warm room at the end of every bridge. From the Low Hang, the underside: ring-beam, hangers, and lantern light through gaps in the deck.

## Variants

1. **Standing (season default).** Six stalls, level ring.
2. **Dropped (post Cable Day).** South half at 40 degrees, two south stalls gone, deck boards sliding.
3. **Cut district (seasonal).** Four stalls, south bridges removed, ring hanging on six hangers and visibly out of level by 3 degrees. Used when HTR holds the Hangtown Claim.
4. **Night market (cosmetic event).** All lanterns doubled, no gameplay change.

## Physics

- Deck segments: six, each Heavy (2,400 kg), clustered into one ring-beam body. Static to lines.
- Stalls: Medium (160 kg frame plus contents). Fracture 300 N s per 05 section 6.5 (market stall), fragments Light planks. Contents: 2 propane cylinders (food stall), 1 fuel drum (generator stall), chairs, crates, laundry.
- Hangers: eight, each a load cable (not cuttable). The south four are the Cable Day casualties: a scripted constraint break at 1:15 remaining drops the south half until the two surviving south hangers stop it at 40 degrees.
- Slope physics after the drop: g sin 40 = 6.3 m/s² along the deck. Against the foot-holds in 05 section 5.3 (Climber 3, Scrapper 4, Rigger 5, Doubler 5, Breaker 5): an unbraced Breaker (5) slides, a braced Rigger (9) stands, a braced Breaker (20) stands, everyone else slides at once. Loose objects reach the low edge (9 m of deck) in 1.7 s at 10.7 m/s. The Claim marker (Heavy, 900 kg on polished runners at friction 0.05 per 05 section 6.1a) slides at 5.9 m/s² and goes into the Well unless held; per 05 section 9.4 it resets to its authored position on entering the Well.
- Impacts: a 160 kg stall sliding at 10 m/s is 1,600 N s. Scrapped.

## Interactions

- **Haul** the Claim marker: 900 kg on runners (friction 0.05). A Breaker's 1,000 N nets 0.62 m/s², a Rigger's 600 N 0.18, a Climber's or Scrapper's 400 N nothing (0.44 against 0.49 of friction). After the drop, no line Hauls it up a 6.3 m/s² slope. A Hang from the high side holds it: the slope component is 900 x 9.8 x sin 40 = 5.7 kN, under a Rigger's 12 kN, Doubler's 8 kN, or Breaker's 10 kN Hang load and over a Climber's or Scrapper's 4 kN, which slip 0.5 m per 0.1 s. A Hang never unseats it (05 section 6.2); it only holds.
- **Jack** between a player and a stall post: Overboard through the center hole. 10 m of nothing, then the Low Hang, then the Well.
- **Hang** across the center hole at 1.2 m: an invisible Clothesline against the sky. Hang from a hanger cable to yourself: a Pendulum through the hole.
- Chain: the Kiss on the food stall's cylinders (racked, 200 N break-free) blasts the north stalls into Light planks and flips everyone within 4 m. The generator drum leaves a 3 m fire patch for 6 s on the only level floor on the map.

## States

| State | Visual |
|---|---|
| Intact | Level ring, lanterns steady. |
| Stressed | Cable Day siren: the south hangers visibly stretch, lanterns swing, dust from the ring-beam welds. 5 s. |
| Failing | Two south hangers part with the snap sound at full scale, the south half drops 0.8 s to 40 degrees and bounces once. |
| Failed | Ring hangs broken-backed, south stalls gone, boards sliding, lanterns dangling off the low edge. |

## VFX and SFX hooks

- `SFX_Market_Ambience` (lantern flutter, canvas, no voices during a match).
- `SFX_Hanger_Strain` during stressed; `SFX_Cable_Snap_Large` on the break.
- `VFX_Deck_Board_Slide` and `VFX_Lantern_Swing` driven by slope.
- Stall fracture uses the shared `VFX_Stall_Splinter`.

## Technical

- LOD0 ring 14,000 tris plus 2,200 per stall, LOD1 half, LOD2 quarter, LOD3 card for the ring from the Rim-equivalent distance.
- Textures: `T_Hangtown_Timber` 2K, `T_Market_Props_Atlas` 4K, `T_Hangtown_Decal_Atlas` 2K, `T_Lantern_Emissive` 512.
- Pivot at ring center, deck height. Sockets: `Hanger_0..7`, `Bridge_N/NE/SE/S/SW/NW`, `Marker_Spawn`, `Stall_0..5`, `Flood_A`, `Flood_B`.
- Chaos and constraints: ring-beam as one rigid body on eight constraints to Sway anchor proxies. Cable Day is a scripted constraint destroy on hangers 4 and 5 and a stiffness reduction on 3 and 6, then a kinematic blend to the authored 40 degree pose over 0.8 s so every client lands on the same pose. Deck boards on the south half swap to Light props on a budget of 24.
- Stalls as geometry collections, level 0 whole stall, level 1 planks. Contents are pooled props placed at round reset.
- Collision: `Kinetic_WorldDynamic` ring, `Kinetic_Prop` stalls and contents, `Kinetic_KillVolume` overlap below the center hole owned by the Well.

## Reference search terms

hanging market platform, floating market stalls corrugated roof, Tibetan bridge market, lantern night market, suspended ring deck wire rope hangers.
