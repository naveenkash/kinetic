// The match layer: the event stream, credit, the Scrap Card, the Scrap match flow
// (game-modes-and-match-flow sections 1, 2, 8, 9) and the match report (backend-architecture 12.1).
#include "Kinetic/Credit.h"
#include "Kinetic/Json.h"
#include "Kinetic/MatchReport.h"
#include "Kinetic/ScrapMatch.h"
#include "Kinetic/Sha256.h"
#include "TestHarness.h"

#include <clocale>
#include <string>

using namespace Kinetic;
using namespace Kinetic::Test;

namespace
{
constexpr double Tick = 1.0 / 60.0;

// Two crews of three: slots 0 to 2 on team 0, 3 to 5 on team 1.
std::vector<FMatchPlayer> Crews(bool bLastIsBot = false)
{
	const EHarnessClass Classes[] = {EHarnessClass::Rigger, EHarnessClass::Climber, EHarnessClass::Breaker};
	std::vector<FMatchPlayer> Roster;
	for (int Slot = 0; Slot < 6; ++Slot)
	{
		FMatchPlayer Player;
		Player.Slot = Slot;
		Player.Team = Slot / 3;
		Player.bIsBot = bLastIsBot && Slot == 5;
		Player.AccountId = Player.bIsBot ? "" : "00000000-0000-4000-8000-00000000000" + std::to_string(Slot);
		Player.Harness = Classes[Slot % 3];
		Player.Face = Slot == 0 ? "teodora-quispe" : "";
		Roster.push_back(Player);
	}
	return Roster;
}

// Steps the match with every rigger giving input, except Quiet.
void Step(FScrapMatch& Match, int Quiet = NoPlayer)
{
	for (int Slot = 0; Slot < 6; ++Slot)
	{
		if (Slot != Quiet)
		{
			Match.OnInput(Slot);
		}
	}
	Match.Update(Tick);
}

void Run(FScrapMatch& Match, double Seconds, int Quiet = NoPlayer)
{
	const int Steps = static_cast<int>(Seconds / Tick + 0.5);
	for (int I = 0; I < Steps; ++I)
	{
		Step(Match, Quiet);
	}
}

// Runs until the round is live (5 s reset, 6 s Drop).
void ToLive(FScrapMatch& Match)
{
	while (Match.Phase() != EScrapPhase::Live && Match.Phase() != EScrapPhase::MatchOver)
	{
		Step(Match);
	}
}

void ToNextRound(FScrapMatch& Match)
{
	while (Match.Phase() == EScrapPhase::RoundEnd || Match.Phase() == EScrapPhase::Reset)
	{
		Step(Match);
	}
	ToLive(Match);
}

int Count(const std::vector<FFlowEvent>& Events, EFlow Kind)
{
	return static_cast<int>(std::count_if(Events.begin(), Events.end(), [Kind](const FFlowEvent& E) { return E.Kind == Kind; }));
}

const FFlowEvent* FirstCall(const std::vector<FFlowEvent>& Events, EMarshalCall Call)
{
	for (const FFlowEvent& E : Events)
	{
		if (E.Kind == EFlow::Call && E.Call == Call)
		{
			return &E;
		}
	}
	return nullptr;
}

void Down(FScrapMatch& Match, int Team)
{
	for (int Slot = Team * 3; Slot < Team * 3 + 3; ++Slot)
	{
		Match.OnStall(Slot);
	}
}

bool Contains(const std::string& Text, const std::string& Part) { return Text.find(Part) != std::string::npos; }

const FRoundEndEvent& LastRound(const FEventLog& Log)
{
	for (auto It = Log.All().rbegin(); It != Log.All().rend(); ++It)
	{
		if (const auto* Round = std::get_if<FRoundEndEvent>(&*It))
		{
			return *Round;
		}
	}
	static const FRoundEndEvent None;
	return None;
}
} // namespace

// FIPS 180-4 test vectors.
KTEST(Sha256MatchesTheStandardVectors)
{
	KCHECK(FSha256::HexOf("") == "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855");
	KCHECK(FSha256::HexOf("abc") == "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad");
	KCHECK(FSha256::HexOf("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq") ==
	       "248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1");
	KCHECK(FSha256::HexOf(std::string(1000000, 'a')) ==
	       "cdc76e5c9914fb9281a1c7e284d73e67f1809a48a497200e046d39ccc7112cd0");
	FSha256 Parts;
	Parts.Update("ab");
	Parts.Update("c");
	const std::array<uint8_t, 32> Digest = Parts.Finish();
	KCHECK(Digest[0] == 0xba && Digest[31] == 0xad);
}

KTEST(JsonIsDeterministicAndEscaped)
{
	FJsonWriter J;
	J.BeginObject().Key("s").String("a\"b\\c\n\x01").Key("n").BeginArray();
	J.Number(2.5).Number(184200.0).Number(0.001).Number(-0.0001).Number(1.0 / 3.0).Number(std::nan("")).EndArray();
	J.Key("i").Integer(-7).Key("b").Bool(true).Key("z").Null().Key("o").BeginObject().EndObject().EndObject();
	KCHECK(J.Text() == R"({"s":"a\"b\\c\n\u0001","n":[2.5,184200,0.001,0,0.333,null],"i":-7,"b":true,"z":null,"o":{}})");
}

