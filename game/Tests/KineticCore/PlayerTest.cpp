// The player state machine and the Stall sequence (player-physics sections 2, 3, 5 to 8;
// bible/05 sections 2, 5 and 7) and the Phase 0 week 3 test: a dragged player taken off a ledge.
#include "Kinetic/PlayerState.h"
#include "SimFixtures.h"

#include <cmath>
#include <vector>

using namespace Kinetic;
using namespace Kinetic::Test;

namespace
{
constexpr double G = 9.81;

FPlayerStepInput Standing() { return FPlayerStepInput{}; }

FPlayerStepInput Pulled(const FVec3& Force)
{
	FPlayerStepInput In;
	In.LineForce = Force;
	return In;
}

FPlayerStepInput Flying(const FVec3& Velocity = {0, 0, -3})
{
	FPlayerStepInput In;
	In.bOnFloor = false;
	In.Velocity = Velocity;
	return In;
}

FPlayerStepInput Landing(double Speed, ESurface Surface = ESurface::Hard, const FVec3& Normal = Up)
{
	FPlayerStepInput In = Flying({0, 0, -Speed});
	In.Ground = FGroundContact{Normal, Speed, Surface};
	return In;
}

FPlayerStepInput Hit(EOutcome Outcome, EStaggerCause Cause = EStaggerCause::Impact,
                     std::optional<EHarnessClass> LineOwner = std::nullopt)
{
	FPlayerStepInput In;
	In.Hit = FPlayerHit{Outcome, Cause, LineOwner};
	return In;
}

FPlayerStateMachine Airborne(EHarnessClass Class)
{
	FPlayerStateMachine Player(Class, Tables());
	Player.Step(Pulled({1500, 0, 0}), StepSeconds);
	Player.Step(Flying(), StepSeconds);
	return Player;
}

double StaggerBase() { return FPlayerStateMachine::StaggerBaseSeconds; }

// Runs the machine on the same input for Seconds.
void Hold(FPlayerStateMachine& Player, const FPlayerStepInput& In, double Seconds)
{
	for (int I = 0; I < static_cast<int>(Seconds / StepSeconds + 0.5); ++I)
	{
		Player.Step(In, StepSeconds);
	}
}
} // namespace

// 5.3's examples: a Breaker's 1,000 N takes every unbraced class off its feet but a Breaker; a
// Scrapper's 400 N takes only a Climber; the switch to the flight body is a handoff.
KTEST(FootHoldDecidesSkidOrFlight)
{
	for (const EHarnessClass Class : {EHarnessClass::Climber, EHarnessClass::Scrapper, EHarnessClass::Rigger,
	                                  EHarnessClass::Doubler, EHarnessClass::Breaker})
	{
		FPlayerStateMachine Player(Class, Tables());
		const FPlayerStepResult Breaker = Player.Step(Pulled({1000, 0, 0}), StepSeconds);
		const bool bBreaker = Class == EHarnessClass::Breaker;
		KCHECK(Breaker.State == (bBreaker ? EPlayerState::Dragged : EPlayerState::Airborne));
		KCHECK(Breaker.bHandoff == !bBreaker && Breaker.Body == (bBreaker ? EPlayerBody::Capsule : EPlayerBody::Flight));

		FPlayerStateMachine Other(Class, Tables());
		const bool bClimber = Class == EHarnessClass::Climber;
		KCHECK(Other.Step(Pulled({400, 0, 0}), StepSeconds).State ==
		       (bClimber ? EPlayerState::Airborne : EPlayerState::Dragged));
	}
	FPlayerStateMachine Dragged(EHarnessClass::Rigger, Tables());
	Dragged.Step(Pulled({400, 0, 0}), StepSeconds);
	KCHECK(Dragged.Step(Standing(), StepSeconds).State == EPlayerState::Grounded);  // The force stops
	KCHECK(Dragged.CanUseSpool() && Dragged.CanMove());
}

// 5.4: crouch held still for 0.3 s braces a Rigger or a Breaker. A braced Rigger holds a
// Breaker's 1,000 N but not a Doubler's 1,200 N; moving, jumping or a hard hit ends it.
KTEST(BraceTakesAWindupAndEnds)
{
	FPlayerStepInput Crouch;
	Crouch.bCrouchHeld = true;
	FPlayerStateMachine Rigger(EHarnessClass::Rigger, Tables());
	Hold(Rigger, Crouch, 0.25);
	KCHECK(!Rigger.IsBraced());
	Hold(Rigger, Crouch, 0.07);
	KCHECK(Rigger.IsBraced());
	FPlayerStepInput Hauled = Crouch;
	Hauled.LineForce = {1000, 0, 0};
	KCHECK(Rigger.Step(Hauled, StepSeconds).State == EPlayerState::Grounded && Rigger.IsBraced());
	Hauled.LineForce = {1200, 0, 0};
	const FPlayerStepResult Overspun = Rigger.Step(Hauled, StepSeconds);
	KCHECK(Overspun.State == EPlayerState::Airborne && !Rigger.IsBraced());

	for (int Way = 0; Way < 3; ++Way)
	{
		FPlayerStateMachine Breaker(EHarnessClass::Breaker, Tables());
		Hold(Breaker, Crouch, 0.35);
		KCHECK(Breaker.IsBraced());
		FPlayerStepInput End = Crouch;
		End.bMoveInput = Way == 0;
		End.bJumpInput = Way == 1;
		if (Way == 2)
		{
			End.Hit = FPlayerHit{EOutcome::Stagger};
		}
		const FPlayerStepResult R = Breaker.Step(End, StepSeconds);
		KCHECK(!Breaker.IsBraced() && R.bBraceBroken);
	}
	FPlayerStateMachine Moving(EHarnessClass::Breaker, Tables());
	FPlayerStepInput Sliding = Crouch;
	Sliding.Velocity = {1, 0, 0};  // Crouched but not still
	Hold(Moving, Sliding, 1.0);
	KCHECK(!Moving.IsBraced());
	FPlayerStateMachine Climber(EHarnessClass::Climber, Tables());
	Hold(Climber, Crouch, 1.0);
	KCHECK(!Climber.IsBraced());  // Only Riggers and Breakers brace
}

