#include "KineticGameMode.h"

#include "EngineUtils.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "KineticCharacter.h"
#include "KineticHUD.h"

DEFINE_LOG_CATEGORY_STATIC(LogKineticGameMode, Log, All);

AKineticGameMode::AKineticGameMode()
{
	DefaultPawnClass = AKineticCharacter::StaticClass();
	HUDClass = AKineticHUD::StaticClass();
}

void AKineticGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);
	const FString Class = UGameplayStatics::ParseOption(Options, TEXT("Class"));
	if (!Class.IsEmpty() && !KineticHarness::Parse(Class, DefaultHarness))
	{
		UE_LOG(LogKineticGameMode, Warning, TEXT("?Class=%s is not a harness class; playing the %s"), *Class,
		       *KineticHarness::Name(DefaultHarness));
	}
}

AActor* AKineticGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	// Spawn A (layout.md: West Mesa) for the player until teams arrive with the match modes.
	for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
	{
		if (It->PlayerStartTag == FName(TEXT("Spawn_A")))
		{
			return *It;
		}
	}
	return Super::ChoosePlayerStart_Implementation(Player);
}

APawn* AKineticGameMode::SpawnDefaultPawnAtTransform_Implementation(AController* NewPlayer, const FTransform& SpawnTransform)
{
	FActorSpawnParameters Params;
	Params.Instigator = GetInstigator();
	Params.ObjectFlags |= RF_Transient;
	Params.bDeferConstruction = true;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	AKineticCharacter* Pawn = GetWorld()->SpawnActor<AKineticCharacter>(DefaultPawnClass, SpawnTransform, Params);
	if (Pawn)
	{
		Pawn->Harness = DefaultHarness;
		Pawn->FinishSpawning(SpawnTransform);
	}
	return Pawn;
}
