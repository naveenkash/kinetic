# BALLAST: Lighting and Mood

_Arena: Ballast. File purpose: time of day, sky, key and fill, practicals per zone, atmosphere, palette, line-emissive compliance (map rule 8), post-process, sound bed, the post-Shear lighting change, and three beauty shots. Sources: docs/bible/03-arenas.md, docs/bible/05-core-mechanics.md. Inventions are marked **PROPOSED**._

## Time, weather, sky

| | |
|---|---|
| Time of day | 16:40 local. HTR's cut sheet on the Spine head kerb says "Shear B 16:40" and the match is scheduled to it |
| Sun | Elevation 25 degrees. Azimuth from the dorsal-port quarter: 35 degrees off the hull's -Y axis toward -X. The Rucu Gorge runs east to west and the climber lies along it, nose in the western floor, dorsal side facing up-gorge toward the afternoon sun, so the sun reaches the Crown through the open Lip and, after the Shear, rakes into the Bays through the port gap |
| Colour temperature | 3,400 K through dry gorge haze. Warm, not orange |
| Weather | Clear and dry. Wind 8 m/s down-gorge, gusting to 14 at the Crown. High thin cirrus. Visibility 2 km |
| Sky | Hazy equatorial blue overhead grading to a pale warm band at the gorge rim. The eastern wall behind the Crown is in direct warm light; the northern wall that carries the Office gallery is in shadow |
| Atmosphere | A dust haze that reads as depth beyond 40 m and is invisible under 10 m. No fog. No volumetrics in the Hold except the floodlights' own low-density cones. God rays only where the Crown's light falls down the Spine head, and only at a density that never raises the shaft's black level above the dim set-line emissive (rule 8) |

## Key and fill

- **Key.** The sun, entering over the Crown Lip and through the roofless dorsal half of the Crown. It lights the Crown deck's grating, the gantry beam's underside by bounce, and sends one shaft down the Spine head that reaches the Mouth Plate at a quarter strength by 16:40 and leaves it by 17:10 (the match is not that long; the god ray is static for a round).
- **Fill.** Sky light from the Lip, cool. Bounce from the eastern gorge wall, warm, into the Crown. In the Bays the fill is the Crown's lit plating seen through the rack frames: a dim warm top-light that leaves the Lower decks dark except where practicals hit. The Hold has no natural fill until the Shear.
- **Ratio.** Crown key to fill 4:1. Bays practical to ambient 8:1. Hold practical to ambient 16:1. Every tier darker than the one above, so a player always reads "down" as "darker" and the Well as the darkest thing on the map.

## Practicals by zone

| Zone | Type (doc) | Colour | Count | Note |
|---|---|---|---|---|
| The Wash | docs/objects/props-light/floodlight-stand.md | 4,000 K white, generator flicker (0.3 s dip every 12 s) | 3 (2 on the ledge, 1 in the Spine Foot) | Caustics from the Wash projected onto the tanks and walls, driven by the shove volume's direction |
| The Wash | docs/objects/environment/evidence-objects.md | Orange LED, 1 s blink | 1 | The flight recorder. The only coloured light below Z 6 |
| The Sump | none | | 0 | Lit only by the Hold floods' spill and, through the rent, the Hooks' red and green navigation lights |
| Hold Ledge | floodlight-stand (counted above) | | | Welding cart's pilot flame, blue, 2 cm |
| Spine Foot | floodlight-stand (counted above) | 4,000 K aimed up the shaft | | Makes every ladder climber a lit silhouette from below |
| The Spine | floodlight-stand | 5,600 K, one up and one down from the collar | 2 | Piston tape catches both |
| Lower Bays | floodlight-stand | 5,600 K daylight-balanced HTR work floods | 2 per bay | Aimed at the Spine; the racks are side-lit, the Belly strip is in their shadow on purpose |
| Mouth Plate | none of its own | | 0 | Lit by the Spine god ray and the four bay floods. The brightest spot on the mid tier, 1.5 stops above the Lower decks |
| Upper Galleries | floodlight-stand | 5,600 K | 1 each | Aimed down onto the stack tops |
| Rack Tops | none | | 0 | Crown spill only |
| Crown Deck | docs/objects/environment/floodlight-mast.md | Off (daylight). 5,600 K if the night variant ships | 2 | Torch pilot on the gantry carriage, blue |
| Cut Gantry | none | | 0 | Sunlit |
| Office gallery | docs/objects/environment/office-gallery.md | Cool white strip, 6,500 K | 1 | Visible over the Lip, never bright enough to read as a line |

