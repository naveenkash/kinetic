# KINETIC: Player Physics

_Status: proposal v0.1, 2026-10-24. Engine: UE 5.8.x. Thresholds and class numbers are from `docs/bible/05-core-mechanics.md` sections 2, 5 and 7 and are not restated where a section reference suffices._

## 1. The character as two bodies

| Body | Type | Active when | Networking |
|---|---|---|---|
| Movement capsule (1.9 m tall, 0.4 m radius) | CharacterMovementComponent, kinematic | Grounded, Staggered, Braced, Perched, Jumping or Falling under own power | CMC client prediction and server correction |
| Flight body (capsule rigid body, same dimensions, class mass) | Chaos simulated | Dragged past foot-hold, Airborne, Hoisted, Flipped, Stalled | Networked physics: Resimulation for the owner, Predictive Interpolation for others |
| Visual ragdoll (physics asset, 14 to 16 bodies) | Chaos simulated, cosmetic, client only | Whenever the flight body is active | Not replicated |

Only one of the first two is in the simulation at a time. The other is disabled (collision off, not ticking). The visual ragdoll is driven toward the flight body's pose by Physics Control, with the upper body animated (the "ragdoll-with-pose" from 05 section 5.1). The Physics Control plugin is marked "ready for production use" in 5.8 ([release notes](https://dev.epicgames.com/documentation/unreal-engine/unreal-engine-5-8-release-notes)) and manages spring/damper Controls on skeletal mesh bodies ([API](https://dev.epicgames.com/documentation/unreal-engine/API/Plugins/PhysicsControl/UPhysicsControlComponent)).

### 1.1 Why CMC and not Mover

