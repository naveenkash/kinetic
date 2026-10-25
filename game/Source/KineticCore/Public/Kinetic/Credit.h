// Who scrapped whom: elimination, Spotter and assist credit for a Stall, and the Scrap Card
// (bible/04 Tonnage; 05 sections 7.4 and 7.5; backend-architecture 12.1, where Spotter credit
// is computed on the server from line ownership at the time of each Stall).
//
// The game notes every step an enemy line acts on a player or an object. At a Stall
// (PROPOSED, the bible names the roles but not the rule):
// - Only enemy lines count, and only those that acted within the last 5 s.
// - The lines that count are those on the victim and, for a hit by an object (impact, blast,
//   crush), those on that object.
// - The killer is the owner of the most recent active line (Haul or Jack) among them, or, if
//   there was none, of the most recent Hang or Set line: a Guillotine dropped by its own rigger
//   is theirs.
// - The Spotter is the owner of the most recent Hang or Set line other than the killer's: the
//   set line the victim tripped on, the load they hung (04: the Spotter earns the larger share).
// - Every other enemy whose line counted assists.
// - No enemy line: no killer (the world did it, or a teammate's line).
#pragma once

#include "Kinetic/Api.h"

#include "Kinetic/MatchEvents.h"

#include <map>
#include <vector>

namespace Kinetic
{
struct FCredit
{
	int Killer = NoPlayer;
	int Spotter = NoPlayer;
	std::vector<int> Assists;  // By slot, ascending
};

class KINETICCORE_API FCreditTracker
{
public:
	static constexpr double WindowSeconds = 5.0;

	explicit FCreditTracker(std::vector<FMatchPlayer> InRoster) : Roster(std::move(InRoster)) {}

	// Call each step a line acts on a player or an object (any mode but Slack).
	void NoteLineOnPlayer(int Owner, int Victim, ELineMode Mode, double Now);
	void NoteLineOnObject(int Owner, int ObjectId, ELineMode Mode, double Now);
	void ClearRound();  // Objects reset between rounds; so does what was done to them

	FCredit Attribute(int Victim, EStallCause Cause, int ObjectId, double Now) const;

private:
	struct FLatest
	{
		double Active = -1e9;   // Last Haul or Jack
		double Passive = -1e9;  // Last Hang or Set
	};
	using FByOwner = std::map<int, FLatest>;

	static void Note(FByOwner& Owners, int Owner, ELineMode Mode, double Now);
	int TeamOf(int Slot) const;

	std::vector<FMatchPlayer> Roster;
	std::map<int, FByOwner> OnPlayer;  // By victim slot
	std::map<int, FByOwner> OnObject;  // By object id
};

// The card shown to the killer and the victim at step 2 of the Stall sequence (7.4), and what
// happens to the body after.
struct FScrapCard
{
	int Victim = NoPlayer;
	int Killer = NoPlayer;
	int Spotter = NoPlayer;
	EStallCause Cause = EStallCause::WorldImpact;
	ERig Rig = ERig::None;
	bool bDownTheWell = false;    // Ring-outs read "down the Well" (7.5) and leave no body
	double BodyMassKg = 0.0;      // The body persists as a Medium prop at the class mass, 4 s
	bool bBodyTakesScrapperBonus = false;  // Under the bonus mass cap: a scrapped Climber (7.4)
	bool bHookRetrieves = true;   // Not for bodies that went into the Anchor's Well
	double HookArrivesSeconds = 4.0;
	double HookLiftSeconds = 2.0;
};

KINETICCORE_API FScrapCard MakeScrapCard(const FStallEvent& Stall, const FMatchPlayer& Victim, bool bIntoAnchorWell,
                         const FKineticTables& Tables);
} // namespace Kinetic