// 7.1, 3.5, 3.7, 4.4: stagger times. The Spool is locked; look and the Cutter stay live.
KTEST(StaggersLockTheSpoolForTheirTime)
{
	struct FCase
	{
		FPlayerStepInput In;
		double Seconds;
	};
	const std::vector<FCase> Cases = {
	    {Hit(EOutcome::Stagger), 0.6},
	    {Hit(EOutcome::Stagger, EStaggerCause::HangLineHit, EHarnessClass::Rigger), 1.0},
	    {Hit(EOutcome::Stagger, EStaggerCause::HangLineHit, EHarnessClass::Breaker), 0.6},
	    {Hit(EOutcome::Stagger, EStaggerCause::OverspinRecoil), 0.8},
	    {Hit(EOutcome::Stagger, EStaggerCause::CutterMelee), 0.3},
	};
	for (const FCase& Case : Cases)
	{
		FPlayerStateMachine Player(EHarnessClass::Scrapper, Tables());
		KCHECK(Player.Step(Case.In, StepSeconds).State == EPlayerState::Staggered);
		KCHECK(!Player.CanUseSpool() && Player.CanUseCutter() && Player.CanLook() && !Player.CanMove());
		KCHECK(Player.Body() == EPlayerBody::Capsule);
		Hold(Player, Standing(), Case.Seconds - 2.0 * StepSeconds);
		KCHECK(Player.State() == EPlayerState::Staggered);
		Hold(Player, Standing(), 2.0 * StepSeconds);
		KCHECK(Player.State() == EPlayerState::Grounded && Player.CanUseSpool());
	}
}

// A flip is a tumble on the flight body until landing, then a stagger.
KTEST(AFlipTumblesUntilItLands)
{
	FPlayerStateMachine Player(EHarnessClass::Rigger, Tables());
	const FPlayerStepResult Flipped = Player.Step(Hit(EOutcome::Flip), StepSeconds);
	KCHECK(Flipped.State == EPlayerState::Flipped && Flipped.bHandoff && Flipped.Body == EPlayerBody::Flight);
	KCHECK(!Player.CanUseSpool() && Player.CanUseCutter());
	Hold(Player, Flying(), 1.0);
	KCHECK(Player.State() == EPlayerState::Flipped);
	const FPlayerStepResult Down = Player.Step(Landing(2.0), StepSeconds);
	KCHECK(Down.State == EPlayerState::Staggered && Down.bHandoff && Down.Body == EPlayerBody::Capsule && Down.bLanded);
}

// 5.1 and 7.1: landing under the stagger speed (5 m/s on hard ground, more on soft) gives control
// back; at or over it, a stagger; at 12 m/s, a Stall. Only walkable ground is a landing.
KTEST(LandingsAreJudgedOnSpeedAndSurface)
{
	FPlayerStateMachine Soft = Airborne(EHarnessClass::Climber);
	const FPlayerStepResult Back = Soft.Step(Landing(4.9), StepSeconds);
	KCHECK(Back.State == EPlayerState::Grounded && Back.bHandoff && Back.bLanded);

	FPlayerStateMachine Hard = Airborne(EHarnessClass::Climber);
	KCHECK(Hard.Step(Landing(5.0), StepSeconds).State == EPlayerState::Staggered);
	FPlayerStateMachine Wood = Airborne(EHarnessClass::Climber);
	KCHECK(Wood.Step(Landing(5.9, ESurface::Wood), StepSeconds).State == EPlayerState::Grounded);
	FPlayerStateMachine Flip = Airborne(EHarnessClass::Climber);
	KCHECK(Flip.Step(Landing(9.0), StepSeconds).State == EPlayerState::Staggered);
	FPlayerStateMachine Fall = Airborne(EHarnessClass::Climber);
	KCHECK(Fall.Step(Landing(12.0), StepSeconds).State == EPlayerState::Stalled);
	FPlayerStateMachine Wall = Airborne(EHarnessClass::Climber);
	const FVec3 Steep{std::sin(1.0), 0, std::cos(1.0)};  // 57° from up: not walkable
	KCHECK(Wall.Step(Landing(2.0, ESurface::Hard, Steep), StepSeconds).State == EPlayerState::Airborne);
}

// 5.2: a line that lifts more than the player's weight, mostly upward, hoists them; release drops them.
KTEST(HoistedWhileTheLineLifts)
{
	FPlayerStateMachine Climber(EHarnessClass::Climber, Tables());
	KCHECK(Climber.Step(Pulled({0, 0, 800}), StepSeconds).State == EPlayerState::Grounded);  // Under 883 N: no drag
	const FPlayerStepResult Lifted = Climber.Step(Pulled({0, 0, 1000}), StepSeconds);
	KCHECK(Lifted.State == EPlayerState::Hoisted && Lifted.Body == EPlayerBody::Flight && Climber.CanUseSpool());
	KCHECK(!Climber.CanMove() && Climber.CanUseCutter());
	FPlayerStepInput Held = Flying({0, 0, 0});
	Held.LineForce = {0, 0, 900};
	KCHECK(Climber.Step(Held, StepSeconds).State == EPlayerState::Hoisted);
	KCHECK(Climber.Step(Flying(), StepSeconds).State == EPlayerState::Airborne);  // Released
}

// The Kick always launches an unbraced grounded player; a strong force volume lifts them too.
KTEST(KicksAndForceVolumesLaunch)
{
	FPlayerStateMachine Kicked(EHarnessClass::Breaker, Tables());
	FPlayerStepInput Kick = Pulled({10, 0, 0});
	Kick.bKickActive = true;
	KCHECK(Kicked.Step(Kick, StepSeconds).State == EPlayerState::Airborne);

	FPlayerStateMachine Volume(EHarnessClass::Rigger, Tables());
	FPlayerStepInput Push = Standing();
	Push.ForceVolumeAcceleration = 4.9;
	KCHECK(Volume.Step(Push, StepSeconds).State == EPlayerState::Grounded);
	Push.ForceVolumeAcceleration = 5.1;
	KCHECK(Volume.Step(Push, StepSeconds).State == EPlayerState::Airborne);
}

// 2.3 and player-physics 7: a Climber perches on a wall for up to 1.5 s, not under a line over
// their foot-hold; nobody else perches.
KTEST(ClimbersPerchForASecondAndAHalf)
{
	FPlayerStateMachine Climber = Airborne(EHarnessClass::Climber);
	FPlayerStepInput Wall = Flying();
	Wall.bTouchingWall = true;
	Wall.bPerchInput = true;
	const FPlayerStepResult On = Climber.Step(Wall, StepSeconds);
	KCHECK(On.State == EPlayerState::Perched && On.Body == EPlayerBody::Capsule && On.bHandoff);
	KCHECK(Climber.CanUseSpool() && !Climber.CanMove());
	Hold(Climber, Wall, 1.5 - 2.0 * StepSeconds);
	KCHECK(Climber.State() == EPlayerState::Perched);
	Hold(Climber, Wall, 2.0 * StepSeconds);
	KCHECK(Climber.State() == EPlayerState::Grounded && !Climber.IsGrounded());  // Dropped: falling

	FPlayerStateMachine Let = Airborne(EHarnessClass::Climber);
	Let.Step(Wall, StepSeconds);
	FPlayerStepInput Released = Wall;
	Released.bPerchInput = false;
	KCHECK(Let.Step(Released, StepSeconds).State == EPlayerState::Grounded);

	FPlayerStateMachine Pulled = Airborne(EHarnessClass::Climber);
	FPlayerStepInput Dragging = Wall;
	Dragging.LineForce = {400, 0, 0};  // 4.4 m/s², over a Climber's 3
	KCHECK(Pulled.Step(Dragging, StepSeconds).State == EPlayerState::Airborne);
	FPlayerStateMachine Perched = Airborne(EHarnessClass::Climber);
	Perched.Step(Wall, StepSeconds);
	KCHECK(Perched.Step(Dragging, StepSeconds).State == EPlayerState::Airborne);  // Pulled off

	FPlayerStateMachine Rigger = Airborne(EHarnessClass::Rigger);
	KCHECK(Rigger.Step(Wall, StepSeconds).State == EPlayerState::Airborne);
}

