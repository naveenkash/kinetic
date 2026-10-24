# KINETIC: Animation Requirements

_System spec v0.2.2, revised to 05-core-mechanics v0.2 and the v0.2.1 and v0.2.2 rulings (CHANGELOG 2026-10-24). Implements bible 05 sections 2.2, 2.3, 3.1, 3.4, 3.7, 4.1, 4.2, 4.5, 5.1 to 5.5, 7.1, 7.4, 9.2. Numbers in 05 are canonical. Frame counts are at 60 fps; seconds are the source of truth. Anything marked **PROPOSED** is new here._

## 1. Stance

One shared humanoid skeleton for all five harnesses. Mass reads from silhouette and from motion, never from a different rig. A Breaker and a Climber share bones and share every animation set; they differ in proportions (harness volumes), timing scalars, and additive weight layers.

Rules:

1. **Timing is gameplay.** Every Spool and Cutter animation lands on the 05 number to the frame. Animation never adds latency to an input the spec has already timed.
2. **The body tells the truth.** A dragged player's feet skid. An airborne player's body is physics. A hoisted player hangs from the plate the Dog is in. Players read enemy state from bodies at 50 m.
3. **Camera first.** In first person, the camera is never animated by the body except where camera-and-feel.md allows. The body follows the camera.

## 2. Locomotion

Speeds from 05 section 2.2. One blend space per class is unnecessary; use one shared 2D blend space (speed, direction) with class-specific playback scalars and additive weight layers.

| | Climber | Scrapper | Rigger | Doubler | Breaker |
|---|---|---|---|---|---|
| Run (m/s) | 7.5 | 7.0 | 6.2 | 6.2 | 5.0 |
| Sprint (m/s) | 9.0 | 8.5 | 7.5 | 7.5 | 6.0 |
| Crouch (m/s) | 3.5 | 3.2 | 2.8 | 2.8 | 2.2 |
| Jump height (m) | 1.4 | 1.3 | 1.1 | 1.1 | 0.8 |
| Mantle height (m) | 2.6 | 2.4 | 2.2 | 2.2 | 1.8 |
| Stride length at run (m) **PROPOSED** | 1.7 | 1.6 | 1.5 | 1.5 | 1.4 |
| Weight layer | 0.0 (light, quick settle) | 0.2 | 0.5 | 0.5 | 1.0 (heavy, long settle, head lead) |

Sets: idle (3 variants per class), walk, run, sprint, crouch-walk, start and stop (one footfall of anticipation; Breaker two), turn-in-place at 45° and 90°, jump (start, apex, land light, land heavy), mantle at three heights (low 1.0 m, mid 1.8 m, high at class max; the system picks the nearest and warps), slide (1.2 s, 72 f: entry 10 f, slide 50 f, exit 12 f; the capsule and the pose must pass under a line at 0.7 m), ledge grab (auto within 0.5 m under 8 m/s, with hang, pull-up, and drop), ladder (Ballast Spine, Anchor Scaffold).

Air control feeds a lean additive (0.2 Breaker to 0.8 Climber) so a Climber visibly steers in the air and a Breaker visibly does not.

## 3. Spool actions

The Spool is on the right forearm. The Cutter is in the left hand (05 section 4.1: off hand). Spool actions are upper-body additive over locomotion, except where noted. All Spool actions can play while hoisted and while perched.

