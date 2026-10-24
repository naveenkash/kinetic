# The Line

_The live filament. The tether. It heats under load and glows. Riggers say a loaded line is singing. The Accord's Visibility Clause makes the glow mandatory: no dark lines._

## Physical description

A 6 mm filament (**PROPOSED** fiction diameter) rendered as a ribbon with a 1.2 cm bright core and a glow halo whose width scales with force. It runs from `S_Spool_LineExit` to Dog A, and from Dog A to Dog B, as a straight segment under force or a catenary when slack. Within 10 cm of the Spool it carries the Rope Mark tape bands (Tape, Band, Mark): 1 cm colored wraps, visible to anyone close enough to be scrapped by it.

Line color is **team tint** by rule (05 §11a): Amber `#E69F00`, Sky `#56B4E9`, or Rose `#CC79A7` for the third crew. Set lines render Bone `#E8E6E0`. The core shifts toward white as force rises: `core = lerp(TeamTint, white, 0.6 x ForceNorm)`. **Enemy lines** additionally carry a 1 m dark tick pattern: a 10 cm segment every 1 m at 20% of the core's value (**PROPOSED** tick dimensions), scrolling toward the puller at 1 m/s under Haul and away under Jack, so team and direction are never carried by color alone. Friendly lines have no tick. Faction never touches the glow.

## Force scale

Pull tension is in newtons (05 v0.2 §3.4). The pull scale runs from 0 to the overspin ceiling of 1,200 N. Hang loads are in kN and are roughly ten times the pull tensions: the filament is strong, the Spool motor is the limit.

`ForceNorm` is the single input to the glow and the audio:

- **Haul and Jack:** `ForceNorm = Force_N / 1200`. A class at its own T sits at Climber and Scrapper 0.33, Rigger 0.50, Doubler 0.67, Breaker 0.83. Only overspin reaches 1.0 and holds it.
- **The Kick:** the first 0.3 s of a Jack, when the Spool's capacitor discharges and the line's force is far above T (about 5 kN for a Climber off a wall). `ForceNorm` is pinned at 1.0 for those 0.3 s without the overspin flicker, then drops to the class T value (**PROPOSED**). A Jack reads as a flash and then a glow; a Haul reads as a glow that does not stop.
- **Hang and set lines:** `ForceNorm = 0.83 x (load / class Hang load)` (**PROPOSED**). A Hang line at its slip limit reads as bright as a Breaker's full pull, never as bright as overspin, so white flicker stays an overspin-only tell. Slipping adds jitter, not brightness.

## Emissive curve against force

Emissive intensity `E = 0.3 + 42 x ForceNorm^1.5` (UE emissive multiplier on the core material). Halo width `W = 4 cm + 8.4 cm x ForceNorm`.

| Pull force (N) | State or class | ForceNorm | Emissive E | Halo (cm) | Core color |
|---|---|---|---|---|---|
| 0 | Slack, between A and B | 0 | 0.3 | 2 (fixed) | Team tint at 30% value |
| 0 | Set line, idle, unloaded | 0 | 1.0 (fixed) | 3 | Bone `#E8E6E0` |
| 0 | Set line, player within 5 m | 0 | 2.5 (fixed) | 4 | Bone, enemy tick visible |
| 100 | Light prop | 0.08 | 1.3 | 4.7 | |
| 200 | Racked cylinder break-free | 0.17 | 3.2 | 5.4 | |
| 400 | Climber T, Scrapper T | 0.33 | 8.4 | 6.8 | |
| 480 | Doubler split line | 0.40 | 10.9 | 7.4 | |
| 500 | Stack break-free | 0.42 | 11.6 | 7.5 | |
| 600 | Rigger T; manifold vent threshold | 0.50 | 15.1 | 8.2 | |
| 720 | Doubler split, overspin | 0.60 | 19.8 | 9.0 | |
| 800 | Doubler T; bolted break-free | 0.67 | 23.2 | 9.6 | |
| 1,000 | Breaker T; rated break-free | 0.83 | 32.2 | 11.0 | |
| 1,200 | Doubler overspin ceiling | 1.00 | 42.3 | 12.4 | Flickers white at `Overspin` = 1 |

