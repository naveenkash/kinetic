# Hook Drone

_Recovery drones. They wait in the dark below the lowest floor. The sound of a Hook's rotors spinning up below you is the sound of a ring-out before it happens._

## Appearance

| Element | Spec (**PROPOSED** unless from the bible) |
|---|---|
| Span | 1.6 m across, hexacopter layout, six 40 cm ducted rotors |
| Body | A flat hexagonal hull, 20 cm thick, Office white `#F2F2F0` with black-and-yellow chevrons on the leading edges, roughness 0.5 |
| Arm | A single articulated recovery arm under the hull, 1.2 m reach, ending in a two-finger clamp that grabs the harness at `S_Hook_Lift` |
| Lights | Four white navigation strobes at the rotor ducts; a red underside lamp when carrying a body; an amber beacon during the Drop phase |
| Markings | Office crest, a unit number (H-01 to H-06 per arena), a Meridian Mutual decal |
| Wear | Grime on the underside, chipped chevrons, one bent duct on unit H-04 per arena |

Hooks are seen from above, briefly, and heard constantly. The silhouette is the hex hull and the dangling arm.

## Behavior

| Phase | Behavior |
|---|---|
| Drop (round start, 6 s) | Hooks lower crews that use the Hook-lowered Drop: the clamp holds `S_Drop_Attach`, descends at 2 m/s, releases 0.3 m above the floor, climbs away. |
| Live | Hold station below the lowest playable floor, out of sight, rotors at idle. In Hangtown and Ballast they are audible in the Well below. |
| Ring-out imminent | When a player falls past the lowest playable floor, the nearest Hook's rotors spin up audibly 0.5 s before the kill volume (05 §7.5). The spin-up is the tell. |
| Stall on the floor | 4 s after the Stall, a Hook arrives from the nearest edge or shaft, clamps the body at `S_Hook_Lift`, and lifts it out over 2 s (05 §7.4). It flies to the nearest Well or map edge and descends out of sight. |
| Ring-out | The Hook catches the body below the kill volume, out of sight. The player hears the clamp. |
| The Anchor | Hooks do not go down the Anchor's Well (03). The spin-up tell still plays from an unseen source. The retrieval is not shown. The Office does not discuss it. |

Hooks never collide with players or lines. Dogs do not seat in Hooks (05 §3.1): they tick off the hull.

## Animation needs

| Clip | Notes |
|---|---|
| Rotor spin | Procedural, blade blur above 600 rpm |
| Idle hover | 10 cm bob, 2 s period |
| Spin-up | 0.5 s rotor pitch and speed ramp, hull tilts 10° forward |
| Approach | Banked flight along a spline from a pool point to the body |
| Clamp | Arm extends 0.5 s, fingers close on the plate, 0.3 s |
| Lift | 2 s, body hangs from the back plate, limbs loose in the locked pose |
| Carry | Hull tilts 5° under load, red lamp on |
| Drop lower | 2 m/s descent, release, climb |
| Damaged idle (H-04) | One duct flutters |

The lifted body keeps its Stall lock pose; the Hook's motion adds a pendulum swing of 5°.

## Audio

| Event | Sound |
|---|---|
| Idle below | A low six-rotor drone, 90 Hz fundamental, filtered by distance and the arena's floor |
| Spin-up | 0.5 s rising pitch, 90 to 180 Hz, with a sharp duct whistle. This is a readability contract item (05 §11). One sample set, never varied by arena except the Anchor. |
| Anchor spin-up | The same spin-up with a longer reverb tail and no source localization |
| Approach | Rotors Doppler past |
| Clamp | A hydraulic clack on the plate |
| Lift | Rotor load pitch drops a semitone |
| Drop release | A click and a rotor climb |

## Technical

| Item | Spec |
|---|---|
| Mesh | 8k tris LOD0, 3k LOD1, 800 LOD2. One 2K texture set. Rotor blades as a separate blurred disc mesh above 600 rpm. |
| Physics | Kinematic. Splined flight paths with a small noise layer. No simulation. |
| Pool | 6 per arena, parked at authored points below the lowest floor or behind spawn structures. |
| Attachment | Body attaches to the clamp socket `S_Hook_Clamp` at the body's `S_Hook_Lift`; the body's physics asset stays locked. |
| Collision | None with players, lines, or props. Blocks Dogs with a bounce response. |
| Replication | Flight is server-driven for the lift timing (4 s then 2 s); clients interpolate. Spin-up tell is a reliable event timed to the kill volume. |
| Lighting | Strobes are small emissives; the red carry lamp is a real light with a 3 m radius. |

## Reference search terms

heavy-lift hexacopter, search and rescue helicopter hoist clamp, airport ground drone with chevrons, ducted fan close-up, long-line helicopter logging, white utility drone grime.