// 2.3: a ledge within reach is grabbed falling under 8 m/s, not faster; a line over foot-hold
// breaks the grab; jumping climbs up.
KTEST(LedgesAreGrabbedUnderEightMetersPerSecond)
{
	FPlayerStepInput Ledge = Flying({0, 0, -7.9});
	Ledge.bLedgeInReach = true;
	FPlayerStateMachine Slow = Airborne(EHarnessClass::Breaker);
	KCHECK(Slow.Step(Ledge, StepSeconds).State == EPlayerState::LedgeHang);
	FPlayerStepInput Yank = Ledge;
	Yank.LineForce = {1200, 0, 0};
	KCHECK(Slow.Step(Yank, StepSeconds).State == EPlayerState::Airborne);

	FPlayerStateMachine Fast = Airborne(EHarnessClass::Breaker);
	FPlayerStepInput TooFast = Ledge;
	TooFast.Velocity = {0, 0, -8.1};
	KCHECK(Fast.Step(TooFast, StepSeconds).State == EPlayerState::Airborne);

	FPlayerStateMachine Climb = Airborne(EHarnessClass::Rigger);
	Climb.Step(Ledge, StepSeconds);
	FPlayerStepInput Jump = Ledge;
	Jump.bJumpInput = true;
	KCHECK(Climb.Step(Jump, StepSeconds).State == EPlayerState::Grounded && Climb.IsGrounded());

	// Falling under own power (off a ledge, no line) can grab too.
	FPlayerStateMachine Walker(EHarnessClass::Climber, Tables());
	FPlayerStepInput Walked = Ledge;
	KCHECK(Walker.Step(Walked, StepSeconds).State == EPlayerState::LedgeHang);
}

// 2.3: each airborne Dog fire keeps 85% of horizontal speed, a Climber all of it.
KTEST(AirborneFireKeepsMostSpeed)
{
	KCHECK(Airborne(EHarnessClass::Climber).AirborneFireSpeedKeep() == 1.0);
	KCHECK_NEAR(Airborne(EHarnessClass::Rigger).AirborneFireSpeedKeep(), 0.85, 1e-12);
	KCHECK(FPlayerStateMachine(EHarnessClass::Rigger, Tables()).AirborneFireSpeedKeep() == 1.0);
}

// Any Stall threshold from any state is terminal for the round.
KTEST(AStallIsTerminal)
{
	FPlayerStateMachine Player(EHarnessClass::Doubler, Tables());
	const FPlayerStepResult Stalled = Player.Step(Hit(EOutcome::Stall), StepSeconds);
	KCHECK(Stalled.State == EPlayerState::Stalled && Stalled.Body == EPlayerBody::Flight && Stalled.bHandoff);
	KCHECK(!Player.CanLook() && !Player.CanUseCutter() && !Player.CanUseSpool());
	KCHECK(Player.Step(Landing(1.0), StepSeconds).State == EPlayerState::Stalled);
	KCHECK(!Player.Step(Landing(1.0), StepSeconds).bHandoff);
	FPlayerStateMachine Crushed = Airborne(EHarnessClass::Breaker);
	FPlayerStepInput Crush = Flying();
	Crush.bStall = true;
	KCHECK(Crushed.Step(Crush, StepSeconds).State == EPlayerState::Stalled);
}

// 7.4: lock, 0.2 s of gel, a prop for 4 s that lines can move, then the Hook lift over 2 s. A
// ring-out leaves no body; the Anchor's Well keeps it until it falls out of the volume.
KTEST(TheStallSequenceRunsOnTime)
{
	const FStallSequence Body(10.0, false, false);
	KCHECK(Body.StageAt(10.1) == EStallStage::Locking && Body.LinesReleased());
	KCHECK(Body.StageAt(10.2) == EStallStage::Prop && Body.BodyMovableByLines(13.9));
	KCHECK(Body.StageAt(14.0) == EStallStage::Lifting && !Body.BodyMovableByLines(14.0));
	KCHECK(Body.StageAt(15.99) == EStallStage::Lifting && Body.StageAt(16.0) == EStallStage::Gone);

	KCHECK(FStallSequence(10.0, true, false).StageAt(10.0) == EStallStage::Gone);
	FStallSequence Well(10.0, false, true);
	KCHECK(Well.StageAt(19.9) == EStallStage::Prop);  // No Hook comes...
	KCHECK(Well.StageAt(20.0) == EStallStage::Gone);  // ...and a body that never falls clear despawns at 10 s
	FStallSequence Fell(10.0, false, true);
	Fell.FellBelowWell();
	KCHECK(Fell.StageAt(12.0) == EStallStage::Gone);
}

// 7.5: the Hooks spin up 0.5 s before a falling player reaches the Well volume.
KTEST(HooksCueHalfASecondBeforeTheWell)
{
	const double FromRest = 0.5 * G * 0.25;  // 1.226 m
	KCHECK(ShouldCueHooks(FromRest - 0.01, 0.0, G) && !ShouldCueHooks(FromRest + 0.01, 0.0, G));
	KCHECK(ShouldCueHooks(24.99, 50.0, G) && !ShouldCueHooks(25.01, 50.0, G));  // At terminal speed
	KCHECK(ShouldCueHooks(0.0, 0.0, G));
	const double Near = 49.0 * 0.0102 + 0.5 * G * 0.0102 * 0.0102 + (0.5 - 0.0102) * 50.0;  // Reaches 50 m/s on the way
	KCHECK(ShouldCueHooks(Near - 0.01, 49.9, G) && !ShouldCueHooks(Near + 0.5, 49.9, G));
}