Hang reference: a Rigger Hang line at its 12 kN slip limit and a Breaker Hang at 10 kN both read ForceNorm 0.83 (E 32.2). A Climber Hang at 4 kN reads the same 0.83; the scale is per class, so "about to slip" always looks the same.

The line must be the brightest thing in frame at any pull above 200 N in every arena lighting condition (03, arena rule 8). Arena art validates against the 400 N row, the lowest class tension.

## Audio curve against force

Fundamental pitch `f = 220 Hz x 2^((Force_N - 200) / 600)`: one octave per 600 N. For Hang lines substitute `1200 x ForceNorm` for `Force_N`. Timbre is a bowed metal string with a breathy upper partial. Loudness rises linearly from -18 dB at 100 N to 0 dB at 1,200 N (**PROPOSED**).

| Pull force (N) | Pitch (Hz) | Note |
|---|---|---|
| Set line, unloaded | 55 Hz hum, constant | Not force-driven |
| 100 | 196 | |
| 200 | 220 | |
| 400 | 277 | Climber, Scrapper |
| 480 | 304 | Doubler split |
| 600 | 349 | Rigger |
| 720 | 401 | Doubler split overspin |
| 800 | 440 | Doubler |
| 1,000 | 554 | Breaker |
| 1,200 | 698 | Overspin ceiling |
| Overspin tell at 1.0 s | 1397 | Pitch jumps one octave above the curve and holds until snap at 1.8 s |

Additional cues: Hang slip adds a creak every 0.1 s per 0.5 m slipped (05 §3.2). A line over a player's own body is mixed 6 dB louder and panned to the seat socket's direction.

## Visual states

| State | Shape | Glow | Audio | Notes |
|---|---|---|---|---|
| Slack (A fired, waiting for B) | Catenary from Spool to Dog A, sag 10% of length | 0.3 | Silent | Walk-through. 4 s then A auto-recalls. |
| Live Haul | Straight, 0.5 cm vibration amplitude at T | Curve | Curve, pitch rising with the 0.4 s ramp | Not solid. Shortens visibly as Dogs close. |
| Kick (first 0.3 s of Jack) | Straight, lengthening fast | White core, halo 12.4 cm, no flicker | A single crack-and-whoosh, then the curve | The Spool's capacitor drives the ends apart to the class launch speed V (18 / 14 / 12 / 14 / 10 m/s). |
| Live Jack (after the Kick) | Straight, lengthening | Curve at T | Curve at T | Pushes at T to class length and auto-releases with a fade. |
| Hang | Dead straight, zero sag, no vibration | Curve by load fraction | Curve by load fraction | Solid to players. Stagger at 3 m/s, flip at 6 m/s (05 §3.5). |
| Hang slipping | Jitters 1 cm | Flickers on each 0.5 m | Creak | Load over the class Hang load. |
| Reel (Hang, holding Haul or Jack) | Rigid rod shortening or lengthening at 1.0 m/s, dead straight | Hang curve: brightness by load / class Hang load, unchanged by the reeling | Hang curve plus a ratchet click every 0.25 m from the Spool | A winch, not a pull. Bears up to the class Hang load; above it the line slips instead of shortening. Hoists a Gallows victim slowly; a hanging player climbs by it. |
| Set line | Straight, held | Bone core, 1.0, brightening to 2.5 within 5 m of any player; loaded set lines use the Hang curve and stay Bone | 55 Hz hum, plus the Hang curve if loaded | Enemy set lines carry the tick. Expires: goes slack and fades over 0.5 s with recall. |
| Overspin 0 to 1.0 s | Straight, vibration amplitude climbs to 2 cm | 800 toward 1,200 N on the curve | Climbs toward 698 Hz | |
| Overspin 1.0 to 1.8 s | 2 cm vibration | White flicker at 12 Hz | 1397 Hz, held | The tell. 0.8 s to snap. |
| Snap | See below | | Universal snap | |
| Dark line | Straight or catenary as above | Emissive 0, black filament, heat shimmer only | Sings, muffled, -12 dB | Cutscene and lore use only. `DarkLine` is never true in a sanctioned match. |

