#include "Kinetic/Impact.h"

#include <algorithm>
#include <cassert>
#include <iterator>

namespace Kinetic
{
namespace
{
constexpr double TimeEpsilon = 1e-9;
constexpr double DeepWaterImpulseMul = 0.5;  // Blasts under more than 1 m of water, 6.4

bool Reaches(const std::optional<double>& Threshold, double Value, double Multiplier)
{
	return Threshold.has_value() && Value >= *Threshold * Multiplier;
}

EImpactTable TableFor(EContactKind Kind)
{
	return Kind == EContactKind::Kinematic ? EImpactTable::PlayerVsKinematic
	       : Kind == EContactKind::Static  ? EImpactTable::PlayerVsStatic
	                                       : EImpactTable::ObjectVsPlayer;
}
} // namespace

EOutcome MoreSevere(EOutcome A, EOutcome B) { return std::max(A, B); }

double SupportShare(const FVec3& Normal)
{
	return Normal.Length() > 0.0 ? std::max(-Normal.SafeNormal().Dot(Up), 0.0) : 1.0;
}

EOutcome Classify(EImpactTable Table, double Value, ESurface Surface, const FKineticTables& Tables)
{
	const FThresholds& T = Tables.Impact(Table);
	const double Multiplier = Tables.SurfaceMultiplier(Surface);
	if (Reaches(T.Stall, Value, Multiplier))
	{
		return EOutcome::Stall;
	}
	if (Reaches(T.Flip, Value, Multiplier))
	{
		return EOutcome::Flip;
	}
	return Reaches(T.Stagger, Value, Multiplier) ? EOutcome::Stagger : EOutcome::None;
}

EOutcome ClassifyPlayerVsPlayer(double OtherMassKg, double RelativeSpeed, const FKineticTables& Tables)
{
	const EOutcome ByImpulse = Classify(EImpactTable::ObjectVsPlayer, ImpactImpulse(OtherMassKg, RelativeSpeed),
	                                    ESurface::Hard, Tables);
	const EOutcome ByFloor = Classify(EImpactTable::PlayerVsPlayerFloor, RelativeSpeed, ESurface::Hard, Tables);
	return MoreSevere(ByImpulse, ByFloor);
}

FBlastResult ClassifyBlast(EVolatile Kind, double DistanceM, bool bDeepUnderwater, const FKineticTables& Tables)
{
	const FVolatileTuning& V = Tables.Volatile(Kind);
	FBlastResult Out;
	if (V.BlastRadius <= 0.0 || DistanceM > V.BlastRadius)
	{
		return Out;
	}
	const double Falloff = 1.0 - DistanceM / V.BlastRadius;
	Out.ImpulseNs = V.BlastImpulseNs * Falloff * (bDeepUnderwater ? DeepWaterImpulseMul : 1.0);
	if (!bDeepUnderwater && V.BlastStallRadius > 0.0 && DistanceM <= V.BlastStallRadius)
	{
		Out.Outcome = EOutcome::Stall;
	}
	else if (!bDeepUnderwater && V.BlastFlipRadius > 0.0 && DistanceM <= V.BlastFlipRadius)
	{
		Out.Outcome = EOutcome::Flip;
	}
	else
	{
		Out.Outcome = Classify(EImpactTable::ObjectVsPlayer, Out.ImpulseNs, ESurface::Hard, Tables);
	}
	return Out;
}

double FImpactMonitor::Measure(const FContact& Summed, double Dt) const
{
	switch (Summed.Kind)
	{
	case EContactKind::Static:
	case EContactKind::Kinematic:
		return Summed.RelativeSpeed;
	case EContactKind::Object:
	{
		const double Support = Summed.OtherMassKg * Tables.Globals().Gravity * Dt * SupportShare(Summed.Normal);
		return std::max(Summed.ImpulseNs - Support, 0.0);
	}
	case EContactKind::Player:
		return ImpactImpulse(Summed.OtherMassKg, Summed.RelativeSpeed);
	}
	return 0.0;
}

EOutcome FImpactMonitor::ClassifyBody(const FContact& Summed, double MeasureValue) const
{
	switch (Summed.Kind)
	{
	case EContactKind::Static:
	case EContactKind::Kinematic:
		return Classify(TableFor(Summed.Kind), MeasureValue, Summed.Surface, Tables);
	case EContactKind::Object:
		return Classify(EImpactTable::ObjectVsPlayer, MeasureValue, Summed.Surface, Tables);
	case EContactKind::Player:
		return ClassifyPlayerVsPlayer(Summed.OtherMassKg, Summed.RelativeSpeed, Tables);
	}
	return EOutcome::None;
}

EOutcome FImpactMonitor::EndStep(const std::vector<FContact>& Contacts, double Now, double Dt)
{
	std::map<int, FContact> PerBody;
	for (const FContact& C : Contacts)
	{
		// Without its normal an object's resting weight would read as a hit (player-physics 4.1).
		assert(C.Kind != EContactKind::Object || C.Normal.Length() > 0.0);
		auto [It, bNew] = PerBody.try_emplace(C.OtherId, C);
		if (!bNew)
		{
			It->second.ImpulseNs += C.ImpulseNs;
			It->second.RelativeSpeed = std::max(It->second.RelativeSpeed, C.RelativeSpeed);
			It->second.Normal = It->second.Normal + C.Normal;  // Averaged by SafeNormal in Measure
		}
	}

	const bool bSettling = SettleStepsLeft > 0;
	SettleStepsLeft = std::max(SettleStepsLeft - 1, 0);
	EOutcome Worst = EOutcome::None;
	for (const auto& [OtherId, Summed] : PerBody)
	{
		const double Value = Measure(Summed, Dt);
		const EOutcome Outcome = ClassifyBody(Summed, Value);
		if (Outcome == EOutcome::None || (bSettling && Outcome < EOutcome::Flip))
		{
			continue;
		}
		const FLastHit& Last = LastHits[OtherId];
		const bool bRecent = Now - Last.Time < DebounceSeconds - TimeEpsilon;
		const bool bEscalated = Value >= DebounceEscalation * Last.Measure;
		if (Outcome != EOutcome::Stall && bRecent && !bEscalated)
		{
			continue;
		}
		LastHits[OtherId] = {Now, Value};
		Worst = MoreSevere(Worst, Outcome);
	}
	return Worst;
}

bool FCrushDetector::IsPinned(const std::vector<FCrushContact>& Contacts) const
{
	struct FBodyPress
	{
		FVec3 Force;  // Summed over the body's contact points
		bool bKinematicOrHeavy = false;
	};
	std::map<int, FBodyPress> PerBody;
	for (const FCrushContact& C : Contacts)
	{
		FBodyPress& Press = PerBody[C.OtherId];
		Press.Force = Press.Force + C.Normal.SafeNormal() * C.ForceN;
		Press.bKinematicOrHeavy = Press.bKinematicOrHeavy || C.bKinematicOrHeavy;
	}
	const double Threshold = Tables.Crush().ForceN;
	for (auto A = PerBody.begin(); A != PerBody.end(); ++A)
	{
		for (auto B = std::next(A); B != PerBody.end(); ++B)
		{
			const FBodyPress& First = A->second;
			const FBodyPress& Second = B->second;
			const bool bOpposing = First.Force.SafeNormal().Dot(Second.Force.SafeNormal()) < OpposingDot;
			const bool bStrong = First.Force.Length() >= Threshold * (1.0 - 1e-9) &&
			                     Second.Force.Length() >= Threshold * (1.0 - 1e-9);
			if (bOpposing && bStrong && (First.bKinematicOrHeavy || Second.bKinematicOrHeavy))
			{
				return true;
			}
		}
	}
	return false;
}

bool FCrushDetector::Update(const std::vector<FCrushContact>& Contacts, double Dt)
{
	Timer = IsPinned(Contacts) ? Timer + Dt : std::max(Timer - DecayRate * Dt, 0.0);
	return Timer + TimeEpsilon >= Tables.Crush().Seconds;
}
} // namespace Kinetic