// Valid JSON for every finite number and every byte string: large values in exponent form,
// one rounding of the exact binary value, invalid UTF-8 and NUL replaced, U+2028 escaped.
KTEST(JsonStaysValidAtTheEdges)
{
	FJsonWriter Numbers;
	Numbers.BeginArray().Number(1e306).Number(-2e305).Number(1e15).Number(1.0, 400).Number(0.1175).Number(1.0005);
	Numbers.Number(-0.0).Number(123456789.987654, 9).EndArray();
	KCHECK(Numbers.Text() == "[1e+306,-2e+305,1e+15,1,0.117,1,0,123456789.987654001]");

	FJsonWriter Strings;
	Strings.BeginArray().String(std::string("a\0b", 3)).String("\xff\xc3").String("\xed\xa0\x80").String("\xe2\x80\xa8");
	Strings.String("caf\xc3\xa9 \xf0\x9f\x9a\x80").EndArray();
	// Overlong, a bad continuation byte, cut off at the end, past U+10FFFF: one U+FFFD per bad byte.
	FJsonWriter Broken;
	Broken.BeginArray().String("\xc0\xaf").String("\xc3\x28").String("\xe2\x82").String("\xf4\x90\x80\x80").EndArray();
	const std::string R = "\xef\xbf\xbd";
	KCHECK(Broken.Text() == "[\"" + R + R + "\",\"" + R + "(\",\"" + R + R + "\",\"" + R + R + R + R + "\"]");
	// A locale with a decimal comma changes nothing.
	const std::string Before = std::setlocale(LC_NUMERIC, nullptr);
	if (std::setlocale(LC_NUMERIC, "de_DE.UTF-8") || std::setlocale(LC_NUMERIC, "fr_FR.UTF-8"))
	{
		FJsonWriter Local;
		Local.BeginArray().Number(2.5).Number(184200.0).Number(-0.0001).EndArray();
		KCHECK(Local.Text() == "[2.5,184200,0]");
	}
	std::setlocale(LC_NUMERIC, Before.c_str());
	KCHECK(Strings.Text() == "[\"a\xef\xbf\xbd" "b\",\"\xef\xbf\xbd\xef\xbf\xbd\",\"\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\",\"\\u2028\",\"caf\xc3\xa9 \xf0\x9f\x9a\x80\"]");
}

// Every event type writes one line with its type and the schema version; the digest is of
// exactly those bytes.
KTEST(EventLogWritesJsonLinesAndTheirDigest)
{
	FEventLog Log;
	Log.Append(FDogSeatEvent{1.25, 0, 3, 7, ESeatTarget::Player, 1, EPlate::Back});
	Log.Append(FModeEvent{2.0, 0, 3, 7, ELineMode::Haul, 1000.0, 0.75, 220.0, 90.0, ELineOutcome::Cut});
	Log.Append(FCutEvent{2.0, 0, 1, 3, 7, 0.75, EPlate::Back, 152.0});
	Log.Append(FStallEvent{9.0, 0, 1, EStallCause::RingOut, NoObject, 0.0, 0.0, 3, 4, {5}, ERig::Overboard, 38.0});
	Log.Append(FHitEvent{3.0, 0, 2, EOutcome::Flip, EStallCause::ObjectImpact, 650.0});
	Log.Append(FSnapEvent{4.0, 0, 4, 9, 1200.0});
	Log.Append(FRatedCommitEvent{5.0, 0, 5, 4000.0});
	Log.Append(FPlayerRoundEvent{0, 1, 30.0, 12.0, 0.0, 4500.0});
	Log.Append(FRoundEndEvent{60.0, 0, 1, 60.0, 3, ERoundEnd::CrewDown});
	Log.Append(FConnectionEvent{61.0, 2, EConnection::Disconnected});
	Log.Append(FServerSecondEvent{62.0, 8.5, 3.25, 140, 6});
	const std::string Lines = Log.JsonLines();
	KCHECK(std::count(Lines.begin(), Lines.end(), '\n') == 11);
	for (const char* Type : {"dog_seat", "mode", "cut", "stall", "hit", "snap", "rated_commit", "player_round",
	                         "round_end", "connection", "server_second"})
	{
		KCHECK(Contains(Lines, std::string("{\"type\":\"") + Type + "\",\"v\":1"));
	}
	KCHECK(Contains(Lines, R"("victim":1,"cause":"ring_out","object":-1,"object_mass_kg":0,"value":0,"killer":3,"spotter":4,"assists":[5],"rig":"Overboard","ringout_height_m":38})"));
	KCHECK(Log.Sha256() == FSha256::HexOf(Lines));
}

// Credit: the killer is the latest enemy active line, the Spotter the latest other enemy Hang
// or Set line, the rest assist; teammates and lines older than 5 s earn nothing.
KTEST(CreditFollowsTheEnemyLines)
{
	FCreditTracker Haul(Crews());
	Haul.NoteLineOnPlayer(3, 1, ELineMode::Haul, 10.0);
	Haul.NoteLineOnPlayer(2, 1, ELineMode::Haul, 10.5);  // A teammate's line: no credit
	const FCredit Hauled = Haul.Attribute(1, EStallCause::WorldImpact, NoObject, 11.0);
	KCHECK(Hauled.Killer == 3 && Hauled.Spotter == NoPlayer && Hauled.Assists.empty());

	FCreditTracker Guillotine(Crews());
	Guillotine.NoteLineOnObject(4, 77, ELineMode::Set, 12.0);  // The load hung over the doorway
	const FCredit Dropped = Guillotine.Attribute(0, EStallCause::ObjectImpact, 77, 12.5);
	KCHECK(Dropped.Killer == 4 && Dropped.Spotter == NoPlayer);
	Guillotine.NoteLineOnObject(3, 77, ELineMode::Haul, 12.4);  // A teammate steered it in
	const FCredit Steered = Guillotine.Attribute(0, EStallCause::ObjectImpact, 77, 12.5);
	KCHECK(Steered.Killer == 3 && Steered.Spotter == 4);

	FCreditTracker Clothesline(Crews());
	Clothesline.NoteLineOnPlayer(5, 2, ELineMode::Set, 20.0);   // Tripped on the set line
	Clothesline.NoteLineOnPlayer(3, 2, ELineMode::Jack, 20.4);  // Jacked off the edge
	Clothesline.NoteLineOnPlayer(4, 2, ELineMode::Haul, 19.0);
	const FCredit Over = Clothesline.Attribute(2, EStallCause::RingOut, NoObject, 21.0);
	KCHECK(Over.Killer == 3 && Over.Spotter == 5 && Over.Assists == std::vector<int>{4});

	// An object's lines count only for a hit by that object.
	FCreditTracker Fall(Crews());
	Fall.NoteLineOnObject(3, 9, ELineMode::Haul, 30.0);
	KCHECK(Fall.Attribute(1, EStallCause::WorldImpact, 9, 30.5).Killer == NoPlayer);
	KCHECK(Fall.Attribute(1, EStallCause::Blast, 9, 30.5).Killer == 3);
	KCHECK(Fall.Attribute(1, EStallCause::Blast, 9, 35.1).Killer == NoPlayer);  // Over 5 s ago
	Fall.ClearRound();
	KCHECK(Fall.Attribute(1, EStallCause::Blast, 9, 30.5).Killer == NoPlayer);

	// Simultaneous lines: the lower slot, never map order.
	FCreditTracker Tie(Crews());
	Tie.NoteLineOnPlayer(5, 0, ELineMode::Haul, 40.0);
	Tie.NoteLineOnPlayer(4, 0, ELineMode::Haul, 40.0);
	const FCredit Tied = Tie.Attribute(0, EStallCause::WorldImpact, NoObject, 40.0);
	KCHECK(Tied.Killer == 4 && Tied.Assists == std::vector<int>{5});
}

