// Force volumes (docs/tech/kinematics-and-force-volumes.md section 2; bible/05 6.6): Updraft,
// Pulse, water rush, gas jet and fire patch. Each gives the reference acceleration a_ref (what a
// 100 kg body would feel) at a point and time; a body of mass m feels a_ref × 100 / m, capped at
// 2 × a_ref (ForceVolumeAcceleration in MassRules.h). Volumes move players and free bodies only:
// loose objects and objects held by a player's line, never placed, rated, cable-hung or
// kinematic ones. They never Stall directly.
#pragma once

#include "Kinetic/Tables.h"
#include "Kinetic/Vec3.h"

#include <vector>

namespace Kinetic
{
// What a volume may move (6.6).
enum class EVolumeTarget : uint8_t { Player, LooseObject, LineHeldObject, PlacedObject, RatedObject, CableHung, Kinematic };
bool VolumeMoves(EVolumeTarget Target);

// A 0 to 1 curve over a repeating cycle, piecewise linear between (time in cycle, value) keys,
// wrapping from the last key to the first key of the next cycle.
struct FGustCurve
{
	double PeriodSeconds = 40.0;
	std::vector<std::pair<double, double>> Keys;  // Sorted by time; empty means always 1
	double ValueAt(double T) const;
};

// Every volume has a height band (world Z): a catwalk over a fire patch is not in it.
struct FZBand
{
	double MinZ = -1e300;
	double MaxZ = 1e300;
	bool Contains(double Z) const { return Z >= MinZ && Z <= MaxZ; }
};

// The Anchor's Updraft: upward a_ref at the Stump's axis, falling linearly to 0 at Radius,
// scaled by the gust curve.
struct FUpdraft
{
	FVec3 Axis{0.0, 0.0, 0.0};  // A point on the Stump's vertical axis
	double CoreARef = 14.0;     // m/s²
	double Radius = 60.0;       // m
	FZBand Band;                // Per arena (the Anchor's layout)
	FGustCurve Gust;
	FVec3 ARefAt(const FVec3& P, double T) const;
	// When each gust begins in [From, To): the curve rising from 0. Its tell leads by 1 s.
	std::vector<double> GustStarts(double From, double To) const;
};

// The Anchor's Pulse: every Period from First, a 0.4 s shove outward at 20 m/s², then a 0.6 s
// inhale toward the Stump at 8 m/s², both falling linearly to 0 at the Rim.
struct FPulse
{
	FVec3 Axis{0.0, 0.0, 0.0};
	double First = 55.0;
	double Period = 75.0;
	double ShoveARef = 20.0;
	double ShoveSeconds = 0.4;
	double InhaleARef = 8.0;
	double InhaleSeconds = 0.6;
	double RimRadius = 240.0;
	FZBand Band{-15.0, 70.0};  // The Anchor: from the Well's lip to above the Crown
	FVec3 ARefAt(const FVec3& P, double T) const;
	double LastStartAt(double T) const;  // Most recent Pulse start at or before T, or -1
};

// Ballast's water rush: a box in the Hold, a_ref 12 m/s² toward the nose (set by the hull's
// tilt), starting 6 s after a tank rupture, for DurationSeconds (PROPOSED: 4 s).
struct FWaterRush
{
	FVec3 Min, Max;          // Axis-aligned box in the Hold
	FVec3 Direction{1, 0, 0};
	double ARef = 12.0;
	double DelaySeconds = 6.0;
	double DurationSeconds = 4.0;
	FVec3 ARefAt(const FVec3& P, double T, double RuptureTime) const;
};

// A compressed-gas manifold's jet (6.4): a 6 m cone pushing along its axis for 3 s after a Jack
// over 600 N or a blast, at 5 m/s² (a 100 kg body reaches 15 m/s in 3 s), no push once a body
// already moves at 15 m/s along the jet. Cone half-angle PROPOSED: 15°.
struct FGasJet
{
	FVec3 Apex;
	FVec3 Axis{1, 0, 0};
	double Length = 6.0;
	double HalfAngleRadians = 15.0 * 3.14159265358979323846 / 180.0;
	double ARef = 5.0;
	double Seconds = 3.0;
	double SpeedCap = 15.0;
	FVec3 ARefAt(const FVec3& P, const FVec3& BodyVelocity, double T, double TriggerTime) const;
};

// A fuel drum's fire patch (6.4): a 3 m disc for 6 s, pushing outward until a body moves away
// at 4 m/s (a_ref PROPOSED: 8 m/s²); entering it staggers (EStaggerCause::FirePatch).
struct FFirePatch
{
	FVec3 Center;
	double Radius = 3.0;
	double Seconds = 6.0;
	double TargetSpeed = 4.0;
	double ARef = 8.0;
	double Height = 2.5;  // The flames' height above the disc (PROPOSED)
	bool Contains(const FVec3& P, double T, double IgniteTime) const;
	FVec3 ARefAt(const FVec3& P, const FVec3& BodyVelocity, double T, double IgniteTime) const;
};

// The acceleration a body of mass m feels from a_ref (direction kept, magnitude scaled and capped).
FVec3 BodyAcceleration(const FVec3& ARef, double MassKg, EVolumeTarget Target, const FSpoolGlobals& Globals);
} // namespace Kinetic
