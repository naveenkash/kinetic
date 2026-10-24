// The browser client's game world: the same job the Unreal integration layer will do, done
// without an engine. KineticCore decides the rules (line forces, Hang solve, footing, break-free,
// impacts, the Cutter); this file supplies the world they act on: grey-box solids, bodies that
// collide as boxes, walking, Dogs in flight, and Catch Plates. It is a prototype for feel and
// for the open design questions, not the shipping physics (Chaos, or the standalone-server
// option's engine, replaces the collision and integration here).
//
// Simplifications (PROPOSED for the prototype only):
// - Bodies are axis-aligned boxes that never rotate; players are 0.8 × 0.8 × 1.8 m.
// - Collisions push bodies out of solids and out of each other, splitting by inverse mass; the
//   speed lost into a contact is what the Impact Monitor judges.
// - A player's line acts at their body centre; it is drawn from, and cut at, their plates.
// - Firing a Dog does not wait for a mode switch; changing mode on a live line waits the class
//   switch delay. Overspin and the Doubler's second line are not modelled.
// - The Cutter's swing has no melee effect on plates (05 4.4).
#pragma once

#include "Kinetic/Impact.h"
#include "Kinetic/LineSolver.h"
#include "Kinetic/MassRules.h"
#include "Kinetic/MatchEvents.h"
#include "Kinetic/Tables.h"
#include "Kinetic/Vec3.h"

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace Kinetic::Web
{
struct FSolid
{
	FVec3 Min, Max;
	ESurface Surface = ESurface::Hard;
	std::string Tag;  // For the renderer: floor, container, post, wall
};

struct FCrateSpec
{
	FVec3 Center;
	FVec3 Half{0.5, 0.5, 0.5};
	double MassKg = 60.0;
	EPlacement Placement = EPlacement::Loose;
};

struct FAnchorSpec
{
	FVec3 Point;  // On a solid's face: a Dog seats there
};

struct FDoorwaySpec
{
	FVec3 Left, Right;  // Jambs at 1.2 m
};

struct FArena
{
	std::string Name;
	std::vector<FSolid> Solids;
	std::vector<FCrateSpec> Crates;
	std::vector<FAnchorSpec> Anchors;
	std::vector<FDoorwaySpec> Doorways;
	std::vector<FVec3> Spawns[2];  // Three per side, feet positions
	double KillZ = -6.0;           // Below this a player is out: ring-out (05 7.5)
};

// The Yard in grey-box: two floors split by a trench with two bridges, containers, the trench
// pylons and the pylons past the east and west edges.
FArena MakeYard();

inline constexpr double PlayerHalfWidth = 0.4;
inline constexpr double PlayerHalfHeight = 0.9;
inline constexpr double EyeHeight = 0.7;  // Above the body centre

// What a player presses this tick. Bots press the same buttons.
struct FPlayerInput
{
	double MoveForward = 0.0;  // -1 to 1, along the camera's yaw
	double MoveRight = 0.0;
	double Yaw = 0.0;          // Radians, 0 along +x, counter-clockwise
	double Pitch = 0.0;        // Radians, positive up
	bool bJump = false;
	bool bSprint = false;
	bool bBrace = false;       // Held: crouch to brace (Rigger, Breaker)
	bool bFire = false;        // Pressed this tick: Dog A, then Dog B, then release and a new A
	bool bSelfDog = false;     // Pressed this tick: the next Dog into your own chest plate
	int Mode = 0;              // Pressed this tick: 1 Haul, 2 Jack, 3 Hang; 0 none
	bool bRelease = false;     // Pressed this tick: Hang becomes a set line; Haul and Jack vanish
	bool bDraw = false;        // Held: the Cutter is out
	bool bSwing = false;       // Pressed this tick, with the Cutter out
};

enum class EDogSeat : uint8_t { None, Static, Crate, Plate, OwnPlate };

struct FLineEnd
{
	EDogSeat Seat = EDogSeat::None;
	FVec3 Point;     // Static: the world point
	int Crate = -1;  // Crate: its index
	int Player = -1; // Plate or OwnPlate: whose
	EPlate Plate = EPlate::None;
};

struct FDog
{
	FVec3 Position;
	FVec3 Velocity;
	double Travelled = 0.0;
};

struct FWorldLine
{
	int Id = 0;
	int Owner = 0;
	FLineEnd A, B;
	std::optional<FDog> FlyingA, FlyingB;  // In flight, not yet seated
	FLineState State;
	FLineParams Params;
	double TensionN = 0.0;
	ELineMode Preselect = ELineMode::Haul;
	ELineMode PendingMode = ELineMode::Slack;  // A mode switch waiting out the class delay
	double PendingAt = -1.0;
	double CreatedAt = 0.0;
	bool bSet = false;            // Released Hang: a set line until it expires
	double SetExpiresAt = 0.0;
	bool bLive() const { return A.Seat != EDogSeat::None && B.Seat != EDogSeat::None; }
};

enum class ELineEnded : uint8_t { Released, Cut, DogsMet, Expired, Missed, BodyStalled, RanOut };

struct FWorldEvent
{
	enum class EKind : uint8_t { Seat, LineEnded, Stall, Stagger, Flip, Swing, Draw } Kind = EKind::Seat;
	int Player = -1;      // Who it happened to (Stall, Stagger, Flip) or who did it (Seat, Swing, Draw)
	int Line = -1;
	int Target = -1;      // Seat: the player seated on, if a plate
	EPlate Plate = EPlate::None;
	ELineEnded Ended = ELineEnded::Released;
	EStallCause Cause = EStallCause::WorldImpact;
	FVec3 At;
};

struct FWorldPlayer
{
	int Slot = 0;
	int Team = 0;
	EHarnessClass Class = EHarnessClass::Rigger;
	FVec3 Position;  // Body centre
	FVec3 Velocity;
	double Yaw = 0.0, Pitch = 0.0;
	bool bGrounded = true;
	bool bStanding = true;   // Not Stalled this round
	bool bBraced = false;
	double BraceHeld = 0.0;
	EFooting Footing = EFooting::Skid;
	double StunnedUntil = -1.0;  // Staggered or flipped: no input
	bool bDrawn = false;
	double DrawnAt = -1.0;
	double NextSwingAt = -1.0;
	bool bSelfDogNext = false;
	ELineMode Preselect = ELineMode::Haul;  // The mode a new line engages in
	bool bUnderLine = false;  // A Haul or Jack acted on them last step
	bool bCanMove = true;    // From the match flow
	bool bCanUseLines = true;
	FImpactMonitor Impacts;
	explicit FWorldPlayer(const FKineticTables& Tables) : Impacts(Tables) {}
};

struct FCrate
{
	FVec3 Position;
	FVec3 Velocity;
	FVec3 Half;
	double MassKg = 0.0;
	FObjectBody Object;
	bool bSmallPropBonus = false;
};

class FWorld
{
public:
	FWorld(const FKineticTables& InTables, FArena InArena);

	int AddPlayer(int Slot, int Team, EHarnessClass Class);
	// Every player to a spawn of their side, crates back, lines cleared, everyone standing.
	void Reset(const int SideOfTeam[2]);
	void Step(const std::vector<FPlayerInput>& Inputs);  // One input per player, in AddPlayer order

	double Time() const { return Now; }
	static constexpr double Dt = 1.0 / 60.0;
	const FArena& Arena() const { return ArenaData; }
	const std::vector<FWorldPlayer>& Players() const { return PlayerList; }
	FWorldPlayer& Player(int Index) { return PlayerList.at(static_cast<size_t>(Index)); }
	const std::vector<FCrate>& Crates() const { return CrateList; }
	const std::vector<FWorldLine>& Lines() const { return LineList; }
	std::vector<FWorldEvent> TakeEvents();

	// Positions the renderer and the cut test use.
	FVec3 EyeOf(int Player) const;
	FVec3 Forward(int Player) const;  // Camera forward from yaw and pitch
	FVec3 PlatePosition(int Player, EPlate Plate) const;
	FVec3 EndPosition(const FLineEnd& End) const;
	FVec3 DogOrEndPosition(const FWorldLine& Line, bool bSideA) const;
	double CutterWarmup(int Player) const;
	// Whether the ground holds at a feet position (for bots that must not walk off).
	bool HasFloorAt(const FVec3& Feet) const;
	double SupportTop(const FVec3& Center, const FVec3& Half) const;  // -inf when nothing is under it

	// The line nearest each player's eye that has a Dog in their plates, for the edge indicator.
	std::vector<int> LinesOnPlayer(int Player) const;

private:
	void ApplyInput(int Index, const FPlayerInput& In);
	void FireDog(int Index);
	// Lets go of the player's active line: a live Hang becomes a set line (05 3.6), within
	// the class's set-line count; anything else ends.
	void ReleaseActive(int Index);
	void HangLineContacts();
	void UpdateDogs();
	void UpdateLines();
	void EndLine(size_t LineIndex, ELineEnded Why);
	void Swing(int Index);
	void Simulate(const std::vector<FPlayerInput>& Inputs);
	void ResolveCollisions(std::vector<std::vector<FContact>>& Contacts);
	void StallPlayer(int Index, EStallCause Cause);
	std::optional<FLineEnd> SweepDog(int Owner, const FVec3& From, const FVec3& To, FVec3& Hit) const;
	FLineEndpoint EndpointOf(const FLineEnd& End, bool bHeldThisStep) const;
	int ActiveLineOf(int Player) const;  // The line being fired or live, not set lines
	double InvMassOfCrate(const FCrate& Crate) const;

	const FKineticTables& Tables;
	FArena ArenaData;
	std::vector<FWorldPlayer> PlayerList;
	std::vector<FCrate> CrateList;
	std::vector<FWorldLine> LineList;
	std::vector<FWorldEvent> Events;
	std::vector<FVec3> LastLineForce;  // Per player, last step's Hang force: the footing mailbox
	double Now = 0.0;
	int NextLineId = 1;
	int Resets = 0;
	double HoldSpawnYawUntil = -1.0;
public:
	// Counts Reset calls: a client adopts the spawn facing when it changes.
	int ResetCount() const { return Resets; }
	// After a reset, input yaw is ignored this long so a client can adopt the spawn facing.
	static constexpr double SpawnYawHoldSeconds = 0.25;
};
} // namespace Kinetic::Web