// 7.4 and 7.5: the card, the body and the Hook.
KTEST(ScrapCardsAndBodies)
{
	const std::vector<FMatchPlayer> Roster = Crews();
	FStallEvent Ring{1.0, 0, 1, EStallCause::RingOut, NoObject, 0.0, 0.0, 3, NoPlayer, {}, ERig::None, 40.0};
	const FScrapCard Well = MakeScrapCard(Ring, Roster[1], false, Tables());
	KCHECK(Well.bDownTheWell && !Well.bHookRetrieves && Well.BodyMassKg == 0.0);

	FStallEvent Hit{1.0, 0, 1, EStallCause::ObjectImpact, 5, 180.0, 1260.0, 3, 4, {}, ERig::Guillotine, 0.0};
	const FScrapCard Climber = MakeScrapCard(Hit, Roster[1], false, Tables());
	KCHECK(!Climber.bDownTheWell && Climber.bHookRetrieves && Climber.BodyMassKg == 90.0);
	KCHECK(Climber.bBodyTakesScrapperBonus);  // A scrapped Climber is a Medium prop under 100 kg
	KCHECK(Climber.Killer == 3 && Climber.Spotter == 4 && Climber.Rig == ERig::Guillotine);
	KCHECK(Climber.HookArrivesSeconds == 4.0 && Climber.HookLiftSeconds == 2.0);
	Hit.Victim = 2;
	const FScrapCard Breaker = MakeScrapCard(Hit, Roster[2], true, Tables());
	KCHECK(Breaker.BodyMassKg == 220.0 && !Breaker.bBodyTakesScrapperBonus && !Breaker.bHookRetrieves);
}

// game-modes 2: the round, second by second.
KTEST(TheScrapRoundRunsOnTheClock)
{
	FEventLog Log;
	FScrapMatch Match(Crews(), {}, Log);
	Match.Start();
	std::vector<FFlowEvent> Events = Match.TakeEvents();
	KCHECK(Count(Events, EFlow::ResetArena) == 1 && FirstCall(Events, EMarshalCall::CrewsOnTheClaim));
	Run(Match, 5.0);
	KCHECK(Match.Phase() == EScrapPhase::Drop && !Match.CanUseLines() && !Match.CanMove(0));
	Run(Match, 4.0 + Tick);
	KCHECK(Match.CanMove(0) && !Match.CanUseLines());  // From 2:26
	Run(Match, 2.0);
	KCHECK(Match.Phase() == EScrapPhase::Live && Match.CanUseLines());
	Events = Match.TakeEvents();
	KCHECK(FirstCall(Events, EMarshalCall::Live) && std::abs(FirstCall(Events, EMarshalCall::Live)->Time - 11.0) < 0.02);

	Run(Match, 64.0);  // To 1:20
	Events = Match.TakeEvents();
	KCHECK(Count(Events, EFlow::Siren) == 1 && FirstCall(Events, EMarshalCall::EventInFive));
	KCHECK(Count(Events, EFlow::MidMatchEvent) == 0);
	Run(Match, 5.0);
	KCHECK(Count(Match.TakeEvents(), EFlow::MidMatchEvent) == 1);
	Run(Match, 65.0);
	KCHECK(Count(Match.TakeEvents(), EFlow::Caution) == 1);
	KCHECK(Match.ClockRemaining() <= 10.0 + 1e-9);

	// Time-out with more riggers standing on team 1.
	Match.OnStall(0);
	Run(Match, 11.0);
	Events = Match.TakeEvents();
	KCHECK(FirstCall(Events, EMarshalCall::TimeCount));
	KCHECK(Match.Phase() == EScrapPhase::RoundEnd && Match.Wins(1) == 1);
	const FRoundEndEvent* Round = &LastRound(Log);
	KCHECK(Round && Round->Reason == ERoundEnd::TimeOut && Round->WinnerTeam == 1 && Round->Stalls == 1);
	KCHECK(std::abs(Round->Seconds - 150.0) < 0.05);
}

// A round ends the moment a crew is down; first to three wins; sides swap every round.
KTEST(BestOfFiveWithSideSwaps)
{
	FEventLog Log;
	FScrapMatch Match(Crews(), {}, Log);
	Match.Start();
	ToLive(Match);
	KCHECK(Match.SpawnOf(0) == 0 && Match.SpawnOf(1) == 1);
	Down(Match, 1);
	KCHECK(Match.Phase() == EScrapPhase::RoundEnd && Match.Wins(0) == 1);
	KCHECK(!Match.OnStall(0));  // Not during the replay
	Run(Match, 4.0 + Tick);
	KCHECK(Match.Phase() == EScrapPhase::Reset && Match.Round() == 1);
	KCHECK(Match.SpawnOf(0) == 1 && Match.IsStanding(0) && Match.IsStanding(3));
	ToLive(Match);
	Down(Match, 0);
	ToNextRound(Match);
	Down(Match, 1);
	ToNextRound(Match);
	Down(Match, 1);
	KCHECK(Match.Wins(0) == 3 && Match.Wins(1) == 1);
	Run(Match, 4.0 + Tick);
	KCHECK(Match.Phase() == EScrapPhase::MatchOver && Match.WinnerTeam() == 0 && !Match.WasForfeit());
	KCHECK(Count(Match.TakeEvents(), EFlow::MatchOver) == 1);
}

