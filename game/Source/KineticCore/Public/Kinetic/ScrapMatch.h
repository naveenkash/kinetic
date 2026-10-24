// The Scrap match flow on the server (bible/05 section 9; game-modes-and-match-flow sections
// 1, 2, 8 and 9): rounds, the clock and its calls, round results, best of 5, side swaps, and
// the forfeit, leaver and idle rules. The game feeds it Stalls, connections and inputs and
// reads what is allowed and what happened; it writes round ends and connection changes to the
// event log.
//
// Choices the docs leave open (PROPOSED):
// - A round's seconds run from the start of the Drop.
// - Two crews down in the same instant, or sudden death running out with nobody fallen, replays
//   the round. It does not count, and sides still swap.
// - In sudden death the round ends the moment one crew has fewer riggers standing.
// - A withdrawn rigger is not standing, and a crew with nobody standing loses the round, even
//   at the Drop. In casual, the bot that backfills takes over the rigger where it stands; a
//   leaver who returns within the window takes it straight back. In Rated, a withdrawn leaver
//   who returns rejoins at the next reset.
// - Idle time counts only while movement is enabled; 60 s idle is a disconnect, and input
//   within the reconnect window is the return.
// - A forfeit needs 2 yes votes, or every connected human if fewer than 2 remain; votes clear
//   at each reset. Once a crew has its third round the match is decided: nothing in the replay
//   can change it. Both crews losing their last humans together (within the 10 s a rigger
//   stands idle) is a no-contest, and so are three replayed rounds in a row (a match where
//   nobody falls would otherwise never end).
#pragma once

#include "Kinetic/MatchEvents.h"

#include <vector>

namespace Kinetic
{
struct FScrapRules
{
	double ResetSeconds = 5.0;
	double RoundSeconds = 150.0;          // 2:30, starting with the Drop
	double DropSeconds = 6.0;             // To 2:24, Live
	double MoveBeforeLiveSeconds = 2.0;   // Movement from 2:26 (PROPOSED in game-modes 2)
	double SirenRemaining = 80.0;         // 1:20
	double EventRemaining = 75.0;         // 1:15: the mid-match event
	double CautionRemaining = 10.0;
	double ReplaySeconds = 4.0;
	int RoundsToWin = 3;                  // Best of 5
	double SuddenDeathSeconds = 90.0;     // Arena-defined; the Yard's Lockout closes at T+90
	double StandIdleSeconds = 10.0;       // A leaver's rigger stands idle, then is withdrawn
	double ReconnectCasualSeconds = 60.0;
	double ReconnectRatedSeconds = 90.0;
	double RatedShortCrewForfeitSeconds = 60.0;
	double IdleWarningSeconds = 20.0;
	double IdleLeaverSeconds = 60.0;
	int ForfeitVotes = 2;
	int MaxConsecutiveReplays = 3;        // Then the match is a no-contest (PROPOSED)
	bool bRated = false;
};

enum class EScrapPhase : uint8_t { Reset, Drop, Live, SuddenDeath, RoundEnd, MatchOver };

enum class EMarshalCall : uint8_t
{
	CrewsOnTheClaim, Live, EventInFive, TimeCount, SuddenDeath, CrewForfeited, OfficeNotesStill, OfficeCrewIn
};

enum class EFlow : uint8_t
{
	Call,           // Marshal: Call
	ResetArena,     // Objects to authored positions, rated consequences reset, set lines cleared
	Siren,          // 5 s before the mid-match event
	MidMatchEvent,
	Caution,        // The clock turns Caution and ticks each second
	ShrinkArena,    // The arena's sudden-death event
	RoundOver,      // Team is the winner, or NoPlayer for a replay
	MatchOver,      // Team is the winner
	Backfill,       // Player's rigger is now a bot's
	Withdraw,       // Player's rigger is withdrawn
	Rejoin,         // Player takes their rigger back
};

struct FFlowEvent
{
	EFlow Kind = EFlow::Call;
	double Time = 0.0;
	int Player = NoPlayer;
	int Team = NoPlayer;
	EMarshalCall Call = EMarshalCall::Live;
};

struct FScrapResult
{
	int WinnerTeam = NoPlayer;        // NoPlayer: no-contest
	int ForfeitTeam = NoPlayer;       // The crew that forfeited, by vote or by losing every human
	bool bForfeitPenaltyFree = false; // Rated, a crewmate gone for 60 s (game-modes 8)
	bool bNoContest = false;
};

struct FPlayerConnection
{
	double ConnectedSeconds = 0.0;
	bool bAbandoned = false;  // Did not return within the reconnect window
};

class FScrapMatch
{
public:
	FScrapMatch(std::vector<FMatchPlayer> Roster, FScrapRules Rules, FEventLog& Log);

