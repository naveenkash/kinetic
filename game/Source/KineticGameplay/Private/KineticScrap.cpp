#include "KineticScrap.h"

#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerStart.h"
#include "KineticCharacter.h"
#include "KineticProp.h"
#include "KineticSpoolWorld.h"

DEFINE_LOG_CATEGORY_STATIC(LogKineticScrap, Log, All);

namespace
{
const TCHAR* CallText(Kinetic::EMarshalCall Call)
{
	using Kinetic::EMarshalCall;
	switch (Call)
	{
	case EMarshalCall::CrewsOnTheClaim: return TEXT("Crews on the claim.");
	case EMarshalCall::Live: return TEXT("Live.");
	case EMarshalCall::EventInFive: return TEXT("Event in five.");
	case EMarshalCall::TimeCount: return TEXT("Ten seconds.");
	case EMarshalCall::SuddenDeath: return TEXT("Sudden death.");
	case EMarshalCall::CrewForfeited: return TEXT("Crew's forfeited. Logged.");
	case EMarshalCall::OfficeNotesStill: return TEXT("Office notes the rigger standing still.");
	case EMarshalCall::OfficeCrewIn: return TEXT("Office crew in.");
	}
	return TEXT("");
}

const TCHAR* CauseText(Kinetic::EStallCause Cause)
{
	using Kinetic::EStallCause;
	switch (Cause)
	{
	case EStallCause::RingOut: return TEXT("ring-out");
	case EStallCause::ObjectImpact: return TEXT("object impact");
	case EStallCause::WorldImpact: return TEXT("hit the world");
	case EStallCause::KinematicImpact: return TEXT("kinematic");
	case EStallCause::PlayerImpact: return TEXT("player impact");
	case EStallCause::Crush: return TEXT("crush");
	case EStallCause::Blast: return TEXT("blast");
	}
	return TEXT("stall");
}

// The crews' colours: the player's crew blue, the other orange.
const FLinearColor CrewColour[2] = {FLinearColor(0.10f, 0.35f, 0.90f), FLinearColor(0.95f, 0.45f, 0.08f)};

constexpr double CrewSpacingCm = 250.0;
}  // namespace

FKineticScrap::FKineticScrap(UWorld& InWorld, const FSettings& InSettings) : World(InWorld), Settings(InSettings)
{
	const EKineticHarness Classes[Slots] = {Settings.HumanClass,    EKineticHarness::Breaker, EKineticHarness::Climber,
	                                        EKineticHarness::Breaker, EKineticHarness::Rigger,  EKineticHarness::Scrapper};
	for (int32 Slot = 0; Slot < Slots; ++Slot)
	{
		Kinetic::FMatchPlayer P;
		P.Slot = Slot;
		P.Team = TeamOf(Slot);
		P.bIsBot = Slot != HumanSlot();
		P.AccountId = P.bIsBot ? "" : "local-player";
		P.Harness = KineticHarness::ToCore(Classes[Slot]);
		Roster.Add(P);
	}
	if (UKineticSpoolWorld* S = Spool())
	{
		S->SetAutoRespawn(false);  // A Stalled rigger is out until the round resets
	}
	SpawnRiggers();
	StartMatch();
}

UKineticSpoolWorld* FKineticScrap::Spool() const { return World.GetSubsystem<UKineticSpoolWorld>(); }

FTransform FKineticScrap::SpawnFor(int32 Slot) const
{
	// The crew's side this round, and the rigger's place in the line across the start.
	const int32 Team = TeamOf(Slot);
	const int32 Side = TheMatch ? TheMatch->SpawnOf(Team) : Team;
	const FName Tag(Side == 0 ? TEXT("Spawn_A") : TEXT("Spawn_B"));
	FTransform Start = FTransform::Identity;
	for (TActorIterator<APlayerStart> It(&World); It; ++It)
	{
		if (It->PlayerStartTag == Tag)
		{
			Start = It->GetActorTransform();
			break;
		}
	}
	const int32 Place = Slot % 3;
	const double Across = Place == 0 ? 0.0 : Place == 1 ? -CrewSpacingCm : CrewSpacingCm;
	const FRotator Facing(0.0, Start.Rotator().Yaw, 0.0);
	const FVector Right = FRotationMatrix(Facing).GetUnitAxis(EAxis::Y);
	return FTransform(Facing, Start.GetLocation() + Right * Across);
}

