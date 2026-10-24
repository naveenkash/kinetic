#include "Kinetic/ForceVolumes.h"

#include "Kinetic/MassRules.h"

#include <algorithm>
#include <cmath>

namespace Kinetic
{
namespace
{
// The horizontal offset from a vertical axis through Axis to P.
FVec3 Radial(const FVec3& Axis, const FVec3& P) { return {P.X - Axis.X, P.Y - Axis.Y, 0.0}; }

double LinearFalloff(double Distance, double Radius)
{
	return Radius > 0.0 ? std::clamp(1.0 - Distance / Radius, 0.0, 1.0) : 0.0;
}

double Wrap(double T, double Period)
{
	const double C = std::fmod(T, Period);
	return C < 0.0 ? C + Period : C;
}
} // namespace

bool VolumeMoves(EVolumeTarget Target)
{
	return Target == EVolumeTarget::Player || Target == EVolumeTarget::LooseObject ||
	       Target == EVolumeTarget::LineHeldObject;
}

double FGustCurve::ValueAt(double T) const
{
	if (Keys.empty())
	{
		return 1.0;
	}
	const double C = Wrap(T, PeriodSeconds);
	// Before the first key the curve is still on its way from the last key of the cycle before.
	const double At = C < Keys.front().first ? C + PeriodSeconds : C;
	for (size_t I = 0; I < Keys.size(); ++I)
	{
		const auto& [T0, V0] = Keys[I];
		const auto [T1, V1] = I + 1 < Keys.size() ? Keys[I + 1] : std::pair{Keys.front().first + PeriodSeconds, Keys.front().second};
		if (At >= T0 && At <= T1)
		{
			return T1 > T0 ? V0 + (V1 - V0) * (At - T0) / (T1 - T0) : V1;
		}
	}
	return Keys.back().second;
}

FVec3 FUpdraft::ARefAt(const FVec3& P, double T) const
{
	if (!Band.Contains(P.Z))
	{
		return {};
	}
	return Up * (CoreARef * LinearFalloff(Radial(Axis, P).Length(), Radius) * Gust.ValueAt(T));
}

std::vector<double> FUpdraft::GustStarts(double From, double To) const
{
	std::vector<double> Out;
	const size_t N = Gust.Keys.size();
	for (size_t I = 0; I < N; ++I)
	{
		// Each edge from a key at 0 to the next key above 0, the wrap from last to first included.
		const auto& Prev = Gust.Keys[I];
		const auto& Next = Gust.Keys[(I + 1) % N];
		if (N > 1 && Prev.second <= 0.0 && Next.second > 0.0)
		{
			const double Phase = Prev.first;
			for (double Start = std::ceil((From - Phase) / Gust.PeriodSeconds) * Gust.PeriodSeconds + Phase; Start < To;
			     Start += Gust.PeriodSeconds)
			{
				Out.push_back(Start);
			}
		}
	}
	std::sort(Out.begin(), Out.end());
	return Out;
}

double FPulse::LastStartAt(double T) const
{
	if (T < First)
	{
		return -1.0;
	}
	return First + std::floor((T - First) / Period + 1e-9) * Period;
}

FVec3 FPulse::ARefAt(const FVec3& P, double T) const
{
	const double Start = LastStartAt(T);
	if (Start < 0.0)
	{
		return {};
	}
	const double Into = T - Start;
	if (!Band.Contains(P.Z))
	{
		return {};
	}
	const FVec3 Out = Radial(Axis, P);
	const double Fall = LinearFalloff(Out.Length(), RimRadius);
	if (Into < ShoveSeconds)
	{
		return Out.SafeNormal() * (ShoveARef * Fall);
	}
	if (Into < ShoveSeconds + InhaleSeconds)
	{
		return Out.SafeNormal() * (-InhaleARef * Fall);
	}
	return {};
}

FVec3 FWaterRush::ARefAt(const FVec3& P, double T, double RuptureTime) const
{
	const double Into = T - (RuptureTime + DelaySeconds);
	const bool bInside = P.X >= Min.X && P.X <= Max.X && P.Y >= Min.Y && P.Y <= Max.Y && P.Z >= Min.Z && P.Z <= Max.Z;
	return bInside && Into >= 0.0 && Into < DurationSeconds ? Direction.SafeNormal() * ARef : FVec3{};
}

FVec3 FGasJet::ARefAt(const FVec3& P, const FVec3& BodyVelocity, double T, double TriggerTime) const
{
	const double Into = T - TriggerTime;
	if (Into < 0.0 || Into >= Seconds)
	{
		return {};
	}
	const FVec3 Dir = Axis.SafeNormal();
	const FVec3 Rel = P - Apex;
	const double Along = Rel.Dot(Dir);
	if (Along < 0.0 || Along > Length)
	{
		return {};
	}
	const double Off = (Rel - Dir * Along).Length();
	if (Off > Along * std::tan(HalfAngleRadians))
	{
		return {};
	}
	return BodyVelocity.Dot(Dir) >= SpeedCap ? FVec3{} : Dir * ARef;
}

bool FFirePatch::Contains(const FVec3& P, double T, double IgniteTime) const
{
	const double Into = T - IgniteTime;
	const FVec3 Flat{P.X - Center.X, P.Y - Center.Y, 0.0};
	const bool bInHeight = P.Z >= Center.Z && P.Z <= Center.Z + Height;
	return Into >= 0.0 && Into < Seconds && bInHeight && Flat.Length() <= Radius;
}

FVec3 FFirePatch::ARefAt(const FVec3& P, const FVec3& BodyVelocity, double T, double IgniteTime) const
{
	if (!Contains(P, T, IgniteTime))
	{
		return {};
	}
	const FVec3 Out = FVec3{P.X - Center.X, P.Y - Center.Y, 0.0}.SafeNormal();
	if (Out.Dot(Out) == 0.0)
	{
		return {};  // Dead center: no outward direction
	}
	return BodyVelocity.Dot(Out) >= TargetSpeed ? FVec3{} : Out * ARef;
}

FVec3 BodyAcceleration(const FVec3& ARef, double MassKg, EVolumeTarget Target, const FSpoolGlobals& Globals)
{
	const double Magnitude = ARef.Length();
	if (!VolumeMoves(Target) || Magnitude <= 0.0 || MassKg <= 0.0)
	{
		return {};
	}
	return ARef / Magnitude * ForceVolumeAcceleration(Magnitude, MassKg, Globals);
}
} // namespace Kinetic
