// A small fixed-step simulator of point bodies joined by lines, used to verify the line
// solver against bible/05's worked examples and to run the panic-window bench headless. It is
// a reference, not the game's physics: in the engine StepLine and the Hang solve run inside
// the Chaos physics thread (spool-and-line section 1), and Chaos integrates the bodies.
//
// Step order mirrors the engine:
// 1. Haul and Jack forces, against each endpoint's free inverse mass.
// 2. Footing, brace and break-free from this step's active force plus last step's Hang
//    force, then integration.
// 3. Hang and Set lines and the floor, solved together on the predicted positions; a grounded
//    player a Hang line lifts leaves the ground.
// 4. The hard velocity cap.
#pragma once

#include "Kinetic/LineSolver.h"
#include "Kinetic/MassRules.h"
#include "Kinetic/Tables.h"

#include <optional>
#include <vector>

namespace Kinetic
{
enum class ESimBodyKind : uint8_t { Static, Kinematic, Object, Player };

struct FSimBody
{
	ESimBodyKind Kind = ESimBodyKind::Object;
	FVec3 Position;
	FVec3 Velocity;
	double MassKg = 1.0;
	bool bGravity = false;
	std::optional<double> FloorZ;  // A frictionless floor at this height
	FPlayerBody Player;            // Kind == Player
	FObjectBody Object;            // Kind == Object
	bool bSmallPropBonus = false;  // Objects only: see IsSmallPropBonusTarget

	// Outputs
	EFooting Footing = EFooting::Skid;
	std::optional<double> AirborneAt;
	bool bBrokenFreeLatched = false;
};

struct FSimLine
{
	int BodyA = 0;
	int BodyB = 0;
	ESeat SeatA = ESeat::Body;
	ESeat SeatB = ESeat::Body;
	FLineParams Params;
	double TensionN = 0.0;
	FLineState State;

	// Outputs
	double MaxAppliedForce = 0.0;
	uint32_t EventsSeen = 0;
};

class FReferenceSim
{
public:
	FReferenceSim(const FKineticTables& InTables, double InDt) : Tables(InTables), Dt(InDt) {}

	int AddBody(const FSimBody& Body);
	int AddLine(const FSimLine& Line);
	void Engage(int LineIndex, ELineMode Mode);
	void SetReelInput(int LineIndex, int Input) { Lines.at(LineIndex).State.ReelInput = Input; }
	void SetTension(int LineIndex, double TensionN) { Lines.at(LineIndex).TensionN = TensionN; }

	void Step();
	double Time() const { return Now; }
	double StepSeconds() const { return Dt; }
	const FSimBody& Body(int Index) const { return Bodies.at(Index); }
	const FSimLine& Line(int Index) const { return Lines.at(Index); }

private:
	struct FStepForces
	{
		std::vector<FVec3> ActivePull;     // Haul, Jack push and Kick force per body this step
		std::vector<FVec3> BreakFreePull;  // The same, with the Kick at T: what object thresholds see
		std::vector<bool> bKick;           // A Kick acts on the body this step
	};

	double FreeInvMass(const FSimBody& Body) const;
	FLineEndpoint EndpointFor(const FSimLine& Line, bool bSideA) const;
	std::vector<bool> HeldThisStep() const;
	FStepForces StepActiveLines();
	void Integrate(const FStepForces& Forces);
	void IntegratePlayer(FSimBody& Body, const FVec3& FootingForce, const FVec3& ActiveForce, bool bKick);
	void IntegrateObject(FSimBody& Body, const FVec3& Force, const FVec3& BreakFreePull);
	double HangInvMass(const FSimBody& Body, const FVec3& ActivePull, const FVec3& BreakFreePull) const;
	void SolveHangAndFloor(const FStepForces& Forces);
	void ApplyCap(FSimBody& Body) const;

	const FKineticTables& Tables;
	double Dt;
	double Now = 0.0;
	std::vector<FSimBody> Bodies;
	std::vector<FSimLine> Lines;
	std::vector<FVec3> LastHangForce;  // Previous step's Hang force per body: the footing mailbox
};

// The panic-window bench (project-structure section 8): a Haul on a grounded victim from a
// braced or anchored attacker, measured from Dog seat.
struct FPanicRun
{
	std::optional<double> FeetOffAt;      // s, when the line lifts the victim off their feet
	std::optional<double> DistanceAt;     // s, when the victim has moved Distance
	double SpeedAtDistance = 0.0;         // m/s
};

FPanicRun RunHaulPanic(const FKineticTables& Tables, EHarnessClass Attacker, EHarnessClass Victim, double DistanceM,
                       double Dt, double MaxSeconds);
} // namespace Kinetic