Fluorescent orange cut-line paint (docs/objects/environment/chalk-and-graffiti.md) on the port wall and the Crown plating is high-albedo, not emissive. Yellow-black structural tape is non-emissive except the **PROPOSED** edge-lit variant on future Well edges, which comes on at the Shear siren at a tenth of line brightness.

## Palette

| Swatch | Hex | Use |
|---|---|---|
| Hull steel | #4A5159 | Bay walls, Spine faces, deck plates. Blue-grey oxidised |
| Scale and rust | #8C4A2F | Every original surface's lower half, water lines, the Slab |
| HTR orange | #F26B1D | Racks' repaint, sled, gantry, cut paint, condemnation notices |
| Rated tape | #F2C01E on #121212 | Yellow-black on every rated load and edge |
| Hold water | #1F3A3D | The Wash and Sump, with warm caustics |
| Sky | #8FB8D9 | Overhead, through the Lip and the Spine head |
| Warm key | #FFC98A | Sunlit grating, dust in the god ray, the eastern gorge wall |
| Shadow | #0F1418 | The Hold's corners, the gutter, the Well |

Faction accents appear only on players, banners and graffiti: Guild ochre, HTR orange, Ascendancy silver, Hanger stencils, Fray neon.

## Map rule 8: lines are the light

Target: a line at full class tension renders at least 2 stops brighter than the brightest non-line pixel in frame, in every zone, at every Breathing phase, before and after the Shear. Set lines (dim) must still be the brightest thing within 5 m when they brighten.

| Zone | Brightest competitor | How the line wins |
|---|---|---|
| The Hold | Floodlight faces (shielded, never pointed at the main sightlines), caustics | Floods have hoods; caustic intensity capped at 0.3 display white. Lines at 8x display white (HDR emissive) |
| The Spine | The sky square at the head, the god ray | Sky square is clamped by exposure keying to the shaft walls; the god ray's dust is capped at 0.5 display white. A line across the shaft reads against both |
| The Bays | Bay floods, the Mouth Plate pool | Floods are 5,600 K white at 0.7 display white on grating; lines are saturated team colour at 8x. The Plate pool is the one place a dim set line is hardest to see, so set lines brighten within 5 m (05 section 11) |
| Crown Deck | Sunlit grating, the sun disc over the Lip | Exposure keys to sunlit grating at 0.6 display white. **PROPOSED**: line emissive is composited after exposure, so a line stays at 8x display white even with the sun disc in frame; lens flare disabled; sun disc luminance clamped to 1.5x line peak. This is the same requirement the Anchor's darkened sectors impose, from the other direction |
| Crown Net | Sky | As the Crown |
| Post-Shear Bays | Raking sun, 17 m stack shadows | Exposure rises 0.7 stops over 2 s; lines unchanged at 8x; the contrast between lit port halves and dark starboard halves is allowed to crush shadows rather than lift them, because lifted shadows would make set lines in the starboard bay read brighter than they should |

Overspin's white flicker, the snap flash, and the Hooks' navigation lights are the only non-line emissives permitted above 1.0 display white.

## Post-process intent

- Warm highlights, cool shadows (split tone, subtle: hue shift under 10 degrees).
- Hull steel desaturated 15%; team colours untouched.
- Bloom only above display white, so only lines, the snap, and overspin bloom. Floods do not.
- Vignette under 10%. No chromatic aberration. No film grain in competitive. Motion blur at its minimum setting, because a sliding container's speed must be readable.
- Exposure adaptation 2 s up, 1 s down, so a player dropping from the Crown into the Spine sees the shaft dark for a moment, which is honest.

