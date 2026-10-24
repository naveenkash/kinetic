#include "Kinetic/MatchReport.h"

#include "Kinetic/Json.h"

#include <algorithm>
#include <map>

namespace Kinetic
{
namespace
{
constexpr double KilogramsPerTonne = 1000.0;

const FMatchPlayer* Find(const std::vector<FMatchPlayer>& Roster, int Slot)
{
	for (const FMatchPlayer& Player : Roster)
	{
		if (Player.Slot == Slot)
		{
			return &Player;
		}
	}
	return nullptr;
}

// When each seat was held by a bot: always for a roster bot; for a leaver's seat, from
// Backfilled until Returned.
class FBotTimeline
{
public:
	FBotTimeline(const std::vector<FMatchPlayer>& Roster, const FEventLog& Log)
	{
		for (const FMatchPlayer& Player : Roster)
		{
			if (Player.bIsBot)
			{
				RosterBots.push_back(Player.Slot);
			}
		}
		for (const FMatchEvent& Event : Log.All())
		{
			const auto* Change = std::get_if<FConnectionEvent>(&Event);
			if (!Change)
			{
				continue;
			}
			std::vector<FSpan>& Spans = Backfills[Change->Player];
			if (Change->Change == EConnection::Backfilled && (Spans.empty() || Spans.back().Until < 1e300))
			{
				Spans.push_back({Change->Time, 1e308});
			}
			else if (Change->Change == EConnection::Returned && !Spans.empty() && Spans.back().Until > 1e300)
			{
				Spans.back().Until = Change->Time;
			}
		}
	}

	bool IsBot(int Slot, double Time) const
	{
		if (std::find(RosterBots.begin(), RosterBots.end(), Slot) != RosterBots.end())
		{
			return true;
		}
		const auto Found = Backfills.find(Slot);
		if (Found == Backfills.end())
		{
			return false;
		}
		return std::any_of(Found->second.begin(), Found->second.end(),
		                   [Time](const FSpan& S) { return Time >= S.From && Time < S.Until; });
	}

private:
	struct FSpan
	{
		double From = 0.0;
		double Until = 0.0;
	};
	std::vector<int> RosterBots;
	std::map<int, std::vector<FSpan>> Backfills;
};

struct FAccumulator
{
	const std::vector<FMatchPlayer>& Roster;
	const FKineticTables& Tables;
	const FBotTimeline& Bots;
	std::map<int, FPlayerFacts> Facts;
	std::vector<FSnapEvent> Snaps;
	std::vector<FStallEvent> Stalls;

	FPlayerFacts* Of(int Slot)
	{
		const auto Found = Facts.find(Slot);
		return Found == Facts.end() ? nullptr : &Found->second;
	}

	// The facts a human earned at Time, or nullptr if a bot held the seat (bots earn nothing).
	FPlayerFacts* Human(int Slot, double Time) { return Bots.IsBot(Slot, Time) ? nullptr : Of(Slot); }

	void Add(const FStallEvent& E)
	{
		Stalls.push_back(E);
		FPlayerFacts* Killer = Human(E.Killer, E.Time);
		for (const int Assist : E.Assists)
		{
			if (FPlayerFacts* Helper = Human(Assist, E.Time))
			{
				++Helper->Assists;
			}
		}
		if (E.Spotter != NoPlayer)
		{
			// A bot's Spotter credit goes to the human it set up (game-modes 9).
			FPlayerFacts* Spotter = Human(E.Spotter, E.Time);
			FPlayerFacts* Credited = Spotter ? Spotter : Killer;
			if (Credited)
			{
				++Credited->SpotterCredits;
			}
			if (Spotter)
			{
				++Spotter->LinesHungUsedByOthers;
			}
		}
		if (!Killer)
		{
			return;
		}
		++Killer->Eliminations;
		if (E.Cause == EStallCause::RingOut)
		{
			Killer->RingOutHeightsM.push_back(E.RingOutHeightM);
		}
		if (E.Rig != ERig::None)
		{
			Killer->RopeMarks.push_back({E.Rig});
		}
		const FMatchPlayer* Victim = Find(Roster, E.Victim);
		if (E.Cause == EStallCause::ObjectImpact && Victim && E.ObjectMassKg > 0.0 &&
		    E.ObjectMassKg < Tables.Harness(Victim->Harness).MassKg)
		{
			++Killer->ScrapsWithLighterObject;
		}
	}