## The snap

Cutting a line under tension, or an overspin reaching 1.8 s, releases the stored energy.

Mechanically (05 v0.2 §3.7, §4.2): a normal cut delivers 150 N·s per 1,000 N of force to both ends, below the stagger threshold. An overspin snap delivers 1,500 N·s x (F / 1,200 N) to the far end, a Stall against a player at full overspin, and throws the Doubler backward at 10 m/s.

VFX (`FX_Line_Snap`):
1. Frame 0: a white flash along the whole line, 1 frame.
2. 0 to 0.15 s: the two halves whip back toward their Dogs as tightening sine waves, core at full white, halo at 2x width.
3. Spark burst at the cut point, 40 sparks in the line's tint (Bone for a set line), 0.4 s life. A 20 cm scorch decal on whatever surface is within 0.5 m of the cut point.
4. 0.15 s: both Dogs release and recall. Camera shake radius 6 m, amplitude scaled by `ForceNorm`.
5. Overspin snap only: a second, larger spark burst at the far Dog and a 0.3 s shockwave ring, 1 m radius, in white. The far end's impact is the Stall, and the VFX should say so.

SFX: the snap is universal and is a gameplay cue; no cosmetic changes it. One sample set, scaled by tension at the moment of cutting: at 200 N a crack, at 1,200 N a crack with a low-end thud and a 0.4 s ring-off. Audible at 80 m. Riggers hear it across the arena and know somebody either lived or didn't.

## Technical

| Item | Spec |
|---|---|
| Rendering | Niagara ribbon per segment (Spool to A, A to B). Catenary solved on CPU for slack; straight line otherwise. Vibration is a vertex-shader sine driven by `ForceNorm`. |
| Collision | Hang and set lines: a thin capsule, solid to players only, ignored by objects (05 §3.5). Haul and Jack lines: no collision. |
| Material params | `TeamTint` (Amber, Sky, or Rose; Bone when `SetLine`), `IsEnemy` (bool, per viewing client, drives the tick), `TickScroll`, `Force_N` (0 to 1,200), `ForceNorm`, `Overspin`, `SetLine`, `Proximity`, `DarkLine`, `RopeMarkTier` (0 to 3, drives the tape band decals). Hang lines feed `ForceNorm` directly from load fraction. |
| Audio params | `Force_N` (or `1200 x ForceNorm` for Hang) to pitch and gain through the shared curve. `OverspinTell` bool triggers the octave hold. `SeatSocket` for panning on the victim's client. |
| Doubler | Two independent line actors from `S_Spool_LineExit` and `S_Spool_LineExit_2`, each at 60% T (480 N, 720 N overspin), each with its own mode and audio voice; the two voices beat against each other by design. Releasing one restores the other to full T over 0.3 s with an audible pitch rise. |
| Multiple lines on one object | Pull forces add as vectors and Hang loads add (05 v0.2). Each line still renders its own force; two Riggers hanging 2,400 kg between them each read at 0.83. |
| Replication | Force at 10 Hz quantized to 25 N, interpolated. Cut and snap are reliable events with the server's tension value. |
| Tells | Dog seat on a player's plate: distinct directional clank, and the HUD edge indicator from the seat socket (05 §4.2, §11). The line itself is never marked by the HUD. |

## Reference search terms

heated nichrome wire glow, bowed cymbal and bowed steel rod recordings, high-voltage line corona at night, suspension bridge cable close-up, climbing rope under load macro, long-exposure light painting with a single line.
