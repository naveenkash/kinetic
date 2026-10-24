# KINETIC: Camera, Feel, and Input

_System spec v0.2.2, revised to 05-core-mechanics v0.2 and the v0.2.1 and v0.2.2 rulings (CHANGELOG 2026-10-24). Implements bible 05 sections 2.1, 2.3, 3.1, 4.2, 5.1, 5.2, 7.1, 9.2. Numbers in 05 are canonical. Anything marked **PROPOSED** is new here._

## 1. Camera modes

| | First person (default; the only camera in Rated) | Third person (over-shoulder, casual only) |
|---|---|---|
| Eye | 1.7 m, at the head bone, body-animation isolated | Pivot at 1.5 m, offset 0.5 m right, 0.3 m up, 2.0 m back, with collision pull-in |
| FOV | 100° horizontal default, 90° to 110° configurable | Locked at 90° horizontal (05 section 2.1) |
| Shoulder swap | n/a | Yes, on input, 0.15 s |
| Rated play | Yes | No. Casual modes only (05 section 2.1). The toggle is hidden in Rated lobbies and the camera is forced to first person at "Live" |

FOV is horizontal and does not change with aspect ratio below 16:9 (vertical is cropped). Sprint adds +4° over 0.2 s. Lines, Dogs, and the Cutter render with the world FOV; first-person arms use a fixed 70° viewmodel FOV so the Spool's mode collar is always readable.

## 2. Camera during line states

What the player controls in each state. "Look" is camera yaw and pitch. The 70° cut cone is always measured from the camera's forward vector.

| State | Look | Move | Spool | Cutter | Camera behavior |
|---|---|---|---|---|---|
| Free | yes | yes | yes | yes | Normal |
| Drag (feet down, skid) | yes | no (overridden) | yes | yes | Camera stays level. Body yaw follows camera (harness swivel, animation.md section 8). Speed vignette from 4 m/s |
| Airborne (ragdoll-with-pose) | yes | air control only | yes (non-Climbers lose 15% horizontal speed per airborne fire, 05 section 2.3) | yes | Camera at the blended head bone. Roll limited to ±15° first person; horizon-locked by default option. Pitch follows velocity only when the player gives no input for 0.5 s |
| Hoisted | yes | Reel only on a line you own (hold Haul to climb, Jack to descend, 1 m/s); none on an enemy's line | yes | yes | Camera free. A gentle 0.3 Hz sway from the line is allowed, ±2°, off with "Reduce motion" |
| Flipped | yes | no | no | no | Camera follows the head at 0.6 animated blend, roll clamped ±30°, pitch clamped ±60°. With "Horizon lock" the camera keeps world-up and only translates. Restores in 8 f on landing |
| Stagger (0.3 to 1.0 s) | yes | no | no | yes | Per 05 section 7.1: movement and Spool locked, look and Cutter live. Camera shake per section 4 |
| Stall | no | no | no | no | Cuts to the Scrap Card and the Hook |
| Perch | yes | no | yes | yes | Camera free; body clings |
| Brace | yes (±90° yaw before brace breaks, **PROPOSED**) | no | yes | yes | |

## 3. The panic-window turn

A back-plate line needs 150° or more of yaw (05 section 5.5). The input system must make that turn possible in about 1.0 s on every device without making normal aiming twitchy.

**Mouse:** raw input, no acceleration, no cap. A 150° turn is whatever the player's sensitivity makes it. Default sensitivity: 360° per 30 cm at 800 DPI.

**Stick:** base turn rate 180°/s at full deflection (configurable 90 to 360). **Panic ramp:** when deflection stays above 90% for 0.25 s, turn rate ramps to 2× over the next 0.2 s. A 150° turn therefore takes about 0.65 s: 0.25 s at 180°/s (45°), 0.2 s ramping (54°), then 0.1 s at 360°/s (36°), total 135°, plus the remainder. Ramp resets when deflection drops under 60%. Inner dead zone 8%, outer 5%, response curve 1.8 exponent.

**Turn-rate assist options:**