- Mover is Experimental in 5.8 and Epic says to use caution shipping with it ([docs](https://dev.epicgames.com/documentation/en-us/unreal-engine/mover-in-unreal-engine)). Epic's own 5.8 notes describe work "toward moving Mover out of Experimental status", so it is not there yet.
- Chaos Mover (physics-thread movement) adds input latency by design because the physics simulation runs ahead and the visible character interpolates; physics-driven Mover actors are not synchronized with Network Prediction actors; and interactable physics objects need their own replication or the server and client desync ([Mover examples](https://dev.epicgames.com/documentation/en-us/unreal-engine/mover-examples-in-unreal-engine)).
- The bible already splits the player into kinematic-while-grounded and simulated-while-airborne. That split maps cleanly onto CMC plus a networked rigid body. We get CMC's mature prediction for the 80 percent of the time players are on their feet, and networked physics for the 20 percent that decides rounds.
- Cost of the decision: two codepaths and a handoff. That handoff is prototype week 2 in `architecture-overview.md`.
- Re-evaluate at the first engine version where Mover and Chaos Mover are Beta or better with documented Network Prediction interop. A third-party guide forecasts promotion around 5.8 or 5.9 ([StraySpark, 2026](https://www.strayspark.studio/blog/mover-2-0-vs-character-movement-component-ue5-7-2026)); that is a forecast, not an Epic commitment.

## 2. State machine

```
                 +----------+
    +----------->| Grounded |<------------------------------+
    |            +----+-----+                               |
    |   line force,   |  a_line <= foot-hold                |
    |   below foot-   v                                     | landing impact
    |   hold     +---------+   a_line > foot-hold,          | < stagger threshold
    |            | Dragged |-- pulled up, or ledge left --+  |
    |            +---------+                              |  |
    |                                                     v  |
    |   +----------+  upward line, feet unloaded   +----------+
    |   | Hoisted  |<------------------------------| Airborne |----+
    |   +----+-----+  release / cut -------------->+----+-----+    |
    |        |                                          |          | landing impact
    |        +------------------------------------------+          | >= stagger threshold
    |                       Hang line hit > 6 m/s,                 v
    |                       impact >= flip threshold       +-----------+
    |                   +--------------------------------->| Staggered |
    |                   |                                  +-----+-----+
    |              +----+----+   lands                          |  0.6 s (0.8 overspin,
    |              | Flipped |------------------------------->  |  1.0 Rigger line)
    |              +---------+                                  |
    +-----------------------------------------------------------+
                           any state --Stall threshold--> +---------+
                                                           | Stalled |  (terminal)
                                                           +---------+
```

Entry and exit conditions, from 05 sections 5 and 7:

| State | Body | Enter when | Exit to |
|---|---|---|---|
| Grounded | CMC | Default; landing with impact under the stagger threshold | Dragged, Airborne, Staggered, Stalled |
| Dragged | CMC, `MOVE_Dragged` | A line force acts on a grounded player and `F / m <= foot-hold` (5.3: 3 / 4 / 5 / 5 / 5 m/s²; braced 9 Rigger, 20 Breaker). Examples from 5.3: a Breaker's 1,000 N takes every unbraced class off its feet except another Breaker (4.5 m/s²), who skids; a Doubler's full overspin (1,200 N, 5.5 m/s²) takes an unbraced Breaker off its feet; a Scrapper's 400 N takes only a Climber (4.4 m/s²) off its feet | Grounded when force stops; Airborne when `F / m > foot-hold`, or the force has an upward component that unloads the feet, or the capsule leaves a ledge |
| Airborne | Flight body | From Dragged as above; from a self-dog Kick (the Jack launch); from a Hang line hit over 6 m/s (that is Flipped); from a force volume whose per-body acceleration `a_ref × 100 / m` exceeds foot-hold (6.6) | Grounded on landing under stagger threshold; Staggered on landing at or over it; Hoisted; Stalled |
| Hoisted | Flight body | Pulled upward by a line with feet off the ground (5.2). Can look, cut, fire Dogs. No locomotion. A hanging player climbs or descends by Reel (hold Haul or Jack while in Hang, 1.0 m/s, 3.2); only a Breaker's 1,000 N Haul lifts a Climber outright | Airborne on release or cut |
| Flipped | Flight body, ragdoll weight 1.0 | Hang line hit over 6 m/s (3.5); object impulse 600 to 1,199 N·s; static hit 8 to 11.99 m/s; kinematic hit 6 to 8.99 m/s; propane blast at 2 to 4 m (7.1, 6.4) | Staggered on landing |
| Staggered | CMC, movement and Spool locked, look and Cutter live (7.1) | 0.6 s and camera shake; Hang line hit 3 to 6 m/s; object impulse 300 to 599 N·s; static hit 5 to 7.99 m/s; kinematic 4 to 5.99 m/s; Cutter melee on a plate 0.3 s; fire patch entry; overspin recoil 0.8 s | Grounded when timer ends |
| Stalled | Flight body frozen | Any Stall threshold in 7.1, crush, blast within 2 m, ring-out | Terminal for the round |

Own-power jumping and falling stay in CMC's `Falling` mode with class air control. Only line-driven, impact-driven and force-volume-driven flight uses the flight body. Firing a Dog while airborne is allowed for every class; the Climber keeps full momentum and every other class loses 15 percent of horizontal speed per airborne fire (2.3), applied as a velocity scale on the flight body or on CMC `Falling` velocity. The Dragged skid accelerates the capsule at `F / m`, so the 3.3 mass split holds whether or not the feet leave the floor. The bible's 5.1 wording "they leave a ledge" applies to a player who is being dragged; a voluntary step off a ledge is CMC falling.

### 2.1 Rulings the implementation needed (PROPOSED)

The engine-free state machine ([game/Source/KineticCore/Public/Kinetic/PlayerState.h](../../game/Source/KineticCore/Public/Kinetic/PlayerState.h)) settles these; design should confirm or change them.

- Foot-hold is judged on the part of the line force along the ground, and holds like friction: a line pulling up takes weight off the feet and lowers foot-hold in proportion, to nothing once it lifts the whole weight (then a mostly upward line hoists). Pulling down does not raise it. A flat pull is judged exactly as 5.3 says, and 5.3 and 5.4's examples hold at 30° up too.
- Look and the Cutter stay live in every state but Stalled, so the panic window survives a stagger or a flip. The Spool is locked while Staggered or Flipped.
- A stagger taken in the air keeps the player on the flight body; its timer runs, and landing after it has run out is a normal landing. A stagger on a perch or a ledge drops the player onto the flight body.
- Landing at or over the stagger speed staggers (the flip, the tumble, belongs to hits in the air); landing at the Stall speed Stalls; a moving platform is judged on the kinematic table. Falls under the player's own power are judged too, on the contact speed or, when the floor appears without a contact report, on the vertical speed. Only a contact within 45° of up is a landing. Any such contact at the Stall speed Stalls, on either body, whatever else happens that step. The switch back to the capsule needs the line and any force volume to leave the player standing there, and waits out the step of a capsule-to-flight handoff and the step after, so the switch never bounces back. Contacts at or over the Flip speed still count while the handoff settles (4.2). A contact the player cannot stand on yet ends a flip, and a hard one staggers, on the flight body; while staggered, only a contact at the Flip speed restarts the stagger. On the capsule, only a walkable contact is floor.
- Stagger times: 0.6 s; a Hang line hit uses the line owner's class value (Rigger 1.0 s); overspin recoil 0.8 s; a Cutter melee 0.3 s.
- One perch per time in the air; standing again, however the player got there, gives it back. A perch ends at 1.5 s, on release, or when a line or force volume pulls harder than the Climber's foot-hold; it drops the player under their own power.
- A ledge is grabbed only falling, at under 8 m/s in all (a player pulled past it fast does not get it). Jumping from a ledge hang climbs up; the drop input (crouch, by default) lets go.
- A body in the Anchor's Well that never falls clear of the volume despawns 10 s after the Stall.

## 3. Handoff between bodies

Grounded to Airborne (server and owning client both run this):

1. Read the capsule's position, velocity and the integrated line force.
2. Enable the flight body at the capsule transform with the capsule velocity. Disable the capsule's collision and movement in the same physics step. The flight body, like every simulated body, is clamped to the 50 m/s hard velocity cap after each step (6.3), however the speed was reached.
3. The line solver switches the endpoint from "kinematic, invMass 0" to "body, invMass 1/m" on that step.
4. The owner's flight body is in Resimulation mode, so the owner is already running ahead; the server correction arrives inside the normal history window.

Airborne to Grounded:

1. The flight body reports a ground contact whose normal is within the CMC walkable angle and whose impact speed is under the stagger threshold (or the stagger has finished).
2. Teleport the capsule to the flight body's position, set CMC velocity to the body's horizontal velocity, disable the flight body.
3. CMC resumes prediction from a fresh move. A one-frame correction is acceptable; a pop over 10 cm is a bug.

Two bodies in the same physics step is forbidden. The switch is atomic inside the sim callback so a resimulation replays it identically.

## 4. Impact detection

All thresholds in 05 section 7.1 are evaluated on the server in the physics thread. The owning client evaluates the same code to show pending Stagger, Flip, and Stall; the server result wins.

### 4.1 Measuring relative speed and impulse

- Chaos exposes per-contact impulses through collision events on the flight body. Enable `bNotifyRigidBodyCollision` on the flight body and read the contact modifier/collision event data in the sim callback, not on the game thread.
- For each contact: `v_rel = dot(v_self - v_other, n)` at the contact point, where `v_other` is the other body's point velocity (for a kinematic, its scripted velocity at that point; for static, zero).
- **Player hits static or kinematic** uses `v_rel`. Static thresholds 5 / 8 / 12 m/s, kinematic 4 / 6 / 9 m/s.
- **Object hits player** uses the contact impulse magnitude from the solver, summed per other body over the frame. Thresholds 300 / 600 / 1,200 N·s.
- **Player hits player** follows 7.2: each party is evaluated with impulse `m_other * v_rel` against the object table. The 14 m/s symmetric floor also applies.
- Surface modifiers (7.3) multiply the stagger, flip and Stall thresholds by the physical material of the other body: hard 1.0, wood 1.2, scrap/laundry/nets 2.0, water over 1 m 3.0, Stall Gel 4.0. Materials are tagged in `DT_PhysMaterial`.
- A player's body is also an impacting object against destructible fracture thresholds (6.5): the Impact Monitor forwards the player-side impulse (class mass × relative speed) to the Destruction Director. A flipped 130 kg Rigger at 5 m/s delivers 650 N·s and breaks a market stall.
- Each body is judged on its own measure: static and kinematic on `v_rel` only (a Breaker's solver impulse against a wall is 220 kg × v and must not decide it, 7.2), objects on impulse, players on `m_other * v_rel`. The most severe outcome across bodies wins. A 2,000 kg container at 0.6 m/s gives 1,200 N·s and Stalls even though `v_rel` is small, which is the bible's intent.
- An object's impulse is classified less the share of its weight the player carries, `m_other * g * dt * max(0, -n·up)` per step, where `n` is the direction the contact pushes the player (PROPOSED). An object arriving sideways bears its weight on the floor, so nothing is subtracted. Without this, a 2,000 kg container sitting still on a player reads 327 N·s every 60 Hz step and staggers them four times a second, more often at lower tick rates. A sustained heavy load is the crush test's job (4.3).

### 4.2 Multi-contact frames

Chaos can report several manifold points against the same body and contacts against several bodies in one step. Rules:

- Sum impulses per other body within a step. Never sum across bodies.
- Debounce: a body that already produced a Stagger or Flip against the player is ignored for 0.25 s unless the measure that judged it (speed, impulse or `m_other * v_rel`) is at least 2x the previous one. A Stall is never debounced (PROPOSED).
- Resting contacts: ignore contacts whose `v_rel < 1 m/s` and impulse under 50 N·s. These are support forces, not hits. With the current tables this holds by construction: every threshold times every surface modifier is above both limits, and a test enforces it.
- The first two physics steps after a body handoff ignore contacts under the Flip threshold, to swallow the teleport settle.

### 4.3 The crush test

"Pinned between two bodies with closing force over 8 kN for 0.5 s" (7.1). Crush is unreachable by lines (v0.2.1): at 1,200 N peak a line cannot supply 8 kN, so the detector ignores any contact force attributable to the line solver and considers only kinematics and stacked or falling weight:

- Each step, collect the contact normal forces on the flight body (impulse / dt) and sum them per other body (Chaos reports several manifold points per body pair). If two bodies press from opposing directions (`dot < -0.7`) and both reach 8 kN (inclusive), start or continue a crush timer. Otherwise decay the timer at 2x rate.
- Crush timer at 0.5 s is a Stall.
- At least one of the two opposing contacts must be a kinematic actor or a rigid body with mass 300 kg or more (Heavy, Rated, or a falling Medium stack). Contacts against a line-constrained body with no kinematic or Heavy on either side never count. This keeps the Kick and Haul out of the crush path by construction.
- A grounded CMC player can also be crushed. The capsule is kinematic so it reports no contact forces. The server runs an overlap each game tick for the CMC capsule against simulated and kinematic bodies; if a body overlaps the capsule from a direction opposed to a blocking surface and the body's push force (mass times its deceleration against the capsule) exceeds 8 kN, the capsule hands off to the flight body so the physics measure takes over. Tune in week 4.

### 4.4 Blasts and fire

Volatiles (6.4) apply impulses through radial fields and the Stall/Flip rules by distance, not through contacts. Fire patch entry staggers and applies a 4 m/s outward velocity; it is a force volume with a one-shot stagger, never damage.

## 5. The Stall sequence

Server:

1. Threshold crossed in the physics thread. The flight body is set kinematic at its current pose on that step (harness lock). The visual ragdoll locks too. All lines on the body release immediately.
2. Game thread next frame: `OnStall(victim, killer, cause)` multicast. Scrap Card to killer and victim. Marshal bark. Replay marker written.
3. 0.2 s later the gel VFX completes; the body becomes a 180 kg non-fracturing Medium prop, simulated, for 4 s. It can be moved by lines.
4. At 4 s a Hook spawns above, attaches a kinematic lift to the body, and raises it over 2 s. Then the body and Hook despawn. Exception: a body that entered the Anchor's Well is not retrieved; it falls out of sight and despawns below the volume (7.4).

Client (owner): on the local prediction of a Stall the client freezes its own camera shake and shows the harness-lock VFX immediately. If the server does not confirm within the grace window (`networking.md` section 6), the client unfreezes. No Scrap Card is shown on prediction.

## 6. Ring-out

- Each arena has a `AKineticWellVolume` below the lowest playable floor. Any player body entering it is Stalled with cause `Well`.
- A second volume 0.5 s of free fall above it (at 50 m/s terminal velocity that is up to 25 m; use per-arena authored height) triggers the Hooks spin-up audio for that player (7.5, 11).
- The Anchor's Well plays a different cue (no Hooks): the Stump's hum drops an octave (v0.2.2).
- Claim marker entering a Well resets to its authored position (9.4).

## 7. Brace and Perch

**Brace** (Rigger, Breaker): crouch held while stationary for 0.3 s. Effects: foot-hold becomes 9 m/s² (Rigger) or 20 m/s² (Breaker). Check against 05 section 5.4: a braced Rigger (130 kg × 9 = 1,170 N) holds a Breaker's 1,000 N and loses to a Doubler's full overspin at 1,200 N; a braced Breaker (220 kg × 20 = 4,400 N) is static to every line in the game and only an object impact above the stagger threshold or a kinematic breaks it. Unbraced, a Breaker's foot-hold is 5 m/s², so a Doubler's overspin takes them off their feet; that gap is why Brace matters for Breakers (v0.2.1). The line solver treats a braced endpoint as invMass 0 while the line force divided by mass is below that value. Brace ends on any movement input, jump, or an incoming impulse at or over the stagger threshold. Brace is a gameplay tag (`State.Braced`) so the solver and animation read one source.


**Perch** (Climber): on contact with any non-walkable surface while airborne, the player may hold for up to 1.5 s. Implementation: handoff to CMC in a custom `MOVE_Perch` mode, capsule kinematic and stuck to the surface point. Dogs may be fired. At 1.5 s or on input, drop to `Falling` with zero velocity. Perch is not available while a line force on the player exceeds the Climber foot-hold.

## 8. Mantle and ledge grab

- **Mantle**: CMC forward trace at class mantle height (2.6 / 2.4 / 2.2 / 2.2 / 1.8 m) while grounded or jumping under own power. Root-motion-less: capsule moves along an authored curve over 0.5 s. Not available from the flight body.
- **Ledge grab**: automatic while falling under own power or from the flight body when the flight body's vertical speed is under 8 m/s and a ledge is within 0.5 m of the hands. Above 8 m/s, no grab, by rule (2.3). From the flight body this is a handoff to CMC `MOVE_LedgeHang`. A line force over foot-hold on a hanging player breaks the grab.
- **Slide**: sprint plus crouch, 1.2 s, capsule half-height 0.35 m so it passes under lines at 0.7 m.
- **Ladders**: CMC custom `MOVE_Ladder` at 2.0 m/s, Breaker 1.6 m/s (v0.2.2). Dogs can be fired from a ladder; a line force above foot-hold pulls the player off it into the flight body.

## 9. Replication and prediction of airborne state

| Aspect | Owning client | Server | Other clients |
|---|---|---|---|
| Flight body transform and velocity | Predicted ahead by about half RTT in Resimulation mode; rewinds on mismatch ([overview](https://dev.epicgames.com/documentation/en-us/unreal-engine/networked-physics-overview)) | Authoritative | Predictive Interpolation |
| State enum (Grounded, Dragged, ...) | Predicted; reconciled with the body | Authoritative, replicated on change | Replicated, drives animation |
| Air control input (class fraction of ground acceleration) | Sent as networked physics input each physics step; applied as a force in the sim callback | Applies the same input | None |
| Camera | Always local; never corrected. First person in rated play; third person only in casual modes, locked at 90° FOV (2.1) | n/a | n/a |
| Cutter and Spool inputs while airborne | Predicted animation; server validates | Authoritative | Replicated events |

Air control is the only input that moves an airborne player, and it is small (0.2 to 0.8 of ground acceleration). Mispredictions therefore come almost entirely from other bodies, which is what Predictive Interpolation on those bodies is for. The 5.8 Networked Actions feature (Predicted and Proposed authoring styles) is the right tool for one-shot physics inputs such as the self-dog Kick ([Devtricks, 5.8](https://vorixo.github.io/devtricks/phys-prediction-use/)).

## 10. Anti-cheat notes

- The client never sends a position for the flight body; it sends inputs. The server ignores any CMC move whose acceleration exceeds class values by more than 5 percent or whose velocity exceeds class sprint speed plus the current line-induced speed.
- Threshold evaluation, Stall, ring-out, seat, cut and break-free are server-only. A client claiming a cut sends the swing frame and camera rotation; the server reruns the cone test from its own line state at that frame.
- Camera rotation rate is clamped server-side at 3,000 degrees per second for the cut cone test, so a scripted instant turn-and-cut is rejected.
- Catch Plate rewind is capped at 150 ms; a client reporting an older frame gets a present-time test.
- Client anti-cheat: server authority is the primary defense; a commercial client anti-cheat is evaluated in Phase 3 ([backend-architecture.md](backend-architecture.md) section 16). Statistical detection of impossible match facts runs in the backend.
- Physics mispredictions are logged per player; a client whose corrections are systematically in its favour (lower impact speeds, later ring-outs) is flagged for review.
