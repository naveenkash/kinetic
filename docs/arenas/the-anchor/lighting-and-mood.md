# THE ANCHOR: Lighting and Mood

_Arena: The Anchor. File purpose: time of day, sky, key and fill, practicals per zone, atmosphere, palette, line-emissive compliance (map rule 8), post-process, sound bed, post-event changes, and three beauty shots._
_Sources: docs/bible/03-arenas.md (The Anchor, map rule 8, engine notes), docs/bible/05-core-mechanics.md (3.5, 11). Inventions are marked **PROPOSED**._

## Time, weather, sky

- Time: 13:40 local, equatorial. Sun elevation 68 degrees, azimuth 255 (west-southwest). The sun sits over the west Rim, so after the Collapse the exposed west face of the Stump is in direct light.
- Weather: clear. Wind 15 knots from the west, steady, shown by the banners and the ribbon fragments on the Crown. No cloud shadow passes during a match (shadow motion competes with the Ring's shadow, which is a tell).
- Sky: deep blue zenith, a white haze band at the horizon over the sea, three or four high cirrus. The haze is beyond 400 m and never enters the arena volume (map rule 8).

## Key, fill, bounce

| Light | Direction | Color | Role |
|---|---|---|---|
| Sun (key) | 68 degrees elevation, from azimuth 255 | 5,600 K | Everything outdoors. Hard shadows, 0.3 m penumbra at deck distance. |
| Sky dome (fill) | Hemisphere | 11,000 K | Cool fill in every shadow. |
| Concrete bounce | Up from the Rim and bowl floor | 6,500 K, strong | Lifts the undersides of the Scaffold decks about one stop. |
| Stump glow | From the Weave surface itself | 9,000 K, faint | Capped at 10% of line emissive. Brightest at the Well mouth and on the bare upper face. |

The Scaffold interiors are the arena's shadow. Each deck shades the one below; a deck floor sits four stops under the Rim. The inner edges catch bounce from the Stump's cladding and the warm practicals. The result is a bright white outside, a dark warm cage inside, and a singing line that reads in both.

## Practical lights per zone

| Zone | Type | Color | Count | Notes |
|---|---|---|---|---|
| Rim | Floodlight masts (docs/objects/environment/floodlight-mast.md), 6 heads each | 5,600 K | 2 | E and W. Aimed at the Scaffold outer face. On during the match for broadcast. |
| Rim | Floodlight stands | 3,200 K | 6 | Aimed at the gangway heads. Warm, low. |
| Rim | Broadcast towers, 8 floods each | 5,600 K | 2 | NE and SW. Aimed at the Scaffold and the Crown. Red aviation beacon at each top, 1 Hz, dim (**PROPOSED**). |
| Rim | Office gallery interior | 4,000 K | 1 volume | Behind glass. Marshals in silhouette. |
| Crane decks | Floodlight stand, cab interior | 3,200 K stand, 2,700 K cab | 1 each | The cab is the only cozy light on the map. |
| Gangways | String lights on the handrail | 2,700 K | 4 runs | Low. They mark the edge at night-side angles (**PROPOSED**). |
| Bowl floor | Floodlight stands | 3,200 K | 2 | At the exit ladders. The footing stays dark. |
| Scaffold decks | Floodlight stands, one per quadrant per level | 3,200 K | 20 | Aimed at the Stump face from the outer rail. The inner edge and the Well mouth read warm. |
| Ladder bays | Caged bulkhead lamps every level | 4,000 K | 20 | Mark the landings. |
| Ring | Work lights under the gantry | 4,000 K | 4 | Their sweep across the deck below is part of the Ring's tell. |
| Stump face | The Weave's own emissive | 9,000 K | continuous | Faint, cold, pulses with the Pulse count. |
| Crown | Floodlight stands | 3,200 K | 2 | Aimed at the marker. |
| Crown | LayCast broadcast board | White, capped | 1 | Emissive capped below any line. |
| Well | Stump glow only | 9,000 K | | Fades to black by Z -15. |

## Fog and atmosphere

None inside the arena volume. No volumetric light shafts in the Scaffold; they compete with lines. Exponential height fog begins at Z -20, inside the Well, black-blue, so the kill volume reads as depth without reading as haze. A thin horizon layer beyond 400 m sells the sea. Dust exists only as a tell: Breath lift, Pulse sheeting, clamp pulls, the Collapse. It clears within 10 s of any event. Heat shimmer off the concrete is allowed at the Rim's far edge only.

## Color palette

| Swatch | Hex | Where |
|---|---|---|
| Structure: galvanized scaffold steel | #8E949A | Scaffold, ladder bays, Ring, crane lattice |
| Ground: sun-bleached socket concrete | #D9D4C7 | Rim, bowl floor |
| Stump: dark Weave | #1E2430, sheen #4A5A7A | The Stump face, the Crown floor, ribbon fragments |
| Accent: Office and LayCast teal | #2F6F8F | Crane bases, gallery, notices, the Office line in Held Breath |
| Accent: sponsor magenta | #C4356B | Banners, broadcast board frame |
| Hazard: structural tape | #F2B705 on #111111 | Every clamp, the slab slings, the boom roots |
| Sky: zenith | #1F5FA8 | Above |
| Sky: horizon haze | #DCE6EE | Beyond 400 m |
| Shadow: Scaffold interior | #2B3340 | Deck undersides, the footing, the Well mouth |

Planks are #A8865A, weathered toward the structure grey. Prayer chalk is #F4F1E8. Ascendant altitude paint is silver #C9CED3.

Line tints are canonical per 05 section 11a and are not an arena choice: Team A Amber #E69F00, Team B Sky #56B4E9, third crew (3v3v3, post-launch) Rose #CC79A7, neutral and set lines Bone #E8E6E0, with a 1 m dark tick on enemy lines. None of the arena swatches above sits near these hues; the teal accent (#2F6F8F) is desaturated and dark enough not to be mistaken for Sky, and the hazard yellow (#F2B705) is matte paint under the line cap, never emissive, so it does not compete with Amber.

## Map rule 8: lines are the light

Line emissive is rendered at a fixed intensity above everything else in the scene, with bloom reserved for it alone. The rule is enforced per zone by capping the scene, not by boosting the line.

| Zone | Brightest non-line thing | Cap applied | Line reads? |
|---|---|---|---|
| Rim, crane decks | Sunlit concrete | Exposure locked at EV 14.5; concrete sits at 70% of display white; lines render at full white plus bloom | Yes, with 1.5 stops of headroom |
| Gangways | Sunlit planks | Same exposure as the Rim | Yes |
| Scaffold decks | Warm floodlight stands | Stand intensity capped at 40% of display white at 2 m | Yes, by a wide margin; this is the darkest tier |
| Ladder bays | Bulkhead lamps | Capped at 30% | Yes |
| Ring | Sun on grating | Grating specular roughness floor 0.5 | Yes |
| Stump face | The Weave's glow | Capped at 10% of line emissive, pulse peaks at 20% | Yes |
| Crown | Sun on glassy Weave | Floor specular roughness floor 0.45; no sun glints above 60% display white | Yes |
| Well | Nothing | | Yes; a line into the Well is the only thing visible below Z -8 |
| Dark sector after a tower fall | Lines | | Must be confirmed first per the 03 engine note; nothing else in the sector is tuned before this test passes |

Set lines render dim in Bone per 05 sections 11 and 11a and brighten within 5 m of a player. In the Scaffold's shadow a dim set line still reads at 10 m; in Rim sun it reads at 5 m. Both are by design.

## Post-process intent

- Exposure: locked, two values (sunlit EV 14.5, shadow EV 11.5) blended by volume inside the Scaffold. No auto-exposure anywhere; a line's brightness must mean the same thing in every frame.
- Bloom: threshold set above the scene cap so only line emissives, the Stump's pulse peak, and blast flashes bloom.
- No lens flare, no chromatic aberration, no vignette (the HUD's line-direction edge indicators live at the frame edge).
- Motion blur off by default.
- Grade: neutral highlights, shadows pulled slightly toward #2B3340, saturation untouched on team glow colors.
- Depth of field: none in play. Allowed in the 4 s replay.

## Sound bed

| Zone | Ambient loop | Notes |
|---|---|---|
| Everywhere | The hum: 55 Hz fundamental with a shifting overtone, louder within 10 m of the Stump face | It climbs a fifth during the Pulse count and drops back at the whip-crack. |
| Rim | Wind over concrete, far sea, gallery PA murmur, banner flap, hoist tick from the cranes | The sea is audible only at r over 50. |
| Crane decks | Generator idle, the boom's lattice whistling | |
| Gangways | Plank creak under foot, string lights ticking in the wind | |
| Bowl floor | Dripping, the footing's standards ringing when struck, wind cut off | The quietest place on the map. |
| Scaffold decks | Wind baffled by the decks, plank creak, clamp chatter, the floodlight stands buzzing | The Ring's drone and grating rumble swell as it passes overhead. |
| Ring | Motor drone, grating under foot, cable slap | A bell at each quadrant seam. |
| Stump face | The hum at its loudest, chalk grit | A Perch on bare Weave adds a faint high ring (**PROPOSED**). |
| Crown | Full wind, the broadcast board's fans, ribbon fragments skittering | The hum is felt more than heard here. |
| Well | The Stump's hum drops an octave 0.5 s before the kill volume (canon, 05 v0.2.2); wind rising as you fall; no rotors | The absence of Hooks is the sound. |

Unique tells: the Breath's intake roar (27 s) and exhale (37 s); the Marshal's five-count, the Pulse's thud at zero, the whip-crack at 0.4 s, and the reverse whoosh of the inhale; the Ring's seam bell and the snap of every line it sweeps; the Reel's ratchet, one click per meter; the slab's brake squeal and the stop of its hoist hum; the manifold's hiss; the Collapse siren and five levels of planks letting go; the hum's octave drop for a Well ring-out; the Office line's rising tone in Held Breath.

## After the mid-match event

- The west quadrant's 4 floodlight stands and 4 bulkhead lamps are gone. The west Scaffold is now the bare Stump in direct sun from azimuth 255: the brightest surface on the map at 68 degrees of incidence, and still under the line cap because the exposure does not move.
- The W floodlight mast survives and rakes the cliff and the debris. The debris on the Rim W arc is in the mast's light and in the shadow of the containers.
- Dust for 10 s, then clear.
- The Ring's work lights now cross a void for 30 s of every revolution and light nothing. Riders see their own line as the only thing between them and the bowl floor.
- If the SW tower is already down, the southwest sector has no practicals at all. Lines, the Stump glow, and the sun are the lighting. Confirm this case first.

## Beauty shots

1. **The Ascension.** Camera on the Rim S parapet at (0, -40, 1.7), 24 mm, tilted up 55 degrees. The Stump fills the frame, the Scaffold rings in warm shadow, the Crown lip a white line against #1F5FA8. At 33 s the Breath's dust column rises past the decks and a single Climber in white wraps goes up inside it, line singing silver behind her, the boom tip and its yellow-taped slab hanging in the top corner. The broadcast board on the north lip is a pinprick. Hold 2 s.

2. **Full exhale.** Camera on the Ring's inner rail at r 14, Z 43, looking down into the Well mouth, 35 mm. Pulse count at "two." Dust sheets off the Weave face in rings. Three taut lines run from the L3 inner edge across the 4 m mouth into the Stump, two Amber and one Sky with its dark ticks, brighter than the sun on the Rim behind. On "zero" the dust blows outward and one line goes slack as a Dog pulls free. Fog of nothing below: the shaft goes to black.

3. **After the cut.** Camera on the Crown's west lip at (-10, 0, 61), 28 mm, looking west and down. Foreground: torn Weave strands of the lip. Middle: the west cliff face in full sun, 20 clamp stubs taped yellow, three ledges with chalk marks. The Ring enters frame from the left, its outer rail over nothing, one rider braced at the rail. Below: the debris field on the Rim W arc, the W mast raking it, the containers' long shadows. Behind everything: the kilometer of bowl, white, and the haze band where the sea begins.
