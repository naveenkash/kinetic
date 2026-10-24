#include "Kinetic/ReferenceSim.h"

#include "Kinetic/HangSolver.h"

#include <algorithm>

namespace Kinetic
{
namespace
{
// A grounded player this far above their floor after the Hang solve has been lifted off it.
constexpr double LiftOffHeight = 1e-6;  // m

bool IsActive(ELineMode Mode) { return Mode == ELineMode::Haul || Mode == ELineMode::Jack; }
bool IsHang(ELineMode Mode) { return Mode == ELineMode::Hang || Mode == ELineMode::Set; }

FObjectBody LatchedObject(const FSimBody& Body)
{
	FObjectBody Object = Body.Object;
	Object.bBrokenFree = Object.bBrokenFree || Body.bBrokenFreeLatched;
	return Object;
}

FVec3 AlongTheGround(const FVec3& Force) { return Force - Up * Force.Dot(Up); }
} // namespace

int FReferenceSim::AddBody(const FSimBody& Body)
{
	Bodies.push_back(Body);
	LastHangForce.emplace_back();
	return static_cast<int>(Bodies.size()) - 1;
}

int FReferenceSim::AddLine(const FSimLine& Line)
{
	Lines.push_back(Line);
	return static_cast<int>(Lines.size()) - 1;
}

void FReferenceSim::Engage(int LineIndex, ELineMode Mode)
{
	FSimLine& Line = Lines.at(LineIndex);
	EngageMode(Line.State, Mode, Now, Bodies.at(Line.BodyA).Position, Bodies.at(Line.BodyB).Position, Tables.Globals());
}

double FReferenceSim::FreeInvMass(const FSimBody& Body) const
{
	switch (Body.Kind)
	{
	case ESimBodyKind::Static:
	case ESimBodyKind::Kinematic:
		return 0.0;
	case ESimBodyKind::Player:
		return 1.0 / Body.MassKg;
	case ESimBodyKind::Object:
		return CanEverMoveUnderALine(LatchedObject(Body)) ? 1.0 / Body.MassKg : 0.0;
	}
	return 0.0;
}

FLineEndpoint FReferenceSim::EndpointFor(const FSimLine& Line, bool bSideA) const
{
	const FSimBody& Body = Bodies.at(bSideA ? Line.BodyA : Line.BodyB);
	FLineEndpoint E;
	E.Seat = bSideA ? Line.SeatA : Line.SeatB;
	E.Position = Body.Position;
	E.Velocity = Body.Velocity;
	E.InvMass = FreeInvMass(Body);
	E.bSmallPropBonus = Body.Kind == ESimBodyKind::Object && Body.bSmallPropBonus;
	return E;
}

// Whether each object stays put this step: the break-free thresholds see T (the Kick at T,
// without the Scrapper bonus), which does not depend on how the force splits, so this is decided
// first, from a trial step on copies of the line states. A Kick off a placed object that holds
// then gives the player the whole of V, as off a wall (3.2).
std::vector<bool> FReferenceSim::HeldThisStep() const
{
	std::vector<FVec3> Pull(Bodies.size());
	std::vector<FVec3> ThresholdPull(Bodies.size());
	for (const FSimLine& Line : Lines)
	{
		if (!IsActive(Line.State.Mode))
		{
			continue;
		}
		FLineState Trial = Line.State;
		const FLineStepResult R = StepLine(Trial, Line.Params, Line.TensionN, EndpointFor(Line, true),
		                                   EndpointFor(Line, false), Tables.Globals(), Dt, Now);
		Pull[Line.BodyA] = Pull[Line.BodyA] + R.ForceOnA;
		Pull[Line.BodyB] = Pull[Line.BodyB] + R.ForceOnB;
		ThresholdPull[Line.BodyA] = ThresholdPull[Line.BodyA] + R.BreakFreeOnA;
		ThresholdPull[Line.BodyB] = ThresholdPull[Line.BodyB] + R.BreakFreeOnB;
	}
	std::vector<bool> Held(Bodies.size(), false);
	for (size_t I = 0; I < Bodies.size(); ++I)
	{
		if (Bodies[I].Kind == ESimBodyKind::Object && FreeInvMass(Bodies[I]) > 0.0)
		{
			const bool bHangOnly = Pull[I].Length() <= 0.0;
			Held[I] = ResolveObjectUnderLines(LatchedObject(Bodies[I]), ThresholdPull[I], bHangOnly, Tables).InvMass <= 0.0;
		}
	}
	return Held;
}

FReferenceSim::FStepForces FReferenceSim::StepActiveLines()
{
	const std::vector<bool> Held = HeldThisStep();
	FStepForces Forces{std::vector<FVec3>(Bodies.size()), std::vector<FVec3>(Bodies.size()),
	                   std::vector<bool>(Bodies.size(), false)};
	const FSpoolGlobals& G = Tables.Globals();
	for (FSimLine& Line : Lines)
	{
		if (Line.State.Mode == ELineMode::Slack)
		{
			StepLine(Line.State, Line.Params, Line.TensionN, {}, {}, G, Dt, Now);  // Reads 0
			continue;
		}
		if (!IsActive(Line.State.Mode))
		{
			continue;
		}
		FLineEndpoint A = EndpointFor(Line, true);
		FLineEndpoint B = EndpointFor(Line, false);
		A.InvMass = Held[Line.BodyA] ? 0.0 : A.InvMass;
		B.InvMass = Held[Line.BodyB] ? 0.0 : B.InvMass;
		const FLineStepResult R = StepLine(Line.State, Line.Params, Line.TensionN, A, B, G, Dt, Now);
		Forces.ActivePull[Line.BodyA] = Forces.ActivePull[Line.BodyA] + R.ForceOnA;
		Forces.ActivePull[Line.BodyB] = Forces.ActivePull[Line.BodyB] + R.ForceOnB;
		Forces.BreakFreePull[Line.BodyA] = Forces.BreakFreePull[Line.BodyA] + R.BreakFreeOnA;
		Forces.BreakFreePull[Line.BodyB] = Forces.BreakFreePull[Line.BodyB] + R.BreakFreeOnB;
		const bool bKicked = (R.Events & LineEvents::KickActive) != 0;
		Forces.bKick[Line.BodyA] = Forces.bKick[Line.BodyA] || bKicked;
		Forces.bKick[Line.BodyB] = Forces.bKick[Line.BodyB] || bKicked;
		Line.MaxAppliedForce = std::max(Line.MaxAppliedForce, R.AppliedForce);
		Line.EventsSeen |= R.Events;
	}
	return Forces;
}

// Velocities only: the Hang solve integrates positions. The Hang force on the player is part
// of the footing decision but not of the integration: the Hang solve applies it as an impulse.
void FReferenceSim::IntegratePlayer(FSimBody& Body, const FVec3& FootingForce, const FVec3& ActiveForce, bool bKick)
{
	const double Gravity = Tables.Globals().Gravity;
	const FPlayerLineResponse R = ResolvePlayerUnderLine(Body.Player, FootingForce, bKick, Gravity);
	Body.Footing = R.Footing;
	if (R.bBraceBroken)
	{
		Body.Player.bBraced = false;
	}
	switch (R.Footing)
	{
	case EFooting::Braced:
		Body.Velocity = {};
		return;
	case EFooting::Skid:
		// Feet planted: moved along the ground; the floor carries the weight in the solve.
		Body.Velocity = Body.Velocity + AlongTheGround(ActiveForce) * (Dt / Body.MassKg);
		Body.Velocity.Z = Body.bGravity ? -Gravity * Dt : 0.0;
		return;
	case EFooting::Airborne:
	case EFooting::Hoisted:
		if (!Body.AirborneAt)
		{
			Body.AirborneAt = Now;
		}
		Body.Player.bGrounded = false;
		Body.Velocity = Body.Velocity + ActiveForce * (Dt / Body.MassKg);
		if (Body.bGravity)
		{
			Body.Velocity.Z -= Gravity * Dt;
		}
		return;
	}
}

void FReferenceSim::IntegrateObject(FSimBody& Body, const FVec3& Force, const FVec3& BreakFreePull)
{
	const bool bHangOnly = Force.Length() <= 0.0;
	const FObjectLineResponse R = ResolveObjectUnderLines(LatchedObject(Body), BreakFreePull, bHangOnly, Tables);
	Body.bBrokenFreeLatched = Body.bBrokenFreeLatched || R.bBreaksFreeNow || R.bReleasedFromPath;
	if (R.InvMass <= 0.0)
	{
		Body.Velocity = {};
		return;
	}
	Body.Velocity = Body.Velocity + Force * (Dt * R.InvMass);
	if (Body.bGravity)
	{
		Body.Velocity.Z -= Tables.Globals().Gravity * Dt;
	}
}

void FReferenceSim::Integrate(const FStepForces& Forces)
{
	for (size_t I = 0; I < Bodies.size(); ++I)
	{
		FSimBody& Body = Bodies[I];
		switch (Body.Kind)
		{
		case ESimBodyKind::Static:
		case ESimBodyKind::Kinematic:
			break;
		case ESimBodyKind::Player:
			IntegratePlayer(Body, Forces.ActivePull[I] + LastHangForce[I], Forces.ActivePull[I], Forces.bKick[I]);
			break;
		case ESimBodyKind::Object:
			IntegrateObject(Body, Forces.ActivePull[I], Forces.BreakFreePull[I]);
			break;
		}
	}
}

double FReferenceSim::HangInvMass(const FSimBody& Body, const FVec3& ActivePull, const FVec3& BreakFreePull) const
{
	switch (Body.Kind)
	{
	case ESimBodyKind::Static:
	case ESimBodyKind::Kinematic:
		return 0.0;
	case ESimBodyKind::Player:
		return Body.Footing == EFooting::Braced ? 0.0 : 1.0 / Body.MassKg;
	case ESimBodyKind::Object:
		return ResolveObjectUnderLines(LatchedObject(Body), BreakFreePull, ActivePull.Length() <= 0.0, Tables).InvMass;
	}
	return 0.0;
}

void FReferenceSim::SolveHangAndFloor(const FStepForces& Forces)
{
	std::vector<FHangBody> Solve(Bodies.size());
	for (size_t I = 0; I < Bodies.size(); ++I)
	{
		const FSimBody& Body = Bodies[I];
		Solve[I] = {Body.Position, Body.Velocity, HangInvMass(Body, Forces.ActivePull[I], Forces.BreakFreePull[I]), Body.FloorZ};
	}
	std::vector<FHangLine> HangLines;
	std::vector<FSimLine*> Owners;
	for (FSimLine& Line : Lines)
	{
		if (IsHang(Line.State.Mode))
		{
			HangLines.push_back({Line.BodyA, Line.BodyB, &Line.State, Line.Params, Line.Params.HangLoadN, 0.0, {}, {}, 0});
			Owners.push_back(&Line);
		}
	}
	SolveHangLines(Solve, HangLines, Tables.Globals(), Dt);

	std::fill(LastHangForce.begin(), LastHangForce.end(), FVec3{});
	for (size_t I = 0; I < HangLines.size(); ++I)
	{
		const FHangLine& Solved = HangLines[I];
		LastHangForce[Solved.BodyA] = LastHangForce[Solved.BodyA] + Solved.ForceOnA;
		LastHangForce[Solved.BodyB] = LastHangForce[Solved.BodyB] + Solved.ForceOnB;
		Owners[I]->MaxAppliedForce = std::max(Owners[I]->MaxAppliedForce, Solved.AppliedForce);
		Owners[I]->EventsSeen |= Solved.Events;
	}
	for (size_t I = 0; I < Bodies.size(); ++I)
	{
		FSimBody& Body = Bodies[I];
		Body.Position = Solve[I].Position;
		Body.Velocity = Solve[I].Velocity;
		const bool bLifted = Body.FloorZ && Body.Position.Z > *Body.FloorZ + LiftOffHeight;
		if (Body.Kind == ESimBodyKind::Player && Body.Player.bGrounded && bLifted)
		{
			Body.Player.bGrounded = false;  // Lifted off their feet by a Hang line (the Gallows)
			Body.Player.bBraced = false;
			Body.Footing = EFooting::Hoisted;
			Body.AirborneAt = Now;
		}
	}
}

void FReferenceSim::ApplyCap(FSimBody& Body) const
{
	const double Speed = Body.Velocity.Length();
	const double Cap = Tables.Globals().HardVelocityCap;
	if (Speed > Cap)
	{
		Body.Velocity = Body.Velocity * (Cap / Speed);
	}
}

void FReferenceSim::Step()
{
	const FStepForces Forces = StepActiveLines();
	Integrate(Forces);
	SolveHangAndFloor(Forces);
	for (FSimBody& Body : Bodies)
	{
		ApplyCap(Body);
	}
	Now += Dt;
}

FPanicRun RunHaulPanic(const FKineticTables& Tables, EHarnessClass Attacker, EHarnessClass Victim, double DistanceM,
                       double Dt, double MaxSeconds)
{
	const FHarnessTuning& AttackerTuning = Tables.Harness(Attacker);
	const FHarnessTuning& VictimTuning = Tables.Harness(Victim);
	FReferenceSim Sim(Tables, Dt);

	// The attacker is braced or anchored: static to their own line (4.3).
	FSimBody Anchor;
	Anchor.Kind = ESimBodyKind::Static;
	const int AnchorIndex = Sim.AddBody(Anchor);

	FSimBody VictimBody;
	VictimBody.Kind = ESimBodyKind::Player;
	VictimBody.Position = {DistanceM + 1.0, 0.0, 0.0};
	VictimBody.MassKg = VictimTuning.MassKg;
	VictimBody.Player = PlayerBodyFor(VictimTuning, false, true);
	VictimBody.bGravity = true;
	VictimBody.FloorZ = 0.0;
	const int VictimIndex = Sim.AddBody(VictimBody);

	FSimLine Line;
	Line.BodyA = AnchorIndex;
	Line.BodyB = VictimIndex;
	Line.SeatA = ESeat::OwnPlate;
	Line.SeatB = ESeat::OtherPlate;
	Line.Params = FLineParams::FromHarness(AttackerTuning);
	Line.TensionN = AttackerTuning.TensionN;
	const int LineIndex = Sim.AddLine(Line);
	Sim.Engage(LineIndex, ELineMode::Haul);

	FPanicRun Out;
	const FVec3 Start = VictimBody.Position;
	while (Sim.Time() < MaxSeconds && !Out.DistanceAt)
	{
		Sim.Step();
		const FSimBody& V = Sim.Body(VictimIndex);
		if (!Out.FeetOffAt && V.AirborneAt)
		{
			Out.FeetOffAt = *V.AirborneAt;
		}
		if ((V.Position - Start).Length() >= DistanceM)
		{
			Out.DistanceAt = Sim.Time();
			Out.SpeedAtDistance = V.Velocity.Length();
		}
	}
	return Out;
}
} // namespace Kinetic
