// Hang and Set lines as a sequential-impulse constraint solver over all Hang lines at once
// (bible/05 sections 3.2, 3.5, 3.6, 6.2a). It takes each body's position at the start of the
// step and its velocity after this step's forces, solves the lines on the velocities, then
// integrates the positions and removes any drift from the rest lengths. The lines and floor
// contacts are solved together exactly (a bounded direct solve with Murty's least-index
// pivoting, which always terminates; each connected group separately), so chains of light and
// heavy bodies hold. Each line's velocity target is exact for its turn over the step, and a step in
// which a line turns more than 0.05 rad is split into substeps, so a swing keeps its speed on
// any line length.
//
// Each line is a rope (tension only) or a rod (both ways) whose impulse per step is capped at
// its Hang load, so several lines on one body add their Hang loads (6.2a). The load a line
// reads is its solved impulse over the step, which is m·g for a static hang and includes the
// centripetal load of a swing.
//
// Slip works like a clutch whose friction is the Hang load: an overloaded line holds at its
// Hang load and pays the load out as fast as the load goes, never by pushing, and stops
// slipping once it can hold the load again (PROPOSED: bible/05 3.2's fixed rate of 0.5 m per
// 0.1 s cannot hold for a load just over the limit without pushing it, nor for a falling load
// without an infinitely strong line; see production/open-items.md 7a). A Reel input moves the
// rest length at the Reel rate when either end can move, except that reeling in against an
// overload gains nothing (3.2), and a reel-out never shortens the line. The rest length stays
// between the Dogs-meet length and the class line length; a line that slips past either lets
// go (PROPOSED).
//
// Bodies may stand on a floor (FloorZ). The floor is solved in the same iterations, so a line
// that lifts less than a body's weight leaves it standing and a line that pulls a body into
// the floor is resisted by it. In the engine, Chaos solves contacts alongside the line joints.
#pragma once

#include "Kinetic/Api.h"

#include "Kinetic/LineSolver.h"

#include <optional>
#include <vector>

namespace Kinetic
{
struct FHangBody
{
	FVec3 Position;
	FVec3 Velocity;
	double InvMass = 0.0;          // 0 for static, kinematic, braced, or placed (Hang never tests break-free)
	std::optional<double> FloorZ;  // A frictionless floor under the body
};

struct FHangLine
{
	int BodyA = 0;
	int BodyB = 0;
	FLineState* State = nullptr;
	FLineParams Params;
	double HangLoadN = 0.0;  // The class Hang load, raised by overspin (Spool.h)

	// Outputs
	double AppliedForce = 0.0;  // N: the solved impulse over the step
	FVec3 ForceOnA;             // N, for the endpoint's footing and brace next step (MassRules)
	FVec3 ForceOnB;
	uint32_t Events = 0;
};

struct FHangSolverSettings
{
	int MaxPositionIterations = 100;
};

void SolveHangLines(std::vector<FHangBody>& Bodies, std::vector<FHangLine>& Lines, const FSpoolGlobals& Globals,
                    double Dt, const FHangSolverSettings& Settings = {});
} // namespace Kinetic
