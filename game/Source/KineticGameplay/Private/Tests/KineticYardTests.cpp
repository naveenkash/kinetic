// Plays the Yard grey-box in the editor: the character spawns on West Mesa (Spawn A), stands,
// runs at its class's run speed and jumps its class's jump height (bible/05 section 2.2); the
// Spool and hits on the mesa; and a Scrap match of six bots. Needs the level built (make
// yard-greybox) and the props imported (make import-props). The one-character tests play the
// walk-the-arena mode (kinetic.Mode Walk).
#include "Camera/CameraComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"
#include "HAL/IConsoleManager.h"
#include "KineticBotController.h"
#include "KineticCharacter.h"
#include "KineticGameMode.h"
#include "KineticProp.h"
#include "KineticRulesSubsystem.h"
#include "KineticSpoolWorld.h"
#include "Misc/AutomationTest.h"
#include "NavigationSystem.h"
#include "Tests/AutomationCommon.h"

#if WITH_EDITOR && WITH_DEV_AUTOMATION_TESTS

#include "Editor.h"
#include "Tests/AutomationEditorCommon.h"

namespace
{
const TCHAR* YardMap = TEXT("/Game/Arenas/YRD_Yard/L_YRD_Greybox");

// The mode the next play session starts in (AKineticGameMode reads it at InitGame).
void PlayMode(const TCHAR* Mode, bool bAllBots)
{
	IConsoleManager::Get().FindConsoleVariable(TEXT("kinetic.Mode"))->Set(Mode, ECVF_SetByCode);
	IConsoleManager::Get().FindConsoleVariable(TEXT("kinetic.AllBots"))->Set(bAllBots ? 1 : 0, ECVF_SetByCode);
}

// Waits for the play world. In the editor, AutomationOpenMap opens the map in PIE itself, so
// nothing else may start a session (a second request tears the first down).
class FWaitForPlay : public IAutomationLatentCommand
{
public:
	explicit FWaitForPlay(FAutomationTestBase* InTest) : Test(InTest), Started(FPlatformTime::Seconds()) {}

	virtual bool Update() override
	{
		if (GEditor->PlayWorld && GEditor->PlayWorld->HasBegunPlay())
		{
			return true;
		}
		if (FPlatformTime::Seconds() - Started > 60.0)
		{
			Test->AddError(TEXT("PIE did not start in 60 s"));
			return true;
		}
		return false;
	}

private:
	FAutomationTestBase* Test;
	double Started;
};

class FPlayTheYard : public IAutomationLatentCommand
{
public:
	explicit FPlayTheYard(FAutomationTestBase* InTest) : Test(InTest) {}

