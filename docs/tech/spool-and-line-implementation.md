# KINETIC: Spool and Line Implementation

_Status: proposal v0.1, 2026-10-24. Engine: UE 5.8.x. Numbers come from `docs/bible/05-core-mechanics.md` sections 3, 4, 5 and 11. Engine units are centimeters and kilograms; forces in the solver are in kg·cm/s² (1 N = 100 kg·cm/s²). Tables below stay in SI; convert at the data-table load boundary._

## 1. Components

| Class | Lives on | Role |
|---|---|---|
| `UKineticSpoolComponent` | Player pawn | Input, class tuning, Dog lifecycle, mode selection, overspin, Doubler second line, recall timers |
| `AKineticDog` | Spawned per shot | Swept projectile, seat logic, surface type, Catch Plate selection |
| `AKineticLine` | Spawned when Dog A seats | Replicated line state, render, audio bridge, set-line timer |
| `FKineticLineSolver` | Physics thread (sim callback) | Per-tick force and constraint resolution for all live lines |
| `UKineticCutterComponent` | Player pawn | Cutter warm-up, swing, cone test, cut request |

The solver runs inside a `Chaos::TSimCallbackObject` registered with rewind support so it participates in networked physics resimulation. Epic's 5.8 overview states that the Network Physics component is a low-level C++ system and that resimulation rewinds physics state from history ([overview](https://dev.epicgames.com/documentation/en-us/unreal-engine/networked-physics-overview)). Community 5.8 guides confirm that during a resimulation only the physics thread ticks, so every force decision belongs in the callback, never in `Tick` ([Devtricks, updated 2026-09-23](https://vorixo.github.io/devtricks/phys-prediction-use/)).

## 2. The Dog projectile

### 2.1 Flight

- Not a rigid body. Each physics step the Dog advances by `v * dt` under gravity (9.81 m/s²) and sweeps a 0.08 m sphere from the previous to the new position.
- Speed by class (3.4): 140, 160, 120, 120, 90 m/s. At 160 m/s and 60 Hz a step is 2.67 m, which is why a sweep is mandatory.
- Max flight distance is the class line length. Past it, or on a miss, the Dog becomes a cosmetic prop and auto-recalls in 1 s.
- Owning client predicts flight and shows a provisional seat. Server runs the same sweep against its rewound world (see 2.3) and sends the authoritative seat or miss. Mismatch resolution: the client snaps the Dog to the server's result; the line, if already drawn, re-targets in one frame.

### 2.2 Seating

Sweep hit result decides the seat type:

| Hit | Seat | Notes |
|---|---|---|
| Static world (any `ECC_WorldStatic`) | `Static` | Anchor point in world space. Surface physical material recorded for Mag/Spike Dog modifiers (04 progression). The gas manifold is static world geometry |
| Simulated rigid body | `Body` | Anchor in body-local space, body handle stored. A demolition charge seats like any body and is not triggered by the seat (6.4) |
| Geometry collection | `GCFragment` | Cluster/particle index stored. On fracture the Dog reseats in the largest fragment (3.5) |
| Kinematic actor | `Kinematic` | Anchor in actor-local space |
| Player Catch Plate | `Plate` | Nearest of chest, back, left shoulder, right shoulder (5.5). Plate id drives the HUD edge indicator |
| Other line, Hook, Marshal gallery | miss | Collision channel `KineticLine` is ignored by the Dog sweep |

Catch Plates are four sockets on the skeletal mesh, each with a 0.18 m sphere. The sweep uses a dedicated `ECC_CatchPlate` object channel so plates win over the capsule.

### 2.3 Hit registration and rewind

The server keeps a 20-frame (333 ms at 60 Hz) ring buffer of every player's four plate transforms and every simulated body's transform. A `FireDog` RPC carries the client's physics frame number. The server rewinds plates and bodies to that frame, capped at 150 ms back, runs the sweep, and seats. Rewind is for the seat test only; the line's forces always act on present-time bodies.

Unreal has no built-in lag-compensated hit test; this buffer is project code. Keep it small and typed: 8 players × 4 plates × 20 frames × 28 bytes is under 20 KB.

### 2.4 Self-dog

Secondary input seats the current Dog in the owner's chest plate instantly, with no projectile. The line's first endpoint becomes `Plate(chest, self)`.

## 3. The Line as a constraint object

### 3.1 State

