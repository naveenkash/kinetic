// The match event stream (event_schema 1): one schema for playtest instrumentation, telemetry,
// the post-match screen and the backend match report (backend-architecture section 12;
// playtest-plan "Instrumentation"; tech-stack-plan section 6). The server appends events as
// they happen; the log is written as JSON Lines, uploaded as the match's raw events, and its
// SHA-256 goes in the report.
#pragma once

#include "Kinetic/Api.h"

#include "Kinetic/Impact.h"
#include "Kinetic/LineSolver.h"
#include "Kinetic/Tables.h"

#include <cstdint>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace Kinetic
{
inline constexpr int EventSchemaVersion = 1;  // Matches the backend manifest's event_schema
inline constexpr int RulesetVersion = 1;
inline constexpr int NoPlayer = -1;
inline constexpr int NoObject = -1;

// A seat in the match: the backend manifest's player entry. Slot is the player's id in every
// event. Team is 0 or 1 here and 1 or 2 in the report (the backend numbers teams from 1).
struct FMatchPlayer
{
	int Slot = 0;
	int Team = 0;
	std::string AccountId;  // Empty for a bot
	bool bIsBot = false;
	EHarnessClass Harness = EHarnessClass::Climber;
	std::string Face;
};

enum class ESeatTarget : uint8_t { Static, Kinematic, Object, Player };
enum class EPlate : uint8_t { None, Chest, Back, LeftShoulder, RightShoulder };  // 5.5
enum class ELineOutcome : uint8_t { Released, Cut, Snapped, Completed, RanOut, BodyStalled };
enum class EStallCause : uint8_t { ObjectImpact, WorldImpact, KinematicImpact, PlayerImpact, Crush, Blast, RingOut };
enum class ERig : uint8_t
{
	None, Gallows, Guillotine, Clothesline, Slingshot, Pendulum, Kiss, Scatter, Caboose, Avalanche, Overboard, AnchorDrop
};
enum class ERoundEnd : uint8_t { CrewDown, TimeOut, SuddenDeath, Replayed, Forfeit };
enum class EConnection : uint8_t { Disconnected, Returned, IdleWarning, Withdrawn, Backfilled, Abandoned };

KINETICCORE_API const char* ToString(ESeatTarget Value);
KINETICCORE_API const char* ToString(EPlate Value);
KINETICCORE_API const char* ToString(ELineOutcome Value);
KINETICCORE_API const char* ToString(EStallCause Value);
KINETICCORE_API const char* ToString(ERig Value);
KINETICCORE_API const char* ToString(ERoundEnd Value);
KINETICCORE_API const char* ToString(EConnection Value);
KINETICCORE_API const char* ToString(ELineMode Value);

// Every Dog seat: shooter, target type, plate if a player.
struct FDogSeatEvent
{
	double Time = 0.0;
	int Round = 0;
	int Shooter = NoPlayer;
	int LineId = 0;
	ESeatTarget Target = ESeatTarget::Static;
	int TargetPlayer = NoPlayer;
	EPlate Plate = EPlate::None;
	std::optional<EHarnessClass> ShooterClass = std::nullopt;
	std::optional<EHarnessClass> TargetClass = std::nullopt;  // Player targets only
};

// Every mode engagement, logged when it ends: mode, tension, duration, mass on each end, outcome.
struct FModeEvent
{
	double Time = 0.0;
	int Round = 0;
	int Owner = NoPlayer;
	int LineId = 0;
	ELineMode Mode = ELineMode::Haul;
	double TensionN = 0.0;
	double Seconds = 0.0;
	double MassAKg = 0.0;  // 0 for static and kinematic ends
	double MassBKg = 0.0;
	ELineOutcome Outcome = ELineOutcome::Released;
};

// Every cut: who, whose line, time from seat to cut, plate, the camera turn it needed.
struct FCutEvent
{
	double Time = 0.0;
	int Round = 0;
	int Cutter = NoPlayer;
	int LineOwner = NoPlayer;
	int LineId = 0;
	double SeatToCutSeconds = 0.0;
	EPlate Plate = EPlate::None;
	double TurnDegrees = 0.0;
};

// Every Stall: cause, the object and its impulse or the speed, credit, the named rig if detected.
struct FStallEvent
{
	double Time = 0.0;
	int Round = 0;
	int Victim = NoPlayer;
	EStallCause Cause = EStallCause::WorldImpact;
	int ObjectId = NoObject;
	double ObjectMassKg = 0.0;
	double Value = 0.0;  // N·s for object, blast and player hits; m/s for world and kinematic; N for crush
	int Killer = NoPlayer;
	int Spotter = NoPlayer;
	std::vector<int> Assists;
	ERig Rig = ERig::None;
	double RingOutHeightM = 0.0;  // Ring-outs: the fall from the last floor to the kill volume
};

// Every stagger and flip.
struct FHitEvent
{
	double Time = 0.0;
	int Round = 0;
	int Player = NoPlayer;
	EOutcome Outcome = EOutcome::Stagger;
	EStallCause Cause = EStallCause::WorldImpact;
	double Value = 0.0;
};

// An overspin snap on the owner's line (3.7).
struct FSnapEvent
{
	double Time = 0.0;
	int Round = 0;
	int Owner = NoPlayer;
	int LineId = 0;
	double ForceN = 0.0;
};

// A rated load committed by the owner's line (6.2a).
struct FRatedCommitEvent
{
	double Time = 0.0;
	int Round = 0;
	int Owner = NoPlayer;
	double MassKg = 0.0;
};

// Per player per round: floor, airborne and hoisted time, and mass moved under their lines.
struct FPlayerRoundEvent
{
	int Round = 0;
	int Player = NoPlayer;
	double FloorSeconds = 0.0;
	double AirborneSeconds = 0.0;
	double HoistedSeconds = 0.0;
	double MassDistanceKgM = 0.0;
	bool bBotControlled = false;  // Earned while a backfill bot held the seat: never credited
};

struct FRoundEndEvent
{
	double Time = 0.0;
	int Round = 0;
	int WinnerTeam = NoPlayer;  // 0 or 1; NoPlayer when the round is replayed
	double Seconds = 0.0;       // From the start of the Drop to the end
	int Stalls = 0;
	ERoundEnd Reason = ERoundEnd::CrewDown;
};

struct FMatchEndEvent
{
	double Time = 0.0;
	int WinnerTeam = NoPlayer;  // NoPlayer: no-contest
	int ForfeitTeam = NoPlayer;
	bool bForfeitPenaltyFree = false;
	bool bNoContest = false;
};

// Changes in a seat's connection. Backfilled starts a bot's control of the seat; Returned
// ends it (casual returns take the rigger straight back).
struct FConnectionEvent
{
	double Time = 0.0;
	int Player = NoPlayer;
	EConnection Change = EConnection::Disconnected;
};

// Server health, once a second.
struct FServerSecondEvent
{
	double Time = 0.0;
	double FrameMs = 0.0;
	double PhysicsMs = 0.0;
	int DynamicBodies = 0;
	int ActiveLines = 0;
};

using FMatchEvent = std::variant<FDogSeatEvent, FModeEvent, FCutEvent, FStallEvent, FHitEvent, FSnapEvent,
                                 FRatedCommitEvent, FPlayerRoundEvent, FRoundEndEvent, FMatchEndEvent,
                                 FConnectionEvent, FServerSecondEvent>;

// The match's raw events, in the order they happened.
class KINETICCORE_API FEventLog
{
public:
	void Append(const FMatchEvent& Event) { Events.push_back(Event); }
	const std::vector<FMatchEvent>& All() const { return Events; }

	// One JSON object per line, each with "type" and the schema version, ending in a newline.
	std::string JsonLines() const;
	std::string Sha256() const;  // Of JsonLines(): the report's raw_events_sha256

private:
	std::vector<FMatchEvent> Events;
};

KINETICCORE_API std::string ToJson(const FMatchEvent& Event);
} // namespace Kinetic