	virtual bool Update() override
	{
		UWorld* World = GEditor ? GEditor->PlayWorld.Get() : nullptr;
		if (!World)
		{
			return Fail(TEXT("no PIE world"));
		}
		AKineticCharacter* Character = Cast<AKineticCharacter>(UGameplayStatics::GetPlayerPawn(World, 0));
		const double Now = World->GetTimeSeconds();
		if (!Character)
		{
			return Now > 10.0 ? Fail(TEXT("no Kinetic character was spawned")) : false;
		}
		UCharacterMovementComponent* Movement = Character->GetCharacterMovement();
		const FVector At = Character->GetActorLocation();
		if (Now > 30.0)
		{
			return Fail(TEXT("timed out"));
		}
		switch (Stage)
		{
		case 0:  // Settle, then check where and how it stands.
			if (Now < 1.5)
			{
				return false;
			}
			Test->TestTrue(TEXT("standing after spawning"), Movement->IsMovingOnGround());
			// Spawn A is the 10 m circle at layout (-68, -39, 8): Unreal (-6800, 3900, 800).
			Test->TestTrue(TEXT("on West Mesa"), FVector::Dist2D(At, FVector(-6800.0, 3900.0, 0.0)) < 600.0);
			Test->TestEqual(TEXT("feet on the 8 m mesa top"), At.Z - Character->GetSimpleCollisionHalfHeight(), 800.0, 5.0);
			Stage = 1;
			StageAt = Now;
			return false;
		case 1:  // Run west across the mesa for a second and a half.
			if (APlayerController* Player = Cast<APlayerController>(Character->GetController()))
			{
				Player->SetControlRotation(FRotator(0.0, 180.0, 0.0));
			}
			Character->Move(FVector2D(0.0, 1.0));
			TopSpeed = FMath::Max(TopSpeed, FVector(Movement->Velocity.X, Movement->Velocity.Y, 0.0).Size());
			if (Now - StageAt < 1.5)
			{
				return false;
			}
			Test->TestEqual(TEXT("top speed is the class's run speed"), TopSpeed / 100.0,
			                static_cast<double>(Character->RunSpeed()), 0.1);
			Stage = 2;
			StageAt = Now;
			return false;
		case 2:  // Stop, then jump and measure the rise.
			if (Now - StageAt < 1.0)
			{
				return false;
			}
			if (JumpFrom < 0.0)
			{
				JumpFrom = At.Z;
				Character->Jump();
				return false;
			}
			Peak = FMath::Max(Peak, At.Z);
			if (Now - StageAt < 2.5)
			{
				return false;
			}
			Character->StopJumping();
			{
				const UKineticRulesSubsystem* Rules = GEngine->GetEngineSubsystem<UKineticRulesSubsystem>();
				const double Height = Rules->Tables()->Harness(KineticHarness::ToCore(Character->Harness)).JumpHeight;
				Test->TestEqual(TEXT("jump rise is the class's jump height"), (Peak - JumpFrom) / 100.0, Height, Height * 0.05);
			}
			return true;
		default:
			return true;
		}
	}

private:
	bool Fail(const TCHAR* Why)
	{
		Test->AddError(Why);
		return true;
	}

	FAutomationTestBase* Test;
	int Stage = 0;
	double StageAt = 0.0;
	double TopSpeed = 0.0;
	double JumpFrom = -1.0;
	double Peak = -1.0e9;
};
// The Spool on West Mesa, next to Spawn A (bible/05 sections 3.2, 3.4), in this order: a Haul
// brings a folding chair to the player until the Dogs meet; a Hang into the mesa top holds the
// player at its length when they walk away, and released it is a set line, which the Cutter
// cuts once warm; a Jack into the mesa top Kicks the player up at the class's launch speed.
class FSpoolOnTheMesa : public IAutomationLatentCommand
{
public:
	explicit FSpoolOnTheMesa(FAutomationTestBase* InTest) : Test(InTest) {}