// Equal at time-out: sudden death, the first fall decides it; if nobody falls, the round is
// replayed and not scored.
KTEST(SuddenDeathAndReplays)
{
	FEventLog Log;
	FScrapMatch Match(Crews(), {}, Log);
	Match.Start();
	ToLive(Match);
	Match.OnStall(0);
	Match.OnStall(3);
	Run(Match, 145.0);
	KCHECK(Match.Phase() == EScrapPhase::SuddenDeath && Match.CanUseLines());
	const std::vector<FFlowEvent> Events = Match.TakeEvents();
	KCHECK(FirstCall(Events, EMarshalCall::SuddenDeath) && Count(Events, EFlow::ShrinkArena) == 1);
	Match.OnStall(4);
	KCHECK(Match.Phase() == EScrapPhase::RoundEnd && Match.Wins(0) == 1);
	KCHECK(LastRound(Log).Reason == ERoundEnd::SuddenDeath);

	ToNextRound(Match);
	Run(Match, 145.0);
	KCHECK(Match.Phase() == EScrapPhase::SuddenDeath);
	Run(Match, 90.0 + Tick);
	KCHECK(Match.Phase() == EScrapPhase::RoundEnd && Match.Wins(0) == 1 && Match.Wins(1) == 0);
	const FRoundEndEvent& Replayed = LastRound(Log);
	KCHECK(Replayed.Reason == ERoundEnd::Replayed && Replayed.WinnerTeam == NoPlayer);

	// A crew going down ends the round at once, even with the other crew down to one.
	ToNextRound(Match);
	for (const int Slot : {0, 1, 3, 4, 2})
	{
		Match.OnStall(Slot);
	}
	KCHECK(Match.Phase() == EScrapPhase::RoundEnd && Match.Wins(1) == 1);
	KCHECK(Match.IsStanding(5));
}

// game-modes 8 and 9, casual: the rigger stands idle 10 s, then a bot takes it; the leaver can
// return within 60 s and takes it straight back.
KTEST(CasualLeaversAreBackfilled)
{
	FEventLog Log;
	FScrapMatch Match(Crews(), {}, Log);
	Match.Start();
	ToLive(Match);
	Match.OnDisconnect(1);
	Run(Match, 9.9);
	KCHECK(!Match.IsBotControlled(1) && Match.IsStanding(1));  // Idle, and a valid target
	Run(Match, 0.2);
	KCHECK(Match.IsBotControlled(1) && Match.IsStanding(1));
	const std::vector<FFlowEvent> Events = Match.TakeEvents();
	KCHECK(Count(Events, EFlow::Backfill) == 1 && FirstCall(Events, EMarshalCall::OfficeCrewIn));
	Match.OnReconnect(1);
	KCHECK(!Match.IsBotControlled(1) && Match.IsStanding(1));
	KCHECK(Count(Match.TakeEvents(), EFlow::Rejoin) == 1);
	KCHECK(!Match.Connection(1).bAbandoned);
	Down(Match, 1);
	ToNextRound(Match);

	Match.OnDisconnect(2);
	Run(Match, 61.0);
	KCHECK(Match.Connection(2).bAbandoned && Match.IsBotControlled(2));
	Match.OnReconnect(2);  // Too late
	KCHECK(Match.Connection(2).bAbandoned && Match.IsBotControlled(2));
	KCHECK(Match.Connection(0).ConnectedSeconds > Match.Connection(2).ConnectedSeconds);
}

// Rated: no backfill. The rigger is withdrawn (not standing); the short crew may forfeit
// without penalty after 60 s; past 90 s the leaver has abandoned.
KTEST(RatedLeaversAreWithdrawn)
{
	FScrapRules Rated;
	Rated.bRated = true;
	FEventLog Log;
	FScrapMatch Match(Crews(), Rated, Log);
	Match.Start();
	ToLive(Match);
	Match.OnDisconnect(0);
	Run(Match, 10.1);
	KCHECK(!Match.IsStanding(0) && !Match.IsBotControlled(0) && Match.StandingCount(0) == 2);
	KCHECK(!Match.CanForfeitWithoutPenalty(0));
	Run(Match, 50.0);
	KCHECK(Match.CanForfeitWithoutPenalty(0) && !Match.CanForfeitWithoutPenalty(1));
	Match.OnStall(1);
	Match.OnStall(2);
	KCHECK(Match.Phase() == EScrapPhase::RoundEnd && Match.Wins(1) == 1);  // The withdrawn rigger is down
	ToNextRound(Match);
	KCHECK(!Match.IsStanding(0) && Match.StandingCount(0) == 2);
	Run(Match, 30.0);
	KCHECK(Match.Connection(0).bAbandoned);
	Match.OnForfeitVote(1, true);
	KCHECK(Match.Phase() != EScrapPhase::MatchOver);
	Match.OnForfeitVote(2, true);
	KCHECK(Match.Phase() == EScrapPhase::MatchOver && Match.WasForfeit() && Match.WinnerTeam() == 1);
	const std::vector<FFlowEvent> Events = Match.TakeEvents();
	KCHECK(FirstCall(Events, EMarshalCall::CrewForfeited) && FirstCall(Events, EMarshalCall::CrewForfeited)->Team == 0);
	KCHECK(LastRound(Log).Reason == ERoundEnd::Forfeit);
	KCHECK(Match.Wins(1) == 1);  // The forfeited round is logged, not scored
}

// A crew of bots cannot exist: a crew with no humans left forfeits.
KTEST(ACrewWithNoHumansForfeits)
{
	FEventLog Log;
	std::vector<FMatchPlayer> Roster = Crews();
	Roster[4].bIsBot = true;
	Roster[5].bIsBot = true;
	FScrapMatch Match(Roster, {}, Log);
	Match.Start();
	ToLive(Match);
	Match.OnDisconnect(3);
	Run(Match, 10.1);
	KCHECK(Match.Phase() == EScrapPhase::MatchOver && Match.WasForfeit() && Match.WinnerTeam() == 0);
}

