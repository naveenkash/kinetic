// The player's state machine (docs/tech/player-physics.md sections 2, 3, 5 to 8; bible/05
// sections 2, 5 and 7). It sits on top of the footing rules in MassRules.h and the impact
// classification in Impact.h, and decides which of the player's two bodies is in the
// simulation: the movement capsule (kinematic, CMC) or the flight body (a simulated rigid body).
//
// Each physics step the game feeds what happened to the player (line force, impacts, ground
// contact, inputs) and reads the state, what is allowed, and any body handoff to perform. The
// handoff is atomic in the step it is reported: copy the position and velocity across, disable
// the old body (player-physics 3).
//
// Choices the docs leave open (PROPOSED):
// - Look and the Cutter stay live in every state but Stalled, so the panic window survives a
//   stagger or a flip (7.1). The Spool is locked while Staggered or Flipped.
// - A stagger taken in the air keeps the player on the flight body; its timer runs, and
//   landing when it has run out is a normal landing.
// - A landing at or over the flip threshold staggers; the flip itself (the tumble) belongs to
//   hits in the air.
// - The flight body lands on a walkable surface: a ground contact whose normal is within 45°
//   of up (CMC's default walkable angle is about 44.8°), and only if the line and any force
//   volume would leave the player standing there. Ground contacts are ignored on the step of a
//   capsule-to-flight handoff and the step after, so the switch never bounces back.
// - Falls under the player's own power are judged on landing too.
// - One perch per time in the air: the perch is used until the next landing.
// - A ledge is grabbed falling (not rising), at under 8 m/s in all; the drop input lets go.
// - A walkable contact the player cannot stand on yet (a line still pulls them off their feet)
//   ends a flip, and staggers if hard, on the flight body; while staggered, only a contact at the
//   Flip speed restarts the stagger. Any walkable contact at the Stall speed Stalls, settling or
//   not, and the settle steps after a handoff let through contacts at the Flip speed (4.2).
// - On the capsule only a walkable contact is floor; a wall contact is not a landing.
#pragma once

#include "Kinetic/Api.h"

#include "Kinetic/Impact.h"
#include "Kinetic/MassRules.h"
#include "Kinetic/Tables.h"

#include <cstdint>
#include <optional>

namespace Kinetic
{
enum class EPlayerState : uint8_t { Grounded, Dragged, Airborne, Hoisted, Flipped, Staggered, Perched, LedgeHang, Stalled };
enum class EPlayerBody : uint8_t { Capsule, Flight };

KINETICCORE_API const char* ToString(EPlayerState State);

// Why a stagger happened, for its duration (7.1, 3.5, 3.7, 4.4).
enum class EStaggerCause : uint8_t { Impact, HangLineHit, OverspinRecoil, CutterMelee, FirePatch };

struct FPlayerHit
{
	EOutcome Outcome = EOutcome::None;
	EStaggerCause Cause = EStaggerCause::Impact;
	std::optional<EHarnessClass> LineOwner = std::nullopt;  // Hang line hits: the stagger is the owner's (3.5)
};

struct FGroundContact
{
	FVec3 Normal = Up;
	double ImpactSpeed = 0.0;  // m/s into the surface
	ESurface Surface = ESurface::Hard;
	bool bKinematic = false;   // A moving platform (the Lift, a Hauler): judged on the kinematic table
};

struct FPlayerStepInput
{
	FVec3 LineForce;                     // Total line force on the player this step, N
	bool bKickActive = false;
	double ForceVolumeAcceleration = 0.0;  // m/s², already scaled for mass (6.6)
	std::optional<FPlayerHit> Hit;       // From the Impact Monitor, the Hang-line hit test, blasts
	bool bStall = false;                 // Crush, ring-out, or anything else that Stalls outright
	// Walkable ground touched this step, by either body: on the capsule, send it with the step the
	// floor appears (without it, the fall is judged on Velocity.Z). Any walkable contact at the
	// Stall speed Stalls; the Impact Monitor need not report floor hits as well.
	std::optional<FGroundContact> Ground;
	bool bOnFloor = true;                // The capsule has floor under it (false: off a ledge)
	FVec3 Velocity;                      // The active body's velocity
	bool bMoveInput = false;
	bool bJumpInput = false;
	bool bCrouchHeld = false;
	bool bPerchInput = false;            // Climber: hold against the surface touched
	bool bTouchingWall = false;          // A non-walkable surface is in contact
	bool bLedgeInReach = false;          // A ledge within 0.5 m of the hands
	bool bDropInput = false;             // Let go of a ledge (crouch, in the default bindings)
};

struct FPlayerStepResult
{
	EPlayerState State = EPlayerState::Grounded;
	EPlayerBody Body = EPlayerBody::Capsule;
	bool bHandoff = false;   // The body changed this step: move position and velocity across
	bool bBraceBroken = false;
	bool bLanded = false;
};

class KINETICCORE_API FPlayerStateMachine
{
public:
	FPlayerStateMachine(EHarnessClass InClass, const FKineticTables& InTables);

