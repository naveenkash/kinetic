# The Cutter

_The Thermal Knife. The mandatory safety tool every rigger carries. A ceramic blade that heats past the filament's melt point in a fifth of a second. Nobody lends their Cutter._

## Physical description

A fixed blade with a ceramic edge, a heating element inside the blade spine, a battery and switch in the handle, and a serial number engraved on the ricasso. The Guild writes the serial next to the rigger's name in the Ledger on the day they stop being a Greenhand. Held in the **left hand**. Does not interrupt Spool use except during the swing.

| Variant | Blade length (**PROPOSED**) | Overall length | Reach (05 §4.1) | Warm-up | Swing | Recovery |
|---|---|---|---|---|---|---|
| **Standard** | 32 cm | 46 cm | 1.5 m | 0.20 s | 0.15 s | 0.30 s |
| **Hot** | 20 cm, thicker spine | 34 cm | 1.1 m | 0.12 s | 0.15 s | 0.30 s |
| **Long** | 55 cm, narrow | 70 cm | 2.0 m | 0.26 s | 0.15 s | 0.30 s |

Class modifiers on warm-up: Scrapper -0.05 s, Rigger -0.03 s, Breaker +0.10 s. Reach includes the arm and a half step; the blade lengths above are chosen so the three variants read as different weapons at arm's length and in first person.

Blade cross-section: a single bevel with a flat back. Unheated, the ceramic is matte bone `#D9D2C0`, roughness 0.6, metallic 0. The spine carries a dark heating channel.

### Sheath positions

| Class | Socket | Position | Draw motion |
|---|---|---|---|
| Climber | `S_Cutter_Sheath` on `lowerarm_l` | Inner left forearm, blade toward the elbow | Right-side-up pull by the left hand's own fingers, a flick |
| Scrapper | `S_Cutter_Sheath` on `spine_03`, 12 cm left of the chest plate | Chest strap, handle down | Single pull down and out: the fastest draw |
| Rigger | `S_Cutter_Sheath` on `thigh_l` | Outer left thigh | Reach down, pull |
| Doubler | `S_Cutter_Sheath` on `thigh_l` | Outer left thigh, handle up | Reach down, pull |
| Breaker | `S_Cutter_Sheath` on `thigh_r` | Right hip, front, handle forward | Cross-draw by the left hand: the slowest draw |

## Heat ramp

The blade's edge emissive runs from 0 to full over the warm-up time, then holds, then cools over recovery.

| Time (fraction of warm-up) | Edge color | Emissive | Spine |
|---|---|---|---|
| 0 | Bone, matte | 0 | Dark |
| 0.3 | Dull red `#8A1E0A` | 2 | Faint red at the channel |
| 0.6 | Orange `#FF6A1A` | 10 | Red |
| 1.0 (cutting) | White-orange `#FFE2B8` at the edge, orange 1 cm behind | 25 | Orange |
| Recovery, 0.30 s | Back through orange to dull red | 25 to 1 | Cools |
| Sheathed hot | Dull red glow visible through the sheath mouth for 2 s | 1 to 0 | |

A heat shimmer sprite follows the edge from 0.6 onward. The glow is the "I am about to cut" tell to a third party: a drawn, warm Cutter is visible at 20 m in low light.

## Animation

| Clip | Duration | Notes |
|---|---|---|
| Draw to warm hold | Warm-up time by variant and class | Blade comes up into the lower-left of the first-person frame, edge toward the camera center. |
| Swing | 0.15 s | A forward-and-across arc through the 70° cone. The arc trail is the edge emissive smeared over 0.15 s. |
| Recover | 0.30 s | Blade drops, cools. Spool use resumes at the start of recovery. |
| Sheath | 0.25 s (**PROPOSED**) | Cosmetic; can be interrupted. |
| Hold-cut (Hangtown slack cables) | 0.6 s continuous contact | Blade pressed to the cable; a rising spark stream, then the cable parts (05 §4.5). |
| Melee on a Catch Plate | Swing | Unseats the Dog, 0.3 s stagger on the target, no kill (05 §4.4). The plate shows a short scorch mark that fades in 5 s. |
| Cut self free, chest / shoulder L / shoulder R / back | Turn plus swing | The turn is the skill test: 0°, 60° to 90°, 150°+ (05 §5.5). The swing clip is shared; the turn is upper-body and camera. |

## Cutting a line

A swing severs any line whose segment passes within reach and inside a 70° cone of the camera's forward vector, at the point nearest the blade (05 §4.2). Contact: a 0.05 s white spark at the blade, then the line's snap VFX and the universal snap SFX scaled by tension (`line.md`). The cut point leaves a 20 cm scorch decal on any surface within 0.5 m.

## VFX and SFX hooks

| Hook | Content |
|---|---|
| `FX_Cutter_Heat` | Edge emissive ramp, heat shimmer, and three to five rising embers per second at full heat |
| `FX_Cutter_Trail` | Swing arc trail, 0.15 s, edge color |
| `FX_Cutter_Contact` | Spark at the cut point |
| `FX_Cutter_Scorch` | Plate scorch (melee) and surface scorch (cut) decals |
| `SFX_Cutter_Draw` | Sheath scrape, by class sheath material (canvas, steel, polymer) |
| `SFX_Cutter_Warm` | A rising electrical whine over the warm-up, ending in a tick at full heat |
| `SFX_Cutter_Swing` | A short hiss |
| `SFX_Cutter_Cut` | The universal snap. Cosmetic cut effects may add a layer; the snap sample is never replaced. |
| `SFX_Cutter_Melee` | Clang on plate, plus the Dog pop |

## Cosmetics

Handle material, wrap, and pommel; blade etch pattern on the flat; a cut-effect layer (color of the contact spark only). Locked: blade length by variant, the edge color ramp, the snap sound. Serial numbers are generated per player from the Ledger and engraved as a decal; Face Cutters carry authored serials (see the Face docs).

## Technical

| Item | Spec |
|---|---|
| Sockets | `S_Cutter_Hand` on `hand_l`; `S_Cutter_Sheath` per class as above. |
| Mesh | LOD0 2k tris, LOD1 700. First person uses LOD0. |
| Textures | 1K per variant plus a 512 handle set per cosmetic. |
| Material params | `Heat` (0 to 1, drives the color ramp and emissive), `TeamTint` (unused on the blade; the Cutter is the one tool with no team light), `Wear`, `SerialIndex`. |
| Hit test | A sweep along the blade from `S_Cutter_Hand` over the 0.15 s swing, against line capsules and Catch Plate shapes only. World geometry is ignored; the Cutter does not cut the map, except marked slack cables by hold. |
| Replication | Draw state and heat are replicated so third parties see the warm blade. The cut is a server event. |

## Reference search terms

ceramic kitchen knife edge, glowing forge steel color temperature chart, firefighter rescue knife sheath placement, electrician's hot knife foam cutter, ironworker tool holster, engraved serial on a knife ricasso.