// Phase 0 week 3: a Scrapper's 400 N Haul drags a Rigger in a skid to the ledge; off the edge
// the Rigger leaves the capsule for the flight body in one step, without a jump in position or
// speed, and falls.
KTEST(ADraggedPlayerLeavesTheLedgeWithoutAPop)
{
	FReferenceSim Sim(Tables(), StepSeconds);
	const int Anchor = Sim.AddBody(StaticAt({20, 0, -6}));
	FSimBody Rigger = PlayerAt({6, 0, 0}, EHarnessClass::Rigger, true);
	Rigger.FloorEndX = 10.0;
	const int Victim = Sim.AddBody(Rigger);
	Sim.Engage(Sim.AddLine(LineOf(EHarnessClass::Scrapper, Anchor, ESeat::OwnPlate, Victim, ESeat::OtherPlate)),
	           ELineMode::Haul);
	FPlayerStateMachine Player(EHarnessClass::Rigger, Tables());
	std::vector<EPlayerState> States;
	int Handoffs = 0;
	int HandoffStep = -1;
	int LeftFloorStep = -1;
	FVec3 LastPosition = Sim.Body(Victim).Position;
	FVec3 LastVelocity = Sim.Body(Victim).Velocity;
	double WorstJump = 0.0;
	double WorstKick = 0.0;
	for (int I = 0; I < 240; ++I)
	{
		Sim.Step();
		FPlayerStepInput In;
		In.LineForce = Sim.LineForceOn(Victim);
		In.bOnFloor = Sim.HasFloorUnder(Victim);
		In.Velocity = Sim.Body(Victim).Velocity;
		const FPlayerStepResult R = Player.Step(In, StepSeconds);
		Handoffs += R.bHandoff ? 1 : 0;
		if (R.bHandoff)
		{
			HandoffStep = I;
		}
		if (!In.bOnFloor && LeftFloorStep < 0)
		{
			LeftFloorStep = I;
		}
		const bool bRamping = States.empty() && R.State == EPlayerState::Grounded;  // The Haul ramps from 0
		if (!bRamping && (States.empty() || States.back() != R.State))
		{
			States.push_back(R.State);
		}
		const FVec3 Position = Sim.Body(Victim).Position;
		const FVec3 Velocity = Sim.Body(Victim).Velocity;
		// Movement beyond what the step's speed explains is a pop.
		WorstJump = std::max(WorstJump, (Position - LastPosition).Length() - Velocity.Length() * StepSeconds);
		WorstKick = std::max(WorstKick, (Velocity - LastVelocity).Length() - (400.0 / 130.0 + G) * StepSeconds);
		LastPosition = Position;
		LastVelocity = Velocity;
	}
	KCHECK(States.size() >= 2 && States[0] == EPlayerState::Dragged && States[1] == EPlayerState::Airborne);
	KCHECK(Handoffs == 1);
	KCHECK(HandoffStep == LeftFloorStep && HandoffStep > 0);  // The switch is on the step the floor ends
	KCHECK(WorstJump < 1e-6 && WorstKick < 1e-6);
	KCHECK(Sim.Body(Victim).Position.X > 10.0 && Sim.Body(Victim).Position.Z < -1.0);  // Over the edge, falling
}

// H1: a fall under the player's own power is judged on landing (2.4): 12 m/s Stalls, a perch
// drop too.
KTEST(OwnPowerFallsAreJudged)
{
	FPlayerStateMachine Walker(EHarnessClass::Rigger, Tables());
	Walker.Step(Flying({0, 0, -1}), StepSeconds);  // Stepped off a ledge: CMC Falling
	KCHECK(Walker.State() == EPlayerState::Grounded && !Walker.IsGrounded());
	FPlayerStepInput Down = Landing(14.0);
	Down.bOnFloor = true;
	const FPlayerStepResult R = Walker.Step(Down, StepSeconds);
	KCHECK(R.State == EPlayerState::Stalled && R.bLanded);

	FPlayerStateMachine Soft(EHarnessClass::Rigger, Tables());
	Soft.Step(Flying({0, 0, -1}), StepSeconds);
	FPlayerStepInput Gentle = Landing(6.0);
	Gentle.bOnFloor = true;
	KCHECK(Soft.Step(Gentle, StepSeconds).State == EPlayerState::Staggered);
	FPlayerStateMachine Easy(EHarnessClass::Rigger, Tables());
	Easy.Step(Flying({0, 0, -1}), StepSeconds);
	FPlayerStepInput Light = Landing(3.0);
	Light.bOnFloor = true;
	const FPlayerStepResult Fine = Easy.Step(Light, StepSeconds);
	KCHECK(Fine.State == EPlayerState::Grounded && Fine.bLanded && Easy.IsGrounded() && !Fine.bHandoff);

	// M5: a moving platform is judged on the kinematic table (9 m/s Stalls).
	FPlayerStateMachine Lift = Airborne(EHarnessClass::Rigger);
	FPlayerStepInput OnLift = Landing(9.5);
	OnLift.Ground->bKinematic = true;
	KCHECK(Lift.Step(OnLift, StepSeconds).State == EPlayerState::Stalled);
}

// H2: one perch per time in the air; landing gives it back.
KTEST(OnePerchPerFlight)
{
	FPlayerStateMachine Climber = Airborne(EHarnessClass::Climber);
	FPlayerStepInput Wall = Flying();
	Wall.bTouchingWall = true;
	Wall.bPerchInput = true;
	int Perched = 0;
	for (int I = 0; I < 360; ++I)
	{
		Climber.Step(Wall, StepSeconds);
		Perched += Climber.State() == EPlayerState::Perched ? 1 : 0;
	}
	KCHECK(Perched == 90);  // 1.5 s, once
	FPlayerStepInput Down = Landing(1.0);
	Down.bOnFloor = true;
	Climber.Step(Down, StepSeconds);
	Climber.Step(Flying(), StepSeconds);
	KCHECK(Climber.Step(Wall, StepSeconds).State == EPlayerState::Perched);  // A new flight, a new perch
	FPlayerStepInput NoWall = Wall;
	NoWall.bTouchingWall = false;
	FPlayerStateMachine Air = Airborne(EHarnessClass::Climber);
	KCHECK(Air.Step(NoWall, StepSeconds).State == EPlayerState::Airborne);
}

// H3: a flight body touching walkable ground stays the flight body while the line or a volume
// would take the player off their feet again; never a body switch every step.
KTEST(NoBodyThrashOnTheGround)
{
	FPlayerStateMachine Sliding(EHarnessClass::Climber, Tables());
	int Handoffs = 0;
	for (int I = 0; I < 60; ++I)
	{
		FPlayerStepInput In = Landing(0.0);
		In.LineForce = {400, 0, 0};  // 4.4 m/s², over a Climber's 3: dragged along the floor
		Handoffs += Sliding.Step(In, StepSeconds).bHandoff ? 1 : 0;
	}
	KCHECK(Handoffs == 1 && Sliding.Body() == EPlayerBody::Flight);

	FPlayerStateMachine Hoisted(EHarnessClass::Climber, Tables());
	Handoffs = 0;
	for (int I = 0; I < 60; ++I)
	{
		FPlayerStepInput In = Landing(0.0);
		In.LineForce = {0, 0, 90.0 * G + 1.0};
		Handoffs += Hoisted.Step(In, StepSeconds).bHandoff ? 1 : 0;
	}
	KCHECK(Handoffs == 1 && Hoisted.State() == EPlayerState::Hoisted);

	FPlayerStateMachine Volume = Airborne(EHarnessClass::Rigger);
	FPlayerStepInput Pushed = Landing(1.0);
	Pushed.ForceVolumeAcceleration = 8.0;
	KCHECK(Volume.Step(Pushed, StepSeconds).State == EPlayerState::Airborne);

	// L3: a flip taken with a ground contact in the same step still tumbles.
	FPlayerStateMachine Flip(EHarnessClass::Rigger, Tables());
	FPlayerStepInput Both = Hit(EOutcome::Flip);
	Both.Ground = FGroundContact{Up, 0.5, ESurface::Hard};
	const FPlayerStepResult R = Flip.Step(Both, StepSeconds);
	KCHECK(R.State == EPlayerState::Flipped && R.bHandoff);
	KCHECK(Flip.Step(Both, StepSeconds).State == EPlayerState::Flipped);  // The step after the handoff too
	KCHECK(Flip.Step(Both, StepSeconds).State == EPlayerState::Staggered);
}

