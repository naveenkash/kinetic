// The game mode for a KINETIC level, in one of three modes (?Mode=, or the kinetic.Mode console
// variable; Scrap by default):
// - Scrap: a match (FKineticScrap): the player in slot 0 of six, bots in the rest, rounds,
//   the clock, best of 5. ?Bots=all (or kinetic.AllBots 1) puts a bot in slot 0 too and the
//   player watches it.
// - TagOut: Claim with Tag-Out (FKineticTagOut, docs/proposals/claim-tag-out.md; PROPOSED):
//   4v4, no Spool lines, codes on the plates, the Call, a beacon each. ?Bots=all as for Scrap;
//   ?Gadget=Dust (or kinetic.TagGadget) for dust bags in place of grease rags.
// - Walk: the walk-the-arena mode: one harness character at Spawn A, back there 3 s after a
//   Stall. The tests that drive one character use it.
// Either way the player's class is the one the URL names (?Class=Breaker; the Rigger by default).
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "KineticHarness.h"
#include "KineticScrap.h"
#include "KineticTagOut.h"

#include "KineticGameMode.generated.h"

UENUM(BlueprintType)
enum class EKineticMode : uint8
{
	Scrap,
	Walk,
	TagOut
};

UCLASS()
class KINETICGAMEPLAY_API AKineticGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AKineticGameMode();

	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual void StartPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;
	virtual APawn* SpawnDefaultPawnAtTransform_Implementation(AController* NewPlayer, const FTransform& SpawnTransform) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Kinetic")
	EKineticHarness DefaultHarness = EKineticHarness::Rigger;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Kinetic")
	EKineticMode Mode = EKineticMode::Scrap;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Kinetic")
	bool bAllBots = false;

	// The match, in Scrap mode (made when the first player arrives, or at the start of play).
	FKineticScrap* Scrap() const { return Match.Get(); }
	// The match, in TagOut mode.
	FKineticTagOut* TagOut() const { return Claim.Get(); }
	// The running match's moments, for an arena's schedule (false in Walk mode).
	bool RoundTimes(FKineticRoundTimes& Out) const;

private:
	FKineticScrap& EnsureScrap();
	FKineticTagOut& EnsureTagOut();

	TUniquePtr<FKineticScrap> Match;
	TUniquePtr<FKineticTagOut> Claim;
	Kinetic::ETagGadget HumanGadget = Kinetic::ETagGadget::Grease;
};
