# Office Gallery

One-line: environment, static world, infinite mass, no Dog seating, a 12 m x 4 m x 3 m glazed Marshal box with an open camera deck on top, mounted high: Ballast on HTR's idle cutting gantry at the Crown, the Yard on the north Rails leg beside the control shack, Hangtown on the west rim's Marshal mast at 24 m, the Anchor on the Rim at the north tower. One per arena.

## Role in play

The gallery is where the Marshals sit and the match is called from (01, 02). It is the neutral frame: the one object in every arena that no line touches, because Dogs do not seat in the Marshal gallery (05 section 3.1). It gives every arena a fixed reference point that is always lit, always in the same relation to the spawns, and always out of play. Its loudspeakers are the Marshal's voice; its underside docks the Hooks. Players learn to orient by it the way they orient by the Well's sound.

| Harness | Relationship |
|---|---|
| Climber | Would perch on it; cannot. The glass is `NoDog` and the perch surface is disabled. The only surface in the game a Climber cannot cling to. |
| Scrapper | Loots nothing from it. The Office's prep stores are elsewhere. |
| Rigger | Cannot anchor to it. A Rigger who fires a Dog at the gallery gets the Dog back in 1 s and a Marshal's dry remark (a bark, once per round). |
| Doubler | The Fray's pirate cameras are bolted to the gallery's underside, where the Office pretends not to see them. |
| Breaker | Cannot move it. It is not rated; it is nothing. |

## Appearance

Silhouette: a long low box with a continuous window band, Office yellow frame, a railed camera deck on the roof with LayCast cameras and a dish, loudspeaker horns at each end, a Hook dock beneath (two drones on charge, rotors folded), a yellow-and-black ladder cage up the mast or leg it stands on. Inside: three Marshals' chairs, screens, a kettle. The Marshals themselves are visible as seated figures who occasionally lean forward.

| Part | Material | Base color | Roughness | Metallic |
|---|---|---|---|---|
| Frame | steel, Office yellow | #F2C500 | 0.55 | 0.6 |
| Glazing | laminated, slight green | #B8D8C8 at 20% opacity | 0.05 | 0.0 |
| Camera deck | grating and rail, grey | #6E7378 | 0.6 | 0.8 |
| Horns | aluminium, grey | #B3B6B8 | 0.5 | 0.9 |
| Hook dock | black steel, blue charge lights | #1A1A1C, #4FC3F7 emissive | 0.5 | 0.6 |
| Office seal | white disc, 1.2 m, black device | #F5F5F0 / #141414 | 0.6 | 0.0 |

Weathering: the best-kept object on any map, and it still shows its age: the yellow is repainted over sixty years of coats and the corners are rounded with paint. Markings: `OFFICE OF THE LAY  MATCH CONTROL` on the window band in black; the seal (a ring with a plumb line through it) on each end; a LayCast placard; a brass plate listing the match number. Fray stickers on the underside. Lighting: interior lit at 3000 K (warm, so the Marshals read as people), the camera deck's tally lights red, the two end boards showing the crews' names in their team tints (05 section 11a: Amber #E69F00 and Sky #56B4E9; Rose #CC79A7 only in the post-launch 3v3v3 format; Bone #E8E6E0 for the Office's own text), the Hook dock's charge lights blue, and a white Office beacon on the roof that pulses once at every Marshal call. The gallery is never brighter than a line; interior light is clamped.

Read by tier: from the top tier, at eye level or below, the Marshals visible. From the mid tier, a lit yellow box above everything. From the low tier, the Hook dock's blue lights and the horns.

## Variants

1. **Ballast.** On the cutting gantry at the Crown, looking down the Spine. Hooks dock beneath and descend the shaft.
2. **The Yard.** On the Gantry's north portal leg head at 40 m beside the cab (**PROPOSED**; `arenas/the-yard/layout.md` instead proposes the control shack at Z 10.5 to 14 as the gallery variant; resolve there). The shack and its columns (see `rated/structural-column.md`) are separate from the gallery in this reading.
3. **Hangtown.** On the west rim's 24 m Marshal mast, over the Lift headframe, with the full ravine in view.
4. **The Anchor.** On the Rim at the north broadcast tower, at Rim height plus 12 m. Its Hook dock is empty: Hooks do not go down the Anchor's Well (03, 05 section 7.4), and the empty dock is the visual statement of that.

## Physics

- Static world. Infinite mass. `Kinetic_NoDog` on every surface: a Dog that hits it falls as a harmless prop and auto-recalls in 1 s per 05 section 3.1. No perch. Not a Hang anchor. Players cannot reach it (no ladder access during a match: the cage is locked and the mast or leg has a `NoClimb` volume).
- Collision: solid to players and objects that reach it (a Pulse-thrown drum can hit the Anchor gallery's glazing and bounces; the glass does not break).
- Hooks: launch from the dock when a player passes the lowest playable floor (05 section 7.5) and return with the body. Their rotor spin-up is the ring-out tell, 0.5 s. The dock is `Kinetic_NoDog` while a Hook is on it and remains so when empty.
- Marshal calls: the horns are the spatial source for the Marshal's voice at a fixed level across the arena (not attenuated beyond 60 m, like the press thud) so that calls are always heard. The Pulse countdown on the Anchor comes from here.

## Interactions

- None by design. The gallery's job is to refuse. The one authored interaction: a Dog fired at the gallery three times in a round triggers a Marshal bark ("That's us. Try the building.").
- Replay cameras on the deck are real: the end-of-round replay director may cut to them.

## States

Static. `Live` (beacon pulses on calls), `Round end` (interior brightens 20%, the Marshals stand), `Between rounds` (horns play the Office's hold tone).

## VFX and SFX hooks

`VO_Marshal_*` (all Marshal lines route through the horns), `SFX_Office_Beacon_Pulse`, `SFX_Hook_Rotor_Spinup` (dock), `SFX_Hook_Depart`, `SFX_Office_Hold_Tone`; `VFX_Tally_Lights`, `VFX_Hook_Charge_Lights`, `VFX_Marshal_Idle` (seated figures, three animation loops).

## Technical

- LOD0 9,000 tris plus 3 Marshal figures at 4,000 each, LOD1 half, LOD2 box with window band, LOD3 lit card. Marshal figures culled beyond 40 m.
- Textures: `T_Office_Gallery` 4K, `T_Office_Decal_Atlas` 2K, `T_Marshal_Figure` 2K shared.
- Pivot at the floor center. Sockets: `Horn_L/R`, `Beacon`, `Camera_0..3` (replay cameras), `HookDock_0/1`, `Seal_L/R`, `Marshal_0..2`.
- Collision: `Kinetic_WorldStatic` plus the `Kinetic_NoDog` response channel on all primitives; `NoClimb` and `NoPerch` volumes on the access route and surfaces.
- Audio: a fixed-level attenuation class `Att_Marshal` shared with the press thud.

## Reference search terms

race control tower glass box, port crane operator cabin yellow, stadium referee gantry, drone docking station charging lights, air traffic control cab interior warm light.
