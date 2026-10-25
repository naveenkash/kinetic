# THE YARD: Lighting and Mood

_Arena: The Yard (Collar Four Breaking Yard). File purpose: time of day, sky, key and fill, practicals per zone, atmosphere, palette, map rule 8 compliance, post-process, sound bed, lighting changes after Shift Change, and three beauty shots._
_Sources: docs/bible/03-arenas.md, docs/bible/05-core-mechanics.md. Anything not in those files is marked **PROPOSED**._

**PROPOSED (2026-10-25, the user's call):** the Yard now plays in daylight by default, a mid-morning sun from the south-east through broken cumulus, with the port and the town round it. The night shift below is still built with `KINETIC_YARD_TIME=night` (game/Tools/unreal/build_yard_greybox.py).

## Time, weather, sky

- **Time:** night shift, 02:40 local. No moon.
- **Weather:** overcast, dry, a light steady wind from the west (the Gantry load swings east). No rain: wet steel would add specular highlights that compete with lines.
- **Sky:** a low cloud deck lit from below by the Basin's sodium streetlights, strongest to the south and east. The underside of the cloud is a dull orange that fades to near black overhead. Sparks drift across it from the cutting crews beyond the east perimeter, intermittent, every 10 to 20 s, from a fixed emitter at (+110, -20, 6) outside the playable volume.

## Key, fill, and direction

| Light | Source | Direction | Color temperature | Notes |
|---|---|---|---|---|
| Key | Four corner floodlight masts (24 m) and the shack roof floods (14 m) | Inboard and down, 50 to 60 degrees off vertical | 5,600 K, cool white | Pooled, not flat. Each mast throws three 30 degree pools. Between pools the Floor drops to a tenth of pool brightness. |
| Secondary key | Press frame work lights and the pit's two floodlight stands | Into the pit and across the press approach | 4,000 K | The pit is the brightest hole on the map, so it reads as a destination. |
| Fill | Sky bounce from the sodium cloud deck | From above, everywhere | 2,200 K | Low. Shadows read warm. |
| Fill | Trench-lip lamps | Up the trench walls | 2,700 K amber | Continuous. Bright 24 m ahead of a train. Red during Shift Change and Lockout. |
| Motion key | Hauler headlamps | Along the trench | 6,500 K | A moving cold spot that precedes the train by 24 m. |
| Motion key | Gantry magnet lamps | Down onto the load and the Floor beneath | 5,600 K, red on the drop tell | The load always carries its own shadow. |

Shadow direction on the Floor is therefore mixed: a player standing between two masts casts two soft shadows at right angles. Under the Rails walkway there is a hard double shadow stripe along Y ±31.5 from the masts, which hides a crouched player on the dock edge.

## Practicals per zone

| Zone | Type | Color | Count |
|---|---|---|---|
| Trenches | Lip lamps on 10 m centers, both lips | Amber 2,700 K, red on alarm | 96 |
| Trenches | Entry and exit plenum floods | 5,600 K | 6 |
| South Dock, North Dock | Mast floods, 3 per mast | 5,600 K | 12 |
| South Dock | Fuel point cage lamp | 4,000 K | 1 |
| Haul Lane | Truck bed lamps, 2 per truck | 4,000 K | 4 |
| Haul Lane | Welding cart arc lamps (static, no flicker) | 6,500 K | 2 |
| Marker Flat | Marker's own LayCast ring light (part of docs/objects/heavy/claim-marker.md) | Team-neutral white, 5,600 K | 1 |
| Marker Flat | Cutting-station cage lamps on NC3's north face | 4,000 K | 2 |
| The Press | Frame work lights | 4,000 K | 4 |
| The Press | Floodlight stands on the east lip | 5,600 K | 2 |
| The Press | Ram lamps in the pit walls at -5 m | Amber | 4 |
| Canyons | Chalk-mark inspection lamps clipped to mesa faces, one per mesa | 3,000 K | 7 |
| Shack Perch | Interior fluorescents (seen through windows) | 4,000 K | 6 |
| Shack Perch | Roof floods | 5,600 K | 4 |
| Rails | Walkway bulkhead lamps every 20 m | 3,000 K | 16 |
| Mast Platforms | Platform lamp | 3,000 K | 4 |
| The Bridge | Cab interior, magnet lamps, LayCast camera mast beacon | 4,000 K, 5,600 K, red beacon (steady) | 3 |
| Perimeter | Sponsor banner backlights | Per banner, desaturated | 8 |

No practical anywhere strobes or flickers. Alarm states are steady color changes.

## Fog and atmosphere

Thin. Height fog at the Floor only, 0 to 2 m, barely visible, so that the trench lamps and floodlight pools get a slight volume and the Floor between pools reads as dark rather than empty. No volumetric god rays from the masts: they would compete with lines. The sparks from the east are the only airborne particulate besides a very light dust kick from Hauler passes. Map rule 8: no haze in the fighting volume.

## Palette

| Swatch | Use | Hex |
|---|---|---|
| Structure steel | Mesas, Gantry, legs, press frame | #4A5560 |
| Ground slag | Floor deck plate, trench bed, oil stains | #2B2622 |
| Guild ochre | Handrails, chalk, Truck A, trench stencils | #C8922A |
| HTR orange | Notices, Truck B, survey tags | #E8651A |
| Hazard tape | Rated loads, pit rail | #F2C318 on #111111 |
| Floodlight pool | Lit steel under 5,600 K | #D9E2EA |
| Sodium sky | Cloud underside | #B4551E fading to #1A1410 |
| Shadow | Unlit Floor, under the Rails | #15171A |

Line glow colors are team-locked and defined by the UI document, not here. The Yard's palette has no saturated cyan, magenta, or green anywhere, so any team glow color in that range owns the frame by default. If the team colors include orange, the HTR orange swatch must be desaturated by 30% on this map (**PROPOSED**).

## Map rule 8 compliance: lines are the light

Every zone is checked against the rule that a singing line is the brightest thing in frame.

| Zone | Brightest competing source | Mitigation |
|---|---|---|
| Trenches | Hauler headlamp (6,500 K, moving) | The headlamp is aimed along the trench, not up. From the lip it is seen edge-on. A line across the trench still reads above it. |
| Docks, Haul Lane, Marker Flat | Floodlight pools | Pools are capped at an exposure where a set line's dim emissive is visible inside the pool and a live line is 2 stops over it. Between pools, lines are the only light. |
| The Press | Pit work lights | Aimed down into the pit. The lip is lit by spill. A line over the pit reads against the dark frame. |
| Canyons | Inspection lamps | Low, warm, 3,000 K. A line is cooler and brighter. |
| Shack Perch | Interior fluorescents through glass | Glass is 40% tinted. |
| Rails | Walkway bulkhead lamps | Low and warm. The Rails are otherwise the darkest tier; the sodium sky is behind every line seen from below and is 3 stops under a live line. |
| The Bridge | Magnet lamps | Aimed down. On the drop tell they turn red, which is a color read, not a brightness jump. |
| After the shack drops | Nothing in the west third | Lines are the only light there. Confirm emissives read at full before anything else is tuned. |

Validation pass (**PROPOSED**): with every practical on, a Rigger's live Haul line at 600 N must be the peak luminance in a 100 degree frame from every spawn, from the marker, from the pit lip, and from the Gantry cab.

## Post-process intent

- Exposure locked per tier (no auto-exposure swings when looking from a pool into shadow). Floor and Stacks share one exposure; the Rails sit one stop darker.
- Bloom only on emissives above the line threshold, so lines bloom and floodlights do not.
- Slight cool-warm split toning: shadows toward the sodium sky, highlights toward the floodlight white.
- No chromatic aberration, no film grain above a trace, no vignette: the broadcast look is clean.
- LayCast's HUD provides the only screen-space color accents.

## Sound bed

| Zone | Ambient loop | Level |
|---|---|---|
| Everywhere | The press ram: hydraulic rise, 2 s hold, clang, fall, 20 s period | Loud at the pit, audible at every spawn |
| Everywhere | Distant cutting: torch roar and grinder whine from the east, with the sparks | Low |
| Everywhere | LayCast PA: Marshal calls, sponsor stings between rounds | As authored |
| Trenches | Rail hum before a train, wind through the trench | Medium |
| Docks | Floodlight ballast buzz, loose chain on the mast bases in the wind | Low |
| Haul Lane | Welding cart fans, billet stack ticking as it cools | Low |
| Marker Flat | The marker's own LayCast tone when held | Medium |
| Canyons | Wind across the mesa tops, a low whistle at 8 m | Low |
| Shack Perch | Radio chatter from the Marshals, muffled | Low |
| Rails | Wind, 36 m, continuous, with the walkway grating ringing underfoot | Medium |
| The Bridge | Hoist motor, trolley wheels, cab radio | Medium, moving |

### The Yard's unique tell sounds

| Tell | Sound |
|---|---|
| South Hauler | One low horn, 3 s, then rail rumble rising |
| Center Hauler | Two-tone horn, 3 s |
| North Hauler | One high horn, 3 s |
| Gantry drop | Two-tone descending, 2 s, from the magnet |
| Shift Change | A factory shift siren, 5 s, plus the PA |
| Counterweight release | Cable pay-out shriek over the 1.0 s groan |
| Shack drop | Bolt shear, then glass |
| Billet topple | Scrape, slow groan, then a sequence of three flat impacts |
| Plank bridge | Dry wood creak, then the splinter |
| Lockout | The shift siren, then chain-mail drag, continuous |
| Pit ring-out | Hooks spin up in the north wall, 0.5 s, then the ram takes it |

The snap of a cut line and the Dog seat clank are universal and are not re-skinned here.

## Lighting after Shift Change

- Trench lamps hold steady red for 60 s, then return to amber. Red is darker than amber, so the trench lips lose contrast and lines across them gain it.
- The Gantry's magnet lamps stay red for the full 60 s because it drops every 20 s.
- The dropped plate on Marker Flat lands in a floodlight pool and casts a 2.7 m shadow east toward the marker. The marker's ring light is now the brightest thing on that side of the Flat, which is correct: it is the objective.
- If the shack has dropped: the west third of the Rails and the west end of Marker Flat are lit only by the sodium sky and by lines. The validation pass above covers this state.
- Lockout: corner masts cut, curtain trolley lamps backlight the advancing walls, and the final 20 m band is lit by the shack floods, the press lights, and the marker ring.

## Beauty shots

**1. The Caboose, from the South Rail Walk.** Camera at (-20, -31.5, 37), 35 mm equivalent, looking east along the south trench. The trench lamps run amber to the vanishing point. A Hauler's headlamp is a cold spot 60 m out. In the foreground a Breaker on the lip has a Haul line singing down into the trench, bright enough that the handrail in front of the camera is lit by it. Sodium sky above the far masts.

**2. The drop, from the pit's east lip.** Camera at (+56, 0, 1.7), 24 mm, looking west and up. The press frame is a black portal against the cloud. The Gantry load hangs at 9.3 m with its magnet lamps red, two seconds before release, its underside lit from below by the pit's work lights. Below it the ram is at top of stroke, amber. A Climber is perched under the load with a Hang line to the hoist cable.

**3. Marker Flat from NC2's top, after Shift Change.** Camera at (-6, +18, 9.7), 28 mm, looking south-west. The dropped plate lies in a floodlight pool with dust still settling. The marker's ring light is 6 m to the left. Two trenches' red lamps frame the shot top and bottom. Beyond, the West Mesa is lit by Spawn A's floodlight pool, and above it the shack windows glow. One set line hums across the gap between the plate and NC1 at chest height, dim, waiting.