// H4: angled lines. A pull up and along the ground lowers foot-hold by the weight it takes,
// so 5.3 and 5.4's examples hold at 30° up as they do flat.
KTEST(AngledLinesTakeWeightOffTheFeet)
{
	const auto At = [](double Newtons, double Degrees) {
		const double R = Degrees * 3.14159265358979323846 / 180.0;
		return FVec3{Newtons * std::cos(R), 0, Newtons * std::sin(R)};
	};
	FPlayerStateMachine Breaker(EHarnessClass::Breaker, Tables());
	KCHECK(Breaker.Step(Pulled(At(1200, 30)), StepSeconds).State == EPlayerState::Airborne);
	FPlayerStepInput Crouch;
	Crouch.bCrouchHeld = true;
	FPlayerStateMachine Rigger(EHarnessClass::Rigger, Tables());
	Hold(Rigger, Crouch, 0.35);
	FPlayerStepInput Overspin = Crouch;
	Overspin.LineForce = At(1200, 30);
	const FPlayerStepResult Broken = Rigger.Step(Overspin, StepSeconds);
	KCHECK(Broken.State == EPlayerState::Airborne && Broken.bBraceBroken);
	FPlayerStateMachine Held(EHarnessClass::Rigger, Tables());
	Hold(Held, Crouch, 0.35);
	FPlayerStepInput Flat = Crouch;
	Flat.LineForce = At(1000, 0);  // A braced Rigger holds a flat Breaker line (7.7 m/s²)
	KCHECK(Held.Step(Flat, StepSeconds).State == EPlayerState::Grounded && Held.IsBraced());

	// In the reference sim: a Breaker's Haul from 60° above lifts a Rigger about as soon as a flat one.
	for (const double Degrees : {0.0, 45.0, 60.0})
	{
		FReferenceSim Sim(Tables(), StepSeconds);
		const double R = Degrees * 3.14159265358979323846 / 180.0;
		const int Anchor = Sim.AddBody(StaticAt({15 * std::cos(R), 0, 15 * std::sin(R)}));
		const int Victim = Sim.AddBody(PlayerAt({0, 0, 0}, EHarnessClass::Rigger, true));
		Sim.Engage(Sim.AddLine(LineOf(EHarnessClass::Breaker, Anchor, ESeat::OwnPlate, Victim, ESeat::OtherPlate)),
		           ELineMode::Haul);
		const double Off = TimeUntil(Sim, 1.0, [&] { return Sim.Body(Victim).AirborneAt.has_value(); });
		KCHECK(Off > 0.0 && Off < 0.31);
	}
}

// M1: a ledge is grabbed falling, not rising past it, and not moving 8 m/s or more in any direction.
KTEST(LedgeGrabsNeedASlowFall)
{
	for (const FVec3 Velocity : {FVec3{0, 0, 20}, FVec3{25, 0, -1}, FVec3{6, 0, -6}})
	{
		FPlayerStateMachine Player = Airborne(EHarnessClass::Rigger);
		FPlayerStepInput Ledge = Flying(Velocity);
		Ledge.bLedgeInReach = true;
		KCHECK(Player.Step(Ledge, StepSeconds).State == EPlayerState::Airborne);
	}
	// Under own power too: too fast is no grab.
	FPlayerStateMachine Walker(EHarnessClass::Rigger, Tables());
	FPlayerStepInput Fast = Flying({0, 0, -8.5});
	Fast.bLedgeInReach = true;
	KCHECK(Walker.Step(Fast, StepSeconds).State == EPlayerState::Grounded);
	// L6: crouch lets go of a ledge.
	FPlayerStateMachine Hanging = Airborne(EHarnessClass::Rigger);
	FPlayerStepInput Ledge = Flying({0, 0, -2});
	Ledge.bLedgeInReach = true;
	Hanging.Step(Ledge, StepSeconds);
	FPlayerStepInput Drop = Ledge;
	Drop.bDropInput = true;
	KCHECK(Hanging.Step(Drop, StepSeconds).State == EPlayerState::Grounded && !Hanging.IsGrounded());
}

// M2 and L2: off the floor with a line on them, players fly, staggered or not, without a step
// of Dragged in mid-air.
KTEST(OffTheFloorUnderALineIsFlight)
{
	FPlayerStateMachine Staggered(EHarnessClass::Rigger, Tables());
	Staggered.Step(Hit(EOutcome::Stagger), StepSeconds);
	FPlayerStepInput Off = Pulled({400, 0, 0});
	Off.bOnFloor = false;
	const FPlayerStepResult R = Staggered.Step(Off, StepSeconds);
	KCHECK(R.State == EPlayerState::Staggered && R.Body == EPlayerBody::Flight && R.bHandoff);

	FPlayerStateMachine Falling(EHarnessClass::Rigger, Tables());
	Falling.Step(Flying({0, 0, -2}), StepSeconds);
	FPlayerStepInput Lined = Flying({0, 0, -2});
	Lined.LineForce = {300, 0, 0};
	KCHECK(Falling.Step(Lined, StepSeconds).State == EPlayerState::Airborne);
}

// M3: a force volume over foot-hold takes the player off a perch or a ledge, and stops a grab.
KTEST(ForceVolumesBreakHolds)
{
	FPlayerStateMachine Climber = Airborne(EHarnessClass::Climber);
	FPlayerStepInput Wall = Flying();
	Wall.bTouchingWall = true;
	Wall.bPerchInput = true;
	Climber.Step(Wall, StepSeconds);
	FPlayerStepInput Blown = Wall;
	Blown.ForceVolumeAcceleration = 20.0;
	KCHECK(Climber.Step(Blown, StepSeconds).State == EPlayerState::Airborne);
	FPlayerStateMachine Grab = Airborne(EHarnessClass::Rigger);
	FPlayerStepInput Ledge = Flying({0, 0, -2});
	Ledge.bLedgeInReach = true;
	Ledge.ForceVolumeAcceleration = 20.0;
	KCHECK(Grab.Step(Ledge, StepSeconds).State == EPlayerState::Airborne);
}

