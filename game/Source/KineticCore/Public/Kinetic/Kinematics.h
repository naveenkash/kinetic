// Kinematic actors (docs/tech/kinematics-and-force-volumes.md section 1; bible/05 6.7): every
// kinematic but the Haul sled is a function of one number, server match time t, so server and
// clients evaluate the same Pose(t) and nothing is replicated per frame. Also the Haul sled's
// ratchet rule (1.1a), the client's estimate of server time (1.2), and the swept test by which
// a kinematic severs any line it passes through (1.3; bible/05 3.5).
#pragma once

#include "Kinetic/Api.h"

#include "Kinetic/Vec3.h"

#include <optional>
#include <utility>
#include <vector>

namespace Kinetic
{
// A rigid pose: position, and a rotation of Angle radians about the unit Axis.
struct KINETICCORE_API FPose
{
	FVec3 Position;
	FVec3 Axis = Up;
	double Angle = 0.0;

	FVec3 Rotate(const FVec3& V) const;         // Local direction to world
	FVec3 Unrotate(const FVec3& V) const;       // World direction to local
	FVec3 ToWorld(const FVec3& Local) const { return Position + Rotate(Local); }
	FVec3 ToLocal(const FVec3& World) const { return Unrotate(World - Position); }
};

KINETICCORE_API FVec3 Cross(const FVec3& A, const FVec3& B);

// A path of straight pieces walked at constant speed (a trench, a rail).
class KINETICCORE_API FPolylinePath
{
public:
	explicit FPolylinePath(std::vector<FVec3> InPoints);

	double Length() const { return Total; }
	FVec3 PointAt(double Distance) const;    // Clamped to the ends
	FVec3 TangentAt(double Distance) const;

private:
	std::vector<FVec3> Points;
	std::vector<double> Cumulative;
	double Total = 0.0;
};

// Departure times for one kinematic (a Hauler's trench): a list of segments, each a first
// departure and a period, running until the next segment begins. A Shift Change is a new
// segment (the Yard: 45 s, then 22.5 s from e 80, then 45 s again from e 140).
struct FDepartureSegment
{
	double First = 0.0;   // First departure in this segment, s
	double Period = 0.0;  // 0: a single departure
	double Until = 1e300; // Departures stop before this time
};

class KINETICCORE_API FTimetable
{
public:
	explicit FTimetable(std::vector<FDepartureSegment> InSegments) : Segments(std::move(InSegments)) {}

	// The latest departure at or before T, if any.
	std::optional<double> LastDepartureAt(double T) const;
	// Every departure in [From, To), ascending.
	std::vector<double> DeparturesBetween(double From, double To) const;

private:
	std::vector<FDepartureSegment> Segments;
};

// A Hauler: departs on its timetable and runs its path at constant speed, nose first, until its
// tail clears the far end; between runs it is out of the arena (in the plenum) and has no pose.
// The pose is at the nose, facing along the path (a rotation about Up), so a box behind the
// nose is local -X whichever way the trench runs.
class KINETICCORE_API FRailRun
{
public:
	FRailRun(FPolylinePath InPath, double InSpeed, FTimetable InTimetable, double InTrainLength = 14.0)
	    : Path(std::move(InPath)), Speed(InSpeed), Timetable(std::move(InTimetable)), TrainLength(InTrainLength)
	{
	}

	double NoseOutSeconds() const { return Path.Length() / Speed; }               // 13.3 s in the Yard
	double ClearSeconds() const { return (Path.Length() + TrainLength) / Speed; }  // 14.5 s, the tail out
	bool IsRunning(double T) const { return NoseAt(T).has_value(); }
	// Distance along the path of the nose at T (past the end while the tail clears), if running.
	std::optional<double> NoseAt(double T) const;
	std::optional<FPose> PoseAt(double T) const;
	FVec3 VelocityAt(double T) const;  // Zero while parked
	// The poses to sweep for the sever test over [T0, T1]: only while the train is in the arena
	// at both ends, so a despawn is never swept across the trench.
	std::optional<std::pair<FPose, FPose>> SweepPoses(double T0, double T1) const;
	const FTimetable& Schedule() const { return Timetable; }
	const FPolylinePath& Route() const { return Path; }

private:
	FPolylinePath Path;
	double Speed;
	FTimetable Timetable;
	double TrainLength;
};

// A cyclic keyframed motion (the Gantry's 120 s cycle): positions at times within the cycle,
// linearly interpolated, repeating every Period, with the cycle's phase at t = 0.
struct FKeyframe
{
	double Time = 0.0;
	FVec3 Position;
};

class KINETICCORE_API FCycle
{
public:
	FCycle(std::vector<FKeyframe> InKeys, double InPeriod, double InPhaseAtZero);

