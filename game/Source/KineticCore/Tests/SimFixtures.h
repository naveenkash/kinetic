// Builders for reference-simulation tests.
#pragma once

#include "Kinetic/ReferenceSim.h"
#include "TestHarness.h"

namespace Kinetic::Test
{
inline constexpr double StepSeconds = 1.0 / 60.0;  // The project's fixed physics step

inline FSimBody StaticAt(const FVec3& Position)
{
	FSimBody Body;
	Body.Kind = ESimBodyKind::Static;
	Body.Position = Position;
	return Body;
}

inline FSimBody ObjectAt(const FVec3& Position, double MassKg, EPlacement Placement = EPlacement::Loose)
{
	FSimBody Body;
	Body.Kind = ESimBodyKind::Object;
	Body.Position = Position;
	Body.MassKg = MassKg;
	Body.Object.MassKg = MassKg;
	Body.Object.Placement = Placement;
	Body.bSmallPropBonus = IsSmallPropBonusTarget(MassKg, Tables());
	return Body;
}

inline FSimBody PlayerAt(const FVec3& Position, EHarnessClass Class, bool bGrounded, bool bBraced = false)
{
	const FHarnessTuning& Harness = Tables().Harness(Class);
	FSimBody Body;
	Body.Kind = ESimBodyKind::Player;
	Body.Position = Position;
	Body.MassKg = Harness.MassKg;
	Body.Player = PlayerBodyFor(Harness, bBraced, bGrounded);
	if (bGrounded)
	{
		Body.bGravity = true;
		Body.FloorZ = Position.Z;
	}
	return Body;
}

inline FSimLine LineOf(EHarnessClass Owner, int BodyA, ESeat SeatA, int BodyB, ESeat SeatB)
{
	const FHarnessTuning& Harness = Tables().Harness(Owner);
	FSimLine Line;
	Line.BodyA = BodyA;
	Line.BodyB = BodyB;
	Line.SeatA = SeatA;
	Line.SeatB = SeatB;
	Line.Params = FLineParams::FromHarness(Harness);
	Line.TensionN = Harness.TensionN;
	return Line;
}

inline void RunFor(FReferenceSim& Sim, double Seconds)
{
	const int Steps = static_cast<int>(Seconds / Sim.StepSeconds() + 0.5);
	for (int I = 0; I < Steps; ++I)
	{
		Sim.Step();
	}
}

// Steps until Predicate is true or MaxSeconds pass; returns the time, or a negative value.
template <typename F>
double TimeUntil(FReferenceSim& Sim, double MaxSeconds, F&& Predicate)
{
	while (Sim.Time() < MaxSeconds)
	{
		Sim.Step();
		if (Predicate())
		{
			return Sim.Time();
		}
	}
	return -1.0;
}
} // namespace Kinetic::Test
