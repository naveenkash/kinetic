// The match report the server posts to the backend: facts, never rewards (backend-architecture
// section 12.1; srv_store_report checks the roster, teams, connected seconds and winner). Built
// from the event log, so the report and the uploaded raw events always agree.
//
// Facts per player:
// - mass_distance_kg_m: mass moved under the player's lines times distance, the player's own
//   body excluded (04: Tonnage is mass moved under your lines times distance).
// - eliminations, assists, spotter_credits: from each Stall's credit (Credit.h). Bots earn
//   nothing, and that includes a backfill bot holding a leaver's seat (from Backfilled until
//   Returned in the connection events, and player_round events marked bot); a bot's Spotter
//   credit goes to the human killer it set up (game-modes 9).
// - ringout_heights_m: the killer's ring-outs, by fall height.
// - rope_marks: the killer's named rigs.
// - floor_seconds, airborne_seconds, hoisted_seconds: summed over rounds.
// The counters are the Contract objectives (bible/04); their meanings are PROPOSED:
// - lines_hung_used_by_others: Stalls where the player's Hang or Set line earned Spotter credit
//   for a teammate's elimination.
// - rated_tons_moved: rated loads the player's lines committed, in tonnes.
// - rounds_without_floor: rounds the player played without touching the floor.
// - scraps_with_lighter_object: eliminations by an object of known mass lighter than its victim.
// - snaps_survived: overspin snaps on the player's own line not followed by their Stall within
//   5 s in the same round.
#pragma once

#include "Kinetic/Api.h"

#include "Kinetic/MatchEvents.h"
#include "Kinetic/ScrapMatch.h"

#include <string>
#include <vector>

namespace Kinetic
{
struct FMatchInfo
{
	std::string MatchId;
	std::string Build;
	std::string Mode = "scrap";
	std::string Arena;
	std::string Variant = "base";
};

struct FRopeMark
{
	ERig Rig = ERig::None;
};

struct FPlayerFacts
{
	int Slot = NoPlayer;
	double MassDistanceKgM = 0.0;
	int Eliminations = 0;
	int Assists = 0;
	int SpotterCredits = 0;
	std::vector<double> RingOutHeightsM;
	std::vector<FRopeMark> RopeMarks;
	int LinesHungUsedByOthers = 0;
	double RatedTonsMoved = 0.0;
	int RoundsWithoutFloor = 0;
	int ScrapsWithLighterObject = 0;
	int SnapsSurvived = 0;
	double FloorSeconds = 0.0;
	double AirborneSeconds = 0.0;
	double HoistedSeconds = 0.0;
};

inline constexpr double SnapSurvivalSeconds = 5.0;

// Facts for every player in the roster, in roster order.
KINETICCORE_API std::vector<FPlayerFacts> ComputeFacts(const std::vector<FMatchPlayer>& Roster, const FEventLog& Log,
                                       const FKineticTables& Tables);

// The report JSON. Teams are written 1 and 2; winner_team is null for a no-contest. The
// result also writes forfeit_team, forfeit_penalty_free and no_contest (PROPOSED additions to
// backend-architecture 12.1). Connections come from the match flow, in roster order.
KINETICCORE_API std::string BuildMatchReport(const FMatchInfo& Info, const std::vector<FMatchPlayer>& Roster, const FEventLog& Log,
                             const std::vector<FPlayerConnection>& Connections, const FScrapResult& Result,
                             const FKineticTables& Tables);
} // namespace Kinetic
