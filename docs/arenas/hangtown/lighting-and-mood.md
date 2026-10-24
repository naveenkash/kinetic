# HANGTOWN: Lighting and Mood

_Arena: Hangtown (Vilca's Hang). File purpose: time, weather, sky, key and fill, practicals per zone, atmosphere, palette, map rule 8 compliance, post-process, sound bed, lighting after Cable Day, three beauty shots._
_Sources: docs/bible/03-arenas.md (map rule 8: lines are the light; Hangtown narrative weight), docs/bible/01-world-and-lore.md (tone and look), docs/bible/05-core-mechanics.md (3.5 line emissive, 11 tells). Zones per docs/arenas/hangtown/layout.md. Everything not in the bible is **PROPOSED**._

## Time, weather, sky

- **Time:** blue hour, 25 minutes after sunset. The sun is below the western Collar half. The sky holds light; the ravine does not.
- **Weather:** dry, a steady 6 m/s wind along the ravine from the south (the Sway's fiction). No rain. No haze above the Low Hang. Dust only where something breaks.
- **Sky:** deep blue zenith grading to a thin orange-grey band in the west behind the West Rim. The two Collar halves are black silhouettes, 200 m tall, with their own lit windows (the rest of the district, evacuated, lights left on). Stars faint. No moon (a moon would compete with lines on the High Lines).
- **The vista:** north and south, the rest of Vilca's Hang recedes along the ravine, strung in warm points of light, swaying. It should look like a city hung on a clothesline.

## Light sources

| Role | Source | Direction and color | Notes |
|---|---|---|---|
| Key | Office floodlight masts (docs/objects/environment/floodlight-mast.md), 3: West Rim (-60, +20, 32), East Rim (+60, -20, 32), headframe top (30, 0, 38) | Aimed down and inward at the Hub and the two Rows. 5000 K, slightly green-white (cheap LED). | Hard louvers. No lamp face is visible from any playable position below Z 30. Each mast lights its half of the Mid Hang from above at about 40° elevation, so roofs are bright and porches are in shadow. |
| Key, local | The Hub pendant (docs/objects/props-light/floodlight-stand.md) | Straight down onto the Claim marker, 5000 K, tight 30° cone | The marker sits in a 6 m pool. The only source visible from the nets, and it is a downlight with a skirt. |
| Fill | Sky | From above, 12,000 K blue, low intensity | Blue fill on every upward face. Shadows are blue, not black. |
| Fill, warm | House practicals | From inside out through windows and doors, 2700 K | Residents left their lights on. Each window throws a warm rectangle onto the next house or the deck. |
| Bounce | None authored | | Timber is dark; let it be dark. |
| Below | Hook beacons in the mist | Red, pulsing, 1 Hz, far below | Visible only when looking down past the nets. Never bright enough to light a face. |

Sun contribution: none direct. Ambient from the sky dome only.

## Practicals per zone

| Zone | Practicals | Type | Color | Count |
|---|---|---|---|---|
| Z1 West Rim, Z2 East Rim | Prep tent work lights, mast base beacons | bare tube, amber beacon | 4000 K, amber | 2 tubes, 1 beacon per rim |
| Z3 High Lines | Hand-cable marker lamps every 10 m | small caged bulb | 2700 K | 5 per line, 20 total |
| Z4 Hub | The pendant (key); two red aviation lamps on the drum | caged bulb | red | 2 |
| Z5 Headframe and Lift | Car lamp (green when departing, white otherwise), winch house window, landing lamps top and bottom | caged bulb, tube | 2700 K, green cue | 4 |
| Z6 Market Ring | String lights on the corner posts around both halves; stall lamps | festoon bulbs, 15 lm each; shaded bulbs | 2200 K sodium-orange | 60 festoon, 8 stall lamps |
| Z7 West Row, Z8 East Row | One interior light per house, porch lamp on the large houses | bulb, shaded | 2700 K | 12 interior, 4 porch |
| Z9 Drip Nets | None of their own. Lit by spill from the ring's underside and the Hub pendant's skirt leak | | | 0 |
| Z10 Garden Terraces | Grow lamps on the planter rails, 1 per section | tube | 3500 K, slightly violet | 12 |
| Z11 Low Shanties | One interior light each | bulb | 2700 K | 2 |
| Z12 West Face | Ladder floodlight-stand at the foot, aimed up the ladder | louvered flood | 5000 K | 1 |
| Z13 Lift bottom landing | Landing lamp | caged bulb | 2700 K | 1 |

Every practical is a point of light in frame but never a flood. The budget rule: no practical surface may exceed one eighth of a line's emissive at its dimmest (a set line beyond 5 m).

## Fog and atmosphere

- Above Z -4: none. No volumetric fog, no height fog, no god rays, no dust haze at rest. The air in the playable volume is clear so that a line is the brightest and sharpest thing in every frame (map rule 8).
- Below Z -4: the ravine mist. A dense height fog with its top at Z -4, 8 m below the nets, grey-blue, lit from below by the Hook beacons. From the nets it reads as a floor you do not want. From the High Lines it reads as the bottom of the world.
- Transient: dust from a pin socket (10 s), smoke from a fuel drum fire (6 s), splinters from a house (2 s). All short, all local, none above 30% opacity.
- Laundry moves in the wind; it is the visual carrier of the Sway.

## Palette

| Swatch | Hex | Use |
|---|---|---|
| Hang timber | #6B5A45 | House walls, decks, bridges, walkways. Weathered, oiled. |
| Cable steel | #4A4F55 | Main cables, hangers, guides, the Hub, the Lift. |
| Collar concrete | #8C8A80 | Rims, the West Face. Stained with sixty years of cable rust. |
| Hanger paint | #2FA3A0 | Doors, shutters, stall frames. The district's color. Teal reads against orange and yellow tape at any distance. |
| Slack cable tape | #FF6A00 | Only on slack cables. Nothing else in the arena is this orange. |
| Rated tape | #F2C200 on #111111 | Anchor pins, counterweight, Hub clamps, hatch cover. Rule 7. |
| Blue hour sky | #1B2A4A | Sky, blue fill, shadows on upward faces. |
| Window warm | #FFB45A | House practicals, festoon lights, the glow a house throws on its neighbor. |
| Mist | #5E6F7F | The ravine fog. The color of the Well. |
| Team Amber | #E69F00 | Team A lines, plate rim bands, Dog trails (05 section 11a, canonical). |
| Team Sky | #56B4E9 | Team B. |
| Bone | #E8E6E0 | Neutral and set lines. |

Team tints are canonical in 05 section 11a: Amber #E69F00, Sky #56B4E9, Rose #CC79A7 (3v3v3 only), Bone #E8E6E0 for neutral and set lines, with a 1 m dark tick on enemy lines. Amber sits 16° of hue from the slack-cable orange and 7° from the rated-tape yellow, so hue does not separate a Team A line from Hangtown's two warning tapes. What separates them: tape is matte and never emissive (albedo capped at 0.6, no specular bloom), a line is emissive at 20 to 600 HDR and sings, and the enemy tick pattern is unique to lines. Hangtown's slack-cable tape also carries black fray marks every 0.2 m so its pattern never reads as a clean Amber line. Validate in the Cable Day state with an Amber crew holding the east half; if a slack cable is ever mistaken for a set line in playtest, the fix is to darken the tape, never to change the tint.

## Map rule 8 compliance per zone

Line emissive scale (unitless HDR, **PROPOSED**): set line beyond 5 m: 20. Set line within 5 m: 40. Live line at zero force: 40. Live line at full class tension: 400. Overspin flicker: 600. Practical bulbs cap at 5. Floodlight faces are never in frame. Sky dome peaks at 1.5. Mist at 0.4.

| Zone | Competing brightness | Resolution |
|---|---|---|
| Z1, Z2 Rims | The floodlight masts are here | Masts face inward and down; from the rim deck you see their housings, not their faces. The parapet lamps are shaded downward. |
| Z3 High Lines | Open sky all around | Sky at 1.5 is under any set line. Marker lamps at 5. A line across the ravine reads from 100 m. |
| Z4 Hub | The pendant | Louvered skirt; from the Hub deck the pendant is below you and invisible. From the nets it is a 30° cone pointed away from your eye. |
| Z5 Lift | Car lamp, landing lamps | Caged bulbs at 5. The green departure cue is a cue, not a flood. |
| Z6 Market Ring | 60 festoon bulbs, the pendant pool | Festoons at 15 lm, capped at 5 HDR. The pool lights the deck, not the air. A line across the ring at 40 is eight times the brightest bulb. |
| Z7, Z8 Rows | Warm windows | Interiors at 5, window glass at 4. Lines through a window read. |
| Z9 Nets | Looking up at the pendant and the ring's underside | The pendant's skirt. The ring's underside is unlit timber. Lines above you against the blue sky are the only hard edges. |
| Z10, Z11 Low | Grow lamps | Tubes at 4, violet so they never read as a line color. |
| Z12 West Face | Ladder flood aimed up | Louvered; from the ladder you are lit, you do not see the lamp. From the rows you see a lit climber, not a lamp. |
| After Cable Day | Arc flash at the NE socket, swinging pendant | Flash capped at 40 for 0.1 s every 4 s (equal to a line at zero force, never above). Pendant sweep stays a downlight. |

Validation: before any other tuning, place a set line in each zone, look at it from the darkest and the brightest vantage, and confirm it is the brightest pixel group in frame. Then do it again in the Cable Day state. 03's Anchor engine note says the same thing; it applies here.

## Post-process intent

- **Exposure:** fixed per tier, no auto-exposure. Upper tier EV 0, Mid EV -0.3, Low EV -0.6, blended over 1.5 s on crossing Z 10 and Z 25. Auto-exposure would re-scale line brightness every time a player looks at a window; it is off.
- **Bloom:** threshold set above every practical and below the dimmest line. Bloom is a line effect and nothing else blooms.
- **Grade:** cool shadows (toward #1B2A4A), neutral mids, warm highlights only where a practical makes them. Saturation +5% on teal and the two tape colors, 0 elsewhere.
- **Vignette** 0.1. **Chromatic aberration** none. **Film grain** none. **Motion blur** off in competitive, 0.2 optional. **Lens flare** off: a flare is a fake line.
- **Shadows:** sharp from the masts, soft from the sky. Houses cast long shadows across the deck; a swinging house's shadow is a free tell.

## Sound bed

| Zone | Ambient loop | Detail layer |
|---|---|---|
| Rims | Wind over concrete, the masts' ballast hum, a Marshal's radio chatter from the gondola | Prep tent canvas flapping |
| High Lines | Wind in the cables (a low chord that shifts with the Sway), slats knocking | Your own footsteps ringing down the cable |
| Hub | The cable chord loudest here, the pendant's chain ticking | |
| Lift Run | Diesel winch idle and load note, sheave squeal, guide hum | The bell. Everyone learns the bell. |
| Market Ring | Festoon bulbs buzzing, a radio someone left on (Hanger station, music, no words), awnings | Cylinders ticking in the rack as they cool |
| Rows | Houses creaking on their hitches, wind through windows, a kettle still on somewhere | A dog, far off in the vista district, once a round, never nearer |
| Nets | Wind from below, laundry snapping, the Hooks' rotors under the mist, the counterweight hum when due | Drips |
| Terraces and Shanties | Leaves, barrels, a shanty radio | |
| West Face | Wind on the ladder, the plank bridge's cables | Your own breathing on the climb |

Arena-unique tell sounds (see events-and-moving-parts.md for leads): the **Sway creak**, timber over cable, localized; the **zip**, the saw-whine of a Cutter held on a slack cable, rising for 0.6 s; the **twang and the long fall**, a dropped house; the **bell**, two strikes; the **guide hum**; the **strand pings** before Cable Day, like a clock speeding up; the **wire scream** of the NE cable going, followed by the pin bang. The Hooks are always under everything.

## After Cable Day

- The east half of the Market Ring loses its festoons: dark except the pendant pool, which now swings on a 3 s period across the seam and the marker. The marker goes light, dark, light. Lines on the east half are the only steady light there.
- The NE pin socket smokes for 10 s and then arcs every 4 s: a 0.1 s white flash capped at line-idle brightness (**PROPOSED**), lighting the East Rim and the Dangle in stills.
- The East Row's three freed houses carry their interior lights with them: three warm windows swinging through the dark on 9, 10, and 11 s periods. Players track them by their windows. This is the single best image in the arena and it should be protected: the freed houses' interior bulbs never break.
- The headframe mast still lights E4, E5, E6 and the Lift. The East Rim mast now lights nothing but air and the Dangle; its pool on the vanished walkway is a visible absence.
- Dust from the socket drifts east out of the playable volume on the wind within 10 s. No lingering haze (rule 8).

## Beauty shots

**1. Blue hour from Spawn A.** Camera at (-56, 2, 33.7), 100° FOV, looking east along the NW High Line. The walkway's marker lamps step away in a shallow sag to the Hub, a black ring against the blue. Below and beyond, the West Row's warm windows, the ring's festoon circle, the East Rim mast's cold pool on the far roofs. The Lift car rises with its green lamp. The vista district hangs north and south in points of light. One set line crosses the ravine at mid-frame, brighter than anything.

**2. Under the Ring.** Camera on the nets at (-4, -6, 1.7), looking up at 60°. The ring's dark underside fills the frame, the hatch cover a taped disc at center, four hanger cables rising past it to the Hub's silhouette and the pendant's skirt. Between the hangers, sky. A Climber on a Pendulum swings through the gap, their line singing white-hot in an arc from the Hub to their chest. Below the frame edge, mist glows red once.

**3. Cable Day, T+4.** Camera on E4's porch at (16, -6, 15.5), looking north-west. The east half of the ring stands at 40° like a tipped table, stalls piled against the low rail, a cylinder rolling, the manifold's white plume. Behind the seam, the west half and the marker sit level in the pendant's pool. E1 enters from the right, a whole house with its window lit, hitch cable taut to a Hub that is out of frame. A Rigger's Hang line, bright, runs from the Hub to a figure holding the seam.
