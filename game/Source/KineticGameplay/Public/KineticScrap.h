// One Scrap match in a level (bible/05 section 9; game-modes-and-match-flow 1, 2, 8): six
// riggers, KineticCore's FScrapMatch for the rounds, the clock and its calls, best of 5 and side
// swaps, and bots for every slot a human does not play. AKineticGameMode owns it and ticks it
// once a frame, before the Spool's step.
//
// - The crews (PROPOSED): slot 0 is the player, on crew 0 with a Breaker and a Climber; crew 1
//   is a Breaker, a Rigger and a Scrapper. With no human (tests, an attract mode) a bot plays
//   slot 0 and the player's view follows it.
// - Each crew starts a round at its side's spawn (Spawn_A or Spawn_B, swapping each round), the
//   three riggers 2.5 m apart across the start's facing.
// - A round's reset puts props back where the level placed them, clears every line, and puts
//   every rigger on their feet; a Stalled rigger stays out until then.
// - Any key or mouse movement is input for the idle rules; a human idle long enough is backfilled
//   by a bot, which plays their rigger until they press something.
#pragma once

#include "CoreMinimal.h"
#include "Kinetic/MatchEvents.h"
#include "Kinetic/ScrapMatch.h"
#include "KineticBotController.h"
#include "KineticHarness.h"

class AKineticCharacter;
class APlayerController;
class UKineticSpoolWorld;

// When this round's moments fall, in match time (FScrapMatch::Time, s): what an arena's
// schedule runs from. -1 for what has not happened yet this round.
struct FKineticRoundTimes
{
	double Now = 0.0;
	double ResetAt = 0.0;
	double LiveAt = 0.0;          // Planned: the reset and the Drop after ResetAt
	double SirenAt = -1.0;        // The mid-match event's siren
	double MidMatchAt = -1.0;     // Its payload
	double PlannedMidMatchAt = 0.0;
	double SuddenDeathAt = -1.0;  // The arena's sudden-death event begins
	int32 Resets = 0;
};

class KINETICGAMEPLAY_API FKineticScrap
{
public:
	struct FSettings
	{
		EKineticHarness HumanClass = EKineticHarness::Rigger;
		bool bHumanPlays = true;
		Kinetic::FScrapRules Rules;
		uint64 Seed = 1;
	};

	FKineticScrap(UWorld& InWorld, const FSettings& InSettings);

	// The player: they play slot 0, or watch it when bots play every slot.
	void AddHuman(APlayerController* Player);
	// One 60 Hz step of the match, before the Spool's.
	void Tick();
	// A new match from the reset (a test's shorter rules, or after a match is over).
	void StartMatch();
	void SetRules(const Kinetic::FScrapRules& Rules) { Settings.Rules = Rules; }

	const Kinetic::FScrapMatch& Match() const { return *TheMatch; }
	const TArray<FString>& Feed() const { return FeedLines; }
	int32 HumanSlot() const { return Settings.bHumanPlays ? 0 : INDEX_NONE; }
	int32 SlotCount() const { return Riggers.Num(); }
	AKineticCharacter* Rigger(int32 Slot) const;
	int32 SlotOf(const AKineticCharacter* Character) const;
	int32 TeamOf(int32 Slot) const { return Slot < 3 ? 0 : 1; }
	const AKineticBotController* Bot(int32 Slot) const;
	int32 MatchesFinished() const { return Finished; }
	int32 StallsSeen() const { return Stalls; }
	int32 ResetsSeen() const { return Resets; }
	FKineticRoundTimes RoundTimes() const;

	static constexpr double RestartSeconds = 8.0;  // After a match, a new one
	static constexpr int32 Slots = 6;

private:
	void SpawnRiggers();
	void ResetArena();
	void Say(FString Text);
	FString Who(int32 Slot) const;
	FTransform SpawnFor(int32 Slot) const;
	UKineticSpoolWorld* Spool() const;

	UWorld& World;
	FSettings Settings;
	TArray<Kinetic::FMatchPlayer> Roster;
	TArray<TWeakObjectPtr<AKineticCharacter>> Riggers;
	TArray<TWeakObjectPtr<AKineticBotController>> Brains;  // One per slot: the human's takes over when backfilled
	TUniquePtr<Kinetic::FEventLog> Log;
	TUniquePtr<Kinetic::FScrapMatch> TheMatch;
	TArray<FString> FeedLines;  // The last few calls and Stalls
	double MatchOverAt = -1.0;
	double ResetAt = 0.0;
	double SirenAt = -1.0;
	double MidMatchAt = -1.0;
	double SuddenDeathAt = -1.0;
	uint32 LastHumanInputs = 0;
	int32 Finished = 0;
	int32 Stalls = 0;
	int32 Resets = 0;
};
