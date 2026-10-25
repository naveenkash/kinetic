// A bot rigger: what a behavior tree will do, in code for now. It presses
// the same buttons a player does (move, look, the Spool's), decided by KineticCore's Bots.h:
// FBotDefense for lines on its own plates, ChooseRig for what to rig next.
//
// The tree's own choices (PROPOSED for the prototype):
// - A bot walks toward the nearest standing enemy until it is within 70% of its line length,
//   along the navmesh: the grate bridges over the trenches, and ladders (AKineticLadder is a
//   navigation link) up and down the Stacks and out of a trench. One climber on a ladder at a
//   time; a bot going down walks backward off the top and takes the rungs.
// - It aims with a lead for the target's velocity and the Dog's drop, plus up to 0.3 m of
//   error, and takes 0.35 s to aim each Dog.
// - A Haul or Jack is held up to 3 s; a Clothesline is Hung and released at once.
// - After a rig it waits 1.0 to 2.0 s before the next.
// - Anchors are the scenery the level tags Kinetic.Anchor (Stacks, walls, structures, the
//   placeholders): the point of each nearest the target enemy, and nearest the bot (a wall
//   behind it, for a Slingshot). The grey-box has no doorways or Haulers yet, so no
//   Clotheslines or Cabooses.
#pragma once

#include "AIController.h"
#include "CoreMinimal.h"
#include "Kinetic/Bots.h"
#include "KineticSpoolWorld.h"

#include "KineticBotController.generated.h"

class AKineticCharacter;
class AKineticLadder;

// A rigger in the match, as a bot sees them.
struct FKineticRiggerView
{
	TWeakObjectPtr<AKineticCharacter> Character;
	int32 Slot = 0;
	int32 Team = 0;
	bool bStanding = true;
};

UCLASS()
class KINETICGAMEPLAY_API AKineticBotController : public AAIController
{
	GENERATED_BODY()

public:
	AKineticBotController();

	// The match sets the bot up once: its slot, the match seed, and the class it plays.
	void Init(int32 InSlot, uint64 MatchSeed, Kinetic::EHarnessClass Class);
	// One tick: the riggers, and the Spool's events since the last tick. Drives Pawn (the
	// bot's own, or a human's rigger it has taken over).
	void Think(AKineticCharacter& Me, const TArray<FKineticRiggerView>& Riggers, const TArray<FKineticSpoolEvent>& Events);
	// A new round: plans and rigs dropped.
	void NewRound();

	static constexpr double AimSeconds = 0.35;
	static constexpr double HoldSeconds = 3.0;
	static constexpr double SeatTimeoutSeconds = 1.5;
	static constexpr double AimErrorCm = 30.0;
	static constexpr double ApproachShare = 0.7;

	// For tests: how many Dogs it has fired, and what it is doing.
	int32 DogsFired() const { return Fired; }

protected:
	// The bot sets its own aim (pitch included); the AI controller's focus would flatten it.
	virtual void UpdateControlRotation(float DeltaTime, bool bUpdatePawn = true) override;

private:
	enum class EStage : uint8 { Idle, AimA, WaitA, AimB, WaitB, Holding, Cooldown };
	enum class ELadderStep : uint8 { None, Up, OffTheTop, Down };

	struct FRoute
	{
		TArray<FVector> Points;
		int32 Next = 0;
		FVector Goal = FVector::ZeroVector;
		double PlannedAt = -100.0;
		double ProgressAt = 0.0;
		double BestDistance = 0.0;
		TWeakObjectPtr<const AKineticLadder> Ladder;
		ELadderStep Step = ELadderStep::None;
		double LadderSince = 0.0;
	};

	Kinetic::FBotView ViewOf(const AKineticCharacter& Me, const TArray<FKineticRiggerView>& Riggers) const;
	FVector AimPoint(const AKineticCharacter& Me, const FVector& Target, const FVector& TargetVelocity) const;
	void FaceToward(const AKineticCharacter& Me, const FVector& Point);
	void Walk(AKineticCharacter& Me, const TArray<FKineticRiggerView>& Riggers);
	void FollowRoute(AKineticCharacter& Me, const FVector& Goal);
	bool Climb(AKineticCharacter& Me);
	bool CloseBehindSomeone(const AKineticCharacter& Me, const FVector& Heading) const;
	bool HasFloorAt(const FVector& Point, double FeetZ) const;
	bool OverAnEdge(const FVector& From, const FVector& To, double FeetZ) const;
	void CoolDown(double Now);
	UKineticSpoolWorld* Spool() const;

	const Kinetic::FKineticTables* Tables = nullptr;
	int32 Slot = 0;
	Kinetic::EHarnessClass Class = Kinetic::EHarnessClass::Rigger;
	TOptional<Kinetic::FBotRandom> Random;
	TOptional<Kinetic::FBotDefense> Defense;
	EStage Stage = EStage::Idle;
	double StageUntil = 0.0;
	Kinetic::FBotIntent Intent;
	int32 LineId = -1;
	TArray<int32> Defending;  // Line ids on its plates the defense is planning for
	bool bDrawn = false;
	int32 Fired = 0;
	double LastThought = -100.0;
	FRoute Route;
	TArray<TWeakObjectPtr<AActor>> Anchors;
};
