#include "Kinetic/PlayerState.h"

#include <algorithm>
#include <cmath>

namespace Kinetic
{
namespace
{
constexpr double StationarySpeed = 0.1;  // m/s: slower than this counts as standing still

bool OnFlight(EPlayerState State)
{
	return State == EPlayerState::Airborne || State == EPlayerState::Hoisted || State == EPlayerState::Flipped ||
	       State == EPlayerState::Stalled;
}
} // namespace

const char* ToString(EPlayerState State)
{
	switch (State)
	{
	case EPlayerState::Grounded: return "grounded";
	case EPlayerState::Dragged: return "dragged";
	case EPlayerState::Airborne: return "airborne";
	case EPlayerState::Hoisted: return "hoisted";
	case EPlayerState::Flipped: return "flipped";
	case EPlayerState::Staggered: return "staggered";
	case EPlayerState::Perched: return "perched";
	case EPlayerState::LedgeHang: return "ledge_hang";
	case EPlayerState::Stalled: return "stalled";
	}
	return "unknown";
}

FPlayerStateMachine::FPlayerStateMachine(EHarnessClass InClass, const FKineticTables& InTables)
    : Class(InClass), Tables(InTables), Harness(InTables.Harness(InClass))
{
}

EPlayerBody FPlayerStateMachine::BodyOf(EPlayerState State, bool bStaggerOnFlightBody)
{
	if (State == EPlayerState::Staggered)
	{
		return bStaggerOnFlightBody ? EPlayerBody::Flight : EPlayerBody::Capsule;
	}
	return OnFlight(State) ? EPlayerBody::Flight : EPlayerBody::Capsule;
}

bool FPlayerStateMachine::IsGrounded() const
{
	const bool bCapsuleOnFloor = Current == EPlayerState::Grounded || Current == EPlayerState::Dragged ||
	                             (Current == EPlayerState::Staggered && !bStaggerOnFlightBody);
	return bCapsuleOnFloor && bOnFloor;
}

bool FPlayerStateMachine::CanMove() const { return Current == EPlayerState::Grounded; }

bool FPlayerStateMachine::CanUseSpool() const
{
	return Current != EPlayerState::Staggered && Current != EPlayerState::Flipped && Current != EPlayerState::Stalled;
}

bool FPlayerStateMachine::CanUseCutter() const { return Current != EPlayerState::Stalled; }

double FPlayerStateMachine::AirborneFireSpeedKeep() const
{
	const bool bInAir = Body() == EPlayerBody::Flight || (Current == EPlayerState::Grounded && !bOnFloor);
	return bInAir ? Harness.AirborneFireSpeedKeep : 1.0;
}

double FPlayerStateMachine::StaggerSeconds(const FPlayerHit& Hit) const
{
	switch (Hit.Cause)
	{
	case EStaggerCause::HangLineHit:
		return Hit.LineOwner ? Tables.Harness(*Hit.LineOwner).HangStaggerSeconds : StaggerBaseSeconds;
	case EStaggerCause::OverspinRecoil:
		return Tables.Globals().OverspinStagger;
	case EStaggerCause::CutterMelee:
		return CutterMeleeStaggerSeconds;
	case EStaggerCause::Impact:
	case EStaggerCause::FirePatch:
		return StaggerBaseSeconds;
	}
	return StaggerBaseSeconds;
}

void FPlayerStateMachine::Enter(EPlayerState Next)
{
	if (Next == EPlayerState::Perched)
	{
		PerchLeft = PerchSeconds;
		bPerchUsed = true;  // Until the next landing
	}
	if (Next != EPlayerState::Grounded && Next != EPlayerState::Dragged)
	{
		bBraced = false;
		BraceHeld = 0.0;
	}
	Current = Next;
}

void FPlayerStateMachine::Stagger(const FPlayerHit& Hit, bool bOnFlightBody)
{
	StaggerLeft = std::max(Current == EPlayerState::Staggered ? StaggerLeft : 0.0, StaggerSeconds(Hit));
	bStaggerOnFlightBody = bOnFlightBody;
	Enter(EPlayerState::Staggered);
}

bool FPlayerStateMachine::UpdateBrace(const FPlayerStepInput& Input, double Dt)
{
	const bool bCanHold = Harness.bCanBrace && (Current == EPlayerState::Grounded || Current == EPlayerState::Dragged) &&
	                      bOnFloor && Input.bCrouchHeld && !Input.bMoveInput && !Input.bJumpInput;
	const bool bStill = Input.Velocity.Length() < StationarySpeed;
	if (!bCanHold)
	{
		const bool bWasBraced = bBraced;
		bBraced = false;
		BraceHeld = 0.0;
		return bWasBraced;
	}
	if (!bBraced)
	{
		BraceHeld = bStill ? BraceHeld + Dt : 0.0;
		bBraced = BraceHeld + 1e-9 >= BraceWindupSeconds;
	}
	return false;
}

// The line and any force volume would leave the player standing on the ground.
bool FPlayerStateMachine::WouldStand(const FPlayerStepInput& Input) const
{
	const FPlayerLineResponse R = ResolvePlayerUnderLine(PlayerBodyFor(Harness, bBraced, true), Input.LineForce,
	                                                     Input.bKickActive, Tables.Globals().Gravity);
	const double HoldFootHold = bBraced ? Harness.BracedFootHold : Harness.FootHold;
	return R.Footing != EFooting::Airborne && R.Footing != EFooting::Hoisted &&
	       !(Input.ForceVolumeAcceleration > HoldFootHold);
}

// A line or a volume stronger than foot-hold takes the player off a perch or a ledge (8, 6.6).
bool FPlayerStateMachine::PulledOff(const FPlayerStepInput& Input) const
{
	return Input.LineForce.Length() / Harness.MassKg > Harness.FootHold ||
	       Input.ForceVolumeAcceleration > Harness.FootHold;
}

bool FPlayerStateMachine::CanPerch(const FPlayerStepInput& Input) const
{
	return Class == EHarnessClass::Climber && !bPerchUsed && Input.bPerchInput && Input.bTouchingWall &&
	       !PulledOff(Input);
}

bool FPlayerStateMachine::CanGrabLedge(const FPlayerStepInput& Input) const
{
	const bool bFalling = Input.Velocity.Z <= 0.0;
	return Input.bLedgeInReach && bFalling && Input.Velocity.Length() < LedgeGrabMaxFallSpeed && !PulledOff(Input);
}

// On the capsule: the line force decides between standing, the drag skid and the flight body.
void FPlayerStateMachine::UpdateLineFootingOnCapsule(const FPlayerStepInput& Input)
{
	const FPlayerLineResponse R = ResolvePlayerUnderLine(PlayerBodyFor(Harness, bBraced, true), Input.LineForce,
	                                                     Input.bKickActive, Tables.Globals().Gravity);
	if (R.bBraceBroken)
	{
		bBraced = false;
		BraceHeld = 0.0;
	}
	const double HoldFootHold = bBraced ? Harness.BracedFootHold : Harness.FootHold;
	const bool bPushedByVolume = Input.ForceVolumeAcceleration > HoldFootHold;
	if (bPushedByVolume && bBraced)
	{
		bBraced = false;  // A volume over the braced foot-hold breaks the brace
		BraceHeld = 0.0;
	}
	const bool bOff = R.Footing == EFooting::Airborne || R.Footing == EFooting::Hoisted || bPushedByVolume;
	if (Current == EPlayerState::Staggered)
	{
		bStaggerOnFlightBody = bOff;  // A line can take a staggered player off their feet too
		return;
	}
	const FVec3 AlongGround = Input.LineForce - Up * Input.LineForce.Dot(Up);
	if (R.Footing == EFooting::Hoisted)
	{
		Enter(EPlayerState::Hoisted);
	}
	else if (bOff)
	{
		Enter(EPlayerState::Airborne);
	}
	else if (R.Footing == EFooting::Skid && AlongGround.Length() > DragEpsilonN)
	{
		Enter(EPlayerState::Dragged);
	}
	else
	{
		Enter(EPlayerState::Grounded);
	}
}

EOutcome FPlayerStateMachine::Judge(const FGroundContact& Ground) const
{
	const EImpactTable Table = Ground.bKinematic ? EImpactTable::PlayerVsKinematic : EImpactTable::PlayerVsStatic;
	return Classify(Table, Ground.ImpactSpeed, Ground.Surface, Tables);
}

// Feet on the ground after a fall, on either body: judged on speed against the surface (2.4).
void FPlayerStateMachine::Land(const FGroundContact& Ground, bool bOnFlightBody, FPlayerStepResult& Out)
{
	const EOutcome Landing = Judge(Ground);
	Out.bLanded = true;
	bOnFloor = true;
	if (Landing == EOutcome::Stall)
	{
		Enter(EPlayerState::Stalled);
	}
	else if (Landing != EOutcome::None || Current == EPlayerState::Flipped)
	{
		Stagger({EOutcome::Stagger, EStaggerCause::Impact}, false);
	}
	else if (Current == EPlayerState::Staggered)
	{
		bStaggerOnFlightBody = false;  // Still staggered, now on the feet
	}
	else if (bOnFlightBody || Current != EPlayerState::Grounded)
	{
		Enter(EPlayerState::Grounded);
	}
}

void FPlayerStateMachine::UpdateFlight(const FPlayerStepInput& Input, FPlayerStepResult& Out)
{
	const double Gravity = Tables.Globals().Gravity;
	const bool bWalkable = Input.Ground && Input.Ground->Normal.SafeNormal().Dot(Up) >= WalkableCosine;
	const bool bSettling = StepStartBody == EPlayerBody::Capsule || SettleSteps > 0;
	if (bWalkable && !bSettling && WouldStand(Input))
	{
		Land(*Input.Ground, true, Out);
		return;
	}
	const EOutcome Contact = bWalkable ? Judge(*Input.Ground) : EOutcome::None;
	// While the handoff settles, only contacts at or over the Flip speed count (4.2).
	const bool bCounts = bWalkable && (bSettling ? Contact >= EOutcome::Flip
	                                             : Current == EPlayerState::Flipped || Contact != EOutcome::None);
	if (bCounts && (Current != EPlayerState::Staggered || Contact >= EOutcome::Flip))
	{
		// Ground the player cannot stand on yet (a line still pulls them): the tumble ends there,
		// and a hard contact staggers, on the flight body. A stagger already running is
		// restarted only by a contact at the Flip speed, not by each scrape that follows (4.2).
		Out.bLanded = true;
		Stagger({EOutcome::Stagger, EStaggerCause::Impact}, true);
		return;
	}
	if (Current == EPlayerState::Flipped || Current == EPlayerState::Staggered)
	{
		return;  // The tumble, or a stagger in the air: until landing
	}
	const double Upward = Input.LineForce.Dot(Up);
	const bool bLifted = Upward >= Harness.MassKg * Gravity && Upward > HoistUpwardShare * Input.LineForce.Length();
	if (bLifted)
	{
		Enter(EPlayerState::Hoisted);
		return;
	}
	if (Current == EPlayerState::Hoisted)
	{
		Enter(EPlayerState::Airborne);  // Released or cut
	}
	if (CanPerch(Input))
	{
		Enter(EPlayerState::Perched);
	}
	else if (CanGrabLedge(Input))
	{
		Enter(EPlayerState::LedgeHang);
	}
}

// Falling under own power on the capsule (CMC Falling): a line or volume makes it a flight;
// otherwise the player can perch or grab a ledge.
void FPlayerStateMachine::UpdateOwnPowerFall(const FPlayerStepInput& Input)
{
	const bool bLine = Input.LineForce.Length() > DragEpsilonN || Input.bKickActive;
	if (Current == EPlayerState::Dragged || bLine || Input.ForceVolumeAcceleration > Harness.FootHold)
	{
		Enter(EPlayerState::Airborne);  // Dragged off a ledge (5.1), or a line on a falling player
	}
	else if (CanPerch(Input))
	{
		Enter(EPlayerState::Perched);
	}
	else if (CanGrabLedge(Input))
	{
		Enter(EPlayerState::LedgeHang);
	}
}

FPlayerStepResult FPlayerStateMachine::Step(const FPlayerStepInput& Input, double Dt)
{
	FPlayerStepResult Out;
	const EPlayerBody Before = Body();
	StepStartBody = Before;
	const bool bWasBraced = bBraced;
	const bool bWasOnFloor = bOnFloor;
	if (Current == EPlayerState::Stalled)
	{
		Out.State = Current;
		Out.Body = Body();
		return Out;  // Terminal for the round
	}
	const bool bWalkable = Input.Ground && Input.Ground->Normal.SafeNormal().Dot(Up) >= WalkableCosine;
	if (Before == EPlayerBody::Capsule && Current != EPlayerState::Perched && Current != EPlayerState::LedgeHang)
	{
		bOnFloor = Input.bOnFloor || bWalkable;  // A wall contact is no floor
	}

	const bool bHitStalls = Input.Hit && Input.Hit->Outcome == EOutcome::Stall;
	// Any walkable contact at the Stall speed Stalls, on either body, in any state (2.4).
	const bool bGroundStalls = bWalkable && Judge(*Input.Ground) == EOutcome::Stall;
	if (Input.bStall || bHitStalls || bGroundStalls)
	{
		Out.bLanded = bGroundStalls;
		Enter(EPlayerState::Stalled);
	}
	else
	{
		const bool bHardHit = Input.Hit && Input.Hit->Outcome >= EOutcome::Stagger;
		UpdateBrace(Input, Dt);
		if (bHardHit)
		{
			bBraced = false;  // An impulse at or over the stagger threshold ends the brace (5.4)
			BraceHeld = 0.0;
		}
		if (Input.Hit && Input.Hit->Outcome == EOutcome::Flip)
		{
			Enter(EPlayerState::Flipped);
		}
		else if (Input.Hit && Input.Hit->Outcome == EOutcome::Stagger)
		{
			Stagger(*Input.Hit, Before == EPlayerBody::Flight || Current == EPlayerState::Perched ||
			                        Current == EPlayerState::LedgeHang || !bOnFloor);
		}
		else if (Current == EPlayerState::Staggered)
		{
			StaggerLeft -= Dt;
			if (StaggerLeft <= 1e-9)
			{
				Enter(bStaggerOnFlightBody ? EPlayerState::Airborne : EPlayerState::Grounded);
			}
		}

		const bool bLandedOnFeet = Before == EPlayerBody::Capsule && !bWasOnFloor && bOnFloor;
		// The floor can appear without a contact report: judge the fall on its vertical speed.
		const FGroundContact Touchdown =
		    bWalkable ? *Input.Ground : FGroundContact{Up, std::max(-Input.Velocity.Z, 0.0), ESurface::Hard};
		if (bLandedOnFeet && Current == EPlayerState::Staggered && !bStaggerOnFlightBody)
		{
			Land(Touchdown, false, Out);  // Staggered on the way down: still judged
		}
		else if (bLandedOnFeet && Current == EPlayerState::Flipped && Judge(Touchdown) == EOutcome::Stall)
		{
			Out.bLanded = true;
			Enter(EPlayerState::Stalled);  // Flipped in the step the fall ends: only a Stall cuts the tumble
		}
		switch (Current)
		{
		case EPlayerState::Grounded:
		case EPlayerState::Dragged:
			if (bLandedOnFeet)
			{
				Land(Touchdown, false, Out);  // A fall under own power is judged too
				if (Current != EPlayerState::Grounded)
				{
					break;
				}
			}
			if (!bOnFloor)
			{
				UpdateOwnPowerFall(Input);
				break;
			}
			UpdateLineFootingOnCapsule(Input);
			break;
		case EPlayerState::Staggered:
			if (bStaggerOnFlightBody)
			{
				UpdateFlight(Input, Out);
			}
			else if (!bOnFloor)
			{
				bStaggerOnFlightBody = true;  // Off the floor while staggered: the flight body
			}
			else
			{
				UpdateLineFootingOnCapsule(Input);
			}
			break;
		case EPlayerState::Perched:
		case EPlayerState::LedgeHang:
		{
			PerchLeft -= Current == EPlayerState::Perched ? Dt : 0.0;
			const bool bPerchOver = Current == EPlayerState::Perched && (PerchLeft <= 1e-9 || !Input.bPerchInput);
			const bool bLetGo = Current == EPlayerState::LedgeHang && Input.bDropInput;
			if (PulledOff(Input))
			{
				Enter(EPlayerState::Airborne);  // A line or volume over foot-hold breaks the hold
			}
			else if (bPerchOver || bLetGo || Input.bJumpInput)
			{
				bOnFloor = Current == EPlayerState::LedgeHang && Input.bJumpInput;  // Jumping climbs up
				Enter(EPlayerState::Grounded);
			}
			break;
		}
		case EPlayerState::Airborne:
		case EPlayerState::Hoisted:
		case EPlayerState::Flipped:
			UpdateFlight(Input, Out);
			break;
		case EPlayerState::Stalled:
			break;
		}
	}
	Out.State = Current;
	Out.Body = Body();
	Out.bHandoff = Out.Body != Before;
	Out.bBraceBroken = bWasBraced && !bBraced;
	if (IsGrounded())
	{
		bPerchUsed = false;  // Standing again, however they got there
	}
	if (Out.Body == EPlayerBody::Flight)
	{
		SettleSteps = Before == EPlayerBody::Capsule ? 1 : std::max(SettleSteps - 1, 0);
	}
	return Out;
}

EStallStage FStallSequence::StageAt(double Now) const
{
	const double T = Now - Start + 1e-9;  // A stage starts on its own instant
	if (bNoBody || bFellBelow || (bNoHook && T >= UnretrievedSeconds))
	{
		return T < 0.0 ? EStallStage::Locking : EStallStage::Gone;
	}
	if (T < GelSeconds)
	{
		return EStallStage::Locking;
	}
	if (bNoHook || T < HookArrivesSeconds)
	{
		return EStallStage::Prop;
	}
	return T < HookArrivesSeconds + HookLiftSeconds ? EStallStage::Lifting : EStallStage::Gone;
}

bool ShouldCueHooks(double HeightM, double VerticalSpeed, double Gravity, double TerminalSpeed, double LeadSeconds)
{
	if (HeightM <= 0.0)
	{
		return true;
	}
	// Distance covered in LeadSeconds, accelerating at g (from rising, if still rising) until
	// terminal speed.
	const double V0 = std::min(VerticalSpeed, TerminalSpeed);
	const double ToTerminal = std::min((TerminalSpeed - V0) / Gravity, LeadSeconds);
	const double Accelerating = V0 * ToTerminal + 0.5 * Gravity * ToTerminal * ToTerminal;
	const double Cruising = (LeadSeconds - ToTerminal) * TerminalSpeed;
	return HeightM <= Accelerating + Cruising;
}
} // namespace Kinetic