// Idle: 20 s, the Office notes it; 60 s, a leaver. Only while the rigger can move.
KTEST(IdleRiggersBecomeLeavers)
{
	FEventLog Log;
	FScrapMatch Match(Crews(), {}, Log);
	Match.Start();
	Run(Match, 9.0, 0);  // Reset and most of the Drop: not counted
	ToLive(Match);       // Slot 0 gives its last input as the round goes live
	Run(Match, 18.0, 0);
	KCHECK(!FirstCall(Match.TakeEvents(), EMarshalCall::OfficeNotesStill));
	Run(Match, 2.5, 0);
	const std::vector<FFlowEvent> Events = Match.TakeEvents();
	const FFlowEvent* Note = FirstCall(Events, EMarshalCall::OfficeNotesStill);
	KCHECK(Note && Note->Player == 0);
	Run(Match, 39.0, 0);
	KCHECK(!Match.IsBotControlled(0));
	Run(Match, 1.0, 0);  // 60 s: a leaver
	Run(Match, 10.1, 0);
	KCHECK(Match.IsBotControlled(0));
	bool bDisconnected = false;
	for (const FMatchEvent& Event : Log.All())
	{
		const auto* Change = std::get_if<FConnectionEvent>(&Event);
		bDisconnected = bDisconnected || (Change && Change->Player == 0 && Change->Change == EConnection::Disconnected);
	}
	KCHECK(bDisconnected);
}

// Facts from the event log, and the report the backend stores.
KTEST(TheReportCarriesTheFacts)
{
	std::vector<FMatchPlayer> Roster = Crews(true);  // Slot 5 is a bot
	FEventLog Log;
	FScrapMatch Match(Roster, {}, Log);
	Match.Start();
	ToLive(Match);
	// Round 0: slot 3 Jacks slot 1 down the Well off slot 4's set line, slot 5 helping.
	Log.Append(FStallEvent{20.0, 0, 1, EStallCause::RingOut, NoObject, 0.0, 0.0, 3, 4, {5}, ERig::Overboard, 38.5});
	Match.OnStall(1);
	// A 45 kg cylinder Hauled into the 130 kg Rigger by slot 3, the bot (slot 5) having hung it.
	Log.Append(FStallEvent{25.0, 0, 0, EStallCause::ObjectImpact, 8, 45.0, 1215.0, 3, 5, {}, ERig::None, 0.0});
	Match.OnStall(0);
	Log.Append(FSnapEvent{26.0, 0, 4, 2, 1200.0});
	Log.Append(FSnapEvent{27.0, 0, 3, 3, 1200.0});
	Log.Append(FStallEvent{29.0, 0, 3, EStallCause::WorldImpact, NoObject, 0.0, 12.5, 2, NoPlayer, {}, ERig::None, 0.0});
	Match.OnStall(3);
	Log.Append(FRatedCommitEvent{30.0, 0, 2, 4000.0});
	Log.Append(FPlayerRoundEvent{0, 3, 0.0, 20.0, 5.0, 3200.0});
	Log.Append(FPlayerRoundEvent{0, 4, 15.0, 10.0, 0.0, 800.0});
	Match.OnStall(2);
	ToNextRound(Match);
	Log.Append(FPlayerRoundEvent{1, 3, 10.0, 5.0, 0.0, 1000.0});
	Down(Match, 0);
	ToNextRound(Match);
	Down(Match, 0);
	Run(Match, 5.0);
	KCHECK(Match.Phase() == EScrapPhase::MatchOver && Match.WinnerTeam() == 1);

	const std::vector<FPlayerFacts> Facts = ComputeFacts(Roster, Log, Tables());
	KCHECK(Facts[3].Eliminations == 2 && Facts[3].RingOutHeightsM == std::vector<double>{38.5});
	KCHECK(Facts[3].RopeMarks.size() == 1 && Facts[3].RopeMarks[0].Rig == ERig::Overboard);
	KCHECK(Facts[3].SpotterCredits == 1);  // The bot's Spotter credit is the human killer's
	KCHECK(Facts[3].ScrapsWithLighterObject == 1);
	KCHECK(Facts[3].SnapsSurvived == 0 && Facts[4].SnapsSurvived == 1);  // Slot 3 fell 2 s after its snap
	KCHECK(Facts[3].RoundsWithoutFloor == 1 && Facts[4].RoundsWithoutFloor == 0);
	KCHECK(std::abs(Facts[3].MassDistanceKgM - 4200.0) < 1e-9);
	KCHECK(Facts[4].SpotterCredits == 1 && Facts[4].LinesHungUsedByOthers == 1);
	KCHECK(Facts[5].Assists == 0 && Facts[5].SpotterCredits == 0);  // Bots earn nothing
	KCHECK(Facts[2].Eliminations == 1 && std::abs(Facts[2].RatedTonsMoved - 4.0) < 1e-12);

	std::vector<FPlayerConnection> Connections;
	for (const FMatchPlayer& Player : Roster)
	{
		Connections.push_back(Match.Connection(Player.Slot));
	}
	const std::string Report =
	    BuildMatchReport({"m-1", "0.1.0", "scrap", "YRD", "base"}, Roster, Log, Connections, Match.Result(), Tables());
	KCHECK(Contains(Report, R"({"match_id":"m-1","build":"0.1.0","ruleset":1,"event_schema":1,"mode":"scrap","arena":"YRD","variant":"base","winner_team":2,"forfeit_team":null,"forfeit_penalty_free":false,"no_contest":false,"rounds":[)"));
	KCHECK(Contains(Report, R"("account_id":"00000000-0000-4000-8000-000000000000","team":1,"harness":"Rigger","face":"teodora-quispe")"));
	KCHECK(Contains(Report, R"("account_id":null,"team":2,"harness":"Breaker","face":null)"));
	KCHECK(Contains(Report, R"("is_bot":true,"facts":{"mass_distance_kg_m":0,"eliminations":0,"assists":0)"));
	KCHECK(Contains(Report, R"("ringout_heights_m":[38.5],"rope_marks":[{"rig":"Overboard"}])"));
	KCHECK(Contains(Report, "\"raw_events_sha256\":\"" + Log.Sha256() + "\"}"));
	const size_t Rounds = std::count_if(Log.All().begin(), Log.All().end(),
	                                    [](const FMatchEvent& E) { return std::holds_alternative<FRoundEndEvent>(E); });
	KCHECK(Rounds == 3);
}

