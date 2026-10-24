#include "Kinetic/LineSolver.h"

#include <algorithm>

namespace Kinetic
{
namespace
{
struct FGeometry
{
	FVec3 Axis;       // A to B
	double Length;
	double Closing;   // Positive when A and B approach
};

FGeometry Measure(const FLineEndpoint& A, const FLineEndpoint& B)
{
	const FVec3 D = B.Position - A.Position;
	const FVec3 N = D.SafeNormal();
	return {N, D.Length(), (A.Velocity - B.Velocity).Dot(N)};
}

double Clamp01(double X) { return std::clamp(X, 0.0, 1.0); }

double BonusMul(const FLineParams& Params, const FLineEndpoint& A, const FLineEndpoint& B)
{
	return (A.bSmallPropBonus || B.bSmallPropBonus) ? Params.SmallPropForceMul : 1.0;
}

FLineStepResult StepHaul(FLineState& State, const FLineParams& Params, double TensionN, const FLineEndpoint& A,
                         const FLineEndpoint& B, const FSpoolGlobals& G, double Now)
{
	FLineStepResult Out;
	const FGeometry Geo = Measure(A, B);
	const double Pull = TensionN * Clamp01((Now - State.ModeStartTime) / G.HaulRamp);
	double Force = Pull * BonusMul(Params, A, B);
	if (Geo.Length <= G.DogsMeet)
	{
		State.Mode = ELineMode::Slack;  // The Haul continues until the Dogs meet (3.2)
		Out.Events |= LineEvents::DogsMet;
		return Out;
	}
	const bool bPastCap = Geo.Closing >= G.HaulCloseCap;
	if (bPastCap)
	{
		Force = 0.0;
		Out.Events |= LineEvents::SlackPastCap;
	}
	Out.ForceOnA = Geo.Axis * Force;
	Out.ForceOnB = Geo.Axis * -Force;
	Out.BreakFreeOnA = Geo.Axis * (bPastCap ? 0.0 : Pull);
	Out.BreakFreeOnB = Out.BreakFreeOnA * -1.0;
	Out.AppliedForce = Force;
	return Out;
}

// The Kick: a velocity-target drive that reaches relative speed V by KickTime, split by inverse
// mass, with a force cap. Returns the force for this step and marks the Kick done at V.
double KickForce(FLineState& State, const FLineParams& Params, double Separating, double SumInvMass, double T,
                 const FSpoolGlobals& G, double Dt)
{
	const double V = Params.KickLaunchSpeedV;
	if (Separating >= V)
	{
		State.bKickDone = true;
		return 0.0;
	}
	// Target the speed due at the end of this step, so the last Kick step reaches V exactly at
	// KickTime (the design pseudocode's start-of-step target never reaches V on a fixed step).
	const double TargetSpeed = V * Clamp01((T + Dt) / G.KickTime);
	const double SpeedNeeded = std::max(TargetSpeed - Separating, 0.0);
	const double EffectiveMass = 1.0 / SumInvMass;
	const double ForceCap = G.KickForceCapMul * EffectiveMass * V / G.KickTime;
	return std::min(SpeedNeeded * EffectiveMass / Dt, ForceCap);
}

FLineStepResult StepJack(FLineState& State, const FLineParams& Params, double TensionN, const FLineEndpoint& A,
                         const FLineEndpoint& B, const FSpoolGlobals& G, double Dt, double Now)
{
	FLineStepResult Out;
	const FGeometry Geo = Measure(A, B);
	if (Geo.Length >= Params.LineLength)
	{
		State.Mode = ELineMode::Slack;
		Out.Events |= LineEvents::AutoRelease;
		return Out;
	}
	const double Separating = -Geo.Closing;
	const double SumInvMass = A.InvMass + B.InvMass;
	if (Separating >= G.JackSeparateCap)
	{
		return Out;  // Past the cap: the line goes slack
	}

	// Static on both ends there is no Kick, but the push still applies T: nothing moves, and a
	// volatile between the Dogs reads the force (the gas manifold vents at 600 N, 6.8).
	const double T = Now - State.ModeStartTime;
	const bool bKickAllowed = IsKickAllowed(A, B);
	double Force = 0.0;
	double ThresholdForce = 0.0;
	bool bKicking = false;
	if (bKickAllowed && !State.bKickDone && T < G.KickTime && SumInvMass > 0.0)
	{
		Force = KickForce(State, Params, Separating, SumInvMass, T, G, Dt);
		bKicking = !State.bKickDone;  // Reaching V this step ends the Kick; the push takes over
	}
	if (bKicking)
	{
		Out.Events |= LineEvents::KickActive;
	}
	else
	{
		// The motor push at T. Under the interlock it is all there is, ramped over 0.3 s.
		const double Ramp = bKickAllowed ? 1.0 : Clamp01(T / G.InterlockRamp);
		ThresholdForce = TensionN * Ramp;
		Force = ThresholdForce * BonusMul(Params, A, B);
	}
	Out.ForceOnA = Geo.Axis * -Force;
	Out.ForceOnB = Geo.Axis * Force;
	if (bKicking)
	{
		ThresholdForce = std::min(Force, TensionN);
	}
	Out.BreakFreeOnA = Geo.Axis * -ThresholdForce;
	Out.BreakFreeOnB = Geo.Axis * ThresholdForce;
	Out.AppliedForce = Force;
	return Out;
}
} // namespace

void EngageMode(FLineState& State, ELineMode Mode, double Now, const FVec3& PositionA, const FVec3& PositionB,
                const FSpoolGlobals& Globals)
{
	const bool bKeepsRest = Mode == ELineMode::Set && State.Mode == ELineMode::Hang;
	State.Mode = Mode;
	State.ModeStartTime = Now;
	State.bKickDone = false;
	State.ReelInput = 0;
	if ((Mode == ELineMode::Hang || Mode == ELineMode::Set) && !bKeepsRest)
	{
		State.L0 = std::max((PositionB - PositionA).Length(), Globals.DogsMeet);
		State.SlipAccum = 0.0;
		State.ReelAccum = 0.0;
	}
}

bool IsKickAllowed(const FLineEndpoint& A, const FLineEndpoint& B)
{
	return A.Seat != ESeat::OtherPlate && B.Seat != ESeat::OtherPlate;
}

FLineStepResult StepLine(FLineState& State, const FLineParams& Params, double TensionN, const FLineEndpoint& A,
                         const FLineEndpoint& B, const FSpoolGlobals& Globals, double Dt, double Now)
{
	FLineStepResult Out;
	switch (State.Mode)
	{
	case ELineMode::Haul:
		Out = StepHaul(State, Params, TensionN, A, B, Globals, Now);
		break;
	case ELineMode::Jack:
		Out = StepJack(State, Params, TensionN, A, B, Globals, Dt, Now);
		break;
	case ELineMode::Hang:
	case ELineMode::Set:
		return Out;  // HangSolver.h
	case ELineMode::Slack:
		break;  // No force: a line that let go reads 0 from the next step
	}
	State.AppliedForce = Out.AppliedForce;
	return Out;
}
} // namespace Kinetic
