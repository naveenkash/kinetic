// The Cutter: warm-up timing per class and variant, and the cut test (bible/05 sections 4.1
// and 4.2). A swing cuts any line whose segment passes within reach and within the cone around
// the camera's forward vector; the cone angle is the full apex angle (70° is 35° either side).
#pragma once

#include "Kinetic/Tables.h"
#include "Kinetic/Vec3.h"

#include <optional>

namespace Kinetic
{
double CutterWarmupSeconds(ECutterVariant Variant, EHarnessClass Class, const FKineticTables& Tables);

struct FCutQuery
{
	FVec3 Eye;          // Blade origin, m
	FVec3 Forward;      // Camera forward, unit vector
	double Reach = 0.0;
	double ConeDeg = 0.0;
};

// The parameter t in [0, 1] along A to B of the cuttable point nearest the blade, or nothing.
std::optional<double> FindCutPoint(const FCutQuery& Query, const FVec3& SegmentA, const FVec3& SegmentB);

inline bool CanCut(const FCutQuery& Query, const FVec3& SegmentA, const FVec3& SegmentB)
{
	return FindCutPoint(Query, SegmentA, SegmentB).has_value();
}
} // namespace Kinetic
