#include "Kinetic/ScrapMatch.h"

#include <algorithm>

namespace Kinetic
{
namespace
{
constexpr double TimeEpsilon = 1e-9;  // Fixed steps of 1/60 s sum a hair under whole seconds
} // namespace

FScrapMatch::FScrapMatch(std::vector<FMatchPlayer> Roster, FScrapRules InRules, FEventLog& InLog)
    : Rules(InRules), Log(InLog)
{
	for (FMatchPlayer& Player : Roster)
	{
		FSeat Seat;
		Seat.bBotControlled = Player.bIsBot;
		Seat.Player = std::move(Player);
		Seats.push_back(std::move(Seat));
	}
}

FScrapMatch::FSeat* FScrapMatch::Find(int Slot)
{
	for (FSeat& Seat : Seats)
	{
		if (Seat.Player.Slot == Slot)
		{
			return &Seat;
		}
	}
	return nullptr;
}

const FScrapMatch::FSeat* FScrapMatch::Find(int Slot) const
{
	return const_cast<FScrapMatch*>(this)->Find(Slot);
}

void FScrapMatch::Emit(EFlow Kind, int Player, int Team, EMarshalCall Call)
{
	Pending.push_back({Kind, Now, Player, Team, Call});
}

void FScrapMatch::LogConnection(int Slot, EConnection Change) { Log.Append(FConnectionEvent{Now, Slot, Change}); }

std::vector<FFlowEvent> FScrapMatch::TakeEvents()
{
	std::vector<FFlowEvent> Out;
	Out.swap(Pending);
	return Out;
}

void FScrapMatch::Start()
{
	Now = 0.0;
	BeginReset();
}

void FScrapMatch::BeginReset()
{
	CurrentPhase = EScrapPhase::Reset;
	PhaseEndsAt = Now + Rules.ResetSeconds;
	RoundStalls = 0;
	for (FSeat& Seat : Seats)
	{
		if (Seat.bPendingRejoin)
		{
			Seat.bPendingRejoin = false;
			Seat.bBotControlled = false;
			Seat.bWithdrawn = false;
			Seat.bStandIdleHandled = false;
			Emit(EFlow::Rejoin, Seat.Player.Slot, Seat.Player.Team);
		}
		Seat.bStanding = !Seat.bWithdrawn;
		Seat.LastInputAt = Now;
		Seat.bIdleWarned = false;
		Seat.bForfeitVote = false;
	}
	Emit(EFlow::ResetArena);
	Emit(EFlow::Call, NoPlayer, NoPlayer, EMarshalCall::CrewsOnTheClaim);
}

void FScrapMatch::BeginDrop()
{
	CurrentPhase = EScrapPhase::Drop;
	Remaining = Rules.RoundSeconds;
	RoundStartedAt = Now;
	CheckRoundEnd();  // A crew withdrawn during the reset is already down
}

bool FScrapMatch::IsLiveRound() const
{
	return CurrentPhase == EScrapPhase::Drop || CurrentPhase == EScrapPhase::Live ||
	       CurrentPhase == EScrapPhase::SuddenDeath;
}

bool FScrapMatch::CanUseLines() const
{
	return CurrentPhase == EScrapPhase::Live || CurrentPhase == EScrapPhase::SuddenDeath;
}

bool FScrapMatch::CanMove(int Slot) const
{
	const FSeat* Seat = Find(Slot);
	if (!Seat || !Seat->bStanding)
	{
		return false;
	}
	const double MovementFrom = Rules.RoundSeconds - Rules.DropSeconds + Rules.MoveBeforeLiveSeconds;
	return CanUseLines() || (CurrentPhase == EScrapPhase::Drop && Remaining <= MovementFrom + TimeEpsilon);
}

bool FScrapMatch::IsStanding(int Slot) const
{
	const FSeat* Seat = Find(Slot);
	return Seat && Seat->bStanding;
}

bool FScrapMatch::IsBotControlled(int Slot) const
{
	const FSeat* Seat = Find(Slot);
	return Seat && Seat->bBotControlled;
}

int FScrapMatch::StandingCount(int Team) const
{
	return static_cast<int>(
	    std::count_if(Seats.begin(), Seats.end(), [Team](const FSeat& S) { return S.Player.Team == Team && S.bStanding; }));
}

int FScrapMatch::ConnectedHumans(int Team) const
{
	return static_cast<int>(std::count_if(Seats.begin(), Seats.end(), [Team](const FSeat& S) {
		return S.Player.Team == Team && !S.Player.bIsBot && S.bConnected;
	}));
}

bool FScrapMatch::CanForfeitWithoutPenalty(int Team) const
{
	return Rules.bRated && std::any_of(Seats.begin(), Seats.end(), [&](const FSeat& S) {
		       return S.Player.Team == Team && !S.Player.bIsBot && !S.bConnected &&
		              Now - S.DisconnectedAt >= Rules.RatedShortCrewForfeitSeconds - TimeEpsilon;
	       });
}

FPlayerConnection FScrapMatch::Connection(int Slot) const
{
	const FSeat* Seat = Find(Slot);
	return Seat ? FPlayerConnection{Seat->ConnectedSeconds, Seat->bAbandoned} : FPlayerConnection{};
}

void FScrapMatch::Update(double Dt)
{
	if (CurrentPhase == EScrapPhase::MatchOver)
	{
		return;
	}
	Now += Dt;
	for (FSeat& Seat : Seats)
	{
		Seat.ConnectedSeconds += Seat.bConnected ? Dt : 0.0;
	}
	switch (CurrentPhase)
	{
	case EScrapPhase::Reset:
		if (Now >= PhaseEndsAt - TimeEpsilon)
		{
			BeginDrop();
		}
		break;
	case EScrapPhase::Drop:
	case EScrapPhase::Live:
	{
		const double Before = Remaining;
		Remaining = Remaining - Dt <= TimeEpsilon ? 0.0 : Remaining - Dt;
		UpdateClock(Before);
		break;
	}
	case EScrapPhase::SuddenDeath:
		if (Now >= SuddenDeathEndsAt - TimeEpsilon)
		{
			EndRound(NoPlayer, ERoundEnd::Replayed);  // Nobody fell: the round is replayed
		}
		break;
	case EScrapPhase::RoundEnd:
		if (Now >= PhaseEndsAt - TimeEpsilon)
		{
			if (bDecided)
			{
				EndMatch(Outcome.WinnerTeam, NoPlayer, false);
				return;
			}
			++RoundIndex;
			BeginReset();
		}
		break;
	case EScrapPhase::MatchOver:
		return;
	}
	UpdateConnections();
}

void FScrapMatch::UpdateClock(double Before)
{
	const auto Crossed = [&](double Mark) { return Before > Mark + TimeEpsilon && Remaining <= Mark + TimeEpsilon; };
	if (Crossed(Rules.RoundSeconds - Rules.DropSeconds))
	{
		CurrentPhase = EScrapPhase::Live;
		Emit(EFlow::Call, NoPlayer, NoPlayer, EMarshalCall::Live);
	}
	if (Crossed(Rules.SirenRemaining))
	{
		Emit(EFlow::Siren);
		Emit(EFlow::Call, NoPlayer, NoPlayer, EMarshalCall::EventInFive);
	}
	if (Crossed(Rules.EventRemaining))
	{
		Emit(EFlow::MidMatchEvent);
	}
	if (Crossed(Rules.CautionRemaining))
	{
		Emit(EFlow::Caution);
	}
	if (Remaining > 0.0)
	{
		return;
	}
	Emit(EFlow::Call, NoPlayer, NoPlayer, EMarshalCall::TimeCount);
	const int A = StandingCount(0);
	const int B = StandingCount(1);
	if (A != B)
	{
		EndRound(A > B ? 0 : 1, ERoundEnd::TimeOut);
		return;
	}
	if (A == 0)
	{
		EndRound(NoPlayer, ERoundEnd::Replayed);
		return;
	}
	CurrentPhase = EScrapPhase::SuddenDeath;
	SuddenDeathEndsAt = Now + Rules.SuddenDeathSeconds;
	Emit(EFlow::Call, NoPlayer, NoPlayer, EMarshalCall::SuddenDeath);
	Emit(EFlow::ShrinkArena);
}

bool FScrapMatch::OnStall(int Slot)
{
	FSeat* Seat = Find(Slot);
	if (!IsLiveRound() || !Seat || !Seat->bStanding)
	{
		return false;
	}
	Seat->bStanding = false;
	++RoundStalls;
	CheckRoundEnd();
	return true;
}

void FScrapMatch::CheckRoundEnd()
{
	if (!IsLiveRound())
	{
		return;
	}
	const int A = StandingCount(0);
	const int B = StandingCount(1);
	if (CurrentPhase == EScrapPhase::SuddenDeath)
	{
		if (A != B)
		{
			EndRound(A > B ? 0 : 1, ERoundEnd::SuddenDeath);
		}
		return;
	}
	if (A == 0 && B == 0)
	{
		EndRound(NoPlayer, ERoundEnd::Replayed);
	}
	else if (A == 0 || B == 0)
	{
		EndRound(A == 0 ? 1 : 0, ERoundEnd::CrewDown);
	}
}

void FScrapMatch::EndRound(int WinningTeam, ERoundEnd Reason)
{
	CurrentPhase = EScrapPhase::RoundEnd;
	PhaseEndsAt = Now + Rules.ReplaySeconds;
	if (WinningTeam != NoPlayer && Reason != ERoundEnd::Forfeit)
	{
		++Score[WinningTeam];
		if (Score[WinningTeam] >= Rules.RoundsToWin)
		{
			bDecided = true;  // Decided now; the replay plays, and nothing in it changes this
			Outcome.WinnerTeam = WinningTeam;
		}
	}
	Log.Append(FRoundEndEvent{Now, RoundIndex, WinningTeam, Now - RoundStartedAt, RoundStalls, Reason});
	Emit(EFlow::RoundOver, NoPlayer, WinningTeam);
}

void FScrapMatch::EndMatch(int WinningTeam, int ForfeitingTeam, bool bPenaltyFree)
{
	if (ForfeitingTeam != NoPlayer && IsLiveRound())
	{
		EndRound(WinningTeam, ERoundEnd::Forfeit);  // Logged, not scored
	}
	CurrentPhase = EScrapPhase::MatchOver;
	Outcome.WinnerTeam = WinningTeam;
	Outcome.ForfeitTeam = ForfeitingTeam;
	Outcome.bForfeitPenaltyFree = bPenaltyFree;
	Outcome.bNoContest = WinningTeam == NoPlayer;
	if (ForfeitingTeam != NoPlayer)
	{
		Emit(EFlow::Call, NoPlayer, ForfeitingTeam, EMarshalCall::CrewForfeited);
	}
	Log.Append(FMatchEndEvent{Now, WinningTeam, ForfeitingTeam, bPenaltyFree, Outcome.bNoContest});
	Emit(EFlow::MatchOver, NoPlayer, WinningTeam);
}

void FScrapMatch::UpdateConnections()
{
	for (FSeat& Seat : Seats)
	{
		if (CurrentPhase == EScrapPhase::MatchOver)
		{
			return;
		}
		if (Seat.Player.bIsBot)
		{
			continue;
		}
		const int Slot = Seat.Player.Slot;
		const int Team = Seat.Player.Team;
		if (!Seat.bConnected)
		{
			const double Gone = Now - Seat.DisconnectedAt;
			const double Window = Rules.bRated ? Rules.ReconnectRatedSeconds : Rules.ReconnectCasualSeconds;
			if (!Seat.bAbandoned && Gone >= Window - TimeEpsilon)
			{
				Seat.bAbandoned = true;
				LogConnection(Slot, EConnection::Abandoned);
			}
			if (Seat.bStandIdleHandled || Gone < Rules.StandIdleSeconds - TimeEpsilon)
			{
				continue;
			}
			Seat.bStandIdleHandled = true;
			if (ConnectedHumans(Team) == 0)
			{
				continue;  // The crew is lost (below): no bot for a crew with no humans
			}
			if (!Rules.bRated)
			{
				Seat.bBotControlled = true;
				Emit(EFlow::Backfill, Slot, Team);
				Emit(EFlow::Call, Slot, Team, EMarshalCall::OfficeCrewIn);
				LogConnection(Slot, EConnection::Backfilled);
				continue;
			}
			Seat.bWithdrawn = true;
			Emit(EFlow::Withdraw, Slot, Team);
			LogConnection(Slot, EConnection::Withdrawn);
			if (Seat.bStanding)
			{
				Seat.bStanding = false;
				CheckRoundEnd();
			}
			continue;
		}
		if (Seat.bBotControlled || !CanMove(Slot))
		{
			Seat.LastInputAt = Now;  // Idle counts only while the rigger can move
			continue;
		}
		const double Idle = Now - Seat.LastInputAt;
		if (Idle >= Rules.IdleLeaverSeconds - TimeEpsilon)
		{
			OnDisconnect(Slot);  // Treated as a leaver
			Seat.bIdleLeaver = true;
		}
		else if (Idle >= Rules.IdleWarningSeconds - TimeEpsilon && !Seat.bIdleWarned)
		{
			Seat.bIdleWarned = true;
			Emit(EFlow::Call, Slot, Team, EMarshalCall::OfficeNotesStill);
			LogConnection(Slot, EConnection::IdleWarning);
		}
	}
	CheckCrewsLost();
}

// A crew of bots cannot exist (game-modes 9): a crew whose humans are all gone, the last of
// them for the 10 s a rigger stands idle, is lost. If the other crew has no human connected
// either at that moment (an outage drops clients a tick or two apart), it is a no-contest.
void FScrapMatch::CheckCrewsLost()
{
	if (bDecided || CurrentPhase == EScrapPhase::MatchOver)
	{
		return;  // A decided match stands whatever happens in its replay
	}
	bool bLost[2] = {false, false};
	for (int Team = 0; Team < 2; ++Team)
	{
		double LastGone = -1e300;
		bool bAnyHuman = false;
		for (const FSeat& Seat : Seats)
		{
			if (Seat.Player.Team == Team && !Seat.Player.bIsBot)
			{
				bAnyHuman = true;
				LastGone = std::max(LastGone, Seat.DisconnectedAt);
			}
		}
		bLost[Team] = bAnyHuman && ConnectedHumans(Team) == 0 && Now - LastGone >= Rules.StandIdleSeconds - TimeEpsilon;
	}
	if (!bLost[0] && !bLost[1])
	{
		return;
	}
	const int Lost = bLost[0] ? 0 : 1;
	if (bLost[1 - Lost] || ConnectedHumans(1 - Lost) == 0)
	{
		EndMatch(NoPlayer, NoPlayer, false);  // Both crews lost their last humans together
		return;
	}
	EndMatch(1 - Lost, Lost, false);
}

void FScrapMatch::OnDisconnect(int Slot)
{
	FSeat* Seat = Find(Slot);
	if (!Seat || Seat->Player.bIsBot || CurrentPhase == EScrapPhase::MatchOver)
	{
		return;
	}
	if (!Seat->bConnected)
	{
		Seat->bIdleLeaver = false;  // An idle leaver whose connection then drops: input cannot return it
		return;
	}
	Seat->bConnected = false;
	Seat->DisconnectedAt = Now;
	Seat->bForfeitVote = false;
	Seat->bPendingRejoin = false;  // Returned and gone again before the reset
	LogConnection(Slot, EConnection::Disconnected);
}

void FScrapMatch::OnReconnect(int Slot)
{
	FSeat* Seat = Find(Slot);
	if (!Seat || Seat->bConnected || Seat->bAbandoned || CurrentPhase == EScrapPhase::MatchOver)
	{
		return;  // Past the window the seat is gone for this match
	}
	Seat->bConnected = true;
	Seat->bIdleLeaver = false;
	Seat->LastInputAt = Now;
	Seat->bIdleWarned = false;
	LogConnection(Slot, EConnection::Returned);
	if (!Seat->bStandIdleHandled)
	{
		return;  // Back within the 10 s: the rigger never left them
	}
	if (Seat->bBotControlled)
	{
		Seat->bBotControlled = false;  // Casual: the rigger comes straight back from the bot
		Seat->bStandIdleHandled = false;
		Emit(EFlow::Rejoin, Slot, Seat->Player.Team);
		return;
	}
	Seat->bPendingRejoin = true;  // Rated: withdrawn until the next reset
}

void FScrapMatch::OnInput(int Slot)
{
	FSeat* Seat = Find(Slot);
	if (Seat && !Seat->bConnected && Seat->bIdleLeaver)
	{
		OnReconnect(Slot);  // An idle leaver moving again is a return
	}
	if (Seat && Seat->bConnected)
	{
		Seat->LastInputAt = Now;
		Seat->bIdleWarned = false;
	}
}

void FScrapMatch::OnForfeitVote(int Slot, bool bYes)
{
	FSeat* Seat = Find(Slot);
	if (!Seat || Seat->Player.bIsBot || !Seat->bConnected || CurrentPhase == EScrapPhase::MatchOver || bDecided)
	{
		return;
	}
	Seat->bForfeitVote = bYes;
	const int Team = Seat->Player.Team;
	const int Yes = static_cast<int>(std::count_if(Seats.begin(), Seats.end(), [Team](const FSeat& S) {
		return S.Player.Team == Team && S.bConnected && S.bForfeitVote;
	}));
	const int Needed = std::min(Rules.ForfeitVotes, ConnectedHumans(Team));
	if (Needed > 0 && Yes >= Needed)
	{
		EndMatch(1 - Team, Team, CanForfeitWithoutPenalty(Team));
	}
}
} // namespace Kinetic