| Action | Frames | Notes |
|---|---|---|
| Fire Dog A | 4 f anticipation, fire on f 4, 8 f settle | Wrist flick, Spool spins. Dog leaves on the exact input frame plus 4 f; the projectile spawns on f 4. Settle never blocks the next input |
| Fire Dog B | Same | A slight shoulder check toward the B target is allowed at 6 f, additive |
| Self-dog | 6 f | Hand taps the chest plate. Dog seats on f 6. Must read clearly as self-dog to enemies: the hand goes to the chest |
| Mode select (Haul, Jack, Hang) | Class mode switch: Climber 12 f, Scrapper 9 f, Rigger 15 f, Doubler 15 f, Breaker 21 f | Thumb on the Spool's mode collar, collar rotates. Force changes on the final frame. Pre-selection before firing plays the same collar turn with no line |
| Haul engaged | 24 f ramp (0.4 s) | Forearm pulls in, elbow back, a brace of the stance against the pull direction. Additive strength scales with force |
| Jack engaged: the Kick, then the push | 18 f (0.3 s) | The Kick is the Spool's capacitor discharge (05 section 3.2). f 1 to 4: anticipation, Spool housing tenses. f 4: the Kick fires; forearm snaps out along the line with a hard recoil through the shoulder and a one-frame housing flare. f 5 to 18: settle into the motor-push pose, a steady lean, Haul reversed. Self-dog Jack (Slingshot): the full-body launch pose begins on the Kick frame, see section 6 |
| Hang engaged | 6 f | Collar locks, hand holds still. A Hang is the stillest pose in the game |
| Reel (in Hang) | Loop, 1.0 m/s | Two-hand reel (05 section 3.2): the Cutter hand comes across to the Spool and both hands crank the collar, 2 cranks per meter (30 f each). Shortening cranks toward the body; lengthening cranks away with a pawl release. Upper-body additive; feet stay planted, or the body hangs. The Cutter stays in the left hand; a Cutter swing interrupts the reel (knife priority). A hoisted player reeling their own line plays a hand-over-hand climb instead of the crank |
| Release | 4 f | Thumb lifts off the collar. Line vanishes on f 4 (or becomes a set line) |
| Recall | Climber 24 f, Scrapper 0 f, Rigger 36 f, Doubler 36 f, Breaker 54 f | Spool reels, Dogs return. Scrapper: an instant snap of the wrist, Dogs are simply back. The recall pose ends exactly when the Dogs are available |
| Overspin (Doubler) | Hold from engagement to 108 f (1.8 s) | Hand clamps the collar; forearm shakes with increasing amplitude from f 60 (1.0 s, the octave jump). Spool housing glows. On release, hand opens. On snap at f 108: launch (section 6) |
| Second line (Doubler) | Same fire set, mirrored left-hand reach to the Spool | The Doubler's Spool has a second collar. Focus swap: 4 f thumb move |

### 3.1 Hand IK on the Spool

The left hand does not touch the Spool during normal use (it holds the Cutter). The right forearm's Spool is skinned. Mode collar rotation is a bone (spool_collar) driven by the mode state with the class switch time. Thumb IK target follows the collar. Doubler second collar is a second bone (spool_collar_b).

## 4. The Cutter

Timings from 05 section 4.1. Warm-up and swing are the panic window's last 0.3 s; they are not allowed to drift.

| Phase | Standard | Hot | Long | Notes |
|---|---|---|---|---|
| Warm-up | 12 f (0.20 s) | 7 f (0.12 s) | 16 f (0.26 s) | Blade comes up into frame, edge begins to glow. Class offsets: Scrapper −3 f, Rigger −2 f, Breaker +6 f |
| Swing | 9 f | 9 f | 9 f | Cut registers on f 5 of the swing (PROPOSED). The arc is a horizontal slash across the 70° cone's center |
| Recovery | 18 f | 18 f | 18 f | Blade drops, hand returns. Spool input is allowed from f 1 of recovery (05: the Cutter does not interrupt Spool use except during the swing) |
| Cable hold (Hangtown) | 36 f (0.6 s) | | | Two-handed press on the cable, blade sawing. Interrupted by any movement input |
| Melee hit on a plate | Uses swing | | | Contact unseats the Dog; the target plays a 18 f (0.3 s) stagger |

The Cutter arrives when the player reaches Grade 2 (the Cutter ceremony, 04). The Greenhand loaner has the same timings and a different model.

## 5. The drag skid

