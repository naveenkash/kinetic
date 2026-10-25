// Bots (game-modes-and-match-flow section 9; characters/npcs/bot-crew.md): the decisions the
// behavior trees call, engine-free. The trees move, aim and press; this says what and when.
// - FBotRandom: each bot's draws come from the match seed and its slot, so a match replays.
// - FBotDefense: the cut reflex on lines seated in the bot's own plates. Backfill bots cut
//   between 1.0 and 1.6 s after seat, uniformly random; the tutorial's Probationers at 1.4 s.
//   The cut is the swing request (the server judges it at that frame, networking 7.1); the
//   draw starts a warm-up earlier. One Cutter: a second line waits for swing and recovery.
//   A swing that does not end the line (a miss, or the server's refusal) is tried again as
//   soon as the Cutter has recovered, until the line ends.
// - ChooseRig: a backfill bot's next rig from the base kit (Slingshot, Haul to an anchor,
//   Clothesline at a doorway, Caboose on a Hauler), never on a teammate.
// - BotMayUse: what bots never do (overspin, map cables, Jack in the tutorial, Hauling a
//   teammate).
// - FProbationerCrew: the tutorial crew's scripted schedule (bot-crew.md's behavior table).
//
// Choices the docs leave open (PROPOSED): ChooseRig's ranges and scores; on a missed Dog the
// firer recalls and may fire again; the lead passes P-01, P-02, P-03 in turn after each cut,
// skipping any that is down; after the player's second scrap the crew's Avalanche and Caboose
// are withdrawn (a wind-up under way is called off) as well as its Dogs missing by 1 m; the
// tutorial crew follows bot-crew.md where onboarding-script.md differs (open-items 7d).
#pragma once

#include "Kinetic/Api.h"