// Once a crew has its third round, nothing in the replay changes the result: not a forfeit
// vote from either crew, not a leaver.
KTEST(ADecidedMatchStandsThroughTheReplay)
{
	for (const int Voters : {0, 1})
	{
		FEventLog Log;
		FScrapMatch Match(Crews(), {}, Log);
		Match.Start();
		for (int Round = 0; Round < 3; ++Round)
		{
			ToNextRound(Match);
			Down(Match, 1);
		}
		KCHECK(Match.Phase() == EScrapPhase::RoundEnd && Match.WinnerTeam() == 0);
		Match.OnForfeitVote(Voters * 3, true);
		Match.OnForfeitVote(Voters * 3 + 1, true);
		Run(Match, 5.0);
		KCHECK(Match.Phase() == EScrapPhase::MatchOver && Match.WinnerTeam() == 0 && !Match.WasForfeit());
	}
	std::vector<FMatchPlayer> Roster = Crews();
	Roster[1].bIsBot = Roster[2].bIsBot = true;
	FEventLog Log;
	FScrapMatch Match(Roster, {}, Log);
	Match.Start();
	for (int Round = 0; Round < 2; ++Round)
	{
		ToNextRound(Match);
		Down(Match, 1);
	}
	ToNextRound(Match);
	Match.OnDisconnect(0);  // The crew's only human leaves 8 s before the winning Stall
	Run(Match, 8.0);
	Down(Match, 1);
	Run(Match, 5.0);
	KCHECK(Match.WinnerTeam() == 0 && !Match.WasForfeit());
}

// Both crews losing their last humans in the same instant is a no-contest, whatever the roster order.
KTEST(BothCrewsGoneTogetherIsANoContest)
{
	for (const bool bReversed : {false, true})
	{
		std::vector<FMatchPlayer> Roster = Crews();
		if (bReversed)
		{
			std::reverse(Roster.begin(), Roster.end());
		}
		FEventLog Log;
		FScrapMatch Match(Roster, {}, Log);
		Match.Start();
		ToLive(Match);
		for (int Slot = 0; Slot < 6; ++Slot)
		{
			Match.OnDisconnect(Slot);
		}
		Run(Match, 10.1);
		KCHECK(Match.Phase() == EScrapPhase::MatchOver && Match.WinnerTeam() == NoPlayer);
		KCHECK(Match.Result().bNoContest && !Match.WasForfeit());
	}
}

// A backfill bot holding a leaver's seat earns nothing for the leaver; its Spotter credit goes
// to the human killer. Credit resumes once the leaver is back.
KTEST(ABackfillBotEarnsNothingForTheLeaver)
{
	const std::vector<FMatchPlayer> Roster = Crews();
	FEventLog Log;
	Log.Append(FConnectionEvent{10.0, 1, EConnection::Backfilled});
	Log.Append(FStallEvent{20.0, 0, 3, EStallCause::WorldImpact, NoObject, 0.0, 0.0, 1, NoPlayer, {2}, ERig::None, 0.0});
	Log.Append(FStallEvent{21.0, 0, 4, EStallCause::WorldImpact, NoObject, 0.0, 0.0, 0, 1, {}, ERig::None, 0.0});
	Log.Append(FPlayerRoundEvent{0, 1, 30.0, 0.0, 0.0, 900.0, true});
	Log.Append(FConnectionEvent{30.0, 1, EConnection::Returned});
	Log.Append(FStallEvent{40.0, 1, 5, EStallCause::WorldImpact, NoObject, 0.0, 0.0, 1, NoPlayer, {}, ERig::None, 0.0});
	Log.Append(FPlayerRoundEvent{1, 1, 30.0, 0.0, 0.0, 100.0, false});
	const std::vector<FPlayerFacts> Facts = ComputeFacts(Roster, Log, Tables());
	KCHECK(Facts[1].Eliminations == 1 && Facts[1].SpotterCredits == 0 && Facts[1].LinesHungUsedByOthers == 0);
	KCHECK(Facts[0].Eliminations == 1 && Facts[0].SpotterCredits == 1);  // The bot set it up
	KCHECK(Facts[2].Assists == 1);
	KCHECK(std::abs(Facts[1].MassDistanceKgM - 100.0) < 1e-12 && std::abs(Facts[1].FloorSeconds - 30.0) < 1e-12);
}

// Rated: a crew whose riggers are all withdrawn by the Drop loses the round there and then.
KTEST(ACrewWithNobodyStandingAtTheDropLoses)
{
	FScrapRules Rated;
	Rated.bRated = true;
	FEventLog Log;
	FScrapMatch Match(Crews(), Rated, Log);
	Match.Start();
	ToLive(Match);
	Match.OnDisconnect(0);
	Match.OnDisconnect(1);
	Run(Match, 10.1);
	Run(Match, 30.0);
	Match.OnDisconnect(2);
	Run(Match, 4.0);
	Down(Match, 1);  // Team 0 wins round 0 with slot 2 still standing, idle
	Run(Match, 5.0);
	KCHECK(Match.Phase() == EScrapPhase::Reset);
	Match.OnReconnect(0);  // A human again, so no forfeit, but withdrawn until the next reset
	Run(Match, 1.5);       // Slot 2 is withdrawn now, in the reset
	while (Match.Phase() == EScrapPhase::Reset)
	{
		Step(Match);
	}
	KCHECK(Match.Phase() == EScrapPhase::RoundEnd && Match.Wins(1) == 1);
	KCHECK(LastRound(Log).Reason == ERoundEnd::CrewDown && LastRound(Log).Seconds < 0.05);
}

