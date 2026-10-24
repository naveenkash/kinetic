#include "Kinetic/Credit.h"

#include "Kinetic/MassRules.h"

#include <algorithm>

namespace Kinetic
{
namespace
{
bool IsActive(ELineMode Mode) { return Mode == ELineMode::Haul || Mode == ELineMode::Jack; }

bool HasObject(EStallCause Cause)
{
	return Cause == EStallCause::ObjectImpact || Cause == EStallCause::Blast || Cause == EStallCause::Crush;
}

struct FCandidate
{
	int Owner = NoPlayer;
	double Active = -1e9;
	double Passive = -1e9;
};
} // namespace

void FCreditTracker::Note(FByOwner& Owners, int Owner, ELineMode Mode, double Now)
{
	if (Mode == ELineMode::Slack)
	{
		return;
	}
	FLatest& Latest = Owners[Owner];
	(IsActive(Mode) ? Latest.Active : Latest.Passive) = Now;
}

void FCreditTracker::NoteLineOnPlayer(int Owner, int Victim, ELineMode Mode, double Now)
{
	Note(OnPlayer[Victim], Owner, Mode, Now);
}

void FCreditTracker::NoteLineOnObject(int Owner, int ObjectId, ELineMode Mode, double Now)
{
	Note(OnObject[ObjectId], Owner, Mode, Now);
}

void FCreditTracker::ClearRound()
{
	OnPlayer.clear();
	OnObject.clear();
}

int FCreditTracker::TeamOf(int Slot) const
{
	for (const FMatchPlayer& Player : Roster)
	{
		if (Player.Slot == Slot)
		{
			return Player.Team;
		}
	}
	return NoPlayer;
}

FCredit FCreditTracker::Attribute(int Victim, EStallCause Cause, int ObjectId, double Now) const
{
	const int VictimTeam = TeamOf(Victim);
	std::map<int, FCandidate> Candidates;
	const auto Gather = [&](const std::map<int, FByOwner>& Source, int Key) {
		const auto Found = Source.find(Key);
		if (Found == Source.end())
		{
			return;
		}
		for (const auto& [Owner, Latest] : Found->second)
		{
			if (Owner == Victim || TeamOf(Owner) == VictimTeam || TeamOf(Owner) == NoPlayer)
			{
				continue;  // Only enemy lines earn credit
			}
			FCandidate& C = Candidates[Owner];
			C.Owner = Owner;
			if (Now - Latest.Active <= WindowSeconds)
			{
				C.Active = std::max(C.Active, Latest.Active);
			}
			if (Now - Latest.Passive <= WindowSeconds)
			{
				C.Passive = std::max(C.Passive, Latest.Passive);
			}
		}
	};
	Gather(OnPlayer, Victim);
	if (HasObject(Cause) && ObjectId != NoObject)
	{
		Gather(OnObject, ObjectId);
	}

	// Most recent first; ties go to the lower slot, so the result never depends on map order.
	const auto Latest = [&](auto Time, int Exclude) {
		int Best = NoPlayer;
		double BestTime = -1e8;
		for (const auto& [Owner, C] : Candidates)
		{
			if (Owner != Exclude && Time(C) > BestTime)
			{
				Best = Owner;
				BestTime = Time(C);
			}
		}
		return Best;
	};
	const auto ActiveTime = [](const FCandidate& C) { return C.Active; };
	const auto PassiveTime = [](const FCandidate& C) { return C.Passive; };

	FCredit Out;
	Out.Killer = Latest(ActiveTime, NoPlayer);
	if (Out.Killer == NoPlayer)
	{
		Out.Killer = Latest(PassiveTime, NoPlayer);
	}
	if (Out.Killer == NoPlayer)
	{
		return Out;
	}
	Out.Spotter = Latest(PassiveTime, Out.Killer);
	for (const auto& [Owner, C] : Candidates)
	{
		const bool bCounted = C.Active > -1e8 || C.Passive > -1e8;
		if (bCounted && Owner != Out.Killer && Owner != Out.Spotter)
		{
			Out.Assists.push_back(Owner);
		}
	}
	return Out;
}

FScrapCard MakeScrapCard(const FStallEvent& Stall, const FMatchPlayer& Victim, bool bIntoAnchorWell,
                         const FKineticTables& Tables)
{
	FScrapCard Card;
	Card.Victim = Stall.Victim;
	Card.Killer = Stall.Killer;
	Card.Spotter = Stall.Spotter;
	Card.Cause = Stall.Cause;
	Card.Rig = Stall.Rig;
	Card.bDownTheWell = Stall.Cause == EStallCause::RingOut;
	if (Card.bDownTheWell)
	{
		Card.bHookRetrieves = false;  // No body: it went down the Well (scrapped-body.md)
		return Card;
	}
	Card.BodyMassKg = Tables.Harness(Victim.Harness).MassKg;
	Card.bBodyTakesScrapperBonus = IsSmallPropBonusTarget(Card.BodyMassKg, Tables);
	Card.bHookRetrieves = !bIntoAnchorWell;
	return Card;
}
} // namespace Kinetic