void FKineticScrap::SpawnRiggers()
{
	for (int32 Slot = 0; Slot < Slots; ++Slot)
	{
		const FTransform At = SpawnFor(Slot);
		AKineticCharacter* Rigger = World.SpawnActorDeferred<AKineticCharacter>(
		    AKineticCharacter::StaticClass(), At, nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
		Rigger->Harness = KineticHarness::FromCore(Roster[Slot].Harness);
		Rigger->FinishSpawning(At);
		Rigger->SetCrewColour(CrewColour[TeamOf(Slot)]);
		FActorSpawnParameters Params;
		Params.ObjectFlags |= RF_Transient;
		AKineticBotController* Brain = World.SpawnActor<AKineticBotController>(Params);
		Brain->Init(Slot, Settings.Seed, Roster[Slot].Harness);
		if (Slot != HumanSlot())
		{
			Brain->Possess(Rigger);
			Rigger->SetTakesLadders(false);  // The bot takes the ladders its route uses
			// Bots steer round each other (the engine's reciprocal avoidance): a rigger run into
			// at full speed is staggered (DT_ImpactThresholds, PlayerVsPlayerFloor).
			UCharacterMovementComponent* Movement = Rigger->GetCharacterMovement();
			Movement->AvoidanceConsiderationRadius = 300.0f;
			Movement->SetAvoidanceEnabled(true);
		}
		Riggers.Add(Rigger);
		Brains.Add(Brain);
	}
}

void FKineticScrap::AddHuman(APlayerController* Player)
{
	AKineticCharacter* Mine = Rigger(0);
	if (!Player || !Mine)
	{
		return;
	}
	if (Settings.bHumanPlays)
	{
		Player->Possess(Mine);
		LastHumanInputs = Mine->InputCount();
	}
	else
	{
		Player->SetViewTarget(Mine);  // Watching slot 0's bot
	}
}

AKineticCharacter* FKineticScrap::Rigger(int32 Slot) const
{
	return Riggers.IsValidIndex(Slot) ? Riggers[Slot].Get() : nullptr;
}

const AKineticBotController* FKineticScrap::Bot(int32 Slot) const
{
	return Brains.IsValidIndex(Slot) ? Brains[Slot].Get() : nullptr;
}

int32 FKineticScrap::SlotOf(const AKineticCharacter* Character) const
{
	for (int32 Slot = 0; Slot < Riggers.Num(); ++Slot)
	{
		if (Riggers[Slot].Get() == Character)
		{
			return Slot;
		}
	}
	return INDEX_NONE;
}

void FKineticScrap::StartMatch()
{
	Log = MakeUnique<Kinetic::FEventLog>();
	TheMatch = MakeUnique<Kinetic::FScrapMatch>(std::vector<Kinetic::FMatchPlayer>(Roster.GetData(), Roster.GetData() + Roster.Num()),
	                                            Settings.Rules, *Log);
	TheMatch->Start();
	MatchOverAt = -1.0;
}

void FKineticScrap::Say(FString Text)
{
	UE_LOG(LogKineticScrap, Log, TEXT("%.1f s: %s"), TheMatch ? TheMatch->Time() : 0.0, *Text);
	FeedLines.Add(MoveTemp(Text));
	while (FeedLines.Num() > 6)
	{
		FeedLines.RemoveAt(0);
	}
}

FString FKineticScrap::Who(int32 Slot) const
{
	if (Slot == HumanSlot())
	{
		return TEXT("You");
	}
	const FString Crew = Settings.bHumanPlays ? (TeamOf(Slot) == 0 ? TEXT("(your crew)") : TEXT("(their crew)"))
	                                          : FString::Printf(TEXT("(crew %c)"), TeamOf(Slot) == 0 ? TEXT('A') : TEXT('B'));
	return KineticHarness::Name(KineticHarness::FromCore(Roster[Slot].Harness)) + TEXT(" ") + Crew;
}

void FKineticScrap::ResetArena()
{
	UKineticSpoolWorld* S = Spool();
	S->ClearLines();
	for (TActorIterator<AKineticProp> It(&World); It; ++It)
	{
		It->ResetToAuthored();
	}
	for (int32 Slot = 0; Slot < Riggers.Num(); ++Slot)
	{
		AKineticCharacter* Rigger = Riggers[Slot].Get();
		if (!Rigger)
		{
			continue;
		}
		const FTransform At = SpawnFor(Slot);
		S->PlaceStanding(Rigger, At.GetLocation(), At.Rotator());
		if (!TheMatch->IsStanding(Slot))
		{
			S->Stall(Rigger, Kinetic::EStallCause::WorldImpact);  // Withdrawn: not in this round
		}
		if (AKineticBotController* Brain = Brains[Slot].Get())
		{
			Brain->NewRound();
		}
	}
	++Resets;
}

void FKineticScrap::Tick()
{
	UKineticSpoolWorld* S = Spool();
	if (!S || !TheMatch)
	{
		return;
	}
	// The Spool's last step: Stalls count against the crews.
	TArray<FKineticSpoolEvent> Events = S->TakeEvents();
	for (const FKineticSpoolEvent& E : Events)
	{
		const int32 Slot = E.Kind == EKineticSpoolEventKind::Stall ? SlotOf(E.Target.Get()) : INDEX_NONE;
		if (Slot != INDEX_NONE && TheMatch->OnStall(Slot))
		{
			++Stalls;
			Say(FString::Printf(TEXT("%s scrapped: %s."), *Who(Slot), CauseText(E.Cause)));
		}
	}

	TheMatch->Update(UKineticSpoolWorld::Dt);
	bool bReset = false;
	for (const Kinetic::FFlowEvent& F : TheMatch->TakeEvents())
	{
		switch (F.Kind)
		{
		case Kinetic::EFlow::ResetArena:
			ResetArena();
			bReset = true;
			break;
		case Kinetic::EFlow::Call:
			Say(CallText(F.Call));
			break;
		case Kinetic::EFlow::RoundOver:
			Say(F.Team == Kinetic::NoPlayer ? FString(TEXT("Round replayed."))
			    : Settings.bHumanPlays      ? FString(F.Team == 0 ? TEXT("Your crew takes the round.") : TEXT("Their crew takes the round."))
			                                : FString::Printf(TEXT("Crew %c takes the round."), F.Team == 0 ? TEXT('A') : TEXT('B')));
			break;
		case Kinetic::EFlow::MatchOver:
			Say(F.Team == Kinetic::NoPlayer ? FString(TEXT("No contest."))
			    : Settings.bHumanPlays      ? FString(F.Team == 0 ? TEXT("Match: your crew.") : TEXT("Match: their crew."))
			                                : FString::Printf(TEXT("Match: crew %c."), F.Team == 0 ? TEXT('A') : TEXT('B')));
			MatchOverAt = TheMatch->Time();
			++Finished;
			break;
		case Kinetic::EFlow::Backfill:
			Say(FString::Printf(TEXT("A bot takes over %s."), *Who(F.Player)));
			break;
		default:
			break;
		}
	}
	if (TheMatch->Phase() == Kinetic::EScrapPhase::MatchOver && MatchOverAt >= 0.0 && TheMatch->Time() - MatchOverAt >= RestartSeconds)
	{
		StartMatch();
		return;
	}
	if (bReset)
	{
		Events.Reset();  // What happened before the reset is gone with it
	}

	S->SetLinesAllowed(TheMatch->CanUseLines());
	TArray<FKineticRiggerView> View;
	for (int32 Slot = 0; Slot < Riggers.Num(); ++Slot)
	{
		AKineticCharacter* Rigger = Riggers[Slot].Get();
		View.Add({Rigger, Slot, TeamOf(Slot), Rigger && TheMatch->IsStanding(Slot) && S->IsStanding(Rigger)});
	}
	for (int32 Slot = 0; Slot < Riggers.Num(); ++Slot)
	{
		AKineticCharacter* Rigger = Riggers[Slot].Get();
		if (!Rigger)
		{
			continue;
		}
		Rigger->SetMovementAllowed(TheMatch->CanMove(Slot));
		AKineticBotController* Brain = Brains[Slot].Get();
		if (Brain && (Slot != HumanSlot() || TheMatch->IsBotControlled(Slot)))
		{
			Brain->Think(*Rigger, View, Events);
		}
	}
	// Any input counts against the idle rules (game-modes 8): moving, looking, any button.
	if (AKineticCharacter* Mine = Rigger(HumanSlot()))
	{
		if (Mine->InputCount() != LastHumanInputs)
		{
			LastHumanInputs = Mine->InputCount();
			TheMatch->OnInput(HumanSlot());
		}
	}
}