	virtual bool Update() override
	{
		UWorld* World = GEditor ? GEditor->PlayWorld.Get() : nullptr;
		AKineticCharacter* Me = World ? Cast<AKineticCharacter>(UGameplayStatics::GetPlayerPawn(World, 0)) : nullptr;
		UKineticSpoolWorld* Spool = World ? World->GetSubsystem<UKineticSpoolWorld>() : nullptr;
		if (!Me || !Spool)
		{
			return Fail(TEXT("no player or no Spool"));
		}
		const double Now = World->GetTimeSeconds();
		if (Now > 40.0)
		{
			return Fail(*FString::Printf(TEXT("timed out in stage %d: movement mode %d at %s"), Stage,
			                             static_cast<int>(Me->GetCharacterMovement()->MovementMode.GetValue()),
			                             *Me->GetActorLocation().ToString()));
		}
		APlayerController* Player = Cast<APlayerController>(Me->GetController());
		const auto Aim = [&](const FVector& At) {
			Player->SetControlRotation((At - Me->Camera->GetComponentLocation()).Rotation());
		};
		switch (Stage)
		{
		case 0:  // Settle, find the nearest chair.
			if (Now < 1.5)
			{
				return false;
			}
			for (TActorIterator<AKineticProp> It(World); It; ++It)
			{
				if (It->GetActorLabel().StartsWith(TEXT("FoldingChair")) &&
				    (!Chair.IsValid() || FVector::Dist(It->GetActorLocation(), Me->GetActorLocation()) <
				                             FVector::Dist(Chair->GetActorLocation(), Me->GetActorLocation())))
				{
					Chair = *It;
				}
			}
			if (!Chair.IsValid())
			{
				return Fail(TEXT("no folding chair on West Mesa"));
			}
			StartGap = FVector::Dist(Chair->Mesh->GetCenterOfMass(), Me->GetActorLocation());
			Aim(Chair->Mesh->GetCenterOfMass());
			Spool->SelectMode(Me, Kinetic::ELineMode::Haul);
			Spool->SelfDogNext(Me);   // Dog A into my chest plate
			Spool->FireDog(Me);       // Dog B at the chair
			Stage = 1;
			StageAt = Now;
			return false;
		case 1:  // The Haul: the chair comes in until the Dogs meet.
			if (Spool->Lines().Num() > 0)
			{
				const FKineticLine& L = Spool->Lines()[0];
				bSeated = bSeated || (L.B.Seat == EKineticSeat::Prop && L.B.Prop == Chair);
				bHauled = bHauled || (L.IsLive() && L.State.Mode == Kinetic::ELineMode::Haul);

			}
			MinGap = FMath::Min(MinGap, FVector::Dist(Chair->Mesh->GetCenterOfMass(), Me->GetActorLocation()));
			if (Now - StageAt < 2.0)
			{
				return false;
			}
			Test->TestTrue(TEXT("Dog B seated in the chair"), bSeated);
			Test->TestTrue(TEXT("the line engaged Haul"), bHauled);
			Test->TestTrue(FString::Printf(TEXT("the chair came in (from %.0f cm to %.0f cm)"), StartGap, MinGap),
			               MinGap < StartGap - 100.0);
			Test->TestEqual(TEXT("the Dogs met and the Haul ended"), Spool->Lines().Num(), 0);
			Spool->Release(Me);
			Stage = 3;
			StageAt = Now;
			return false;
		case 2:  // A Jack off the mesa top just ahead: the Kick launches me up.
			if (Now - StageAt < 1.0)
			{
				return false;
			}
			if (!bJackFired)
			{
				Aim(Me->GetActorLocation() - FVector(0.0, 0.0, 300.0));  // Dog B straight down, under my feet
				Spool->SelectMode(Me, Kinetic::ELineMode::Jack);
				Spool->SelfDogNext(Me);
				Spool->FireDog(Me);
				bJackFired = true;
				JackAt = Now;
				return false;
			}
			TopRise = FMath::Max(TopRise, Me->GetCharacterMovement()->Velocity.Z / 100.0);
			if (Now - JackAt < 1.0)
			{
				return false;
			}
			{
				const UKineticRulesSubsystem* Rules = GEngine->GetEngineSubsystem<UKineticRulesSubsystem>();
				const double V = Rules->Tables()->Harness(KineticHarness::ToCore(Me->Harness)).KickLaunchSpeedV;
				Test->TestTrue(FString::Printf(TEXT("the Kick launched me up (%.1f m/s against V = %.1f)"), TopRise, V),
				               TopRise > 0.8 * V);
			}
			return true;
		case 3:  // A Hang into the mesa top 3 m ahead, then I walk away: it holds me at its length (05 3.2).
			if (Now - StageAt < 1.0 || !Me->GetCharacterMovement()->IsMovingOnGround())
			{
				return false;  // Stand still after the Haul first
			}
			if (!bHangFired)
			{
				const FVector Ahead = Me->GetActorForwardVector().GetSafeNormal2D();
				Aim(Me->GetActorLocation() + Ahead * 300.0 - FVector(0.0, 0.0, 95.0));
				Spool->SelectMode(Me, Kinetic::ELineMode::Hang);
				Spool->SelfDogNext(Me);
				Spool->FireDog(Me);
				bHangFired = true;
				HangAt = Now;
				return false;
			}
			if (Spool->Lines().Num() > 0 && Spool->Lines()[0].IsLive() && Spool->Lines()[0].State.Mode == Kinetic::ELineMode::Hang &&
			    HangLength < 0.0)
			{
				const FKineticLine& L = Spool->Lines()[0];
				Anchor = L.B.Point;
				HangLength = L.State.L0 * 100.0;
				WalkFrom = Me->GetActorLocation();
			}
			if (HangLength < 0.0)
			{
				return Now - HangAt > 2.0 ? Fail(TEXT("the Hang line never seated and engaged")) : false;
			}
			// Walk directly away from the anchor.
			Player->SetControlRotation((Me->GetActorLocation() - Anchor).GetSafeNormal2D().Rotation());
			Me->Move(FVector2D(0.0, 1.0));
			MaxStretch = FMath::Max(MaxStretch, FVector::Dist(Me->GetActorLocation(), Anchor) - HangLength);
			if (Now - HangAt < 2.5)
			{
				return false;
			}
			Test->TestTrue(TEXT("the line is still a Hang"), Spool->Lines().Num() > 0 &&
			                                                     Spool->Lines()[0].State.Mode == Kinetic::ELineMode::Hang);
			Test->TestTrue(FString::Printf(TEXT("the line held me at its length (%.0f cm past its %.0f cm rest length)"),
			                               MaxStretch, HangLength),
			               MaxStretch < 25.0);
			Spool->Release(Me);  // A released Hang is a set line, kept until it expires or is cut (05 3.6)
			Test->TestTrue(TEXT("released, the Hang is a set line"), Spool->Lines().Num() == 1 && Spool->Lines()[0].bSet);
			Spool->SetCutterDrawn(Me, true);
			Stage = 4;
			StageAt = Now;
			return false;
		case 4:  // The Cutter: once warm, a swing at the set line cuts it (05 4.1).
			Aim((Spool->DrawnEndPosition(Spool->Lines()[0], true) + Spool->DrawnEndPosition(Spool->Lines()[0], false)) * 0.5);
			if (Now - StageAt < 0.6)
			{
				return false;
			}
			Spool->Swing(Me);
			Test->TestEqual(TEXT("the swing cut the set line"), Spool->Lines().Num(), 0);
			Spool->SetCutterDrawn(Me, false);
			Stage = 2;
			StageAt = Now;
			return false;
		default:
			return true;
		}
	}

private:
	bool Fail(const TCHAR* Why)
	{
		Test->AddError(Why);
		return true;
	}