// An idle leaver who moves again within the window is back, not abandoned.
KTEST(AnIdleLeaverCanComeBack)
{
	FEventLog Log;
	FScrapMatch Match(Crews(), {}, Log);
	Match.Start();
	ToLive(Match);
	Run(Match, 60.5, 0);
	KCHECK(Match.Connection(0).ConnectedSeconds < Match.Connection(1).ConnectedSeconds);
	Run(Match, 12.0, 0);
	KCHECK(Match.IsBotControlled(0));
	Match.OnInput(0);
	KCHECK(!Match.IsBotControlled(0));
	Run(Match, 80.0);
	KCHECK(!Match.Connection(0).bAbandoned);
	KCHECK(Match.Connection(1).ConnectedSeconds - Match.Connection(0).ConnectedSeconds < 13.0);
}

// Rated: a leaver who returns and drops again before the reset does not get the rigger back.
KTEST(AReturnThatDropsAgainIsCancelled)
{
	FScrapRules Rated;
	Rated.bRated = true;
	FEventLog Log;
	FScrapMatch Match(Crews(), Rated, Log);
	Match.Start();
	ToLive(Match);
	Match.OnDisconnect(0);
	Run(Match, 10.1);
	Match.OnReconnect(0);
	Match.OnDisconnect(0);
	Down(Match, 1);
	Match.TakeEvents();
	ToNextRound(Match);
	KCHECK(Count(Match.TakeEvents(), EFlow::Rejoin) == 0 && !Match.IsStanding(0));
}

// A forfeit is in the log and the report, with whether it was penalty-free, even outside a round.
KTEST(ForfeitsReachTheReport)
{
	FScrapRules Rated;
	Rated.bRated = true;
	FEventLog Log;
	std::vector<FMatchPlayer> Roster = Crews();
	FScrapMatch Match(Roster, Rated, Log);
	Match.Start();
	ToLive(Match);
	Match.OnDisconnect(2);
	Run(Match, 61.0);
	KCHECK(Match.CanForfeitWithoutPenalty(0));
	Down(Match, 1);
	Run(Match, 5.0);
	KCHECK(Match.Phase() == EScrapPhase::Reset);
	Match.OnForfeitVote(0, true);
	Match.OnForfeitVote(1, true);
	KCHECK(Match.Phase() == EScrapPhase::MatchOver && Match.WinnerTeam() == 1);
	KCHECK(Match.Result().ForfeitTeam == 0 && Match.Result().bForfeitPenaltyFree);
	const auto* End = std::get_if<FMatchEndEvent>(&Log.All().back());
	KCHECK(End && End->ForfeitTeam == 0 && End->bForfeitPenaltyFree);
	std::vector<FPlayerConnection> Connections;
	for (const FMatchPlayer& Player : Roster)
	{
		Connections.push_back(Match.Connection(Player.Slot));
	}
	const std::string Report = BuildMatchReport({"m-2", "0.1.0", "scrap", "YRD", "base"}, Roster, Log, Connections,
	                                            Match.Result(), Tables());
	KCHECK(Contains(Report, R"("winner_team":2,"forfeit_team":1,"forfeit_penalty_free":true,"no_contest":false)"));
	KCHECK(Contains(Log.JsonLines(), R"({"type":"match_end","v":1)"));

	FScrapMatch Casual(Crews(), {}, Log);  // Never penalty-free in casual
	Casual.Start();
	ToLive(Casual);
	Casual.OnDisconnect(2);
	Run(Casual, 61.0);
	KCHECK(!Casual.CanForfeitWithoutPenalty(0));
}

// Votes clear at each reset and on a disconnect; with one human left, one vote is enough.
KTEST(ForfeitVotesAreCurrent)
{
	FEventLog Log;
	FScrapMatch Match(Crews(), {}, Log);
	Match.Start();
	ToLive(Match);
	Match.OnForfeitVote(0, true);
	Down(Match, 1);
	ToNextRound(Match);
	Match.OnForfeitVote(1, true);
	KCHECK(Match.Phase() != EScrapPhase::MatchOver);  // Round 0's vote is gone
	Match.OnDisconnect(1);
	Match.OnReconnect(1);
	Match.OnForfeitVote(2, true);
	KCHECK(Match.Phase() != EScrapPhase::MatchOver);  // Slot 1's vote went with the disconnect

	FScrapMatch Alone(Crews(), {}, Log);
	Alone.Start();
	ToLive(Alone);
	Alone.OnDisconnect(1);
	Alone.OnDisconnect(2);
	Alone.OnForfeitVote(0, true);
	KCHECK(Alone.Phase() == EScrapPhase::MatchOver && Alone.WasForfeit());
}

// The thresholds exactly: idle is counted from 2:26, warned at 20 s, a leaver at 60 s; a
// rated leaver is abandoned at 90 s, not 60; a late return changes nothing.
KTEST(LeaverAndIdleThresholdsExactly)
{
	FEventLog Log;
	FScrapMatch Idle(Crews(), {}, Log);
	Idle.Start();
	Run(Idle, 28.9, 0);  // Movement from 9.0 s: 19.9 s idle
	KCHECK(!FirstCall(Idle.TakeEvents(), EMarshalCall::OfficeNotesStill));
	Run(Idle, 0.2, 0);
	KCHECK(FirstCall(Idle.TakeEvents(), EMarshalCall::OfficeNotesStill));
	Run(Idle, 39.8, 0);  // 59.9 s
	KCHECK(Idle.Connection(0).ConnectedSeconds > 68.8);  // Still connected at 59.9 s idle
	Run(Idle, 0.2, 0);
	Run(Idle, 1.0, 0);
	KCHECK(Idle.Connection(0).ConnectedSeconds < 69.3);

	FScrapRules Rated;
	Rated.bRated = true;
	FScrapMatch Gone(Crews(), Rated, Log);
	Gone.Start();
	ToLive(Gone);
	Gone.OnDisconnect(4);
	Run(Gone, 89.9);
	KCHECK(!Gone.Connection(4).bAbandoned);
	Run(Gone, 0.2);
	KCHECK(Gone.Connection(4).bAbandoned);
	const double Before = Gone.Connection(4).ConnectedSeconds;
	Gone.OnReconnect(4);
	Run(Gone, 5.0);
	KCHECK(Gone.Connection(4).ConnectedSeconds == Before);  // Too late: still gone
}