// M4: the brace's end is reported however it ends: a line, a volume, a hard hit; not a light one.
KTEST(BraceBreaksAreReported)
{
	FPlayerStepInput Crouch;
	Crouch.bCrouchHeld = true;
	FPlayerStateMachine Line(EHarnessClass::Rigger, Tables());
	Hold(Line, Crouch, 0.35);
	FPlayerStepInput Pull = Crouch;
	Pull.LineForce = {1200, 0, 0};
	KCHECK(Line.Step(Pull, StepSeconds).bBraceBroken);
	FPlayerStateMachine Volume(EHarnessClass::Breaker, Tables());
	Hold(Volume, Crouch, 0.35);
	FPlayerStepInput Push = Crouch;
	Push.ForceVolumeAcceleration = 25.0;
	const FPlayerStepResult R = Volume.Step(Push, StepSeconds);
	KCHECK(R.bBraceBroken && R.State == EPlayerState::Airborne);
	FPlayerStateMachine Light(EHarnessClass::Breaker, Tables());
	Hold(Light, Crouch, 0.35);
	FPlayerStepInput Tap = Crouch;
	Tap.Hit = FPlayerHit{EOutcome::None};
	KCHECK(!Light.Step(Tap, StepSeconds).bBraceBroken && Light.IsBraced());
	FPlayerStateMachine Early(EHarnessClass::Rigger, Tables());
	Hold(Early, Crouch, 0.3 - StepSeconds);
	KCHECK(!Early.IsBraced());  // Exactly 0.3 s, not a step less
}

// L5: a player still rising toward the top of their arc is not cued.
KTEST(HooksIgnoreARisingPlayer)
{
	KCHECK(!ShouldCueHooks(1.0, -10.0, G));
	KCHECK(ShouldCueHooks(0.5, -1.0, G));  // Rising only slowly: 0.5 m is within reach
}

// L7: stagger timers in detail. A second stagger keeps the longer time; a stagger on a perch,
// a ledge or in a fall moves to the flight body; an air stagger that runs out leaves the player
// airborne, and its landing puts them on the capsule.
KTEST(StaggerTimersInDetail)
{
	FPlayerStateMachine Twice(EHarnessClass::Rigger, Tables());
	Twice.Step(Hit(EOutcome::Stagger, EStaggerCause::OverspinRecoil), StepSeconds);  // 0.8 s
	Hold(Twice, Standing(), 0.1);
	Twice.Step(Hit(EOutcome::Stagger, EStaggerCause::CutterMelee), StepSeconds);     // 0.3 s: shorter
	KCHECK(Twice.StaggerRemaining() > 0.6);
	Hold(Twice, Standing(), 0.6);
	KCHECK(Twice.State() == EPlayerState::Staggered);
	Hold(Twice, Standing(), 0.1);
	KCHECK(Twice.State() == EPlayerState::Grounded);

	FPlayerStateMachine OnLedge = Airborne(EHarnessClass::Rigger);
	FPlayerStepInput Ledge = Flying({0, 0, -2});
	Ledge.bLedgeInReach = true;
	OnLedge.Step(Ledge, StepSeconds);
	const FPlayerStepResult Knocked = OnLedge.Step(Hit(EOutcome::Stagger), StepSeconds);
	KCHECK(Knocked.State == EPlayerState::Staggered && Knocked.Body == EPlayerBody::Flight);

	FPlayerStateMachine InAir = Airborne(EHarnessClass::Rigger);
	FPlayerStepInput AirHit = Flying();
	AirHit.Hit = FPlayerHit{EOutcome::Stagger};
	InAir.Step(AirHit, StepSeconds);
	Hold(InAir, Flying(), 0.7);
	KCHECK(InAir.State() == EPlayerState::Airborne && InAir.Body() == EPlayerBody::Flight);
	const FPlayerStepResult Down = InAir.Step(Landing(1.0), StepSeconds);
	KCHECK(Down.State == EPlayerState::Grounded && Down.Body == EPlayerBody::Capsule);

	FPlayerStateMachine Early = Airborne(EHarnessClass::Rigger);
	Early.Step(AirHit, StepSeconds);
	const FPlayerStepResult Landed = Early.Step(Landing(1.0), StepSeconds);
	KCHECK(Landed.State == EPlayerState::Staggered && Landed.Body == EPlayerBody::Capsule && Landed.bHandoff);
	KCHECK(Early.AirborneFireSpeedKeep() == 1.0);

	FPlayerStateMachine Falling(EHarnessClass::Rigger, Tables());
	Falling.Step(Flying({0, 0, -1}), StepSeconds);
	KCHECK_NEAR(Falling.AirborneFireSpeedKeep(), 0.85, 1e-12);  // A fall under own power is airborne too
}

// N1: any walkable contact at the Stall speed Stalls: with a hit in the same step, inside the
// settle window after a handoff, and while a line keeps the player off their feet. Without a
// contact report, the capsule's fall is judged on its vertical speed.
KTEST(EveryLandingIsJudgedForAStall)
{
	FPlayerStateMachine HitToo(EHarnessClass::Rigger, Tables());
	HitToo.Step(Flying({0, 0, -1}), StepSeconds);
	FPlayerStepInput Both = Landing(14.0);
	Both.bOnFloor = true;
	Both.Hit = FPlayerHit{EOutcome::Stagger};
	const FPlayerStepResult R = HitToo.Step(Both, StepSeconds);
	KCHECK(R.State == EPlayerState::Stalled && R.bLanded);

	FPlayerStateMachine Settling(EHarnessClass::Rigger, Tables());
	Settling.Step(Pulled({1500, 0, 0}), StepSeconds);  // The handoff: the next step still settles
	KCHECK(Settling.Step(Landing(14.0), StepSeconds).State == EPlayerState::Stalled);

	FPlayerStateMachine Lined = Airborne(EHarnessClass::Rigger);
	FPlayerStepInput Pulling = Landing(14.0);
	Pulling.LineForce = {1500, 0, 0};
	KCHECK(Lined.Step(Pulling, StepSeconds).State == EPlayerState::Stalled);

	FPlayerStateMachine Steep = Airborne(EHarnessClass::Rigger);
	KCHECK(Steep.Step(Landing(14.0, ESurface::Hard, {1, 0, 0.5}), StepSeconds).State == EPlayerState::Airborne);

	for (const auto& [Speed, Expected] : {std::pair{14.0, EPlayerState::Stalled}, std::pair{6.0, EPlayerState::Staggered},
	                                      std::pair{3.0, EPlayerState::Grounded}})
	{
		FPlayerStateMachine Walker(EHarnessClass::Rigger, Tables());
		Walker.Step(Flying({0, 0, -1}), StepSeconds);
		FPlayerStepInput Floor = Flying({0, 0, -Speed});
		Floor.bOnFloor = true;  // The floor, no contact report
		const FPlayerStepResult Down = Walker.Step(Floor, StepSeconds);
		KCHECK(Down.State == Expected && Down.bLanded);
	}
	FPlayerStateMachine Staggered(EHarnessClass::Rigger, Tables());
	Staggered.Step(Flying({0, 0, -1}), StepSeconds);
	FPlayerStepInput Floor = Flying({0, 0, -14.0});
	Floor.bOnFloor = true;
	Floor.Hit = FPlayerHit{EOutcome::Stagger};
	KCHECK(Staggered.Step(Floor, StepSeconds).State == EPlayerState::Stalled);

	// A hard contact the player cannot stand on staggers them on the flight body.
	FPlayerStateMachine Dragged = Airborne(EHarnessClass::Rigger);
	FPlayerStepInput Hard = Landing(6.0);
	Hard.LineForce = {1500, 0, 0};
	const FPlayerStepResult Knocked = Dragged.Step(Hard, StepSeconds);
	KCHECK(Knocked.State == EPlayerState::Staggered && Knocked.Body == EPlayerBody::Flight && Knocked.bLanded);
}