```cpp
enum class ELineMode : uint8 { Slack, Haul, Jack, Hang, Set };

struct FLineEndpoint {
    ELineSeat Seat;            // Static, Body, GCFragment, Kinematic, Plate
    FPhysicsObjectHandle Body; // null for Static
    FVector LocalAnchor;       // body or actor local space, cm
    uint8 PlateId;             // Plate only
};

struct FLineState {           // replicated, and stored in physics history for resim
    FLineEndpoint A, B;
    ELineMode Mode;
    float ModeStartTime;       // server physics time, s
    float L0;                  // length at Hang engage, cm
    float TensionT;            // current class tension, N (overspin raises it)
    float HangLoad;            // N
    float SlipAccum;           // cm extended by slipping
    float ReelAccum;           // cm of rest-length change from Reel (negative = reeled in)
    int8  ReelInput;           // -1 held Haul (reel in), +1 held Jack (reel out), 0 none; input, lives in the physics input history
    float AppliedForce;        // last tick, N, for render and audio
    float KickLaunchSpeedV;    // Jack's Kick, m/s (class value)
    uint8 Flags;               // bSet, bOverspin, bDoublerSplit, bSlackPastCap, bKickDone
};
```

### 3.2 Per-tick force resolution (physics thread)

Pseudocode for one line. `T` is the class pull tension in newtons (3.4: 400, 400, 600, 800, 1,000), reduced to 60 percent (480 N, 720 N at overspin) when a Doubler has two lines live (3.8), raised toward 1,200 N during overspin (3.7). `V` is the class Kick launch speed (18, 14, 12, 14, 10 m/s); the Kick is the capacitor discharge that opens every Jack and is separate from motor pull T (3.2). Several lines on one object add their forces as vectors; several Hang lines add their loads (6.2a). The Scrapper's 30 percent bonus (6.1) multiplies the Haul force and the post-Kick Jack push on Light objects and Medium objects under 100 kg, never V. After every step the solver clamps every simulated body it touched to the 50 m/s hard velocity cap (6.3).

