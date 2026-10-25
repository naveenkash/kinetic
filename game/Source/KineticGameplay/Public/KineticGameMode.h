// The game mode for a KINETIC level: spawns a harness character of the class the URL names
// (?Class=Breaker; the Rigger by default) at the level's Spawn A player start. Match flow
// (bible/05 section 9) arrives with the Scrap mode; this is the walk-the-arena mode.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "KineticHarness.h"

#include "KineticGameMode.generated.h"

UCLASS()
class KINETICGAMEPLAY_API AKineticGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AKineticGameMode();

	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;
	virtual APawn* SpawnDefaultPawnAtTransform_Implementation(AController* NewPlayer, const FTransform& SpawnTransform) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Kinetic")
	EKineticHarness DefaultHarness = EKineticHarness::Rigger;
};