	FAutomationTestBase* Test;
	int Stage = 0;
	double StageAt = 0.0;
	TWeakObjectPtr<AKineticProp> Chair;
	double StartGap = 0.0;
	bool bHangFired = false;
	double HangAt = 0.0, HangLength = -1.0, MaxStretch = -1.0e9;
	FVector WalkFrom = FVector::ZeroVector;
	FVector Anchor = FVector::ZeroVector;
	double MinGap = TNumericLimits<double>::Max();
	bool bSeated = false, bHauled = false, bJackFired = false;
	double JackAt = 0.0;
	double TopRise = 0.0;
};
}  // namespace

// Hits in the Yard (bible/05 sections 7.1 to 7.5): a fall at over 5 m/s staggers, a fall at
// over 12 m/s Stalls against the world, and a fall into the Well is a ring-out; a Stalled
// player is back at Spawn A after the respawn time.
class FHitsInTheYard : public IAutomationLatentCommand
{
public:
	explicit FHitsInTheYard(FAutomationTestBase* InTest) : Test(InTest) {}

	virtual bool Update() override
	{
		UWorld* World = GEditor ? GEditor->PlayWorld.Get() : nullptr;
		AKineticCharacter* Me = World ? Cast<AKineticCharacter>(UGameplayStatics::GetPlayerPawn(World, 0)) : nullptr;
		UKineticSpoolWorld* Spool = World ? World->GetSubsystem<UKineticSpoolWorld>() : nullptr;
		if (!Me || !Spool)
		{
			Test->AddError(TEXT("no player or no Spool"));
			return true;
		}
		const double Now = World->GetTimeSeconds();
		if (Now > 40.0)
		{
			Test->AddError(FString::Printf(TEXT("timed out in stage %d"), Stage));
			return true;
		}
		const FKineticSpoolPlayer* P = Spool->PlayerState(Me);
		// Layout (x, y, feet z) in metres to the capsule's centre in Unreal.
		const auto Drop = [&](double X, double Y, double FeetZ) {
			Me->TeleportTo(FVector(X * 100.0, -Y * 100.0, FeetZ * 100.0 + 95.0), Me->GetActorRotation());
			Me->GetCharacterMovement()->Velocity = FVector::ZeroVector;
			Me->GetCharacterMovement()->SetMovementMode(MOVE_Falling);
		};
		switch (Stage)
		{
		case 0:  // A 1.8 m drop onto Marker Flat: 5.9 m/s, over the 5 m/s stagger threshold, under the flip's 8.
			if (Now < 1.5)
			{
				return false;
			}
			Drop(-10.0, 10.0, 1.8);
			Next(Now);
			return false;
		case 1:
			if (Now - StageAt < 1.2)
			{
				return false;
			}
			Test->TestTrue(TEXT("a 5.9 m/s landing staggers"), P && P->LastOutcome == Kinetic::EOutcome::Stagger &&
			                                                     P->LastOutcomeAt > StageAt);
			Test->TestTrue(TEXT("staggered, still standing"), P && P->bStanding);
			Drop(-10.0, 10.0, 20.0);  // 20 m: about 19.8 m/s, a Stall against the world (12 m/s)
			Next(Now);
			return false;
		case 2:
			if (Now - StageAt < 2.6)
			{
				return false;
			}
			Test->TestTrue(TEXT("a 20 m fall Stalls"), P && !P->bStanding);
			Test->TestTrue(TEXT("against the world"), P && P->StallCause == Kinetic::EStallCause::WorldImpact);
			Next(Now);
			return false;
		case 3:  // Back at Spawn A after the respawn time.
			if (Now - StageAt < UKineticSpoolWorld::RespawnSeconds + 0.5)
			{
				return false;
			}
			Test->TestTrue(TEXT("standing again"), P && P->bStanding);
			Test->TestTrue(TEXT("back on West Mesa"), FVector::Dist2D(Me->GetActorLocation(), FVector(-6800.0, 3900.0, 0.0)) < 600.0);
			Drop(45.0, 0.0, 1.0);  // Into the Well
			Next(Now);
			return false;
		case 4:
			if (Now - StageAt < 2.0)
			{
				return false;
			}
			Test->TestTrue(TEXT("the Well is a ring-out"), P && !P->bStanding && P->StallCause == Kinetic::EStallCause::RingOut);
			return true;
		default:
			return true;
		}
	}

private:
	void Next(double Now)
	{
		++Stage;
		StageAt = Now;
	}