Per 05 section 5.1. A grounded player under line force below their foot-hold is kinematic; the line overrides movement while the feet stay planted and skid.

- Full-body state, not additive. Locomotion is replaced.
- Pose: hips low, knees bent, both feet planted, torso leaning away from the pull, arms free (Spool and Cutter remain usable). Lean angle scales with line acceleration from 0 at 0 m/s² to 35° at the class foot-hold (Climber 3, Scrapper 4, Rigger 5, Doubler 5, Breaker 5 m/s², 05 section 5.3). A Doubler's full overspin (1,200 N, 5.5 m/s²) lifts an unbraced Breaker, so the Breaker's skid-to-airborne transition is a real animation, not a theoretical one; Brace is what keeps a Breaker planted. In the 05 section 4.3 reference case a Rigger's feet leave the floor 0.26 s after a Breaker's Dog seats, so the skid is short and must read on its first frames. A Scrapper's 400 N only lifts a Climber; everyone else skids against it.
- Feet: IK-pinned to the ground, sliding. A 4-frame micro-step is triggered every 0.6 m of drag so the feet do not look welded.
- Direction: the body's facing is free (follows camera yaw, section 8), the skid vector is the line's.
- Exit: when acceleration exceeds foot-hold, or the player is pulled upward, or leaves a ledge, blend to airborne in 3 f.

## 6. Airborne: ragdoll-with-pose

Per 05 section 5.1: a simulated ragdoll-with-pose, physical body, animated upper body.

| Bones | Mode |
|---|---|
| pelvis, spine_01, thigh_l, thigh_r, calf_l, calf_r, foot_l, foot_r | **Physical.** Simulated. Receive line force and collisions |
| spine_02, spine_03, neck, head | **Blended.** Physical at 0.4 blend weight, animated at 0.6, so the head stays readable and the camera (first person) stays stable |
| clavicles, upper arms, forearms, hands | **Animated.** Driven by the Spool and Cutter sets so the player can fire and cut in the air. Collide as kinematic |
| spool_collar, cutter | **Animated** |

Pose targets (the animated layer the physics is driven toward, via physical animation strength):

- **Launched** (the Kick, at class launch speed V: Climber 18, Scrapper 14, Rigger 12, Doubler 14, Breaker 10 m/s; overspin snap recoil at 10 m/s): arms tucked briefly then spread, 12 f, then a flight pose facing the velocity. Pose intensity scales with V.
- **Airborne fire** (all classes, 05 section 2.3): the fire set plays on the animated arms. Non-Climbers add a 6 f upper-body check as they lose 15% horizontal speed; the Climber's Updraft kit keeps full momentum and plays no check.
- **Falling** (under 8 m/s vertical): upright-ish, arms out.
- **Falling fast** (over 8 m/s): ragdoll weight rises to 0.8 on the spine; the player is losing the pose. No ledge grab above 8 m/s.
- **Dragged airborne** (line force off the ground): body trails the line attachment plate. The plate bone leads; everything else follows. A back-plate drag is head-back, feet-forward.
- **Flipped** (over a Hang line above 6 m/s): full ragdoll weight 1.0 for the arc, momentum preserved, pose recovers on landing.

Landing: under the stagger threshold (5 m/s vs static), a 6 f land and control returns. Over it: stagger set (section 7). The physical layer blends back to animated over 8 f.

## 7. Hoisted, flipped, stagger, Stall

