#include "Kinetic/Kinematics.h"

#include <algorithm>
#include <cmath>

namespace Kinetic
{
namespace
{
constexpr double Pi = 3.14159265358979323846;
constexpr double TimeEpsilon = 1e-9;  // A departure at exactly T counts as at or before T
constexpr double VelocityProbe = 1e-3;  // s, for finite-difference velocities
}

FVec3 Cross(const FVec3& A, const FVec3& B)
{
	return {A.Y * B.Z - A.Z * B.Y, A.Z * B.X - A.X * B.Z, A.X * B.Y - A.Y * B.X};
}

// Rodrigues' rotation of V about the unit Axis by Angle.
static FVec3 RotateAbout(const FVec3& V, const FVec3& Axis, double Angle)
{
	const FVec3 K = Axis.SafeNormal();
	const double C = std::cos(Angle);
	const double S = std::sin(Angle);
	return V * C + Cross(K, V) * S + K * (K.Dot(V) * (1.0 - C));
}

FVec3 FPose::Rotate(const FVec3& V) const { return RotateAbout(V, Axis, Angle); }
FVec3 FPose::Unrotate(const FVec3& V) const { return RotateAbout(V, Axis, -Angle); }

FPolylinePath::FPolylinePath(std::vector<FVec3> InPoints) : Points(std::move(InPoints))
{
	Cumulative.push_back(0.0);
	for (size_t I = 1; I < Points.size(); ++I)
	{
		Total += (Points[I] - Points[I - 1]).Length();
		Cumulative.push_back(Total);
	}
}

FVec3 FPolylinePath::PointAt(double Distance) const
{
	if (Points.size() < 2 || Distance <= 0.0)
	{
		return Points.empty() ? FVec3{} : Points.front();
	}
	if (Distance >= Total)
	{
		return Points.back();
	}
	const size_t I = static_cast<size_t>(std::upper_bound(Cumulative.begin(), Cumulative.end(), Distance) - Cumulative.begin());
	const double Piece = Cumulative[I] - Cumulative[I - 1];
	const double Alpha = Piece > 0.0 ? (Distance - Cumulative[I - 1]) / Piece : 0.0;
	return Points[I - 1] + (Points[I] - Points[I - 1]) * Alpha;
}

FVec3 FPolylinePath::TangentAt(double Distance) const
{
	if (Points.size() < 2)
	{
		return {};
	}
	const double D = std::clamp(Distance, 0.0, Total);
	size_t I = static_cast<size_t>(std::upper_bound(Cumulative.begin(), Cumulative.end(), D) - Cumulative.begin());
	I = std::clamp<size_t>(I, 1, Points.size() - 1);
	return (Points[I] - Points[I - 1]).SafeNormal();
}

std::optional<double> FTimetable::LastDepartureAt(double T) const
{
	std::optional<double> Last;
	for (const FDepartureSegment& Seg : Segments)
	{
		if (T + TimeEpsilon < Seg.First)
		{
			continue;
		}
		double Dep = Seg.First;
		if (Seg.Period > 0.0)
		{
			// The grid index at or before T, robust to T landing exactly on a departure.
			double K = std::floor((T - Seg.First) / Seg.Period);
			if (Seg.First + (K + 1.0) * Seg.Period <= T + TimeEpsilon)
			{
				K += 1.0;
			}
			// Departures stop before Until: step back to the last one that ran.
			if (Seg.Until < 1e299)
			{
				K = std::min(K, std::ceil((Seg.Until - TimeEpsilon - Seg.First) / Seg.Period));
			}
			while (K > 0.0 && Seg.First + K * Seg.Period >= Seg.Until - TimeEpsilon)
			{
				K -= 1.0;
			}
			if (K < 0.0)
			{
				continue;  // A segment authored with Until before First never departs
			}
			Dep = Seg.First + K * Seg.Period;
		}
		if (Dep >= Seg.Until - TimeEpsilon || Dep > T + TimeEpsilon)
		{
			continue;
		}
		if (!Last || Dep > *Last)
		{
			Last = Dep;
		}
	}
	return Last;
}

std::vector<double> FTimetable::DeparturesBetween(double From, double To) const
{
	std::vector<double> Out;
	for (const FDepartureSegment& Seg : Segments)
	{
		for (int K = 0;; ++K)
		{
			const double Dep = Seg.First + K * Seg.Period;
			if (Dep >= To || Dep >= Seg.Until - TimeEpsilon)
			{
				break;
			}
			if (Dep >= From)
			{
				Out.push_back(Dep);
			}
			if (Seg.Period <= 0.0)
			{
				break;
			}
		}
	}
	std::sort(Out.begin(), Out.end());
	return Out;
}

std::optional<double> FRailRun::NoseAt(double T) const
{
	const std::optional<double> Dep = Timetable.LastDepartureAt(T);
	if (!Dep)
	{
		return std::nullopt;
	}
	const double S = (T - *Dep) * Speed;
	return S >= 0.0 && S <= Path.Length() + TrainLength ? std::optional<double>(S) : std::nullopt;
}

std::optional<FPose> FRailRun::PoseAt(double T) const
{
	const std::optional<double> S = NoseAt(T);
	if (!S)
	{
		return std::nullopt;
	}
	// Past the end the nose runs on along the last piece while the tail clears.
	const double Along = std::min(*S, Path.Length());
	const FVec3 Tangent = Path.TangentAt(Along);
	const FVec3 Nose = Path.PointAt(Along) + Tangent * (*S - Along);
	return FPose{Nose, Up, std::atan2(Tangent.Y, Tangent.X)};
}

FVec3 FRailRun::VelocityAt(double T) const
{
	const std::optional<double> S = NoseAt(T);
	return S ? Path.TangentAt(std::min(*S, Path.Length())) * Speed : FVec3{};
}

std::optional<std::pair<FPose, FPose>> FRailRun::SweepPoses(double T0, double T1) const
{
	const std::optional<FPose> From = PoseAt(T0);
	const std::optional<FPose> To = PoseAt(T1);
	const std::optional<double> Dep0 = Timetable.LastDepartureAt(T0);
	const std::optional<double> Dep1 = Timetable.LastDepartureAt(T1);
	// Both ends on the same run: a new departure inside the step is a fresh spawn, not a sweep.
	if (!From || !To || !Dep0 || !Dep1 || std::abs(*Dep0 - *Dep1) > 1e-9)
	{
		return std::nullopt;
	}
	return std::pair{*From, *To};
}

FCycle::FCycle(std::vector<FKeyframe> InKeys, double InPeriod, double InPhaseAtZero)
    : Keys(std::move(InKeys)), Period(InPeriod), Phase(InPhaseAtZero)
{
	// Stable: two keys at one time are an authored step, kept in order.
	std::stable_sort(Keys.begin(), Keys.end(), [](const FKeyframe& A, const FKeyframe& B) { return A.Time < B.Time; });
}

double FCycle::CycleTimeAt(double T) const
{
	double C = std::fmod(T + Phase, Period);
	C = C < 0.0 ? C + Period : C;
	return C >= Period ? 0.0 : C;  // -1e-20 + Period rounds to Period
}

FVec3 FCycle::PositionAt(double T) const
{
	if (Keys.empty())
	{
		return {};
	}
	const double C = CycleTimeAt(T);
	// Between the last key and the first key of the next cycle, wrap.
	for (size_t I = 0; I < Keys.size(); ++I)
	{
		const FKeyframe& A = Keys[I];
		const FKeyframe& B = I + 1 < Keys.size() ? Keys[I + 1] : FKeyframe{Keys.front().Time + Period, Keys.front().Position};
		const double CA = C < Keys.front().Time ? C + Period : C;
		if (CA >= A.Time && CA <= B.Time)
		{
			const double Span = B.Time - A.Time;
			return Span > 0.0 ? A.Position + (B.Position - A.Position) * ((CA - A.Time) / Span) : A.Position;
		}
	}
	return Keys.back().Position;
}

FVec3 FCycle::VelocityAt(double T) const
{
	return (PositionAt(T + VelocityProbe) - PositionAt(T - VelocityProbe)) / (2.0 * VelocityProbe);
}

double FSpin::AngleAt(double T) const { return 2.0 * Pi * T / PeriodSeconds; }

static FPose CarryAbout(const FPose& Rest, const FVec3& Pivot, const FVec3& Axis, double Angle)
{
	// Rest's own rotation is kept when it shares the axis; KineticCore's kinematics rotate about one axis.
	return {Pivot + RotateAbout(Rest.Position - Pivot, Axis, Angle), Axis, Rest.Angle + Angle};
}

FPose FSpin::PoseOf(const FPose& Rest, double T) const { return CarryAbout(Rest, Pivot, Axis, AngleAt(T)); }

double FRock::AngleAt(double T) const { return AmplitudeRadians * std::sin(2.0 * Pi * T / PeriodSeconds); }

FPose FRock::PoseOf(const FPose& Rest, double T) const { return CarryAbout(Rest, Pivot, Axis, AngleAt(T)); }

void FHaulSled::Step(const FVec3& LineForce, double Dt)
{
	const double FRail = LineForce.Dot(Rail.TangentAt(S));
	const double FNet = FRail - PawlDragN;  // The counterbalance cancels gravity
	if (FNet <= 0.0 && V <= 0.0)
	{
		V = 0.0;  // The pawl holds: no backward motion
	}
	else
	{
		V = std::max(0.0, V + FNet / MassKg * Dt);
	}
	S = std::clamp(S + V * Dt, 0.0, Rail.Length());
	if (S >= Rail.Length())
	{
		V = 0.0;
	}
}

int FHaulSled::TeethPassed() const { return static_cast<int>(std::floor(S / ToothSpacingM + 1e-9)); }

void FServerClock::Sample(double ServerTime, double RoundTrip, double LocalNow)
{
	const double Measured = ServerTime + RoundTrip / 2.0 - LocalNow;
	if (!bSynced)
	{
		Offset = TargetOffset = Measured;
		bSynced = true;
		return;
	}
	if (std::abs(Measured - TargetOffset) > ResnapThreshold)
	{
		// A real step shows in two samples that agree; a lone spike does not.
		if (bBigPending && std::abs(Measured - PendingMeasured) <= ResnapThreshold)
		{
			Offset = Now(LocalNow) - LocalNow;  // Start from where the estimate is now
			TargetOffset = Measured;
			SlewFrom = Offset;
			SlewStartLocal = LocalNow;
			bSlewing = true;
			bBigPending = false;
		}
		else
		{
			bBigPending = true;
			PendingMeasured = Measured;
		}
		return;
	}
	bBigPending = false;
	TargetOffset += Smoothing * (Measured - TargetOffset);
	if (!bSlewing)
	{
		Offset = TargetOffset;
	}
}

double FServerClock::Now(double LocalNow)
{
	if (!bSynced)
	{
		return LocalNow;  // No estimate yet; nothing remembered
	}
	if (bSlewing)
	{
		const double Alpha = std::clamp((LocalNow - SlewStartLocal) / ResnapSeconds, 0.0, 1.0);
		Offset = SlewFrom + (TargetOffset - SlewFrom) * Alpha;
		bSlewing = Alpha < 1.0;
	}
	LastNow = std::max(LastNow, LocalNow + Offset);
	return LastNow;
}

// Slab test of segment A-B against the box in the pose's frame.
std::optional<double> SegmentHitsBox(const FBox& Box, const FPose& Pose, const FVec3& A, const FVec3& B)
{
	const FVec3 P = Pose.ToLocal(A) - Box.Center;
	const FVec3 D = Pose.ToLocal(B) - Box.Center - P;
	double Enter = 0.0;
	double Exit = 1.0;
	const double Starts[3] = {P.X, P.Y, P.Z};
	const double Dirs[3] = {D.X, D.Y, D.Z};
	const double Halves[3] = {Box.HalfExtents.X, Box.HalfExtents.Y, Box.HalfExtents.Z};
	for (int Axis = 0; Axis < 3; ++Axis)
	{
		if (std::abs(Dirs[Axis]) < 1e-12)
		{
			if (std::abs(Starts[Axis]) > Halves[Axis])
			{
				return std::nullopt;
			}
			continue;
		}
		double T0 = (-Halves[Axis] - Starts[Axis]) / Dirs[Axis];
		double T1 = (Halves[Axis] - Starts[Axis]) / Dirs[Axis];
		if (T0 > T1)
		{
			std::swap(T0, T1);
		}
		Enter = std::max(Enter, T0);
		Exit = std::min(Exit, T1);
		if (Enter > Exit)
		{
			return std::nullopt;
		}
	}
	return Enter;
}

// The angle turns the short way: a heading that wraps from +π to -π in a step is a tiny turn.
FPose Lerp(const FPose& From, const FPose& To, double Alpha)
{
	const double Turn = std::remainder(To.Angle - From.Angle, 2.0 * Pi);
	return {From.Position + (To.Position - From.Position) * Alpha, To.Axis, From.Angle + Turn * Alpha};
}

std::optional<double> SweptSever(const FBox& Box, const FPose& From, const FPose& To, int Substeps, const FVec3& A,
                                 const FVec3& B)
{
	const int N = std::max(1, Substeps);
	for (int I = 1; I <= N; ++I)
	{
		const std::optional<double> Hit = SegmentHitsBox(Box, Lerp(From, To, static_cast<double>(I) / N), A, B);
		if (Hit)
		{
			return Hit;
		}
	}
	return std::nullopt;
}
} // namespace Kinetic