	FAutomationTestBase* Test;
	int Stage = 0;
	double StageAt = 0.0;
};


// A Scrap match of six bots on the Yard, with shorter rounds (120 s, 20 s of sudden death): the
// crews start at their sides' spawns and are held through the reset; lines open at Live; the
// bots come down off the Stacks (the ladders are navigation links) and fire Dogs; the round
// ends, and at the next reset the sides have swapped.
class FScrapInTheYard : public IAutomationLatentCommand
{
public:
	explicit FScrapInTheYard(FAutomationTestBase* InTest) : Test(InTest) {}

	virtual bool Update() override
	{
		UWorld* World = GEditor ? GEditor->PlayWorld.Get() : nullptr;
		const AKineticGameMode* Mode = World ? World->GetAuthGameMode<AKineticGameMode>() : nullptr;
		FKineticScrap* Scrap = Mode ? Mode->Scrap() : nullptr;
		UKineticSpoolWorld* Spool = World ? World->GetSubsystem<UKineticSpoolWorld>() : nullptr;
		if (!Scrap || !Spool)
		{
			return Fail(TEXT("no Scrap match (kinetic.Mode Scrap)"));
		}
		const double Now = World->GetTimeSeconds();
		const Kinetic::FScrapMatch& M = Scrap->Match();
		if (Stage > 0 && Now - StartedAt > 260.0)
		{
			return Fail(*FString::Printf(TEXT("timed out in stage %d: round %d, phase %d, clock %.1f"), Stage, M.Round(),
			                             static_cast<int>(M.Phase()), M.ClockRemaining()));
		}
		const auto SpawnOfSide = [World](int32 Side) {
			for (TActorIterator<APlayerStart> It(World); It; ++It)
			{
				if (It->PlayerStartTag == FName(Side == 0 ? TEXT("Spawn_A") : TEXT("Spawn_B")))
				{
					return It->GetActorLocation();
				}
			}
			return FVector::ZeroVector;
		};
		const auto AllAtSpawns = [&]() {
			for (int32 Slot = 0; Slot < Scrap->SlotCount(); ++Slot)
			{
				const AKineticCharacter* R = Scrap->Rigger(Slot);
				const FVector Spawn = SpawnOfSide(M.SpawnOf(Scrap->TeamOf(Slot)));
				if (!R || FVector::Dist2D(R->GetActorLocation(), Spawn) > 600.0)
				{
					return false;
				}
			}
			return true;
		};
		switch (Stage)
		{
		case 0:  // A fresh match on short rules.
		{
			if (Now < 0.5)
			{
				return false;
			}
			Test->TestEqual(TEXT("six riggers"), Scrap->SlotCount(), 6);
			Kinetic::FScrapRules Rules;
			Rules.RoundSeconds = 120.0;
			Rules.SirenRemaining = 30.0;
			Rules.EventRemaining = 25.0;
			Rules.SuddenDeathSeconds = 20.0;
			Scrap->SetRules(Rules);
			Scrap->StartMatch();
			StartedAt = Now;
			Stage = 1;
			return false;
		}
		case 1:  // The reset: at the spawns, held.
			if (Now - StartedAt < 1.0)
			{
				return false;
			}
			Test->TestEqual(TEXT("the reset phase"), static_cast<int>(M.Phase()), static_cast<int>(Kinetic::EScrapPhase::Reset));
			Test->TestTrue(TEXT("every rigger at their crew's spawn"), AllAtSpawns());
			Test->TestFalse(TEXT("held in the reset"), Scrap->Rigger(0)->IsMovementAllowed());
			Test->TestFalse(TEXT("no lines in the reset"), Spool->LinesAllowed());
			for (int32 Slot = 0; Slot < Scrap->SlotCount(); ++Slot)
			{
				LowestFeet.Add(TNumericLimits<double>::Max());
			}
			Stage = 2;
			return false;
		case 2:  // Live: lines open.
			if (M.Phase() != Kinetic::EScrapPhase::Live)
			{
				return false;
			}
			Test->TestTrue(TEXT("lines from Live"), Spool->LinesAllowed());
			Test->TestFalse(TEXT("the navmesh is built by Live"), UNavigationSystemV1::IsNavigationBeingBuilt(World));
			Stage = 3;
			return false;
		case 3:  // The round: the bots come down and rig, until it ends.
			for (int32 Slot = 0; Slot < Scrap->SlotCount(); ++Slot)
			{
				if (const AKineticCharacter* R = Scrap->Rigger(Slot))
				{
					if (Spool->IsStanding(R))
					{
						LowestFeet[Slot] = FMath::Min(LowestFeet[Slot], R->GetActorLocation().Z - 95.0);
					}
				}
			}
			MostLines = FMath::Max(MostLines, Spool->Lines().Num());
			if (M.Round() < 1)
			{
				return false;
			}
			{
				int32 Down = 0;
				int32 Dogs = 0;
				for (int32 Slot = 0; Slot < Scrap->SlotCount(); ++Slot)
				{
					Down += LowestFeet[Slot] < 700.0 ? 1 : 0;
					Dogs += Scrap->Bot(Slot) ? Scrap->Bot(Slot)->DogsFired() : 0;
				}
				Test->AddInfo(FString::Printf(TEXT("round 1 over at %.0f s: %d of 6 came off the Stacks, %d Dogs fired, %d lines at once, %d Stalls, score %d-%d"),
				                              Now - StartedAt, Down, Dogs, MostLines, Scrap->StallsSeen(), M.Wins(0), M.Wins(1)));
				for (const FString& Line : Scrap->Feed())
				{
					Test->AddInfo(Line);
				}
				Test->TestTrue(TEXT("most bots came down off the 8 m Stacks"), Down >= 4);
				Test->TestTrue(TEXT("the bots fired Dogs"), Dogs > 0);
			}
			Stage = 4;
			StageAt = Now;
			return false;
		case 4:  // The next reset: sides swapped.
			if (M.Phase() != Kinetic::EScrapPhase::Reset || Now - StageAt < 1.0)
			{
				return M.Phase() == Kinetic::EScrapPhase::MatchOver ? Fail(TEXT("the match ended after one round")) : false;
			}
			Test->TestEqual(TEXT("crew 0 starts round 2 at Spawn B"), M.SpawnOf(0), 1);
			for (int32 Slot = 0; Slot < Scrap->SlotCount(); ++Slot)
			{
				const AKineticCharacter* R = Scrap->Rigger(Slot);
				const FVector Spawn = SpawnOfSide(M.SpawnOf(Scrap->TeamOf(Slot)));
				Test->TestTrue(*FString::Printf(TEXT("slot %d at their crew's new spawn (%s, spawn %s)"), Slot,
				                                R ? *R->GetActorLocation().ToCompactString() : TEXT("-"), *Spawn.ToCompactString()),
				               R && FVector::Dist2D(R->GetActorLocation(), Spawn) < 600.0);
			}
			Test->TestTrue(TEXT("two resets"), Scrap->ResetsSeen() >= 2);
			return true;
		default:
			return true;
		}
	}

private:
	bool Fail(const TCHAR* Why)
	{
		Test->AddError(Why);
		return true;
	}

