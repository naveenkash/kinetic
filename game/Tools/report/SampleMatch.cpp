// Plays a scripted Scrap match through the match flow, credit and the event log, and writes
// the report and the raw events, so CI can check them as real JSON against the rules
// srv_store_report applies (Tools/ci/check_report.py).
//
//   sample-match REPORT_PATH EVENTS_PATH
#include "Kinetic/Credit.h"
#include "Kinetic/MatchReport.h"
#include "Kinetic/ScrapMatch.h"

#include <cstdio>
#include <fstream>
#include <string>

using namespace Kinetic;

namespace
{
constexpr double Tick = 1.0 / 60.0;

void Run(FScrapMatch& Match, double Seconds)
{
	for (int I = 0; I < static_cast<int>(Seconds / Tick + 0.5); ++I)
	{
		for (int Slot = 0; Slot < 6; ++Slot)
		{
			Match.OnInput(Slot);
		}
		Match.Update(Tick);
	}
}

void ToLive(FScrapMatch& Match)
{
	while (Match.Phase() != EScrapPhase::Live && Match.Phase() != EScrapPhase::MatchOver)
	{
		Run(Match, Tick);
	}
}

// A Stall credited from the lines on the victim, logged and fed to the flow.
void Scrap(FScrapMatch& Match, FCreditTracker& Credit, FEventLog& Log, double Now, int Victim, EStallCause Cause,
           ERig Rig = ERig::None, double RingOutHeight = 0.0)
{
	const FCredit Who = Credit.Attribute(Victim, Cause, NoObject, Now);
	Log.Append(FStallEvent{Now, Match.Round(), Victim, Cause, NoObject, 0.0, 0.0, Who.Killer, Who.Spotter, Who.Assists,
	                       Rig, RingOutHeight});
	Match.OnStall(Victim);
}

bool Write(const char* Path, const std::string& Text)
{
	std::ofstream File(Path, std::ios::binary);
	File << Text;
	return static_cast<bool>(File);
}
} // namespace

int main(int Argc, char** Argv)
{
	if (Argc != 3)
	{
		std::fprintf(stderr, "usage: sample-match REPORT_PATH EVENTS_PATH\n");
		return 2;
	}
	const FKineticTables Tables = FKineticTables::LoadFromDirectory(KINETIC_TABLES_DIR);
	const EHarnessClass Classes[] = {EHarnessClass::Rigger, EHarnessClass::Climber, EHarnessClass::Breaker};
	std::vector<FMatchPlayer> Roster;
	for (int Slot = 0; Slot < 6; ++Slot)
	{
		const bool bBot = Slot == 5;
		Roster.push_back({Slot, Slot / 3, bBot ? "" : "7d1b2c3a-0000-4000-8000-00000000000" + std::to_string(Slot), bBot,
		                  Classes[Slot % 3], Slot == 0 ? "teodora-quispe" : ""});
	}
	FEventLog Log;
	FScrapMatch Match(Roster, {}, Log);
	FCreditTracker Credit(Roster);
	Match.Start();
	// Team 1 wins rounds 0, 2 and 3; team 0 wins round 1.
	while (Match.Phase() != EScrapPhase::MatchOver)
	{
		ToLive(Match);
		const int Round = Match.Round();
		const int Losers = Round == 1 ? 1 : 0;
		Credit.ClearRound();
		Log.Append(FDogSeatEvent{Match.Time(), Round, (1 - Losers) * 3, 1, ESeatTarget::Player, Losers * 3, EPlate::Back});
		Run(Match, 20.0);
		for (int I = 0; I < 3; ++I)
		{
			const int Victim = Losers * 3 + I;
			const int Killer = (1 - Losers) * 3 + I;
			Credit.NoteLineOnPlayer(Killer, Victim, ELineMode::Haul, Match.Time());
			Scrap(Match, Credit, Log, Match.Time(), Victim, I == 0 ? EStallCause::RingOut : EStallCause::WorldImpact,
			      I == 0 ? ERig::Overboard : ERig::None, I == 0 ? 38.0 : 0.0);
			Run(Match, 3.0);
		}
		for (int Slot = 0; Slot < 6; ++Slot)
		{
			Log.Append(FPlayerRoundEvent{Round, Slot, 20.0, 9.0, 0.0, 1500.0 * (Slot + 1)});
		}
		while (Match.Phase() == EScrapPhase::RoundEnd)
		{
			Run(Match, Tick);
		}
	}
	std::vector<FPlayerConnection> Connections;
	for (const FMatchPlayer& Player : Roster)
	{
		Connections.push_back(Match.Connection(Player.Slot));
	}
	const FMatchInfo Info{"5b0f6a3e-9c1d-4e2f-8a7b-1c2d3e4f5a6b", "0.1.0", "scrap", "YRD", "base"};
	const std::string Report = BuildMatchReport(Info, Roster, Log, Connections, Match.Result(), Tables);
	if (!Write(Argv[1], Report + "\n") || !Write(Argv[2], Log.JsonLines()))
	{
		std::fprintf(stderr, "sample-match: cannot write the output\n");
		return 2;
	}
	std::printf("sample-match: %zu events, winner team %d\n", Log.All().size(), Match.WinnerTeam() + 1);
	return 0;
}