// A withdrawal alone can end the round; a Stall in the Drop counts.
KTEST(WithdrawalsAndDropStallsEndRounds)
{
	FScrapRules Rated;
	Rated.bRated = true;
	FEventLog Log;
	FScrapMatch Match(Crews(), Rated, Log);
	Match.Start();
	ToLive(Match);
	Match.OnStall(1);
	Match.OnStall(2);
	Match.OnDisconnect(0);
	Run(Match, 10.1);
	KCHECK(Match.Phase() == EScrapPhase::RoundEnd && Match.Wins(1) == 1 && LastRound(Log).Stalls == 2);

	FScrapMatch Drop(Crews(), {}, Log);
	Drop.Start();
	Run(Drop, 6.0);
	KCHECK(Drop.Phase() == EScrapPhase::Drop && Drop.OnStall(3));
	KCHECK(!Drop.IsStanding(3) && Drop.StandingCount(1) == 2);
}

// Credit windows: Hang and Set lines count for 5 s, as do assists; snaps are survived if the
// owner is not Stalled within 5 s in the same round.
KTEST(CreditAndSnapWindows)
{
	FCreditTracker Credit(Crews());
	Credit.NoteLineOnPlayer(3, 0, ELineMode::Set, 10.0);
	KCHECK(Credit.Attribute(0, EStallCause::RingOut, NoObject, 15.0).Killer == 3);
	KCHECK(Credit.Attribute(0, EStallCause::RingOut, NoObject, 15.1).Killer == NoPlayer);
	Credit.NoteLineOnPlayer(4, 0, ELineMode::Haul, 14.0);
	Credit.NoteLineOnPlayer(5, 0, ELineMode::Jack, 8.0);  // Too long ago to assist
	const FCredit C = Credit.Attribute(0, EStallCause::RingOut, NoObject, 14.5);
	KCHECK(C.Killer == 4 && C.Spotter == 3 && C.Assists.empty());

	const std::vector<FMatchPlayer> Roster = Crews();
	FEventLog Log;
	Log.Append(FSnapEvent{10.0, 0, 3, 1, 1200.0});
	Log.Append(FStallEvent{14.9, 1, 3, EStallCause::WorldImpact, NoObject, 0.0, 0.0, NoPlayer, NoPlayer, {}, ERig::None, 0.0});
	Log.Append(FSnapEvent{20.0, 1, 4, 1, 1200.0});
	Log.Append(FStallEvent{25.1, 1, 4, EStallCause::WorldImpact, NoObject, 0.0, 0.0, NoPlayer, NoPlayer, {}, ERig::None, 0.0});
	Log.Append(FSnapEvent{30.0, 1, 5, 1, 1200.0});
	Log.Append(FStallEvent{34.9, 1, 5, EStallCause::WorldImpact, NoObject, 0.0, 0.0, NoPlayer, NoPlayer, {}, ERig::None, 0.0});
	const std::vector<FPlayerFacts> Facts = ComputeFacts(Roster, Log, Tables());
	KCHECK(Facts[3].SnapsSurvived == 1);  // Fell in another round
	KCHECK(Facts[4].SnapsSurvived == 1);  // Fell after 5 s
	KCHECK(Facts[5].SnapsSurvived == 0);
}

// Facts at their edges: a round with no time is not a round without floor; time is clamped to
// time connected; an object as heavy as the victim, or of unknown mass, is not lighter.
KTEST(FactsAtTheirEdges)
{
	const std::vector<FMatchPlayer> Roster = Crews();
	FEventLog Log;
	Log.Append(FPlayerRoundEvent{0, 0, 0.0, 0.0, 0.0, 0.0});
	Log.Append(FPlayerRoundEvent{1, 0, 1000.0, 0.0, 0.0, 0.0});
	Log.Append(FStallEvent{1.0, 0, 0, EStallCause::ObjectImpact, 7, 130.0, 1300.0, 3, NoPlayer, {}, ERig::None, 0.0});
	Log.Append(FStallEvent{2.0, 0, 1, EStallCause::ObjectImpact, 7, 0.0, 1300.0, 3, NoPlayer, {}, ERig::None, 0.0});
	Log.Append(FStallEvent{3.0, 0, 2, EStallCause::ObjectImpact, 7, 219.0, 1300.0, 3, NoPlayer, {}, ERig::None, 0.0});
	const std::vector<FPlayerFacts> Facts = ComputeFacts(Roster, Log, Tables());
	KCHECK(Facts[0].RoundsWithoutFloor == 0);
	KCHECK(Facts[3].ScrapsWithLighterObject == 1);  // Only the 219 kg object against the 220 kg Breaker
	std::vector<FPlayerConnection> Connections(Roster.size(), FPlayerConnection{100.0, false});
	const std::string Report = BuildMatchReport({"m-3", "b", "scrap", "YRD", "base"}, Roster, Log, Connections,
	                                            FScrapResult{}, Tables());
	KCHECK(Contains(Report, R"("floor_seconds":100,)"));
	KCHECK(Contains(Report, R"("winner_team":null,"forfeit_team":null,"forfeit_penalty_free":false,"no_contest":false)"));

	FEventLog Seats;
	Seats.Append(FDogSeatEvent{1.0, 0, 3, 7, ESeatTarget::Player, 1, EPlate::Back, EHarnessClass::Rigger, EHarnessClass::Climber});
	Seats.Append(FDogSeatEvent{2.0, 0, 3, 8, ESeatTarget::Static, NoPlayer, EPlate::None, EHarnessClass::Rigger});
	KCHECK(Contains(Seats.JsonLines(), R"("plate":"back","shooter_class":"Rigger","target_class":"Climber"})"));
	KCHECK(Contains(Seats.JsonLines(), R"("plate":"none","shooter_class":"Rigger","target_class":null})"));
}