	void Start();
	void Update(double Dt);

	// Inputs from the game. OnStall returns false if the player was not standing in a live round.
	bool OnStall(int Slot);
	void OnDisconnect(int Slot);
	void OnReconnect(int Slot);
	void OnInput(int Slot);
	void OnForfeitVote(int Slot, bool bYes);

	// What is allowed now.
	EScrapPhase Phase() const { return CurrentPhase; }
	bool CanUseLines() const;   // Lines and the Cutter: from Live
	bool CanMove(int Slot) const;
	bool IsStanding(int Slot) const;
	bool IsBotControlled(int Slot) const;
	int SpawnOf(int Team) const { return (Team + RoundIndex) % 2; }  // Sides swap each round
	double ClockRemaining() const { return Remaining; }
	double Time() const { return Now; }  // Seconds since Start
	int Round() const { return RoundIndex; }
	int Wins(int Team) const { return Score[Team]; }
	int StandingCount(int Team) const;
	bool CanForfeitWithoutPenalty(int Team) const;  // Rated, a crewmate gone for 60 s

	// Results.
	int WinnerTeam() const { return Outcome.WinnerTeam; }
	bool WasForfeit() const { return Outcome.ForfeitTeam != NoPlayer; }
	const FScrapResult& Result() const { return Outcome; }
	FPlayerConnection Connection(int Slot) const;
	std::vector<FFlowEvent> TakeEvents();

private:
	struct FSeat
	{
		FMatchPlayer Player;
		bool bStanding = false;
		bool bConnected = true;
		bool bBotControlled = false;   // A bot in the roster, or a casual backfill
		bool bWithdrawn = false;
		bool bAbandoned = false;
		bool bPendingRejoin = false;
		bool bIdleWarned = false;
		bool bIdleLeaver = false;      // Disconnected for idling: input is the return
		bool bStandIdleHandled = false;
		bool bForfeitVote = false;
		double DisconnectedAt = 0.0;
		double LastInputAt = 0.0;
		double ConnectedSeconds = 0.0;
	};

	FSeat* Find(int Slot);
	const FSeat* Find(int Slot) const;
	void Emit(EFlow Kind, int Player = NoPlayer, int Team = NoPlayer, EMarshalCall Call = EMarshalCall::Live);
	void LogConnection(int Slot, EConnection Change);
	void BeginReset();
	void BeginDrop();
	void UpdateClock(double Before);
	void UpdateConnections();
	void CheckCrewsLost();
	void CheckRoundEnd();
	void EndRound(int WinningTeam, ERoundEnd Reason);
	void EndMatch(int WinningTeam, int ForfeitingTeam, bool bPenaltyFree);
	int ConnectedHumans(int Team) const;
	bool IsLiveRound() const;

	std::vector<FSeat> Seats;
	FScrapRules Rules;
	FEventLog& Log;
	std::vector<FFlowEvent> Pending;
	EScrapPhase CurrentPhase = EScrapPhase::Reset;
	double Now = 0.0;
	double PhaseEndsAt = 0.0;
	double Remaining = 0.0;
	double RoundStartedAt = 0.0;
	double SuddenDeathEndsAt = 0.0;
	int RoundIndex = 0;
	int RoundStalls = 0;
	int Score[2] = {0, 0};
	bool bDecided = false;  // A crew has its third round; the replay is still running
	int ConsecutiveReplays = 0;
	FScrapResult Outcome;
};
} // namespace Kinetic