```text
RESOLVE_LINE(line, dt, now):
  pA, pB  = world anchors of A and B
  d       = pB - pA ; L = |d| ; n = d / L
  vA, vB  = anchor velocities (zero for Static; actor velocity for Kinematic)
  closing = dot(vA - vB, n)          // positive when A and B approach

  switch line.Mode:

  case Haul:
    ramp = clamp((now - ModeStartTime) / 0.4, 0, 1)
    F = T * ramp
    if L <= 50 cm: F = 0; EMIT DogsMet                // 0.5 m
    if closing >= 2500 cm/s: F = 0; Flags |= SlackPastCap   // 25 m/s cap
    APPLY(A, +n * F) ; APPLY(B, -n * F)

  case Jack:
    if L >= ClassLineLength: RELEASE(line); return      // auto-release at max length
    separating = -closing
    if separating >= 3000 cm/s: F = 0; break            // 30 m/s separation cap
    wA = invMass(A) ; wB = invMass(B)                   // 0 for Static/Kinematic/braced
    // Kick exception: when the Kick fires, an unbraced grounded player uses 1/m here. The Kick
    // acceleration always exceeds foot-hold, so the character hands off to the flight body on this
    // step. Under the interlock (a player on either Dog) there is no Kick and the normal drag model
    // applies: a 600 N push on a Rigger is a skid, not a launch.
    if wA + wB == 0: F = 0; break                       // static-static, or braced vs static: sings, nothing moves
    t = now - ModeStartTime
    // KICK INTERLOCK (3.2): no Kick when either Dog sits in another player's Catch Plate.
    // A self-dog in the owner's own plate does not count, so a self-dog Jack to a wall still Kicks.
    bKickAllowed = !(A.Seat == Plate && A.Owner != line.Owner) && !(B.Seat == Plate && B.Owner != line.Owner)
    if t < 0.3 and !bKickDone and bKickAllowed:
        // THE KICK: velocity-target drive. Reach relative speed V by 0.3 s,
        // split by inverse mass. Implemented as a per-step velocity change with a force cap.
        v_target = V * clamp(t / 0.3, 0, 1)
        dv_needed = max(v_target - separating, 0)
        m_eff = 1 / (wA + wB)
        F_cap = 1.5 * m_eff * V / 0.3                  // enough to reach V with slack for catch-up
        F = min(dv_needed * m_eff / dt, F_cap)
        if separating >= V: bKickDone = true
    else:
        // PUSH PHASE: motor push at T until max length (Scrapper bonus applies here, not to V).
        // Under the interlock the push is all there is, ramped over 0.3 s like Haul:
        // 600 N on a 130 kg Rigger is a 4.6 m/s² skid, about two seconds to cross nine meters (Overboard).
        F = T * (bKickAllowed ? 1 : clamp(t / 0.3, 0, 1)) * ScrapperBonus(line, far object)
    APPLY(A, -n * F) ; APPLY(B, +n * F)
    // A Static far end has w = 0, so the free end receives all of V (self-dog Slingshot).
    // Two free ends split V by inverse mass: a 220 kg Breaker and a 90 kg Climber split 29:71.

  case Hang, Set:
    // rigid distance constraint, solved as XPBD with compliance 0 (rigid)
    // Hang never tests break-free (6.2): a placed object keeps its placement and carries the
    // static load up to the class Hang load. invMass for a placed object is therefore 0 here.
    // REEL (3.2): while in Hang (not Set), a held Haul input shortens the rest length at 1.0 m/s,
    // a held Jack input lengthens it at 1.0 m/s. A winch, not a pull: the constraint stays rigid
    // and bears the class Hang load the whole time.
    if Mode == Hang and ReelInput != 0:
        reelStep = ReelInput * 100 cm/s * dt
        ReelAccum = max(ReelAccum + reelStep, 50 cm - L0 - SlipAccum)   // never below the 0.5 m Dogs-meet length
        ReelAccum = min(ReelAccum, ClassLineLength - L0 - SlipAccum)     // never past class line length
    R = L0 + SlipAccum + ReelAccum                       // rest length
    C = L - R
    if C <= 0 and !bRodMode: return                     // rope: slack under compression
    wA = invMass(A) ; wB = invMass(B)                   // 0 for Static/Kinematic/braced
    if wA + wB == 0: return                             // static-static: tripwire, nothing to solve
    lambda = -C / (wA + wB)
    constraint force F = |lambda| / dt^2  (per step; smooth over 3 steps for the load reading)
    if F > HangLoad:
        SlipAccum += 50 cm * (dt / 0.1)                 // 0.5 m per 0.1 s over load
        if ReelInput < 0: ReelAccum -= reelStep         // reeling against an overload gains nothing
        EMIT Creak(F / HangLoad)
        recompute C, lambda
    POSITION_CORRECT(A, +n * lambda * wA) ; POSITION_CORRECT(B, -n * lambda * wB)
    VELOCITY_PROJECT(A, B, n)                           // remove approach/separation velocity along n
    line.AppliedForce = F

  line.AppliedForce = F   (Haul/Jack)
```

`APPLY` on a simulated body adds a force at the anchor point. On a kinematic character it does not touch the body; it writes `LineForceOnCharacter` into the character's drag input (section 3.4). On a Static or Kinematic endpoint it is discarded. On a braced player it is compared with the brace foot-hold first (05 section 5.4). The Kick on a grounded player is always above foot-hold (V / 0.3 s is 33 to 60 m/s²), so a Jacked player goes Airborne on the first Kick step; that is the Slingshot and Overboard.

Hang defaults to rope behaviour (slack under compression). The bible says "rigid rod". A two-sided rod is enabled per line by `bRodMode` for object-to-object bolas. Which default ships is a decision to confirm; see `README.md`.

### 3.3 Mass and what moves

The solver never splits force by mass. It applies equal and opposite force; mass does the split, as 05 section 3.3 requires. Special cases are implemented as inverse mass:

| Endpoint | invMass used by the solver |
|---|---|
| Static world | 0 |
| Kinematic actor | 0 |
| Grounded player below foot-hold | 0 (the drag model moves the character on the game thread instead). Exception: during a Kick (self-dog to a wall or object; never against another player's plate) an unbraced grounded player uses 1/m, because the Kick always exceeds foot-hold and the player becomes a flight body on the first step |
| Braced player below brace foot-hold | 0. A braced Breaker (4,400 N) is static to every line including the Kick, so an Overboard against a braced Breaker sends the Jacker instead |
| Rated load with line force under 1,000 N | 0 |
| Kinematic rated attachment (Lift counterweight, Gantry load) with line force under 1,000 N | 0, and it stays on its path. At 1,000 N or more it becomes a simulated rated load (6.7) |
| Gas manifold | 0 always (fixed world geometry). A Jack over 600 N on it vents the jet (6.4) |
| Object inside its break-free threshold, under active pull (Haul, Jack push, Kick) | 0 until the vector sum of active pull meets the threshold, then normal |
| Placed object on a Hang line | 0. Hang never tests break-free (6.2); the placement carries the static load up to the class Hang load, so a Pendulum from a racked cylinder or a bolted beam holds. A Hang to a loose object (break-free 0) brings it along as dead weight |
| Heavy object directly on a floor (friction 0.3 or more) | 0 always. Fixed scenery (6.1a). Movable Heavies are authored on rollers (0.02) or skids (0.05) and use 1/m with that friction |
| Everything else | 1 / mass |

Break-free uses "greater than or equal" (6.2: loose 0 N, rack 200 N, stack 500 N, bolted 800 N, rated 1,000 N). The 6.2 table lists the Doubler (800 N) as able to start a bolted object and the Breaker (1,000 N) a rated one, which only works with `>=`. Forces from several lines on one object add as vectors before the comparison (6.2a). Only active pull is compared: Haul force, the post-Kick Jack push and the Kick itself. Hang lines never test break-free. All thresholds are inclusive (v0.2.1).

### 3.4 Applying a line to a kinematic character (the drag model)

The character is a CMC capsule. Per physics tick the solver writes the line's force vector on that character into a thread-safe mailbox. On the game thread the character's custom movement mode `MOVE_Dragged` reads it:

```text
a_line = F / m_class
if a_line > FootHold(class, braced):      -> transition to Airborne (flight body)
else if dot(F, up) > 0.5 * |F| and feet unloaded: -> Airborne, Hoisted
else:
    Velocity += a_line * dt  (project out the ground normal)
    if |Velocity along n| > 25 m/s: clamp
    play skid; input acceleration is overridden, look is free
```

CMC predicts this on the owner because the force comes from the owner's predicted line state, and the server reconciles through the normal CMC move correction. The 25 m/s cap and the ramp make the predicted and server results converge even when the force arrived a few frames late.

Transition to the flight body: see `player-physics.md` section 3. Once the player is a flight body, the solver treats the endpoint as `Body` with invMass = 1/m.

### 3.5 Applying a line to a ragdoll

The networked body is the flight body (one capsule rigid body). The line attaches to the flight body at the plate's offset. The visual ragdoll is cosmetic and is pulled toward the flight body by Physics Control (production-ready in 5.8, [release notes](https://dev.epicgames.com/documentation/unreal-engine/unreal-engine-5-8-release-notes)). The line's rendered endpoint uses the ragdoll's plate socket so the visual line leaves the body at the right place; the solver uses the flight body. If the two diverge more than 25 cm, the visual snaps to the flight body.

### 3.6 Applying a line to a geometry collection fragment

Seat stores the GC component and the particle (bone) index. The solver resolves the current physics proxy for that bone each tick (bones re-parent on break). Force is applied to the cluster that currently owns the bone. On a break event (`notify_breaks`, see [GC component API](https://dev.epicgames.com/documentation/en-us/unreal-engine/python-api/class/GeometryCollectionComponent)) the Dog reseats in the largest fragment within 0.5 m of the old anchor, else the largest fragment of the old cluster (3.5).

### 3.7 Rigid Hang line collision for players

Hang and Set lines are solid to players and only players (3.5). Implementation:

- Each Hang line owns a `UKineticLineColliderComponent`: a capsule swept along the segment, radius 0.05 m, collision object type `KineticLine`, which blocks only the player capsule channel. Objects ignore it.
- Overlap at speed `v` along the line normal: `v > 3 m/s` staggers 0.6 s (1.0 s for a Rigger's line); `v > 6 m/s` flips. Flip hands the player to the flight body with velocity preserved and a 0.3 m vertical lift so they land on the far side.
- Crouching height 1.0 m and sliding height 0.7 m are handled by the capsule half-height, so no special case is needed: a line above the capsule top does not overlap.
- Lines never push players as rigid bodies. The line is a trigger plus a scripted response, which is what prevents the physics explosions the bible warns about.

## 4. Lifecycle

```text
Idle --fire A--> DogA in flight --seat--> Slack line (A seated, B pending, 4 s timer)
   --fire B--> B in flight --seat--> Live line in pre-selected mode (ramp starts)
Live --mode input (0.25 s class switch delay)--> Live in new mode (L0 captured if Hang)
Live Haul/Jack --release / new Dog A / cut--> Vanish
Live Hang --hold Haul input--> Reel in at 1.0 m/s ; --hold Jack input--> Reel out at 1.0 m/s (bears Hang load; slips above it)
Live Hang --release / new Dog A--> Set line (class set-line life, hum, dim)
Set --expire--> slack, 0.5 s fade recall
Set --owner recall input--> recall in the class recall time (Scrapper instant); the Guillotine drop
Any --cut--> Snap(tension at cut) ; both Dogs release
Any --a kinematic sweeps through the segment--> Snap(tension) with the 4.2 recoil ; both Dogs release (lines seated on that kinematic are exempt)
Any --owner scrapped--> release
```

- Recall: both Dogs return in the class recall time (Scrapper instant).
- Set lines: a player holds the class count; setting another retires the oldest. The owner can recall a set line early at any time (3.6); the recall input with no live line recalls the newest set line, and the HUD lists set lines so a specific one can be picked. Recall takes the class recall time and the Hang collider is disabled on the first frame.
- Reel input mapping: while a line is in Hang, holding the Haul or Jack mode input for longer than 0.2 s is Reel; a press shorter than that is a mode switch after the class switch delay. The threshold is a decision to confirm (`README.md`).
- Kinematic sever (3.5): the Kinematic Director runs a per-tick swept test on the server of every kinematic's collision shapes against every live and set line segment, four sub-samples per physics step (a 12 m/s Hauler moves 5 cm per sub-sample). A hit is a cut at the crossing point: snap, 4.2 recoil, both Dogs release, reliable multicast. Lines with a Dog seated in that kinematic are skipped. Clients do not predict this cut even though kinematic poses are deterministic, because the line endpoints are not.
- Two-line Doubler: `UKineticSpoolComponent` owns up to two `AKineticLine`. While both are live each has `TensionT = 0.6 * T`. Releasing one ramps the other back to `T` over 0.3 s.
- Overspin: holding the mode input past engagement ramps `TensionT` toward 1,200 N over 1.0 s. At 1.0 s the audio parameter `Overspin` triggers (octave jump) and the emissive flickers white. At 1.8 s: both Dogs release, the Doubler's flight body receives an impulse `m * 10 m/s` opposite the line, stagger 0.8 s, line unavailable 3 s. The far endpoint receives one impulse of `1500 N·s * (AppliedForce / 1200 N)` along the line (3.7): 1,500 N·s at full overspin, a Stall against a player; proportionally less if snapped early. On Hang, overspin raises the Hang load instead.
- Cut recoil: a normal cut applies `150 N·s * (AppliedForce / 1000 N)` to both endpoints as one impulse (4.2). At most 180 N·s, below the 300 N·s stagger threshold.

## 5. Replication

### 5.1 What replicates

| Field | Rate | To whom |
|---|---|---|
| `FLineState` without `AppliedForce` | On change, reliable | Everyone relevant |
| `AppliedForce` (quantized to 8 bits over 0 to 6 kN; pull forces stay under 1.2 kN, Kick forces reach 5.4 kN) | 30 Hz, 60 Hz for lines touching the viewer | Everyone relevant |
| Endpoint positions | Not replicated. Clients derive them from the replicated bodies and anchors | |
| `SlipAccum` | On change | Everyone relevant |
| `ReelAccum` (rest length change, quantized to 1 cm) | 30 Hz while reeling, on change otherwise | Everyone relevant. The owner predicts it from its own `ReelInput`, which travels in the networked physics input stream; it is part of the physics history so a rewind restores it |
| Kinematic sever events | Reliable multicast with the crossing point | Everyone relevant |
| Cut and snap events | Reliable multicast with cut point | Everyone relevant |

Lines are top priority for the player they are attached to (`networking.md` section 5).

### 5.2 Owner prediction

The owner spawns a predicted `AKineticLine` on Dog A seat and runs the solver locally in the resimulated physics world. The owner's flight body is in Resimulation mode so a Slingshot launches on the frame the input is pressed. If the server rejects the seat, the predicted line is destroyed and the real one (if any) replaces it within one net update. Line state is included in the physics history (`FKineticLineSolver` registers a custom state struct through the Network Physics component's data history API, as in Epic's pawn tutorial) so a rewind restores `ModeStartTime`, `L0` and `SlipAccum`.

### 5.3 Other players' lines

Shown from replicated state. Endpoints come from the interpolated proxies, so an enemy's line always lines up with the enemy's visible body. Force-driven effects (emissive, pitch) use the replicated `AppliedForce`. No client ever applies an enemy line's force to its own predicted body: the force on the victim arrives through the victim's own line solver because the victim's body is one endpoint and the line state is replicated to it at 60 Hz.

## 6. Rendering and audio

- **Live line geometry**: a Niagara ribbon with two user parameters (endpoints) and a float (`Force01`). Lines under force are straight, so a two-point ribbon with 8 segments is enough. Slack lines (between A and B, or set lines about to expire) switch to a Cable Component for the sag, which is cosmetic Verlet with no physics coupling ([docs](https://dev.epicgames.com/documentation/en-us/unreal-engine/cable-components-in-unreal-engine)).
- **Emissive**: material parameter `Emissive = lerp(0.15, 1.0, Force01)` where `Force01 = AppliedForce / 1200 N`, clamped to 1 (the Kick pins it at full for 0.3 s, which is the Kick tell). Set lines sit at 0.1 and ramp to 0.4 when a player is within 5 m (11). Overspin past 1.0 s pulses to white at 8 Hz. Team color locked (04).
- **Audio bridge**: each line owns an `AudioComponent` playing `MS_Line`. Each frame: `SetFloatParameter("Force", Force01)`, `SetIntParameter("Mode", mode)`, `SetFloatParameter("Length", L)`. Triggers: `Seat`, `Overspin`, `Creak`, `Snap(tension)`. The MetaSound quick start shows exactly this pattern with `Set Float Parameter` and `Execute Trigger Parameter` on a spawned sound ([quick start](https://dev.epicgames.com/documentation/en-us/unreal-engine/metasounds-quick-start)). Pitch is a function of `Force01` inside the MetaSound so audio and emissive share one input.
- **Edge indicator**: HUD reads the plate id and the line direction in camera space. It never draws the line.

## 7. Class sketch

```cpp
UCLASS() class UKineticSpoolComponent : public UActorComponent {
  GENERATED_BODY()
public:
  UPROPERTY(EditDefaultsOnly) FDataTableRowHandle ClassTuning;   // DT_HarnessClass row
  void FireDog();                 // A, then B, then release+A
  void SelfDog();
  void SelectMode(ELineMode);     // honours class switch delay
  void Release();
  void Recall();
  void BeginOverspin(); void EndOverspin();
protected:
  UPROPERTY(Replicated) TArray<TObjectPtr<AKineticLine>> ActiveLines;   // 1, or 2 for Doubler
  UPROPERTY(Replicated) TArray<TObjectPtr<AKineticLine>> SetLines;
  UFUNCTION(Server, Reliable) void ServerFireDog(FVector Origin, FVector Dir, uint32 ClientPhysFrame);
  UFUNCTION(Server, Reliable) void ServerSelectMode(ELineMode, uint32 ClientPhysFrame);
  UFUNCTION(Server, Reliable) void ServerRelease(uint32 ClientPhysFrame);
  float RecallTimer, ModeSwitchTimer, OverspinTime;
};

UCLASS() class AKineticDog : public AActor {
  GENERATED_BODY()
public:
  void Launch(FVector Origin, FVector Dir, float Speed, float MaxDistance);
  void StepFlight(float dt);     // sweep, gravity, seat
  FLineEndpoint SeatResult; bool bSeated;
  UPROPERTY(Replicated) uint8 DogIndex;   // A or B (or C, D)
};

UCLASS() class AKineticLine : public AActor {
  GENERATED_BODY()
public:
  UPROPERTY(ReplicatedUsing=OnRep_State) FLineState State;
  UPROPERTY(Replicated) uint8 AppliedForceQ;        // 0..255 -> 0..6 kN
  UPROPERTY() TObjectPtr<UNiagaraComponent> Ribbon;
  UPROPERTY() TObjectPtr<UCableComponent> SlackCable;
  UPROPERTY() TObjectPtr<UAudioComponent> Voice;
  UPROPERTY() TObjectPtr<UKineticLineColliderComponent> HangCollider;
  void Engage(ELineMode); void Release(); void Cut(FVector At, AActor* By);
  void SetAsSetLine(float Life);
  int32 SolverIndex;             // index into FKineticLineSolver::Lines on the physics thread
};

// Physics thread
class FKineticLineSolver : public Chaos::TSimCallbackObject<FLineSolverInput, FLineSolverOutput,
      Chaos::ESimCallbackOptions::Presimulate | Chaos::ESimCallbackOptions::Rewind> {
  TArray<FLineState> Lines;      // mirrored to history for resim
  void OnPreSimulate_Internal() override;   // RESOLVE_LINE for each live line
  void ResolveHaul(...), ResolveJack(...), ResolveHang(...);
  void ApplyToEndpoint(FLineEndpoint&, FVector Force);
};
```

## 8. Test matrix

| Test | Pass |
|---|---|
| Breaker Hauls a static anchor: reaches 25 m/s cap, line goes slack, no overshoot | Speed never exceeds 25.5 m/s |
| Breaker vs Climber Haul | 71:29 split within 1 percent on server and both clients |
| Jack self-dog to wall | Wall receives nothing; player reaches the class V within 0.3 s (Climber 18 m/s, range about 33 m at 45°), then is pushed at T to max length |
| Jack between two free bodies | Relative speed V at 0.3 s, split by inverse mass (Breaker 29 percent, Climber 71 percent) |
| Scrapper bonus | Haul on a 45 kg cylinder accelerates 30 percent faster; the Kick's V is unchanged; a 180 kg drum gets no bonus |
| Pendulum from a racked cylinder | Rack holds; cylinder never breaks free under Hang; a Haul on the same cylinder at 200 N or more breaks it free |
| Hard cap | No body on a line ever exceeds 50 m/s |
| Gas manifold | A Jack over 600 N vents; the manifold never moves |
| Hang over Hang load | Slips exactly 0.5 m per 0.1 s; creak fires at the first slip |
| Reel (Gallows by Reel) | Rigger Hangs a 220 kg Breaker from an overhead anchor and holds Haul: rest length shortens at 1.0 m/s, the Breaker rises at 1.0 m/s, load 2.2 kN under the 12 kN Hang load |
| Reel against overload | Rigger reels a 1,300 kg load (12.7 kN): no lift, slip at 0.5 m per 0.1 s, creak; releasing the input stops the slip once under load |
| Reel at 80 ms RTT | Owner's predicted rest length and the server's agree within 2 cm after 3 s of reeling |
| Kick interlock (Overboard) | Self-dog plus enemy plate, Jack: no Kick; 600 N push ramps over 0.3 s; a 130 kg Rigger skids at 4.6 m/s² and crosses 9 m in about 2 s; the line emissive never shows the Kick pin |
| Set line early recall | Guillotine: owner recalls the set line; the load drops under gravity; the Hang collider is off on the first frame |
| Kinematic sever | A Hauler crosses a trench-bridge Hang line: snap with 4.2 recoil, both Dogs release, all clients within one net update; a line Dogged to the Hauler itself survives |
| Set line expiry | Fade recall over 0.5 s; collider disabled at expiry start |
| Doubler two lines | Each at 480 N; release one, the other reaches 800 N in 0.3 s |
| Overspin snap | Far endpoint impulse 1,500 N·s × F/1,200 N; Doubler launched at 10 m/s |
| Normal cut recoil | 150 N·s per 1,000 N to both ends; never staggers |
| Demolition charge | Dog seats without a blast; a 6 m/s object or player impact triggers it |
| Cut at 150 ms RTT | Snap plays at the cut point on all clients within one net update |

## 9. Findings from the engine-free implementation (2026-10-24)

The solver in section 3.2 is implemented in [game/Source/KineticCore](../../game/README.md) and tested against bible/05's worked examples, then reviewed twice by an independent physics pass. The tests forced these changes to the design. The code has them, and the sections above should be read with them.

1. **Kick target at the end of the step.** `v_target = V * clamp(t / 0.3, 0, 1)` uses the time at the start of the step. On a fixed 60 Hz step the last Kick step then reaches only 94 percent of V, and the Kick window closes before V. The target is `V * clamp((t + dt) / 0.3, 0, 1)`, so V is reached exactly at 0.3 s.
2. **Active lines use the free inverse mass; thresholds are decided on the same step.** The design resolves each endpoint's inverse mass from last step's force, which is a chicken-and-egg loop: a placed object, a braced player or a skidding player has `w = 0`, so the Jack applies 0 N, so the force never reaches the threshold. A Jack could never start a racked cylinder, vent the manifold from a wall, or break a brace with the Kick, and Overboard applied no force. Now:
   - Haul and Jack forces are computed against each endpoint's free inverse mass. Only static, kinematic, fixed scenery and rated loads have 0.
   - Footing, brace and break-free are then decided from that force, before integration.
   - A Jack with both ends static still applies T (nothing moves, and a volatile reads the force).
   - The Kick still needs `wA + wB > 0`.
3. **Hang lines are one constraint solve, not per-line projections.** Section 3.2's per-line `C = L - R` projection, with the load read from the stretch, failed in five ways:
   - Two lines on one load did not share it (6.2a).
   - A swing read about half its load.
   - Jolt slip depended on the tick rate.
   - A slipping line could push.
   - A Hang line never acted on a grounded or braced player, so the Gallows, Anchor Drop and the Overboard counter did nothing.

   `HangSolver` replaces it:
   - It solves all Hang and Set lines and the floor together as one bounded direct solve. Lines at their Hang load and slack ropes are the bounds. An iterative (Gauss-Seidel) solve did not converge for chains of light and heavy bodies: seven 5 kg links under 1,000 kg stretched 30 m in 4 s.
   - Each line's impulse is capped at its Hang load per step (rope: tension only; rod: both ways).
   - The load a line reads is its solved impulse, which is m·g hanging and m·v²/L swinging. Loads add across lines.
   - The solve runs on velocities before positions are integrated. Each line's target is exact for its turn over the step, and a step in which a line turns more than 0.05 rad is split into substeps, so a swing keeps its speed on any line length (a 1 m Bola rod at 15 m/s included). Drift is removed by position passes, at most 5 cm per line per step, never by a teleport. Lines between the same two bodies share the load in proportion to their Hang loads. A line neither of whose ends can move (a kinematic dragging a braced player or a placed object) meets its full Hang load and slips, so the brace breaks next step (5.4) and a placed object, which Hang never starts (6.2), lets the line pay out until it runs out. Connected groups of bodies are solved separately, and where a line is parallel to the floor under a body (a line straight above a standing player) the floor carries the weight first.
   - A grounded player is a free body for the Hang solve, and the floor holds them up. A line that lifts more than their weight lifts them off their feet. A braced player is static unless the Hang force breaks the brace.
   - The Hang force feeds the endpoint's footing on the next step, the same way an active line's force does.
4. **Slip is a clutch whose friction is the Hang load** (PROPOSED). An overloaded line holds at its Hang load and pays the load out as fast as the load goes, never by pushing. A fixed rate of 0.5 m per 0.1 s cannot be honored either way: from rest it needs the line to push, and capping a falling load at 5 m/s needs an infinitely strong line (a 4 kN Climber line read 51 kN catching a container). So the bible's slip rate is not applied, and a load just over the Hang load creeps (500 kg on a 4 kN line falls at 1.8 m/s²). See production/open-items.md 7a.
   - A Reel moves the rest length at the Reel rate when either end can move, and slows at 1 m/s² over the last stretch before a bound, so a load up to the Hang load / (g + 1) stops there without slipping (PROPOSED). Reeling in against an overload gains nothing, and a reel-out never shortens the line.
   - The rest length stays between 0.5 m and the class line length. A line that slips past either lets go (PROPOSED: holding there made it infinitely strong, so a Climber line engaged near full length held 3,000 kg).
   - Hang captures L0 no shorter than the Dogs-meet length.
5. **The Kick counts toward break-free at T** (PROPOSED). bible/05 6.2 counts the Kick toward break-free, but 6.2 and 6.7 also say which classes can start a bolted object, a stacked container or a Lift counterweight, judged at T; at its full force a Climber's Kick started all three. Players' brace still sees the full Kick. Because the thresholds see T, whether a placed object holds is decided before the Kick is split, so a self-dog Kick off an object that holds gives the player the whole of V, as off a wall (3.2). The Scrapper's small-prop bonus is speed (6.1), so it is left out of the break-free force too: a Scrapper's 400 N does not start a stack.
6. **Haul ends when the Dogs meet**: the line goes Slack. Before, point bodies overshot and were pulled back.
7. **Set from any mode other than Hang captures L0.** Before, Set on a fresh line snapped to a stale rest length.
8. **Inclusive thresholds need a tolerance.** A pull built as `Axis × 1,000 N` measures 999.9999999 N on about a third of axes, so the Breaker's 1,000 N missed a rated load. Break-free and the rated commit compare with a relative tolerance of 1e-9.
9. **A kinematic attachment released from its path stays released** (6.7). It was falling back to static whenever the pull dropped.

Also found: bible/05 disagrees with itself on the slip rate. Section 3.2 says 0.5 m per 0.1 s (5 m/s); the tells table in section 11 says 0.3 s per 0.5 m (1.7 m/s). The tables keep section 3.2 until design settles it (production/open-items.md 7a).