	double CycleTimeAt(double T) const;  // c in [0, Period)
	FVec3 PositionAt(double T) const;
	FVec3 VelocityAt(double T) const;

private:
	std::vector<FKeyframe> Keys;
	double Period;
	double Phase;
};

// Steady rotation about an axis through a pivot (the Anchor's Maintenance Ring, 120 s per turn).
struct KINETICCORE_API FSpin
{
	FVec3 Pivot;
	FVec3 Axis = Up;
	double PeriodSeconds = 120.0;
	double AngleAt(double T) const;
	FPose PoseOf(const FPose& Rest, double T) const;  // Rest pose carried round the pivot
};

// A rocking rotation (Ballast's Breathing hull, ±6° over 90 s): Amplitude × sin(2πt / Period).
struct KINETICCORE_API FRock
{
	FVec3 Pivot;
	FVec3 Axis{1.0, 0.0, 0.0};
	double AmplitudeRadians = 6.0 * 3.14159265358979323846 / 180.0;
	double PeriodSeconds = 90.0;
	double AngleAt(double T) const;
	FPose PoseOf(const FPose& Rest, double T) const;
};

// The Ballast Haul sled (1.1a): one free coordinate S along a one-way rail, driven by the rail
// component of the line forces on it, against pawl drag, never backward.
class KINETICCORE_API FHaulSled
{
public:
	explicit FHaulSled(FPolylinePath InRail) : Rail(std::move(InRail)) {}

	// One physics step with the summed active line force on the sled and spool.
	void Step(const FVec3& LineForce, double Dt);
	void Reset() { S = 0.0; V = 0.0; }

	double Distance() const { return S; }
	double Speed() const { return V; }
	FVec3 Position() const { return Rail.PointAt(S); }
	// Rail teeth passed so far (a pawl clack each, the sled's tell).
	int TeethPassed() const;

	static constexpr double MassKg = 4000.0;
	static constexpr double PawlDragN = 200.0;
	static constexpr double ToothSpacingM = 0.25;

private:
	FPolylinePath Rail;
	double S = 0.0;
	double V = 0.0;
};

// The client's estimate of server match time (1.2): each sample is the server's time, the
// round trip and the local clock when it arrived. A small error is smoothed in; an error over
// the resnap threshold, seen in two samples in a row (one spike is ignored), is slewed out over
// ResnapSeconds instead of jumping. Once synced the estimate never runs backward, so a cue never
// fires twice. Before the first sample there is no estimate: Now returns the local time.
class KINETICCORE_API FServerClock
{
public:
	void Sample(double ServerTime, double RoundTrip, double LocalNow);
	double Now(double LocalNow);
	bool Synced() const { return bSynced; }

	static constexpr double Smoothing = 0.1;         // Weight of each new sample
	static constexpr double ResnapThreshold = 0.050;  // s
	static constexpr double ResnapSeconds = 0.5;

private:
	double Offset = 0.0;        // Server time minus local time, as applied
	double TargetOffset = 0.0;  // As estimated
	double SlewStartLocal = 0.0;
	double SlewFrom = 0.0;
	bool bSlewing = false;
	bool bSynced = false;
	bool bBigPending = false;   // The last sample was over the threshold
	double PendingMeasured = 0.0;
	double LastNow = -1e300;
};

// The sever test (1.3): a kinematic's box, swept from one pose to the next in Substeps
// samples (four per physics step), against a line segment. Returns where along the segment
// (0 at A, 1 at B) it first crosses, or nothing.
struct FBox
{
	FVec3 HalfExtents;  // In the kinematic's local frame
	FVec3 Center;       // Local offset of the box from the pose (a Hauler's pose is its nose)
};

KINETICCORE_API std::optional<double> SegmentHitsBox(const FBox& Box, const FPose& Pose, const FVec3& A, const FVec3& B);
KINETICCORE_API std::optional<double> SweptSever(const FBox& Box, const FPose& From, const FPose& To, int Substeps, const FVec3& A,
                                 const FVec3& B);
KINETICCORE_API FPose Lerp(const FPose& From, const FPose& To, double Alpha);
} // namespace Kinetic