## Sound bed

| Zone | Ambient loops | Spot sounds |
|---|---|---|
| Everywhere | Hull hum, 40 Hz, rising a semitone at full lean | The Breathing groan, 1.0 s before each reversal: a 40 Hz moan rising to a steel shriek over 0.8 s, then 0.5 s of silence |
| The Hold | Water lap tied to the shove volume; drips every 1 to 3 s; generator chug on a 12 s loop; Hook rotors through the rent (110 Hz drone with a 23 Hz blade pass) | Tank boom (hollow, 1 s), then the rush. Crate scrape on the Slab |
| The Spine | Wind whistle down the shaft; ladder rattle; drive chain clink, louder on the lean | Ratchet rail pawl click per 0.5 m (Haul). Drive dog groan: hydraulic 60 Hz with a rising metal scream, 1.0 s, then the fall, then the Foot impact heard across the map |
| The Bays | Rack dogs ticking as the lean loads them; container skins ticking; floodlight ballast buzz; cutter sparks from above the cut face every 30 s | Break-free lurch. Rack-dog snap on release. Container-on-grating impact |
| The Crown | Wind 8 m/s with gusts; banners flapping; gantry cable twang; HTR cutters above | Column shear (bolt pops, then the beam) |
| Marshal | From the Office gallery PA over the Lip, and in every player's comms | "Live." "Shear." "Full shear." Scrap calls |

Unique tells, in priority order for the mixer: the Breathing groan, the Shear siren (HTR's two-tone site siren, 440 and 660 Hz alternating, 5 s), the drive dog groan, the tank boom, the rack-dog snap, the pawl click. Lines singing and the snap sit above all of these in the mix per the bible.

## After the Shear

| Change | Detail |
|---|---|
| Light | The port Bay wall is gone from Z 6 to 42. The sun rakes into both bays through the gap at 25 degrees: the port half of each Lower deck is lit, the starboard half is in the stacks' 17 m shadows. The Hold Ledge's port end is lit. The Mouth Plate is now side-lit as well as top-lit |
| Exposure | Bays rise 0.7 stops over 2 s. The Hold rises 0.3 stops |
| Practicals | The Port Upper Gallery floodlight fell with the gallery. Both Port Lower Bay floods remain (loose on the deck) and now compete with sunlight; they read as off |
| Particles | A 10 s dust burst at T+0, then clear. Never a lingering haze |
| New lights | Two Hooks take station off the port side at Z 20 with red and green navigation lights, visible from both bays and the Crown |
| Tape | The **PROPOSED** edge-lit tape on the new edges holds at a tenth of line brightness for the rest of the round |
| Sound | Wind enters the Bays. Hook rotors are audible in the Bays for the first time. Banners on the port side are gone |

## Beauty shots

1. **The Shaft.** Camera on the Mouth Plate, looking straight up the Spine. Six piston ends step up the shaft, port and starboard, each with its yellow-black band. Two drive chains hang 6 degrees off plumb. At Z 30 a silver Ascendancy set line crosses the shaft, dim, with a Climber's flight path about to meet it. Above everything, the square of sky, pale blue, the gantry beam a black bar across it.
2. **Full Lean.** Camera on the Starboard Upper Gallery at t = 22 s, looking across the shaft to the port stacks. Every chain in the bay hangs the same way. The god ray from the Spine head is full of dust sifting from the Crown. A container's rack dog is sparking under load. Two riggers on the Lower deck below are lit only by their own singing lines, one ochre, one orange, pulling against each other across the Plate.
3. **After the Shear.** Camera on the Crown deck's port edge, looking down. The port wall is gone. The Lower deck ends in a raw edge 28 m below; the gorge floor is 100 m below that, in shadow. Two Hooks hang in the gap with their lights on. The sun rakes across the port stacks from the left and throws their shadows across the bay. On the edge, a container is sliding, slowly, with a line on it, and the line is the brightest thing in the frame.