| Option | Behavior | Rated |
|---|---|---|
| Panic ramp | As above | Allowed, default on for stick |
| Indicator friction | When a line on your body enters the cut cone, look sensitivity drops 30% for 0.3 s so the swing lands | Allowed, default on for stick, off for mouse |
| Assisted turn (**PROPOSED**, accessibility) | Tap the ping button while a line is on your body: the camera turns toward the nearest edge indicator at 540°/s until the line is in the cone, then stops. Still requires the warm-up and swing | Off in Rated. On request in casual |
| Look inversion, sensitivity per axis, ADS not applicable | Standard | |

## 4. Camera shake

Amplitude is peak rotation in degrees (pitch, yaw, roll), with duration and falloff. Shake never moves the camera's forward vector by more than 3° so the cut cone stays honest. Global scale 0 to 100%.

| Event | Amplitude (p, y, r) | Duration | Notes |
|---|---|---|---|
| Dog seats in own plate | 0.8, 0.4, 0.3 | 0.15 s | Directional toward the plate |
| Haul ramp on self | 0.3 continuous | ramp | Low-frequency pull toward the line |
| The Kick (Jack launch, class V 10 to 18 m/s) | 1.5, 0, 1.0 at V 18, scaled by V / 18 | 0.3 s | One hard hit on the Kick frame, then nothing for the push |
| Snap, own line | 2.5, 1.5, 2.0 | 0.5 s | Plus the 10 m/s launch |
| Snap, within 10 m | 1.0, 0.5, 0.5 | 0.3 s | Scaled by tension |
| Stagger | 2.0, 1.0, 1.5 | duration | Decays to 0 by the end |
| Clothesline stagger | 3.0, 0, 2.0 | 0.6 s | Pitch forward |
| Landing under threshold | 0.6, 0, 0 | 0.1 s | |
| Blast, propane 4 m | 3.0, 2.0, 2.0 at 2 m, 0 at 8 m | 0.6 s | |
| Blast, charge 5 m | 4.0, 2.0, 2.5 at 2.5 m, 0 at 10 m | 0.8 s | |
| Rated load consequence within 15 m | 1.5, 0.5, 1.0 | 1.5 s | Low frequency rumble |
| Hauler passing within 3 m | 1.0, 0, 0.5 | pass | |
| The Breathing (Ballast) | 0, 0, up to 6° world tilt | 90 s cycle | Not shake: the world tilts. The camera stays world-up; the hull tilts around it |
| Pulse (Anchor) | 2.0, 1.0, 1.0 | 1.0 s | Outward from the Stump |
| Stall | 4.0, 2.0, 3.0 | 0.2 s | Then the lock |

## 5. The victim-camera replay

At round end (05 section 9.2), 4 s from the victim's first-person camera, starting 3.5 s before the Stall and ending 0.5 s after. The HUD shows the victim Scrap Card (ui-hud.md section 4.16). Additions in replay only: a Bone outline of the line through the body for the first 1 s (so viewers see what the victim could not), the edge indicator as it was, and the degrees-turned counter ticking live in the corner. In-round scraps (Claim, Haul, Free Scrap) do not replay; the victim sees their card while the reserve timer counts.

Replay is also the Ledger's clip format: the three post-match clips use the same 4 s window from the relevant camera.

## 6. Spectator cameras

| Camera | Behavior |
|---|---|
| Follow, first person | The followed player's exact camera, HUD per ui-hud.md section 4.18 |
| Follow, third person | Orbit, 3 m, collision pull-in, free yaw |
| Free cam | 10 m/s, boost 30 m/s, clamped to the arena bound plus 20 m |
| Line cam (**PROPOSED**) | Attaches to a chosen line's midpoint, looks along it; auto-cuts between the two Dogs on force events |
| Gallery | The Marshals' overhead gallery, fixed, slow pan |
| Rails | Authored broadcast rails per arena (Ballast Spine descent, Yard gantry ride, Hangtown cable run, Anchor Ring orbit) |
| Hook cam | Below the lowest floor, looking up the Well. Switches in automatically for 2 s on any ring-out when the director is on auto |
| Auto director | Cuts on seat-in-plate, overspin octave, event sirens, and Stalls, with a 1.5 s minimum shot length |

## 7. Motion sickness mitigations