	FAutomationTestBase* Test;
	int Stage = 0;
	double StartedAt = 0.0;
	double StageAt = 0.0;
	TArray<double> LowestFeet;
	int32 MostLines = 0;
};

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKineticYardScrapTest, "Kinetic.Yard.Scrap",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FKineticYardScrapTest::RunTest(const FString& Parameters)
{
	PlayMode(TEXT("Scrap"), true);
	IConsoleManager::Get().FindConsoleVariable(TEXT("kinetic.Seed"))->Set(7, ECVF_SetByCode);
	if (!AutomationOpenMap(YardMap))
	{
		AddError(TEXT("the Yard grey-box did not open (make yard-greybox)"));
		return false;
	}
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForPlay(this));
	ADD_LATENT_AUTOMATION_COMMAND(FScrapInTheYard(this));
	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKineticYardHitsTest, "Kinetic.Yard.Hits",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FKineticYardHitsTest::RunTest(const FString& Parameters)
{
	PlayMode(TEXT("Walk"), false);
	if (!AutomationOpenMap(YardMap))
	{
		AddError(TEXT("the Yard grey-box did not open (make yard-greybox)"));
		return false;
	}
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForPlay(this));
	ADD_LATENT_AUTOMATION_COMMAND(FHitsInTheYard(this));
	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKineticYardSpoolTest, "Kinetic.Yard.Spool",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FKineticYardSpoolTest::RunTest(const FString& Parameters)
{
	PlayMode(TEXT("Walk"), false);
	if (!AutomationOpenMap(YardMap))
	{
		AddError(TEXT("the Yard grey-box did not open (make yard-greybox)"));
		return false;
	}
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForPlay(this));
	ADD_LATENT_AUTOMATION_COMMAND(FSpoolOnTheMesa(this));
	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKineticYardPlayTest, "Kinetic.Yard.Play",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FKineticYardPlayTest::RunTest(const FString& Parameters)
{
	PlayMode(TEXT("Walk"), false);
	if (!AutomationOpenMap(YardMap))
	{
		AddError(TEXT("the Yard grey-box did not open (make yard-greybox)"));
		return false;
	}
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForPlay(this));
	ADD_LATENT_AUTOMATION_COMMAND(FPlayTheYard(this));
	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
	return true;
}

#endif
