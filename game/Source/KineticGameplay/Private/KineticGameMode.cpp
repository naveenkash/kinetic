#include "KineticGameMode.h"

#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerStart.h"
#include "HAL/IConsoleManager.h"
#include "Kismet/GameplayStatics.h"
#include "KineticCharacter.h"
#include "KineticHUD.h"
#include "KineticScrap.h"

DEFINE_LOG_CATEGORY_STATIC(LogKineticGameMode, Log, All);

namespace
{
TAutoConsoleVariable<FString> CVarMode(TEXT("kinetic.Mode"), TEXT("Scrap"),
                                       TEXT("The game mode a level plays when its URL names none: Scrap (a match) or Walk"));
TAutoConsoleVariable<int32> CVarAllBots(TEXT("kinetic.AllBots"), 0,
                                        TEXT("1: in a Scrap match, a bot plays slot 0 too and the player watches it"));
TAutoConsoleVariable<int32> CVarSeed(TEXT("kinetic.Seed"), 0, TEXT("The bots' match seed; 0 picks one"));
}  // namespace

AKineticGameMode::AKineticGameMode()
{
	DefaultPawnClass = AKineticCharacter::StaticClass();
	HUDClass = AKineticHUD::StaticClass();
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PrePhysics;
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
	FString ModeName = UGameplayStatics::ParseOption(Options, TEXT("Mode"));
	if (ModeName.IsEmpty())
	{
		ModeName = CVarMode.GetValueOnGameThread();
	}
	Mode = ModeName.Equals(TEXT("Walk"), ESearchCase::IgnoreCase) ? EKineticMode::Walk : EKineticMode::Scrap;
	bAllBots = UGameplayStatics::ParseOption(Options, TEXT("Bots")).Equals(TEXT("all"), ESearchCase::IgnoreCase) ||
	           CVarAllBots.GetValueOnGameThread() != 0;
	UE_LOG(LogKineticGameMode, Log, TEXT("%s mode%s"), Mode == EKineticMode::Scrap ? TEXT("Scrap") : TEXT("Walk"),
	       Mode == EKineticMode::Scrap && bAllBots ? TEXT(", bots in every slot") : TEXT(""));
}

FKineticScrap& AKineticGameMode::EnsureScrap()
{
	if (!Match)
	{
		FKineticScrap::FSettings Settings;
		Settings.HumanClass = DefaultHarness;
		Settings.bHumanPlays = !bAllBots;
		const int32 Seed = CVarSeed.GetValueOnGameThread();
		Settings.Seed = Seed != 0 ? static_cast<uint64>(Seed) : FPlatformTime::Cycles64();
		Match = MakeUnique<FKineticScrap>(*GetWorld(), Settings);
	}
	return *Match;
}

void AKineticGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	if (Mode != EKineticMode::Scrap)
	{
		Super::HandleStartingNewPlayer_Implementation(NewPlayer);
		return;
	}
	EnsureScrap().AddHuman(NewPlayer);  // The match's own rigger, not a default pawn
}

void AKineticGameMode::StartPlay()
{
	if (Mode == EKineticMode::Scrap)
	{
		EnsureScrap();
	}
	Super::StartPlay();
}

void AKineticGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (Match)
	{
		Match->Tick();
	}
}

AActor* AKineticGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	// Walk mode: Spawn A (layout.md: West Mesa).
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