#include "Kinetic/LineSolver.h"
#include "Kinetic/Tables.h"
#include "Kinetic/Vec3.h"

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace Kinetic
{
// splitmix64 over the match seed and the slot.
class KINETICCORE_API FBotRandom
{
public:
	FBotRandom(uint64_t MatchSeed, int Slot);
	double Uniform();  // [0, 1)
	double Uniform(double Lo, double Hi) { return Lo + (Hi - Lo) * Uniform(); }

private:
	uint64_t State;
};

enum class EBotProfile : uint8_t { Backfill, Probationer };

inline constexpr double BackfillCutMinSeconds = 1.0;
inline constexpr double BackfillCutMaxSeconds = 1.6;
inline constexpr double ProbationerCutSeconds = 1.4;  // PROPOSED in bot-crew.md

// "Office crew" in matches, the Probationers in the tutorial: O-01 or P-01.
KINETICCORE_API std::string BotCallsign(EBotProfile Profile, int Number);

struct FBotCutPlan
{
	int LineId = 0;
	double SeatedAt = 0.0;
	double DrawAt = 0.0;   // Warm-up starts
	double SwingAt = 0.0;  // The cut
};

class KINETICCORE_API FBotDefense
{
public:
	// The bot's class and Cutter set the warm-up, and the Cutter its swing and recovery.
	FBotDefense(EBotProfile InProfile, EHarnessClass Class, ECutterVariant Variant, const FKineticTables& Tables,
	            FBotRandom InRandom);

	// A Dog of this line seated in one of the bot's plates. A re-seat starts over.
	void Seated(int LineId, double Now);
	// The line is gone (cut by anyone, recalled, snapped): its plan is dropped. Until then a
	// swing at it is tried again after each recovery.
	void Ended(int LineId);
	double Warmup() const { return WarmupSeconds; }
	double SwingCycle() const { return CycleSeconds; }

	struct FActions
	{
		bool bDraw = false;      // Draw the Cutter now
		std::optional<int> Swing;  // Swing at this line now
	};
	FActions Tick(double Now);
	const std::vector<FBotCutPlan>& Plans() const { return Pending; }

private:
	EBotProfile Profile;
	double WarmupSeconds;
	double CycleSeconds;  // Swing and recovery
	FBotRandom Random;
	std::vector<FBotCutPlan> Pending;
	double DrawnAt = -1.0;          // When the current draw started, or -1 when sheathed
	double NextSwingFree = -1e300;  // One Cutter: swing then recovery
};

// A backfill bot's picture of the arena this tick.
struct FBotOther
{
	int Slot = 0;
	int Team = 0;
	FVec3 Position;
	FVec3 Velocity;
	bool bStanding = true;  // Not Stalled or withdrawn
};

struct FBotAnchor
{
	FVec3 Point;              // A wall, post or ledge a Dog can seat in
	bool bOverAnEdge = false;  // Hauling someone here takes them over a drop
};

struct FBotDoorway
{
	FVec3 Left, Right;  // The jambs at 1.2 m
	FVec3 Centre() const { return (Left + Right) * 0.5; }
};

struct KINETICCORE_API FBotHauler
{
	FVec3 TrackA, TrackB;          // The stretch of track the train is about to run, A first
	double ArrivesInSeconds = 0.0;  // At TrackA, from the klaxon's schedule; negative once past it
	double SpeedMps = 1.0;         // Along the stretch
	// When the train's nose reaches the point of the stretch nearest P.
	double ArrivesAt(const FVec3& P) const;
};

struct FBotView
{
	int Slot = 0;
	int Team = 0;
	EHarnessClass Class = EHarnessClass::Rigger;
	FVec3 Position;
	bool bLineLive = false;   // The bot is already working a line
	bool bLineOnSelf = false;  // Someone's Dog is in the bot's plate: defend first
	std::vector<FBotOther> Others;
	std::vector<FBotAnchor> Anchors;
	std::vector<FBotDoorway> Doorways;
	std::vector<FBotHauler> Haulers;  // Haulers whose klaxon has sounded
};

enum class EBotRig : uint8_t { None, Slingshot, HaulToAnchor, Clothesline, Caboose };

struct FBotIntent
{
	EBotRig Rig = EBotRig::None;
	int TargetSlot = -1;    // The enemy the rig is for, or -1
	bool bSelfDogA = false;  // Dog A into the bot's own chest plate (Slingshot)
	FVec3 DogA, DogB;       // Aim points (DogA unused when bSelfDogA). Caboose: DogB is where the
	                        // train will be beside the enemy; Dog B goes into the train there
	ELineMode Mode = ELineMode::Slack;
	bool bReleaseAfterSet = false;  // Clothesline: Hang, then release into a set line
};

struct FBotRigRules
{
	double CabooseLeadSeconds = 1.5;   // Fire Dog A this long before the train arrives
	double CabooseTrenchM = 10.0;      // The enemy within this of the track (Dog B at the track beside them)
	double ClotheslineNearM = 8.0;     // An enemy this close to a doorway, heading for it
	double HaulMinDragM = 4.0;         // An anchor nearer the enemy than this is no rig
	double HaulClearOfPathM = 2.0;     // The bot stays this far off the drag: no Hauling them through itself
	double SlingshotMinAngleDeg = 120.0;  // The wall must be behind the bot
};

FBotIntent ChooseRig(const FBotView& View, const FKineticTables& Tables, const FBotRigRules& Rules = {});

// What a bot may never do. Overspin is a hold past engagement, so bots release on engagement.
inline constexpr double BotOverspinHeldSeconds = 0.0;
struct FBotUse
{
	EBotProfile Profile = EBotProfile::Backfill;
	ELineMode Mode = ELineMode::Slack;
	bool bTargetIsTeammate = false;
	bool bMapCable = false;          // Hangtown's cuttable cables
	bool bRatedLoad = false;
};
KINETICCORE_API bool BotMayUse(const FBotUse& Use);

// The tutorial's Probationers, P-01 to P-03 (index 0 to 2), against one player.
enum class EProbationerTask : uint8_t
{
	WalkRoute,        // The honest route along the Floor, never sprinting
	HoldMarker,       // On the marker, standing still
	WindUp,           // The 1 s telegraphed pose before a Dog
	FireDog,          // This tick: fire Dog A at the player's nearest plate
	AwaitSeat,        // Dog in flight
	Brace,            // 0.3 s on a seat
	Haul,
	Recall,           // The confused look and the recall after a cut, or a recall after a miss
	AvalancheWindUp,  // 2 s
	Avalanche,        // This tick: Dog the stack and Haul
	Caboose,          // This tick: Dog the player, then the Hauler
	Down,             // Scrapped or out of the round: no orders
};

struct FProbationerOrder
{
	EProbationerTask Task = EProbationerTask::WalkRoute;
	double AimMissM = 0.0;  // FireDog: aim this far off the plate (the scrap cap)
};

struct FProbationerView
{
	double Now = 0.0;
	std::array<double, 3> DistanceToPlayer{1e9, 1e9, 1e9};
	std::array<bool, 3> bOnMarker{false, false, false};
	double PlayerToStackM = 1e9;   // The nearest authored billet stack
	double PlayerToTrenchM = 1e9;
	bool bKlaxon = false;          // A Hauler klaxon is sounding
	int PlayerScrapped = 0;        // Times the crew has scrapped the player
	std::array<bool, 3> bAlive{true, true, true};  // Standing, not Stalled
};

class KINETICCORE_API FProbationerCrew
{
public:
	static constexpr double FireRangeM = 15.0;
	static constexpr double WindUpSeconds = 1.0;
	static constexpr double BraceSeconds = 0.3;
	static constexpr double ConfusedSeconds = 0.5;
	static constexpr double RecallSeconds = 0.9;  // Breaker recall, 05 3.4
	static constexpr double RefireAfterCutSeconds = 6.0;
	static constexpr double AvalancheRangeM = 6.0;
	static constexpr double AvalancheWindUpSeconds = 2.0;
	static constexpr double CabooseRangeM = 10.0;
	static constexpr int ScrapCap = 2;
	static constexpr double MissM = 1.0;

	// The game reports what happened to each bot's line. A bot keeps its line (Hauling the
	// player, the stack or the Hauler) until one of these says it has gone.
	void Seated(int Bot, double Now);  // The Dog seated in the player
	void Missed(int Bot, double Now);  // The Dog hit nothing it could hold the player by
	void Cut(int Bot, double Now);     // The player cut it
	void Ended(int Bot, double Now);   // It ended any other way (the player Stalled, the round reset)

	std::array<FProbationerOrder, 3> Tick(const FProbationerView& View);
	int Lead() const { return LeadBot; }
	bool AvalancheUsed() const { return bAvalancheDone; }
	bool CabooseUsed() const { return bCabooseDone; }

private:
	enum class EStage : uint8_t { Idle, WindUp, InFlight, Brace, Haul, Recall };
	struct FBot
	{
		EStage Stage = EStage::Idle;
		double Until = 0.0;
		double NoFireUntil = -1e300;
	};
	void PassLead(const std::array<bool, 3>& bAlive);

	std::array<FBot, 3> Bots{};
	int LeadBot = 0;
	bool bAvalancheDone = false;
	bool bCabooseDone = false;
	double AvalancheAt = -1.0;  // When P-02's wind-up started, or -1
};
} // namespace Kinetic