- Horizon lock during flips and airborne (default on).
- Head bob: off by default in first person; sway amplitude 0 to 100%.
- Speed vignette on drags and falls above 8 m/s, 0 to 100%.
- Camera roll global cap: 0°, 15°, 30° (default 15°).
- Reduce motion preset: no roll, no sway, shake 30%, vignette 50%, flip camera horizon-locked and pitch-clamped ±30°.
- FOV floor 90°; the game never lowers FOV below the setting (no "aim" zoom exists).
- The Breathing and the Sway are world motion, not camera motion; a "stable horizon" option keeps the camera world-up at all times, which is also the default.
- Target 60 fps minimum on all platforms; camera updates on the render tick, not the physics tick.

## 8. Input bindings

Every input in 05: fire Dog, self-dog, mode select (radial or three buttons), release, recall, Cutter, brace, perch, slide, ping, plus movement.

### 8.1 Mouse and keyboard

| Action | Default | Notes |
|---|---|---|
| Move | W A S D | |
| Jump, mantle | Space | Mantle is automatic on jump toward a ledge within class height |
| Sprint | Shift (hold) | Toggle option |
| Crouch | Ctrl (hold) | |
| Slide | Ctrl while sprinting | 1.2 s |
| Brace | Ctrl held while stationary, 0.3 s | Rigger, Breaker only |
| Perch | Space held against a surface | Climber only, 1.5 s |
| Fire Dog (A, then B, then new A) | Mouse 1 | |
| Self-dog | Mouse 2 | Seats the next Dog in own chest plate |
| Haul | 1 | Three-button mode select, pre-selectable |
| Jack | 2 | |
| Hang | 3 | |
| Reel (in Hang) | Hold 1 to shorten, hold 2 to lengthen | Same buttons, Hang context (05 section 3.2): the rod reels at 1.0 m/s bearing the class Hang load. A tap still switches mode; a hold of 0.2 s or longer while in Hang reels instead of switching (**PROPOSED** hold threshold) |
| Mode radial | Mouse 3 (hold) | Flick, release to select. Either scheme may be used at any time |
| Release | X | |
| Recall | R | |
| Cutter | F (tap). Hold 0.6 s on a Hangtown cable | Swing |
| Line focus (Doubler) | Mouse wheel click, or Q | Which line the mode and overspin inputs address |
| Ping (Rig Calls) | Z tap: contextual. Z hold: wheel | |
| Line on me | V | Direct call, no wheel |
| Scoreboard and crew | Tab | |
| Camera toggle | C | Casual only. First and third person, 0.3 s blend, not during a live line on body (**PROPOSED**) |
| Shoulder swap | Mouse 5 | Third person |
| Spectate next, previous | Mouse 1, Mouse 2 | When scrapped |

### 8.2 Controller (Xbox layout; PlayStation equivalents map by position)

| Action | Default | Notes |
|---|---|---|
| Move | Left stick | |
| Look | Right stick | Panic ramp on |
| Sprint | L3 (toggle) | |
| Jump, mantle, perch (hold) | A | |
| Crouch, slide, brace (hold while still) | B (hold) | Toggle option |
| Fire Dog | RT | |
| Self-dog | LT | |
| Cutter | RB | The fastest button on the pad belongs to the knife |
| Mode radial | LB (hold) + right stick flick | Release to select |
| Mode cycle | LB (tap) | Haul, Jack, Hang, repeating |
| Haul, Jack, Hang direct | D-pad left, right, down | Three-button alternative, active alongside the radial |
| Reel (in Hang) | Hold D-pad left to shorten, D-pad right to lengthen; or hold LB and hold the radial direction | Same buttons, Hang context, 0.2 s hold (**PROPOSED**) |
| Release | X | |
| Recall | Y | |
| Line focus (Doubler) | R3 | On other classes R3 is contextual ping |
| Ping (Rig Calls) | D-pad up: tap contextual, hold wheel | |
| Line on me | D-pad up double-tap | |
| Scoreboard and crew | View | |
| Pause, menu | Menu | |
| Camera toggle | Hold View 0.5 s | Casual only |

Rumble: seat in own plate (sharp, directional by trigger side where supported), singing line on body (continuous low rumble scaling with force), Cutter ready tick (one pulse), the Kick (one hard pulse, then the push's low rumble), snap (hard), overspin (rising, octave jump as a double pulse). Adaptive triggers where supported: RT resistance rises with the line's current force.

Remapping: every action is rebindable on both devices; the Cutter cannot be bound to a chord or a hold, because its latency is a gameplay number.