	FPlayerStepResult Step(const FPlayerStepInput& Input, double Dt);

	EPlayerState State() const { return Current; }
	EPlayerBody Body() const { return BodyOf(Current, bStaggerOnFlightBody); }
	bool IsBraced() const { return bBraced; }
	bool IsGrounded() const;  // For the footing rules: feet on the floor

	// What the player may do now.
	bool CanMove() const;      // Locomotion under own power
	bool CanUseSpool() const;  // Fire Dogs, change mode, Reel
	bool CanUseCutter() const;
	bool CanLook() const { return Current != EPlayerState::Stalled; }

	// Firing a Dog while airborne: the share of horizontal speed kept (2.3).
	double AirborneFireSpeedKeep() const;
	double StaggerRemaining() const { return StaggerLeft; }
	double PerchRemaining() const { return PerchLeft; }

	static constexpr double BraceWindupSeconds = 0.3;
	static constexpr double PerchSeconds = 1.5;
	static constexpr double LedgeGrabMaxFallSpeed = 8.0;  // m/s (2.3)
	static constexpr double StaggerBaseSeconds = 0.6;        // 7.1
	static constexpr double CutterMeleeStaggerSeconds = 0.3;  // 4.4
	static constexpr double WalkableCosine = 0.70710678118654752;  // 45°
	static constexpr double DragEpsilonN = 1e-6;  // A line pull along the ground below this is no drag

private:
	static EPlayerBody BodyOf(EPlayerState State, bool bStaggerOnFlightBody);
	double StaggerSeconds(const FPlayerHit& Hit) const;
	void Enter(EPlayerState Next);
	void Stagger(const FPlayerHit& Hit, bool bOnFlightBody);
	bool UpdateBrace(const FPlayerStepInput& Input, double Dt);
	void UpdateLineFootingOnCapsule(const FPlayerStepInput& Input);
	void UpdateFlight(const FPlayerStepInput& Input, FPlayerStepResult& Out);
	void UpdateOwnPowerFall(const FPlayerStepInput& Input);
	void Land(const FGroundContact& Ground, bool bOnFlightBody, FPlayerStepResult& Out);
	EOutcome Judge(const FGroundContact& Ground) const;
	bool WouldStand(const FPlayerStepInput& Input) const;
	bool PulledOff(const FPlayerStepInput& Input) const;
	bool CanPerch(const FPlayerStepInput& Input) const;
	bool CanGrabLedge(const FPlayerStepInput& Input) const;

	EHarnessClass Class;
	const FKineticTables& Tables;
	const FHarnessTuning& Harness;
	EPlayerState Current = EPlayerState::Grounded;
	bool bStaggerOnFlightBody = false;
	bool bOnFloor = true;
	bool bPerchUsed = false;        // One perch per time in the air
	int SettleSteps = 0;            // Steps left in which ground contacts are ignored after a handoff
	EPlayerBody StepStartBody = EPlayerBody::Capsule;
	bool bBraced = false;
	double BraceHeld = 0.0;
	double StaggerLeft = 0.0;
	double PerchLeft = 0.0;
};

// The Stall sequence after the threshold is crossed (bible/05 7.4; player-physics 5): the
// harness locks, the gel sets in 0.2 s, the body is a Medium prop at the class mass for 4 s,
// then a Hook lifts it out over 2 s. A ring-out leaves no body; a body that went into the
// Anchor's Well is not retrieved and despawns when it falls below the volume.
enum class EStallStage : uint8_t { Locking, Prop, Lifting, Gone };

class KINETICCORE_API FStallSequence
{
public:
	FStallSequence(double StartTime, bool bRingOut, bool bIntoAnchorWell)
	    : Start(StartTime), bNoBody(bRingOut), bNoHook(bIntoAnchorWell)
	{
	}

	EStallStage StageAt(double Now) const;
	bool LinesReleased() const { return true; }  // At step 1, the instant of the Stall
	bool BodyMovableByLines(double Now) const { return StageAt(Now) == EStallStage::Prop; }
	void FellBelowWell() { bFellBelow = true; }  // The Anchor: no Hook; gone below the volume

	static constexpr double GelSeconds = 0.2;
	static constexpr double UnretrievedSeconds = 10.0;  // An Anchor body that never falls clear despawns (PROPOSED)
	static constexpr double HookArrivesSeconds = 4.0;
	static constexpr double HookLiftSeconds = 2.0;

private:
	double Start;
	bool bNoBody;
	bool bNoHook;
	bool bFellBelow = false;
};

// True when a player HeightM above the Well volume, at VerticalSpeed (m/s, positive down,
// negative while still rising), will reach it within LeadSeconds (0.5 s: the Hooks spin up
// audibly, 7.5). Falls are capped at the 50 m/s terminal speed (2.4).
KINETICCORE_API bool ShouldCueHooks(double HeightM, double VerticalSpeed, double Gravity, double TerminalSpeed = 50.0,
                    double LeadSeconds = 0.5);
} // namespace Kinetic
