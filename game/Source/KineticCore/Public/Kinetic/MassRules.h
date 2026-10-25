// What moves under a line: the inverse mass each endpoint gets, footing and brace for
// players, break-free and rated loads for objects, mass tiers, and force volumes.
// bible/05 sections 3.3, 5.1 to 5.4, 6.1 to 6.3, 6.6; spool-and-line section 3.3.
#pragma once

#include "Kinetic/Api.h"

#include "Kinetic/Tables.h"
#include "Kinetic/Vec3.h"

#include <cstdint>

namespace Kinetic
{
// Players --------------------------------------------------------------------------

enum class EFooting : uint8_t
{
	Braced,    // Static to this line: below the brace foot-hold
	Skid,      // Grounded drag: feet planted, moved by the line on the game thread
	Airborne,  // A simulated flight body the line acts on fully
	Hoisted,   // Airborne, pulled upward with feet off the ground
};

struct FPlayerBody
{
	double MassKg = 0.0;
	double FootHold = 0.0;        // m/s², 5.3
	double BracedFootHold = 0.0;  // m/s², 5.4 (0 for classes that cannot brace)
	bool bBraced = false;
	bool bGrounded = true;
};

struct FPlayerLineResponse
{
	EFooting Footing = EFooting::Skid;
	double InvMass = 0.0;    // For the Hang solve: 0 only while braced (a skidding player moves with the line)
	FVec3 DragAcceleration;  // m/s² along the ground while skidding
	bool bBraceBroken = false;
};

KINETICCORE_API FPlayerBody PlayerBodyFor(const FHarnessTuning& Harness, bool bBraced, bool bGrounded);

// Over this share of the line force upward, a pull that reaches the player's weight is a hoist.
inline constexpr double HoistUpwardShare = 0.5;

// Total line force on the player decides the footing: this step's Haul, Jack and Kick force
// plus last step's Hang force (the Hang is solved after the forces). Foot-hold is judged on
// the part of the force along the ground, lowered in proportion to the weight an upward pull
// takes off the feet. A Kick always lifts an unbraced grounded player into a
// flight body (spool-and-line section 3.3). A mostly upward pull that reaches the player's
// weight unloads the feet: hoisted (bible/05 section 5.2).
KINETICCORE_API FPlayerLineResponse ResolvePlayerUnderLine(const FPlayerBody& Player, const FVec3& LineForce, bool bKickActive,
                                           double Gravity);

// Objects --------------------------------------------------------------------------

enum class EMassTier : uint8_t { Light, Medium, Heavy, Rated };

KINETICCORE_API EMassTier TierOf(double MassKg, bool bStructural, const FMassTierBounds& Bounds);

// Light, or Medium under the bonus mass cap (a propane cylinder yes, a fuel drum no), 6.1.
KINETICCORE_API bool IsSmallPropBonusTarget(double MassKg, const FKineticTables& Tables);

struct FObjectBody
{
	double MassKg = 0.0;
	EPlacement Placement = EPlacement::Loose;
	bool bFixedScenery = false;         // Heavy on a bare floor, water tanks, the gas manifold
	bool bRated = false;                // Moves only through its commit timeline (FRatedCommit)
	bool bKinematicAttachment = false;  // Gantry load, Lift counterweight: on a path until pulled free
	bool bBrokenFree = false;           // Latched once its placement threshold was met, or once released from its path
};

struct FObjectLineResponse
{
	double InvMass = 0.0;
	bool bBreaksFreeNow = false;     // Placement threshold met this step: the lurch
	bool bReleasedFromPath = false;  // A kinematic rated attachment pulled off its carrier
};

// False for fixed scenery and rated loads (their commit timeline moves them). The active line
// solver uses 1/m for everything else, and the thresholds below then decide what moves.
KINETICCORE_API bool CanEverMoveUnderALine(const FObjectBody& Object);

// ActivePull is the vector sum of every Haul, Jack push and Kick force on the object this step
// (6.2a). Hang lines never test break-free (6.2): bOnlyHangLines keeps a placed object static.
// A kinematic attachment released from its path is a simulated body from then on (6.7).
KINETICCORE_API FObjectLineResponse ResolveObjectUnderLines(const FObjectBody& Object, const FVec3& ActivePull, bool bOnlyHangLines,
                                            const FKineticTables& Tables);

// Inclusive (v0.2.1), with a relative tolerance: a pull built as Axis × 1,000 N can measure
// 999.9999999 N, and a Breaker's 1,000 N must still start a rated load on any axis.
inline bool MeetsBreakFree(double PullN, double ThresholdN) { return PullN >= ThresholdN * (1.0 - 1e-9); }

// A rated load commits once active pull along its authored axis has held at or above the
// rated threshold for RatedCommitHold seconds; its scripted timeline then carries the motion.
class KINETICCORE_API FRatedCommit
{
public:
	bool Update(double PullAlongAxisN, double Dt, const FKineticTables& Tables);
	bool IsCommitted() const { return bCommitted; }
	double HeldSeconds() const { return Held; }

private:
	double Held = 0.0;
	bool bCommitted = false;
};

// Force volumes ----------------------------------------------------------------------

// Acceleration a body of MassKg feels in a volume authored as ARef for the reference body,
// capped at ForceVolumeCapMul × ARef (6.6).
KINETICCORE_API double ForceVolumeAcceleration(double ARef, double MassKg, const FSpoolGlobals& Globals);
} // namespace Kinetic