	void Add(const FPlayerRoundEvent& E)
	{
		FPlayerFacts* Player = E.bBotControlled || Bots.IsBot(E.Player, -1.0) ? nullptr : Of(E.Player);
		if (!Player)
		{
			return;
		}
		Player->MassDistanceKgM += E.MassDistanceKgM;
		Player->FloorSeconds += E.FloorSeconds;
		Player->AirborneSeconds += E.AirborneSeconds;
		Player->HoistedSeconds += E.HoistedSeconds;
		const bool bPlayed = E.FloorSeconds + E.AirborneSeconds + E.HoistedSeconds > 0.0;
		if (bPlayed && E.FloorSeconds <= 0.0)
		{
			++Player->RoundsWithoutFloor;
		}
	}

	void Add(const FRatedCommitEvent& E)
	{
		if (FPlayerFacts* Owner = Human(E.Owner, E.Time))
		{
			Owner->RatedTonsMoved += E.MassKg / KilogramsPerTonne;
		}
	}

	void Add(const FSnapEvent& E) { Snaps.push_back(E); }

	template <typename T>
	void Add(const T&)
	{
	}

	void CountSurvivedSnaps()
	{
		for (const FSnapEvent& Snap : Snaps)
		{
			FPlayerFacts* Owner = Human(Snap.Owner, Snap.Time);
			if (!Owner)
			{
				continue;
			}
			bool bStalled = false;
			for (const FStallEvent& Stall : Stalls)
			{
				const double After = Stall.Time - Snap.Time;
				bStalled = bStalled || (Stall.Victim == Snap.Owner && Stall.Round == Snap.Round && After >= 0.0 &&
				                        After <= SnapSurvivalSeconds);
			}
			Owner->SnapsSurvived += bStalled ? 0 : 1;
		}
	}
};

void WriteFacts(FJsonWriter& J, const FPlayerFacts& F, double ConnectedSeconds)
{
	J.Key("facts").BeginObject();
	J.Key("mass_distance_kg_m").Number(F.MassDistanceKgM, 1);
	J.Key("eliminations").Integer(F.Eliminations);
	J.Key("assists").Integer(F.Assists);
	J.Key("spotter_credits").Integer(F.SpotterCredits);
	J.Key("ringout_heights_m").BeginArray();
	for (const double Height : F.RingOutHeightsM)
	{
		J.Number(Height, 1);
	}
	J.EndArray();
	J.Key("rope_marks").BeginArray();
	for (const FRopeMark& Mark : F.RopeMarks)
	{
		J.BeginObject().Key("rig").String(ToString(Mark.Rig)).EndObject();
	}
	J.EndArray();
	J.Key("counters").BeginObject();
	J.Key("lines_hung_used_by_others").Integer(F.LinesHungUsedByOthers);
	J.Key("rated_tons_moved").Number(F.RatedTonsMoved, 3);
	J.Key("rounds_without_floor").Integer(F.RoundsWithoutFloor);
	J.Key("scraps_with_lighter_object").Integer(F.ScrapsWithLighterObject);
	J.Key("snaps_survived").Integer(F.SnapsSurvived);
	J.EndObject();
	// Time on the floor, in the air and hoisted cannot exceed time connected.
	J.Key("floor_seconds").Number(std::min(F.FloorSeconds, ConnectedSeconds), 1);
	J.Key("airborne_seconds").Number(std::min(F.AirborneSeconds, ConnectedSeconds), 1);
	J.Key("hoisted_seconds").Number(std::min(F.HoistedSeconds, ConnectedSeconds), 1);
	J.EndObject();
}
} // namespace

std::vector<FPlayerFacts> ComputeFacts(const std::vector<FMatchPlayer>& Roster, const FEventLog& Log,
                                       const FKineticTables& Tables)
{
	const FBotTimeline Bots(Roster, Log);
	FAccumulator Acc{Roster, Tables, Bots, {}, {}, {}};
	for (const FMatchPlayer& Player : Roster)
	{
		Acc.Facts[Player.Slot].Slot = Player.Slot;
	}
	for (const FMatchEvent& Event : Log.All())
	{
		std::visit([&Acc](const auto& E) { Acc.Add(E); }, Event);
	}
	Acc.CountSurvivedSnaps();
	std::vector<FPlayerFacts> Out;
	for (const FMatchPlayer& Player : Roster)
	{
		Out.push_back(Acc.Facts[Player.Slot]);
	}
	return Out;
}

std::string BuildMatchReport(const FMatchInfo& Info, const std::vector<FMatchPlayer>& Roster, const FEventLog& Log,
                             const std::vector<FPlayerConnection>& Connections, const FScrapResult& Result,
                             const FKineticTables& Tables)
{
	const std::vector<FPlayerFacts> Facts = ComputeFacts(Roster, Log, Tables);
	FJsonWriter J;
	J.BeginObject();
	J.Key("match_id").String(Info.MatchId).Key("build").String(Info.Build);
	J.Key("ruleset").Integer(RulesetVersion).Key("event_schema").Integer(EventSchemaVersion);
	J.Key("mode").String(Info.Mode).Key("arena").String(Info.Arena).Key("variant").String(Info.Variant);
	J.Key("winner_team");
	Result.WinnerTeam == NoPlayer ? J.Null() : J.Integer(Result.WinnerTeam + 1);
	J.Key("forfeit_team");
	Result.ForfeitTeam == NoPlayer ? J.Null() : J.Integer(Result.ForfeitTeam + 1);
	J.Key("forfeit_penalty_free").Bool(Result.bForfeitPenaltyFree).Key("no_contest").Bool(Result.bNoContest);

	J.Key("rounds").BeginArray();
	for (const FMatchEvent& Event : Log.All())
	{
		if (const auto* Round = std::get_if<FRoundEndEvent>(&Event))
		{
			J.BeginObject().Key("winner_team");
			Round->WinnerTeam == NoPlayer ? J.Null() : J.Integer(Round->WinnerTeam + 1);
			J.Key("seconds").Number(Round->Seconds, 1).Key("stalls").Integer(Round->Stalls);
			J.Key("end").String(ToString(Round->Reason)).EndObject();
		}
	}
	J.EndArray();

	J.Key("players").BeginArray();
	for (size_t I = 0; I < Roster.size(); ++I)
	{
		const FMatchPlayer& Player = Roster[I];
		const FPlayerConnection Connection = I < Connections.size() ? Connections[I] : FPlayerConnection{};
		J.BeginObject();
		J.Key("account_id");
		Player.bIsBot || Player.AccountId.empty() ? J.Null() : J.String(Player.AccountId);
		J.Key("team").Integer(Player.Team + 1);
		J.Key("harness").String(ToString(Player.Harness));
		J.Key("face");
		Player.Face.empty() ? J.Null() : J.String(Player.Face);
		J.Key("connected_seconds").Number(Connection.ConnectedSeconds, 1);
		J.Key("abandoned").Bool(Connection.bAbandoned);
		J.Key("is_bot").Bool(Player.bIsBot);
		WriteFacts(J, Facts[I], Connection.ConnectedSeconds);
		J.EndObject();
	}
	J.EndArray();
	J.Key("raw_events_sha256").String(Log.Sha256());
	J.EndObject();
	return J.Text();
}
} // namespace Kinetic
