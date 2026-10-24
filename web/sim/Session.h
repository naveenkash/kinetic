// One browser match: the world, the bots, and KineticCore's Scrap match flow (rounds, the
// clock, best of 5, side swaps). Slot 0 is the player, on team 0 with two bot crewmates,
// against a bot crew of three. The renderer reads two JSON documents: the arena once, and a
// snapshot after every tick.
#pragma once

#include "Brains.h"
#include "World.h"

#include "Kinetic/MatchEvents.h"
#include "Kinetic/ScrapMatch.h"

#include <deque>
#include <memory>
#include <string>
#include <vector>

namespace Kinetic::Web
{
class FSession
{
public:
	// bHumanPlays false puts a bot in slot 0 too (the tests, and an attract mode).
	FSession(const FKineticTables& Tables, uint64_t Seed, EHarnessClass PlayerClass, bool bHumanPlays = true);

	void Step(const FPlayerInput& Human);  // One 60 Hz tick
	std::string ArenaJson() const;
	std::string SnapshotJson() const;

	const FWorld& World() const { return TheWorld; }
	const FScrapMatch& Match() const { return *TheMatch; }
	int MatchesFinished() const { return Finished; }
	int StallsSeen() const { return Stalls; }

	static constexpr double RestartSeconds = 8.0;  // After a match, a new one

private:
	void StartMatch();
	void Say(std::string Text);

	const FKineticTables& Tables;
	uint64_t Seed;
	bool bHumanPlays;
	std::vector<FMatchPlayer> Roster;
	FWorld TheWorld;
	std::unique_ptr<FEventLog> Log;
	std::unique_ptr<FScrapMatch> TheMatch;
	std::vector<std::unique_ptr<FBotBrain>> Brains;  // Null for the human
	std::vector<FWorldEvent> Pending;                // World events for the brains next tick
	std::deque<std::string> Feed;                    // The last few calls and Stalls
	double MatchOverAt = -1.0;
	double LastYaw = 0.0, LastPitch = 0.0;
	int Finished = 0;
	int Stalls = 0;
};
} // namespace Kinetic::Web
