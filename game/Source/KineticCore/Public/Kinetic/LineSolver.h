// The Line's active modes between two endpoints: Haul, and Jack (the Kick, then the motor
// push). bible/05 sections 3.2 to 3.4; spool-and-line-implementation.md section 3.2 is the
// algorithm. Hang and Set lines are solved together after integration by HangSolver.h.
//
// The solver never splits force by mass: it applies equal and opposite force and the
// endpoints' inverse masses do the split (bible/05 section 3.3). Each endpoint's inverse mass
// here is its free inverse mass: 0 only for what can never move under a line (static,
// kinematic, fixed scenery, rated loads). Footing, brace and break-free are then decided from
// the force this returns, before integration (MassRules.h), so a Jack can start a placed
// object or break a brace on the step it fires.
#pragma once

#include "Kinetic/Api.h"

#include "Kinetic/Tables.h"
#include "Kinetic/Vec3.h"

#include <cstdint>

namespace Kinetic
{
enum class ELineMode : uint8_t { Slack, Haul, Jack, Hang, Set };

// Where a Dog is seated. OwnPlate is the line owner's own Catch Plate (a self-dog);
// OtherPlate is another player's plate, which blocks the Kick (the interlock, 3.2).
enum class ESeat : uint8_t { Static, Body, Kinematic, OwnPlate, OtherPlate };

struct FLineEndpoint
{
	ESeat Seat = ESeat::Static;
	FVec3 Position;
	FVec3 Velocity;
	double InvMass = 0.0;
	// A Light object, or a Medium one under the bonus mass cap: the owner's Scrapper bonus applies.
	bool bSmallPropBonus = false;
};

// Per-line values from the owner's class (DT_HarnessClass).
struct FLineParams
{
	double LineLength = 0.0;
	double KickLaunchSpeedV = 0.0;
	double HangLoadN = 0.0;
	double SmallPropForceMul = 1.0;

	static FLineParams FromHarness(const FHarnessTuning& Harness)
	{
		return {Harness.LineLength, Harness.KickLaunchSpeedV, Harness.HangLoadN, Harness.SmallPropForceMul()};
	}
};

// Replicated line state; in the engine this also lives in the networked physics history so a
// rewind restores it (spool-and-line section 5.2).
struct FLineState
{
	ELineMode Mode = ELineMode::Slack;
	double ModeStartTime = 0.0;
	double L0 = 0.0;          // Length when Hang (or Set from another mode) engaged, m
	double SlipAccum = 0.0;   // m added by slipping
	double ReelAccum = 0.0;   // m of rest-length change from Reel (negative = reeled in)
	int ReelInput = 0;        // -1 reel in (held Haul), +1 reel out (held Jack), 0 none
	bool bKickDone = false;
	bool bRodMode = false;    // Hang as a two-sided rod; the default is rope (slack under compression)
	double AppliedForce = 0.0;  // N, last step, for render and audio
};

namespace LineEvents
{
inline constexpr uint32_t DogsMet = 1u << 0;
inline constexpr uint32_t SlackPastCap = 1u << 1;
inline constexpr uint32_t AutoRelease = 1u << 2;
inline constexpr uint32_t KickActive = 1u << 3;
inline constexpr uint32_t Slipping = 1u << 4;   // The creak: Hang load exceeded
inline constexpr uint32_t RanOut = 1u << 5;     // Slipped past the end of the filament or in to the Dogs: let go
inline constexpr uint32_t SolverLimit = 1u << 6;  // The Hang solve hit its pivot limit (not expected; finished approximately)
} // namespace LineEvents

struct FLineStepResult
{
	FVec3 ForceOnA;  // N: apply over this step
	FVec3 ForceOnB;
	// The force objects' break-free and rated thresholds see: T, without the Scrapper bonus
	// (6.1: a speed bonus; 6.2: a Scrapper's 400 N does not start a stack), and the Kick at T (PROPOSED:
	// bible/05 6.2 counts the Kick toward break-free, but 6.2 and 6.7 also say which classes can
	// start a bolted object, a stacked container or a Lift counterweight, judged at T; at full
	// Kick force a Climber would start all three).
	FVec3 BreakFreeOnA;
	FVec3 BreakFreeOnB;
	uint32_t Events = 0;
	double AppliedForce = 0.0;  // N
};

// Starts a mode and resets the per-engagement state. Hang captures L0, never under the
// Dogs-meet length; Set keeps a Hang's rest length when it follows one and captures L0 otherwise.
KINETICCORE_API void EngageMode(FLineState& State, ELineMode Mode, double Now, const FVec3& PositionA, const FVec3& PositionB,
                const FSpoolGlobals& Globals);

// The Kick fires unless either Dog sits in another player's Catch Plate (3.2).
KINETICCORE_API bool IsKickAllowed(const FLineEndpoint& A, const FLineEndpoint& B);

// One physics step of a Haul or Jack line, before integration with current positions.
// TensionN is the owner's current pull tension (Spool.h: class T, overspin, Doubler split).
// A Haul goes Slack when the Dogs meet; a Jack at the class line length. A Slack line reads
// 0; Hang and Set lines are solved by HangSolver.h.
KINETICCORE_API FLineStepResult StepLine(FLineState& State, const FLineParams& Params, double TensionN, const FLineEndpoint& A,
                         const FLineEndpoint& B, const FSpoolGlobals& Globals, double Dt, double Now);
} // namespace Kinetic