// N2: the perch comes back whenever the player stands, a ledge climb included.
KTEST(StandingGivesThePerchBack)
{
	FPlayerStateMachine Climber = Airborne(EHarnessClass::Climber);
	FPlayerStepInput Wall = Flying();
	Wall.bTouchingWall = true;
	Wall.bPerchInput = true;
	Hold(Climber, Wall, 2.0);
	KCHECK(Climber.State() != EPlayerState::Perched && !Climber.IsGrounded());  // Perch used up, falling
	FPlayerStepInput Ledge = Flying({0, 0, -2});
	Ledge.bLedgeInReach = true;
	KCHECK(Climber.Step(Ledge, StepSeconds).State == EPlayerState::LedgeHang);
	FPlayerStepInput Climb = Ledge;
	Climb.bJumpInput = true;
	Climber.Step(Climb, StepSeconds);
	KCHECK(Climber.IsGrounded());
	Climber.Step(Standing(), StepSeconds);
	FPlayerStepInput OffAgain = Wall;
	KCHECK(Climber.Step(OffAgain, StepSeconds).State == EPlayerState::Perched);
}

// N3: a flipped player dragged along the floor stops tumbling on the first walkable contact
// after the handoff settles: staggered on the flight body, the timer running.
KTEST(ADraggedFlipEndsOnTheFloor)
{
	FPlayerStateMachine Player(EHarnessClass::Rigger, Tables());
	FPlayerStepInput Flip = Hit(EOutcome::Flip);
	Flip.LineForce = {1500, 0, 0};
	Player.Step(Flip, StepSeconds);
	FPlayerStepInput Sliding = Landing(0.5);
	Sliding.LineForce = {1500, 0, 0};
	KCHECK(Player.Step(Sliding, StepSeconds).State == EPlayerState::Flipped);  // Still settling
	const FPlayerStepResult R = Player.Step(Sliding, StepSeconds);
	KCHECK(R.State == EPlayerState::Staggered && R.Body == EPlayerBody::Flight && R.bLanded);
	Player.Step(Sliding, StepSeconds);
	KCHECK(Player.StaggerRemaining() < StaggerBase() && Player.StaggerRemaining() > 0.0);
	Hold(Player, Sliding, 0.7);
	KCHECK(Player.State() == EPlayerState::Airborne && Player.Body() == EPlayerBody::Flight);
	const FPlayerStepResult Freed = Player.Step(Landing(0.5), StepSeconds);
	KCHECK(Freed.State == EPlayerState::Grounded && Freed.Body == EPlayerBody::Capsule);
}

// N6: the details mutation testing found unguarded.
KTEST(PlayerRulesInDetail)
{
	// A stagger's timer starts on the step after the hit.
	FPlayerStateMachine Hit1(EHarnessClass::Rigger, Tables());
	Hit1.Step(Hit(EOutcome::Stagger), StepSeconds);
	KCHECK_NEAR(Hit1.StaggerRemaining(), StaggerBase(), 1e-12);

	// No ledge grab while rising, however slowly.
	FPlayerStateMachine Rising = Airborne(EHarnessClass::Rigger);
	FPlayerStepInput Up2 = Flying({0, 0, 2});
	Up2.bLedgeInReach = true;
	KCHECK(Rising.Step(Up2, StepSeconds).State == EPlayerState::Airborne);

	// A stagger during a fall under own power moves to the flight body.
	FPlayerStateMachine Falling(EHarnessClass::Rigger, Tables());
	Falling.Step(Flying({0, 0, -1}), StepSeconds);
	FPlayerStepInput FallHit = Flying({0, 0, -1});
	FallHit.Hit = FPlayerHit{EOutcome::Stagger};
	const FPlayerStepResult Fell = Falling.Step(FallHit, StepSeconds);
	KCHECK(Fell.State == EPlayerState::Staggered && Fell.Body == EPlayerBody::Flight && Fell.bHandoff);

	// A line takes a staggered player off their feet, on the floor too.
	FPlayerStateMachine Staggered(EHarnessClass::Rigger, Tables());
	Staggered.Step(Hit(EOutcome::Stagger), StepSeconds);
	const FPlayerStepResult Lifted = Staggered.Step(Pulled({1500, 0, 0}), StepSeconds);
	KCHECK(Lifted.State == EPlayerState::Staggered && Lifted.Body == EPlayerBody::Flight && Lifted.bHandoff);

	// Stalled takes no more hits, flips or landings.
	FPlayerStateMachine Out(EHarnessClass::Rigger, Tables());
	Out.Step(Hit(EOutcome::Stall), StepSeconds);
	KCHECK(Out.Step(Hit(EOutcome::Flip), StepSeconds).State == EPlayerState::Stalled);
	KCHECK(Out.Step(Hit(EOutcome::Stagger), StepSeconds).State == EPlayerState::Stalled);
	KCHECK(Out.Step(Landing(1.0), StepSeconds).State == EPlayerState::Stalled);

	// A hoist needs over half the line force upward, on either body.
	const double Weight = Tables().Harness(EHarnessClass::Rigger).MassKg * G;
	const auto At = [Weight](double Degrees) {
		const double R = Degrees * 3.14159265358979323846 / 180.0;
		const double Newtons = 1.2 * Weight / std::sin(R);  // The upward part is always over the weight
		return FVec3{Newtons * std::cos(R), 0, Newtons * std::sin(R)};
	};
	FPlayerStateMachine Steep(EHarnessClass::Rigger, Tables());
	KCHECK(Steep.Step(Pulled(At(31)), StepSeconds).State == EPlayerState::Hoisted);
	FPlayerStateMachine Shallow(EHarnessClass::Rigger, Tables());
	KCHECK(Shallow.Step(Pulled(At(29)), StepSeconds).State == EPlayerState::Airborne);
	FPlayerStepInput AirSteep = Flying();
	AirSteep.LineForce = At(31);
	FPlayerStepInput AirShallow = Flying();
	AirShallow.LineForce = At(29);
	FPlayerStateMachine High = Airborne(EHarnessClass::Rigger);
	KCHECK(High.Step(AirSteep, StepSeconds).State == EPlayerState::Hoisted);
	FPlayerStateMachine Low = Airborne(EHarnessClass::Rigger);
	KCHECK(Low.Step(AirShallow, StepSeconds).State == EPlayerState::Airborne);

	// A force volume over foot-hold makes a fall under own power a flight.
	FPlayerStateMachine Blown(EHarnessClass::Rigger, Tables());
	Blown.Step(Flying({0, 0, -1}), StepSeconds);
	FPlayerStepInput Gust = Flying({0, 0, -1});
	Gust.ForceVolumeAcceleration = 20.0;
	const FPlayerStepResult Gusted = Blown.Step(Gust, StepSeconds);
	KCHECK(Gusted.State == EPlayerState::Airborne && Gusted.bHandoff);

	// The brace sets at exactly 0.3 s.
	FPlayerStepInput Crouch;
	Crouch.bCrouchHeld = true;
	FPlayerStateMachine Brace(EHarnessClass::Rigger, Tables());
	Hold(Brace, Crouch, 0.3);
	KCHECK(Brace.IsBraced());
	FPlayerStateMachine Quick = Airborne(EHarnessClass::Rigger);
	FPlayerStepInput JustTooFast = Flying({0, 0, -8.02});
	JustTooFast.bLedgeInReach = true;
	KCHECK(Quick.Step(JustTooFast, StepSeconds).State == EPlayerState::Airborne);  // 8 m/s, not more

	// A downward pull adds no foot-hold: the along-ground part alone takes the player off.
	const FHarnessTuning& Rigger = Tables().Harness(EHarnessClass::Rigger);
	FPlayerStateMachine Down(EHarnessClass::Rigger, Tables());
	KCHECK(Down.Step(Pulled({Rigger.MassKg * Rigger.FootHold * 1.2, 0, -Weight}), StepSeconds).State ==
	       EPlayerState::Airborne);
}