- **Hoisted** (05 section 5.2): hanging from the seated plate. Four hoisted poses, one per plate. Chest: facing up and back, arms free. Back: facing down, arms free, head turning. Shoulders: hanging sideways, the seated shoulder high. Legs are physical and swing. Spool and Cutter sets play on top. Cannot move; no locomotion.
- **Stagger**: 36 f (0.6 s); 60 f (1.0 s) against a Rigger's Hang line; 48 f (0.8 s) after a snap; 18 f (0.3 s) from a Cutter hit on a plate. Full-body stumble in the impact direction, arms recover last. Movement and the Spool set are locked; look and the Cutter stay live (05 section 7.1), so the Cutter set must play over the stumble and the stumble must not throw the camera more than camera-and-feel.md allows.
- **Clothesline stagger** (over 3 m/s into a Hang line): a specific chest-catch stumble, line height passed in as a parameter (0.7 to 1.9 m) so the body bends at the line.
- **Stall lock** (05 section 7.4): 12 f (0.2 s). The ragdoll freezes in its current pose: physical animation strength ramps to full lock, joints go rigid, gel VFX. Then the body is a prop at its class mass (90 to 220 kg, Medium tier, 05 v0.2.1) for 4 s; the physics asset keeps the class mass through the lock. The lock must hold any pose, including upside down.
- **Hook retrieval**: at 4 s a Hook clamps the harness back plate; a 2 s lift. The body is still locked; only the Hook animates (descend 1 s, clamp, lift 2 s, exit).

## 8. The three cut-self-free turn cases

Per 05 sections 4.2 and 5.5. The line must be inside the 70° camera cone to cut it. The plate determines the turn.

| Plate | Turn needed | Body solution |
|---|---|---|
| Chest | 0° (already visible) | None. The line leaves the chest and is in frame. Cutter swing targets the chest root |
| Left or right shoulder | 60° to 90° | Aim-offset torso twist covers up to 90° of yaw with feet planted (grounded). The upper body (spine_02 up) twists toward the camera; the Spool arm stays on the line's side |
| Back | 150°+ | Torso twist cannot reach it. Grounded and free: turn-in-place steps (a 90° step is 8 f, chained). Dragged or airborne: the **harness swivel** rule below |

**Harness swivel (PROPOSED):** while dragged, hoisted, or airborne, body yaw follows camera yaw directly, with no step animation and no animation-imposed limit. The fiction: the rigger twists in the harness. The limit on how fast a player can find a back-plate line is therefore the camera turn rate (camera-and-feel.md), not a body animation. This is a deliberate decision: an animation-limited turn would make the panic window unfair to controller players and would violate pillar 5 ("the knife is always enough, if you're fast").

**Camera constraint:** in first person the camera never leads or lags the yaw input during the panic window. Torso twist and foot steps are visual only. In third person (casual only, 90° FOV) the camera orbits freely; the body's twist is cosmetic and the cut still resolves against the camera's forward vector.

Line attachment: the line's body end is a socket on the plate bone (plate_chest, plate_back, plate_shoulder_l, plate_shoulder_r). Sockets are placed so that a back-plate line exits between the shoulder blades and is occluded by the body from the first-person camera until the camera has turned roughly 150°. Verify with a test: at 149° the line is not in the cone; at 151° it is.

## 9. Brace, Perch, Drops, Rig Calls

- **Brace** (Rigger, Breaker): 18 f (0.3 s) to engage from a stationary crouch. Feet widen, weight drops, Breaker locks the harness (a visible plate click). Braced idle: nearly still. Unbraced foot-hold is 5 m/s² for both; braced it is 9 m/s² (Rigger) and 20 m/s² (Breaker) per 05 section 5.4: a braced Rigger's pose visibly strains against a Breaker's line (7.7 m/s²) and breaks under a Doubler's full overspin (9.2 m/s²); a braced Breaker is static to every line in the game and shows no strain at all. Breaks on movement, jump, or an over-threshold object hit, 6 f out.
- **Perch** (Climber): contact with any surface, up to 90 f (1.5 s). A one-hand, one-foot cling, body flat to the surface, Spool arm free. Drop at 90 f with a 6 f push-off. Perch idle breathes; the 1.5 s drain must be visible in the body (the grip hand slowly opens from f 60).
- **Drops** (match entry, 6 s): lowered by a Hook (cable descent, unclip, land), sliding a line (hands on line, slide, release, land), stepping off a Hauler (ride, step, land), dropped like cargo (Breaker: crate fall, land hard, stand). Each Face has a personality pass on the land and stand. Lines disabled during the Drop.
- **Rig Calls**: eight hand signals (ui-hud.md section 4.17), upper-body additive, 20 to 40 f, usable while moving. Ascendancy prayer gestures and Fray bookie signals are cosmetic replacements with the same timing. The rude ones exist.

