#include "Kinetic/Cutter.h"

#include <algorithm>
#include <cmath>
#include <numbers>
#include <vector>

namespace Kinetic
{
namespace
{
constexpr double Epsilon = 1e-9;

// Real roots of A t² + B t + C = 0 (linear when A is ~0).
std::vector<double> Roots(double A, double B, double C)
{
	if (std::abs(A) < Epsilon)
	{
		return std::abs(B) < Epsilon ? std::vector<double>{} : std::vector<double>{-C / B};
	}
	const double Disc = B * B - 4.0 * A * C;
	if (Disc < 0.0)
	{
		return {};
	}
	const double Sqrt = std::sqrt(Disc);
	return {(-B - Sqrt) / (2.0 * A), (-B + Sqrt) / (2.0 * A)};
}

struct FSegmentView
{
	FVec3 Start;  // Segment start relative to the eye
	FVec3 Dir;    // Segment end minus start
	FVec3 At(double T) const { return Start + Dir * T; }
};

bool IsCuttable(const FCutQuery& Q, const FVec3& Offset, double CosHalf)
{
	const double Distance = Offset.Length();
	if (Distance <= Epsilon)
	{
		return true;  // The blade is on the line
	}
	return Distance <= Q.Reach + Epsilon && Offset.Dot(Q.Forward) >= CosHalf * Distance - Epsilon;
}

// Every t where the cuttable set can start or end: the reach sphere, the cone and its apex
// plane, the segment ends, and the point nearest the eye. The cuttable set is a union of
// intervals bounded by these, so testing them (and the midpoints between) finds it exactly.
std::vector<double> Candidates(const FSegmentView& S, const FCutQuery& Q, double CosHalf)
{
	const double A = S.Dir.Dot(S.Dir);
	const double B = 2.0 * S.Start.Dot(S.Dir);
	const double C0 = S.Start.Dot(S.Start);
	const double K0 = S.Start.Dot(Q.Forward);
	const double K1 = S.Dir.Dot(Q.Forward);
	const double C2 = CosHalf * CosHalf;

	std::vector<double> T{0.0, 1.0};
	const auto Add = [&T](const std::vector<double>& Values) { T.insert(T.end(), Values.begin(), Values.end()); };
	Add(Roots(A, B, C0 - Q.Reach * Q.Reach));
	Add(Roots(K1 * K1 - C2 * A, 2.0 * K0 * K1 - C2 * B, K0 * K0 - C2 * C0));
	if (std::abs(K1) > Epsilon)
	{
		T.push_back(-K0 / K1);
	}
	if (A > Epsilon)
	{
		T.push_back(-B / (2.0 * A));
	}

	std::vector<double> Clamped;
	for (const double Value : T)
	{
		Clamped.push_back(std::clamp(Value, 0.0, 1.0));
	}
	std::sort(Clamped.begin(), Clamped.end());
	std::vector<double> WithMidpoints;
	for (size_t I = 0; I < Clamped.size(); ++I)
	{
		WithMidpoints.push_back(Clamped[I]);
		if (I + 1 < Clamped.size())
		{
			WithMidpoints.push_back(0.5 * (Clamped[I] + Clamped[I + 1]));
		}
	}
	return WithMidpoints;
}
} // namespace

double CutterWarmupSeconds(ECutterVariant Variant, EHarnessClass Class, const FKineticTables& Tables)
{
	return Tables.Cutter(Variant).Warmup + Tables.Harness(Class).CutterWarmupOffset;
}

std::optional<double> FindCutPoint(const FCutQuery& Query, const FVec3& SegmentA, const FVec3& SegmentB)
{
	const double CosHalf = std::cos(Query.ConeDeg * 0.5 * std::numbers::pi / 180.0);
	const FSegmentView Segment{SegmentA - Query.Eye, SegmentB - SegmentA};
	std::optional<double> Best;
	double BestDistance = 0.0;
	for (const double T : Candidates(Segment, Query, CosHalf))
	{
		const FVec3 Offset = Segment.At(T);
		if (!IsCuttable(Query, Offset, CosHalf))
		{
			continue;
		}
		const double Distance = Offset.Length();
		if (!Best || Distance < BestDistance)
		{
			Best = T;
			BestDistance = Distance;
		}
	}
	return Best;
}
} // namespace Kinetic