// The footing mailbox carries the Hang force: a player hanging still on a Hang line reads
// their weight in it.
KTEST(LineForceIncludesTheHang)
{
	FReferenceSim Sim(Tables(), StepSeconds);
	const int Anchor = Sim.AddBody(StaticAt({0, 0, 10}));
	FSimBody Player = PlayerAt({0, 0, 6}, EHarnessClass::Rigger, false);
	Player.bGravity = true;
	const int Hanging = Sim.AddBody(Player);
	Sim.Engage(Sim.AddLine(LineOf(EHarnessClass::Climber, Anchor, ESeat::Static, Hanging, ESeat::OtherPlate)),
	           ELineMode::Hang);
	RunFor(Sim, 2.0);
	const double Weight = Tables().Harness(EHarnessClass::Rigger).MassKg * G;
	KCHECK_NEAR(Sim.LineForceOn(Hanging).Z, Weight, 0.05 * Weight);
}

// Round 3: contacts at the Flip speed count while the handoff settles; a stagger is not
// restarted by every scrape; a wall is no floor; a staggered landing is reported.
KTEST(ContactsInDetail)
{
	FPlayerStateMachine Caught(EHarnessClass::Rigger, Tables());
	Caught.Step(Flying({0, 0, -10}), StepSeconds);
	FPlayerStepInput Line = Flying({0, 0, -10});
	Line.LineForce = {200, 0, 0};
	KCHECK(Caught.Step(Line, StepSeconds).bHandoff);  // Caught by a line: the flight body
	const FPlayerStepResult Hard = Caught.Step(Landing(10.0), StepSeconds);  // Settling, over Flip speed
	KCHECK(Hard.State == EPlayerState::Staggered && Hard.bLanded);
	FPlayerStateMachine Soft(EHarnessClass::Rigger, Tables());
	Soft.Step(Pulled({1500, 0, 0}), StepSeconds);
	KCHECK(Soft.Step(Landing(5.0), StepSeconds).State == EPlayerState::Airborne);  // Settling, under Flip speed

	FPlayerStateMachine Scraped = Airborne(EHarnessClass::Climber);
	FPlayerStepInput Scrape = Landing(6.0);
	Scrape.LineForce = {400, 0, 0};
	KCHECK(Scraped.Step(Scrape, StepSeconds).State == EPlayerState::Staggered);
	Hold(Scraped, Scrape, 0.3);
	KCHECK_NEAR(Scraped.StaggerRemaining(), StaggerBase() - 0.3, 1e-9);  // Not restarted by each scrape
	FPlayerStepInput Slam = Scrape;
	Slam.Ground->ImpactSpeed = 9.0;  // The Flip speed restarts it
	Scraped.Step(Slam, StepSeconds);
	KCHECK_NEAR(Scraped.StaggerRemaining(), StaggerBase(), 1e-9);

	FPlayerStateMachine Walled(EHarnessClass::Rigger, Tables());
	Walled.Step(Flying({0, 0, -1}), StepSeconds);
	FPlayerStepInput Wall = Landing(0.5, ESurface::Hard, {1, 0, 0});
	const FPlayerStepResult Side = Walled.Step(Wall, StepSeconds);
	KCHECK(!Side.bLanded && !Walled.IsGrounded());
	FPlayerStepInput HardWall = Landing(9.0, ESurface::Hard, {1, 0, 0});
	KCHECK(Walled.Step(HardWall, StepSeconds).State == EPlayerState::Grounded && !Walled.IsGrounded());
	FPlayerStepInput WallAtFloor = Landing(1.0, ESurface::Hard, {1, 0, 0});
	WallAtFloor.Velocity = {0, 0, -14.0};
	WallAtFloor.bOnFloor = true;  // The floor at 14 m/s; the only contact is the wall's
	KCHECK(Walled.Step(WallAtFloor, StepSeconds).State == EPlayerState::Stalled);

	FPlayerStateMachine FlipDown(EHarnessClass::Rigger, Tables());
	FlipDown.Step(Flying({0, 0, -1}), StepSeconds);
	FPlayerStepInput FlipFloor = Flying({0, 0, -14.0});
	FlipFloor.bOnFloor = true;
	FlipFloor.Hit = FPlayerHit{EOutcome::Flip};
	const FPlayerStepResult Flipped = FlipDown.Step(FlipFloor, StepSeconds);
	KCHECK(Flipped.State == EPlayerState::Stalled && Flipped.bLanded);

	FPlayerStateMachine Knocked(EHarnessClass::Rigger, Tables());
	Knocked.Step(Flying({0, 0, -1}), StepSeconds);
	FPlayerStepInput Down = Landing(6.0);
	Down.bOnFloor = true;
	Down.Hit = FPlayerHit{EOutcome::Stagger};
	const FPlayerStepResult R = Knocked.Step(Down, StepSeconds);
	KCHECK(R.State == EPlayerState::Staggered && R.bLanded && Knocked.IsGrounded());
}