## 10. First-person arms

A separate first-person arm set, matching the third-person timings exactly. Right forearm with Spool (mode collar visible and readable at the bottom-right of frame), left hand with Cutter. The Cutter is visible at rest, low left; it rises during warm-up and is in frame for the swing. Class differences: Breaker's Spool housing is larger and worn low; Climber's is a bracelet; Doubler's has two collars and heat scoring; Scrapper's is modified; Rigger's is taped. Rope Mark tape is visible on the line near the Spool in first person (04: readable as a résumé).

First-person arm sets required: idle, locomotion sway (per class weight), fire A, fire B, self-dog, mode select, Haul, Jack (Kick and push), Hang, Reel (two-hand crank and hand-over-hand climb), release, recall, overspin, Cutter warm-up, swing, recovery, cable hold, drag skid (arms brace), airborne, hoisted (four plates), stagger, Rig Calls, Drop.

## 11. NPC sets

- **Marshals**: gallery idle, lean, point, note, call (lip-sync to the callout list). Never excited.
- **Prep crew**: pre-match only. Place charge, tape a rated load, paint Tallies, wave the crew in, leave.
- **Hooks**: descend, clamp, lift, exit. Rotor loop.
- **Tutorial bot crew** (HTR orange): the full player set, driven by behavior trees. No special animations; they are riggers.
- **Teodora Quispe and Baskoro Hanif** (Apprenticeship): the player set plus a tutorial gesture set: point at a plate, tap the chest, hold up the Cutter, the ceremony handover (two-handed, 90 f, held).

## 12. Retarget and rigging plan

- **Skeleton**: one UE5 skeleton, Mannequin-compatible proportions at Rigger scale. Class proportions are achieved by harness meshes and a small per-class bone scale on clavicles and spine (Breaker +8% width, Climber −5%), never by bone length changes that would break the shared set.
- **Extra bones**: plate_chest, plate_back, plate_shoulder_l, plate_shoulder_r (line sockets, physics-collidable), spool_collar, spool_collar_b, cutter_blade, harness_root (for the Stall lock and Hook clamp).
- **Control Rig**: a shared rig with FK/IK switching on arms and legs. Foot IK with slope and tilt compensation for the Ballast hull (up to 6° per axis on the Breathing) and Hangtown Sway (dynamic). Pelvis offset is computed from the tilt so a player standing on a tilted hull looks planted, not slid. Hand IK on the Spool collar and on perch surfaces. Look-at on head and spine_02 up for aim offset to ±90° yaw, ±70° pitch.
- **Physics asset**: one shared asset with class mass distribution (90 to 220 kg) set per class at runtime. Plate bones carry small collision boxes for Dog seating. Capsule 1.9 m tall, 0.4 m radius for all classes (05 section 2.1); the harness mesh may exceed it visually by up to 15 cm (Breaker).
- **Retargeting**: IK Retargeter from the shared skeleton to Face-specific meshes (same skeleton, no retarget needed in practice) and to NPC Marshals and prep crew (Mannequin). Mocap retargets through the same chain.
- **Animation budget**: at 60 fps, 6 characters (3v3 at launch; 9 for post-launch 3v3v3) at full rate within 30 m, half rate 30 to 60 m, quarter beyond. Physical animation runs full rate on any airborne body regardless of distance, because an airborne body is a tell.
- **Network**: airborne ragdoll-with-pose is simulated on the server with the client predicting; the plate socket transform replicates at full rate because the line's visual end depends on it. Mass differences (05 open question 6 in 04) must resolve identically on both clients before this plan is final.
