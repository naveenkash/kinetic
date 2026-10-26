// Plays the Yard grey-box in the editor: the character spawns on West Mesa (Spawn A), stands,
// runs at its class's run speed and jumps its class's jump height (bible/05 section 2.2); the
// Spool and hits on the mesa; and a Scrap match of six bots. Needs the level built (make
// yard-greybox) and the props imported (make import-props). The one-character tests play the
// walk-the-arena mode (kinetic.Mode Walk).
#include "Camera/CameraActor.h"
#include "GroomAsset.h"
#include "GroomBindingAsset.h"
#include "GroomComponent.h"
#include "Camera/CameraComponent.h"
#include "UnrealClient.h"
#include "Components/SkeletalMeshComponent.h"
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
#include "KineticHazardWorld.h"
#include "KineticKinematic.h"
#include "KineticLadder.h"
#include "KineticProp.h"
#include "KineticYardDirector.h"
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
			// The West Mesa's nearest ladder from the top: stand at its top, facing away from the wall.
			for (TActorIterator<AKineticLadder> It(World); It; ++It)
			{
				if (FMath::Abs(It->TopZ() - 800.0f) < 50.0f &&
				    (!Ladder || FVector::Dist2D(It->StepOff(), At) < FVector::Dist2D(Ladder->StepOff(), At)))
				{
					Ladder = *It;
				}
			}
			if (!Ladder)
			{
				return Fail(TEXT("no ladder up the West Mesa"));
			}
			Character->TeleportTo(Ladder->StepOff() + FVector(0.0, 0.0, Character->GetSimpleCollisionHalfHeight() + 5.0),
			                      FRotator(0.0, (-Ladder->IntoWall).Rotation().Yaw, 0.0));
			Stage = 3;
			StageAt = Now;
			return false;
		case 3:  // Walk forward off the top, the wrong way round: caught on the rungs, not dropped.
			if (Now - StageAt < 0.5)
			{
				return false;  // Settle on the mesa top
			}
			if (APlayerController* Player = Cast<APlayerController>(Character->GetController()))
			{
				Player->SetControlRotation(FRotator(0.0, (-Ladder->IntoWall).Rotation().Yaw, 0.0));
			}
			if (!Character->IsClimbing())
			{
				Character->Move(FVector2D(0.0, 1.0));
			}
			if (Character->IsClimbing() || Now - StageAt > 3.0)
			{
				Test->TestTrue(TEXT("walking off a ladder's top catches the rungs"), Character->IsClimbing());
				Test->TestTrue(TEXT("caught near the top"), At.Z > 800.0 - 250.0);
				// Then the mesa's south edge, away from any ladder, running at it.
				Character->LeaveLadder();
				Character->TeleportTo(FVector(-6400.0, 3600.0, 800.0 + Character->GetSimpleCollisionHalfHeight() + 5.0),
				                      FRotator(0.0, 90.0, 0.0));
				Stage = 4;
				StageAt = Now;
			}
			return false;
		case 4:  // Run off the 8 m edge: the walk stops at it (the drop would Stall).
			if (APlayerController* Player = Cast<APlayerController>(Character->GetController()))
			{
				Player->SetControlRotation(FRotator(0.0, 90.0, 0.0));  // Unreal +Y: layout south
			}
			Character->Move(FVector2D(0.0, 1.0));
			if (Now - StageAt < 3.0)
			{
				return false;
			}
			// Feet on the lip: the capsule's round bottom rests over it, a little below the top.
			Test->TestTrue(TEXT("still on the mesa top after running at its edge"),
			               At.Z - Character->GetSimpleCollisionHalfHeight() > 700.0);
			Test->TestTrue(TEXT("standing at the edge"), Character->GetCharacterMovement()->IsMovingOnGround());
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
	AKineticLadder* Ladder = nullptr;
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


// The Yard's moving parts and hazards in the walk-the-arena mode (e is play time; Shift Change at
// e 69, the default Scrap rules' mid-match event): a set line across the south trench is severed
// by the e 10 train; the player standing in the trench ahead of it is Stalled by it (a
// kinematic); the train sweeps the trench's plank bridges; a propane cylinder dropped from 6 m
// beside the player arms on the impact and its blast Stalls them; Shift Change releases the
// Gantry's plate over Marker Flat, where it lands as scenery, and re-phases the Haulers.
class FEventsInTheYard : public IAutomationLatentCommand
{
public:
	explicit FEventsInTheYard(FAutomationTestBase* InTest) : Test(InTest) {}

	virtual bool Update() override
	{
		UWorld* World = GEditor ? GEditor->PlayWorld.Get() : nullptr;
		AKineticCharacter* Me = World ? Cast<AKineticCharacter>(UGameplayStatics::GetPlayerPawn(World, 0)) : nullptr;
		UKineticSpoolWorld* Spool = World ? World->GetSubsystem<UKineticSpoolWorld>() : nullptr;
		UKineticHazardWorld* Hazards = World ? World->GetSubsystem<UKineticHazardWorld>() : nullptr;
		AKineticYardDirector* Yard = nullptr;
		for (TActorIterator<AKineticYardDirector> It(World); It; ++It)
		{
			Yard = *It;
		}
		if (!Me || !Spool || !Hazards || !Yard)
		{
			return Fail(TEXT("no player, Spool, hazards or Yard director (make yard-greybox)"));
		}
		const double E = Yard->Elapsed();
		if (E > 85.0)
		{
			return Fail(*FString::Printf(TEXT("timed out in stage %d at e %.1f"), Stage, E));
		}
		for (const FKineticSpoolEvent& Ev : Spool->TakeEvents())
		{
			bSevered |= Ev.Kind == EKineticSpoolEventKind::LineEnded && Ev.Why == EKineticLineEnded::Severed;
		}
		// Layout metres to Unreal (Y flipped).
		const auto At = [](double X, double Y, double Z) { return FVector(X * 100.0, -Y * 100.0, Z * 100.0); };
		APlayerController* Player = Cast<APlayerController>(Me->GetController());
		const auto Aim = [&](const FVector& Point) {
			Player->SetControlRotation((Point - Me->Camera->GetComponentLocation()).Rotation());
		};
		const FKineticSpoolPlayer* State = Spool->PlayerState(Me);
		switch (Stage)
		{
		case 0:  // On the south trench's north lip; before the first train nothing runs.
			if (E < 1.0)
			{
				return false;
			}
			Test->TestFalse(TEXT("no Hauler before e 10"), Yard->IsHaulerRunning(0) || Yard->IsHaulerRunning(1));
			Me->TeleportTo(At(-54.0, -25.5, 0.0) + FVector(0, 0, 100), FRotator::ZeroRotator);
			Spool->SelectMode(Me, Kinetic::ELineMode::Hang);
			Next(E);
			return false;
		case 1:  // A Hang across the trench, lip to lip, released into a set line.
			if (E - StageAt < 0.5)
			{
				return false;
			}
			if (!bFiredA)
			{
				Aim(At(-54.0, -34.5, 0.0));
				Spool->FireDog(Me);
				bFiredA = true;
				return false;
			}
			if (!bFiredB && E - StageAt > 1.0)
			{
				Aim(At(-54.0, -26.6, 0.0));
				Spool->FireDog(Me);
				bFiredB = true;
				return false;
			}
			if (E - StageAt > 1.6)
			{
				const FKineticLine* L = Spool->ActiveLine(Me);
				Test->TestTrue(TEXT("a Hang across the south trench"), L && L->IsLive() && L->State.Mode == Kinetic::ELineMode::Hang);
				Spool->Release(Me);
				Test->TestTrue(TEXT("released into a set line"), Spool->Lines().Num() == 1 && Spool->Lines()[0].bSet);
				// Into the trench, 34 m ahead of where the train comes in.
				Me->TeleportTo(At(-46.0, -30.0, -4.0) + FVector(0, 0, 100), FRotator::ZeroRotator);
				Next(E);
			}
			return false;
		case 2:  // The e 10 train: the set line severed, the player hit and Stalled.
			if (State && !State->bStanding)
			{
				Test->TestTrue(TEXT("the South Hauler is running"), Yard->IsHaulerRunning(0));
				Test->TestTrue(TEXT("the train severed the set line across the trench"), bSevered);
				Test->TestEqual(TEXT("Stalled by the train (a kinematic)"), static_cast<int>(State->StallCause),
				                static_cast<int>(Kinetic::EStallCause::KinematicImpact));
				Test->TestTrue(TEXT("hit as the nose reaches X -46, at e 12.8"), E > 12.4 && E < 13.9);
				Next(E);
			}
			else if (E > 16.0)
			{
				return Fail(*FString::Printf(TEXT("not hit by e 16 (at %s, severed %d)"), *Me->GetActorLocation().ToCompactString(),
				                             bSevered ? 1 : 0));
			}
			return false;
		case 3:  // The planks at X -30 and +10 go on the first pass; then a propane cylinder.
		{
			if (E < 19.0 || !State || !State->bStanding)
			{
				return false;
			}
			int32 Swept = 0, Planks = 0;
			for (TActorIterator<AActor> It(World); It; ++It)
			{
				if (It->ActorHasTag(TEXT("Kinetic.PlankBridge")) && FMath::Abs(It->GetActorLocation().Y - 3000.0) < 400.0)
				{
					++Planks;
					Swept += It->IsHidden() ? 1 : 0;
				}
			}
			Test->TestTrue(TEXT("the south trench's plank bridges are swept"), Planks >= 2 && Swept == Planks);
			for (TActorIterator<AKineticProp> It(World); It; ++It)
			{
				if (It->Volatile == EKineticVolatile::Propane && !Cylinder.IsValid())
				{
					Cylinder = *It;
				}
			}
			if (!Cylinder.IsValid())
			{
				return Fail(TEXT("no propane cylinder in the level"));
			}
			Me->TeleportTo(At(-40.0, -40.0, 0.0) + FVector(0, 0, 100), FRotator::ZeroRotator);
			Cylinder->BreakFree();
			Cylinder->SetActorLocation(At(-38.8, -40.0, 6.0), false, nullptr, ETeleportType::ResetPhysics);
			Cylinder->Mesh->SetPhysicsLinearVelocity(FVector::ZeroVector);
			Blasts = Hazards->BlastsSeen();
			Next(E);
			return false;
		}
		case 4:  // The impact arms it; the blast, 0.3 s on, Stalls the player 1.2 m away.
			if (Hazards->BlastsSeen() > Blasts && State && !State->bStanding)
			{
				Test->TestEqual(TEXT("Stalled by the blast"), static_cast<int>(State->StallCause), static_cast<int>(Kinetic::EStallCause::Blast));
				Test->TestTrue(TEXT("the cylinder is spent"), Cylinder.IsValid() && Cylinder->IsSpent());
				Next(E);
			}
			else if (E - StageAt > 4.0)
			{
				return Fail(*FString::Printf(TEXT("no blast (cylinder at %s)"),
				                             Cylinder.IsValid() ? *Cylinder->GetActorLocation().ToCompactString() : TEXT("-")));
			}
			return false;
		case 5:  // Shift Change at e 69: the plate released over Marker Flat lands as scenery.
			if (E < 69.5)
			{
				return false;
			}
			if (!bChecked69)
			{
				bChecked69 = true;
				Test->TestTrue(TEXT("Shift Change released the Gantry's plate"), Yard->DropsSeen() >= 1 && !Yard->IsCarrying());
			}
			if (E < 76.0)
			{
				return false;
			}
			{
				bool bLanded = false;
				for (TActorIterator<AKineticProp> It(World); It; ++It)
				{
					const FVector P = It->GetActorLocation();
					bLanded |= It->MassKg > 2000.0f && It->Placement == EKineticPlacement::Rated && It->bFixedScenery && P.Z > -100.0 &&
					           P.Z < 600.0 && P.X < 0.0;
				}
				Test->TestTrue(TEXT("the plate lies on Marker Flat as scenery"), bLanded);
				Test->TestTrue(TEXT("the Haulers re-phased: the South train from e 72"), Yard->IsHaulerRunning(0));
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
	void Next(double E)
	{
		++Stage;
		StageAt = E;
	}

	FAutomationTestBase* Test;
	int Stage = 0;
	double StageAt = 0.0;
	bool bFiredA = false, bFiredB = false, bSevered = false, bChecked69 = false;
	int32 Blasts = 0;
	TWeakObjectPtr<AKineticProp> Cylinder;
};

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKineticYardEventsTest, "Kinetic.Yard.Events",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FKineticYardEventsTest::RunTest(const FString& Parameters)
{
	PlayMode(TEXT("Walk"), false);
	if (!AutomationOpenMap(YardMap))
	{
		AddError(TEXT("the Yard grey-box did not open (make yard-greybox)"));
		return false;
	}
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForPlay(this));
	ADD_LATENT_AUTOMATION_COMMAND(FEventsInTheYard(this));
	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
	return true;
}


// A rated load in the walk-the-arena mode: a Breaker Hauls hull section R2 (30 to 42, 19 to 25)
// from across the north trench at 1,000 N. It commits after the 0.6 s hold, groans for 1.0 s,
// slides off its skids into the north trench and lies across it; the e 40 North train takes
// it to the plenum.
class FRatedInTheYard : public IAutomationLatentCommand
{
public:
	explicit FRatedInTheYard(FAutomationTestBase* InTest) : Test(InTest) {}

	virtual bool Update() override
	{
		UWorld* World = GEditor ? GEditor->PlayWorld.Get() : nullptr;
		AKineticCharacter* Me = World ? Cast<AKineticCharacter>(UGameplayStatics::GetPlayerPawn(World, 0)) : nullptr;
		UKineticSpoolWorld* Spool = World ? World->GetSubsystem<UKineticSpoolWorld>() : nullptr;
		AKineticYardDirector* Yard = nullptr;
		for (TActorIterator<AKineticYardDirector> It(World); It; ++It)
		{
			Yard = *It;
		}
		if (!Me || !Spool || !Yard)
		{
			return Fail(TEXT("no player, Spool or Yard director"));
		}
		const double E = Yard->Elapsed();
		if (E > 60.0)
		{
			return Fail(*FString::Printf(TEXT("timed out in stage %d"), Stage));
		}
		const auto At = [](double X, double Y, double Z) { return FVector(X * 100.0, -Y * 100.0, Z * 100.0); };
		if (!Section.IsValid())
		{
			for (TActorIterator<AKineticKinematic> It(World); It; ++It)
			{
				if (It->ActorHasTag(TEXT("Kinetic.Anchor")) && FVector::Dist2D(It->GetActorLocation(), At(36.0, 22.0, 0.0)) < 100.0)
				{
					Section = *It;
				}
			}
			if (!Section.IsValid())
			{
				return Fail(TEXT("no hull section R2"));
			}
		}
		APlayerController* Player = Cast<APlayerController>(Me->GetController());
		switch (Stage)
		{
		case 0:  // A Breaker on the North Dock across the north trench from R2, facing it.
			if (E < 1.0)
			{
				return false;
			}
			Me->Harness = EKineticHarness::Breaker;
			Me->ApplyHarness();
			Me->TeleportTo(At(36.0, 34.5, 0.0) + FVector(0, 0, 100), FRotator(0.0, 90.0, 0.0));
			Spool->SelectMode(Me, Kinetic::ELineMode::Haul);
			Next(E);
			return false;
		case 1:  // Dog A into its north face, Dog B into the Breaker's own chest plate: a Haul.
			if (E - StageAt < 0.6)
			{
				return false;
			}
			if (!bFired)
			{
				Player->SetControlRotation((At(36.0, 24.95, 1.3) - Me->Camera->GetComponentLocation()).Rotation());
				Spool->FireDog(Me);
				bFired = true;
				return false;
			}
			if (E - StageAt > 1.0 && !bSelf)
			{
				Spool->SelfDogNext(Me);
				bSelf = true;
				return false;
			}
			if (E - StageAt > 1.4)
			{
				const FKineticLine* L = Spool->ActiveLine(Me);
				Test->TestTrue(TEXT("a Haul from the section to the Breaker"), L && L->IsLive() && L->State.Mode == Kinetic::ELineMode::Haul &&
				                                                                    L->A.Seat == EKineticSeat::Kinematic);
				Next(E);
			}
			return false;
		case 2:  // It commits, groans and goes: across the trench within 0.6 + 1.0 + 5 s.
		{
			const FVector P = Section->GetActorLocation();
			if (E - StageAt < 7.5)
			{
				return false;
			}
			Spool->Release(Me);
			Test->TestEqual(TEXT("it lies across the north trench"), P.Y / -100.0, 30.0, 0.5);
			Test->TestEqual(TEXT("turned across it"), FMath::Abs(Section->GetActorRotation().Yaw), 90.0, 1.0);
			Next(E);
			return false;
		}
		case 3:  // The North train from e 40 carries it to the plenum.
			if (E < 56.0)
			{
				return false;
			}
			Test->TestFalse(TEXT("carried off by the train"), Section->IsActive());
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
	void Next(double E)
	{
		++Stage;
		StageAt = E;
	}

	FAutomationTestBase* Test;
	int Stage = 0;
	double StageAt = 0.0;
	bool bFired = false, bSelf = false;
	TWeakObjectPtr<AKineticKinematic> Section;
};

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKineticYardRatedTest, "Kinetic.Yard.Rated",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FKineticYardRatedTest::RunTest(const FString& Parameters)
{
	PlayMode(TEXT("Walk"), false);
	if (!AutomationOpenMap(YardMap))
	{
		AddError(TEXT("the Yard grey-box did not open (make yard-greybox)"));
		return false;
	}
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForPlay(this));
	ADD_LATENT_AUTOMATION_COMMAND(FRatedInTheYard(this));
	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
	return true;
}


// The generated figure on the player (art/characters, make import-characters): it loads for the
// class, faces the actor's forward with its left side on the actor's left, and plays the take
// for what the character is doing: Idle standing, Run moving, Fall in the air, Brace braced.
class FFigureInTheYard : public IAutomationLatentCommand
{
public:
	explicit FFigureInTheYard(FAutomationTestBase* InTest) : Test(InTest) {}

	virtual bool Update() override
	{
		UWorld* World = GEditor ? GEditor->PlayWorld.Get() : nullptr;
		AKineticCharacter* Me = World ? Cast<AKineticCharacter>(UGameplayStatics::GetPlayerPawn(World, 0)) : nullptr;
		UKineticSpoolWorld* Spool = World ? World->GetSubsystem<UKineticSpoolWorld>() : nullptr;
		if (!Me || !Spool)
		{
			return Fail(TEXT("no player"));
		}
		const double Now = World->GetTimeSeconds();
		if (Now > 20.0)
		{
			return Fail(*FString::Printf(TEXT("timed out in stage %d"), Stage));
		}
		using ETake = AKineticCharacter::EFigureTake;
		switch (Stage)
		{
		case 0:
			if (Now < 1.5)
			{
				return false;
			}
			if (!Me->HasFigure())
			{
				return Fail(TEXT("no figure for the class (make import-characters)"));
			}
			{
				// In the actor's frame: toes ahead of the heel, the left leg on the left (-Y).
				const FTransform Actor = Me->GetActorTransform();
				const FVector Toe = Actor.InverseTransformPosition(Me->GetMesh()->GetBoneLocation(TEXT("ball_l")));
				const FVector Heel = Actor.InverseTransformPosition(Me->GetMesh()->GetBoneLocation(TEXT("foot_l")));
				const FVector Left = Actor.InverseTransformPosition(Me->GetMesh()->GetBoneLocation(TEXT("thigh_l")));
				Test->TestTrue(TEXT("the figure faces forward"), Toe.X > Heel.X);
				Test->TestTrue(TEXT("its left leg is on the left"), Left.Y < 0.0);
				Test->TestTrue(TEXT("its feet are at the capsule's foot"), FMath::Abs(Heel.Z + 95.0) < 15.0);
			}
			Test->TestEqual(TEXT("idle standing"), static_cast<int>(Me->FigureTake()), static_cast<int>(ETake::Idle));
			Next(Now);
			return false;
		case 1:  // Running.
			Cast<APlayerController>(Me->GetController())->SetControlRotation(FRotator(0.0, 180.0, 0.0));
			Me->Move(FVector2D(0.0, 1.0));
			if (Now - StageAt < 0.8)
			{
				return false;
			}
			Test->TestEqual(TEXT("run while moving"), static_cast<int>(Me->FigureTake()), static_cast<int>(ETake::Run));
			Me->Jump();
			Next(Now);
			return false;
		case 2:  // In the air.
			if (Now - StageAt < 0.2)
			{
				return false;
			}
			Test->TestEqual(TEXT("fall in the air"), static_cast<int>(Me->FigureTake()), static_cast<int>(ETake::Fall));
			Me->StopJumping();
			Next(Now);
			return false;
		case 3:  // Braced (a Rigger braces).
			if (Now - StageAt < 1.5)
			{
				return false;
			}
			Spool->SetBrace(Me, true);
			Me->Crouch();
			if (Now - StageAt < 2.2)
			{
				return false;
			}
			Test->TestEqual(TEXT("brace when braced"), static_cast<int>(Me->FigureTake()), static_cast<int>(ETake::Brace));
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
	void Next(double Now)
	{
		++Stage;
		StageAt = Now;
	}

	FAutomationTestBase* Test;
	int Stage = 0;
	double StageAt = 0.0;
};

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKineticYardFigureTest, "Kinetic.Yard.Figure",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FKineticYardFigureTest::RunTest(const FString& Parameters)
{
	PlayMode(TEXT("Walk"), false);
	if (!AutomationOpenMap(YardMap))
	{
		AddError(TEXT("the Yard grey-box did not open (make yard-greybox)"));
		return false;
	}
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForPlay(this));
	ADD_LATENT_AUTOMATION_COMMAND(FFigureInTheYard(this));
	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
	return true;
}


// Views of the Yard for a look at it (not a check): with rendering on (an editor run without
// -nullrhi), a Scrap match of six bots is held at its reset while a camera visits the crews at
// their spawns, West Mesa from the Haul Lane, and Marker Flat under the Gantry; each view is
// saved to Saved/Kinetic/Shots/. With no rendering it does nothing.
class FShotsOfTheYard : public IAutomationLatentCommand
{
public:
	explicit FShotsOfTheYard(FAutomationTestBase* InTest) : Test(InTest) {}

	virtual bool Update() override
	{
		UWorld* World = GEditor ? GEditor->PlayWorld.Get() : nullptr;
		APlayerController* Player = World ? World->GetFirstPlayerController() : nullptr;
		if (!World || !Player)
		{
			return Next < 0;
		}
		const double Now = World->GetTimeSeconds();
		struct FView
		{
			const TCHAR* Name;
			FVector From, To;  // Layout metres
		};
		static const FView Views[] = {
		    {TEXT("Crew_WestMesa"), FVector(-62.0, -36.5, 10.0), FVector(-68.0, -39.0, 9.0)},
		    {TEXT("Crew_EastMesa"), FVector(62.0, 36.5, 10.0), FVector(68.0, 39.0, 9.0)},
		    {TEXT("WestMesa_HaulLane"), FVector(-45.0, -20.0, 3.0), FVector(-68.0, -39.0, 6.0)},
		    {TEXT("MarkerFlat_Gantry"), FVector(-30.0, 2.0, 12.0), FVector(10.0, 14.0, 6.0)},
		    {TEXT("Trench_South"), FVector(-70.0, -27.5, 3.0), FVector(0.0, -30.0, -2.0)},
		    {TEXT("Press_Pit"), FVector(30.0, -12.0, 8.0), FVector(45.0, 0.0, -4.0)},
		    {TEXT("Sky_FromTheFlat"), FVector(-20.0, 0.0, 2.0), FVector(40.0, 30.0, 30.0)},
		    {TEXT("Port_FromEastMesa"), FVector(70.0, 36.0, 10.0), FVector(20.0, 200.0, 5.0)},
		    {TEXT("Port_ZoomedOut"), FVector(-220.0, -260.0, 140.0), FVector(0.0, 60.0, 0.0)},
		};
		const auto At = [](const FVector& M) { return FVector(M.X * 100.0, -M.Y * 100.0, M.Z * 100.0); };
		if (Now < 2.0)
		{
			return false;
		}
		if (const AKineticGameMode* Mode = World->GetAuthGameMode<AKineticGameMode>(); Mode && Mode->Scrap() && !bHeld)
		{
			Kinetic::FScrapRules Rules;
			Rules.ResetSeconds = 600.0;  // Hold the crews at their spawns for the pictures
			Mode->Scrap()->SetRules(Rules);
			Mode->Scrap()->StartMatch();
			bHeld = true;
		}
		if (Next >= static_cast<int32>(UE_ARRAY_COUNT(Views)))
		{
			return Lineup(World, Player, Now, At);
		}
		if (Now - LastAt < 2.0)
		{
			return false;  // Let the frame settle, and the last shot land
		}
		const FView& V = Views[Next];
		if (!Camera)
		{
			Camera = World->SpawnActor<ACameraActor>();
		}
		Camera->SetActorLocationAndRotation(At(V.From), (At(V.To) - At(V.From)).Rotation());
		Camera->GetCameraComponent()->SetFieldOfView(70.0f);
		Camera->GetCameraComponent()->SetConstraintAspectRatio(false);  // Fill the frame
		Player->SetViewTarget(Camera);
		if (bAimed)
		{
			const FString File = FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir() / TEXT("Kinetic/Shots") / V.Name) + TEXT(".png");
			FScreenshotRequest::RequestScreenshot(File, false, false);
			Test->AddInfo(FString::Printf(TEXT("shot %s"), *File));
			if (Next == 0)
			{
				for (TActorIterator<AKineticCharacter> It(World); It; ++It)
				{
					const FVector P = It->GetActorLocation() / 100.0;
					Test->AddInfo(FString::Printf(TEXT("rigger %s crew %d at (%.1f, %.1f, %.1f) hidden %d mesh %d"), *It->GetName(), It->Crew,
					                              P.X, -P.Y, P.Z, It->IsHidden() ? 1 : 0, It->GetMesh()->IsVisible() ? 1 : 0));
					TArray<USkeletalMeshComponent*> Parts;
					It->GetComponents<USkeletalMeshComponent>(Parts);
					for (const USkeletalMeshComponent* Part : Parts)
					{
						const FVector Head = Part->GetBoneIndex(TEXT("head")) != INDEX_NONE ? Part->GetBoneLocation(TEXT("head")) / 100.0 : FVector::ZeroVector;
						Test->AddInfo(FString::Printf(TEXT("  part %s mesh %s visible %d registered %d bounds %s head (%.1f, %.1f, %.1f)"), *Part->GetName(),
						                              Part->GetSkeletalMeshAsset() ? *Part->GetSkeletalMeshAsset()->GetName() : TEXT("none"),
						                              Part->IsVisible() ? 1 : 0, Part->IsRegistered() ? 1 : 0, *Part->Bounds.ToString(), Head.X, -Head.Y, Head.Z));
					}
					TArray<UGroomComponent*> Hair;
					It->GetComponents<UGroomComponent>(Hair);
					for (const UGroomComponent* Groom : Hair)
					{
						Test->AddInfo(FString::Printf(TEXT("  groom %s asset %s binding %s lods %d visible %d registered %d attach %s bounds %s"), *Groom->GetName(),
						                              Groom->GroomAsset ? *Groom->GroomAsset->GetName() : TEXT("none"),
						                              Groom->BindingAsset ? *Groom->BindingAsset->GetName() : TEXT("none"), Groom->GetNumLODs(),
						                              Groom->IsVisible() ? 1 : 0, Groom->IsRegistered() ? 1 : 0,
						                              Groom->GetAttachParent() ? *Groom->GetAttachParent()->GetName() : TEXT("none"), *Groom->Bounds.ToString()));
					}
				}
			}
			++Next;
			bAimed = false;
		}
		else
		{
			bAimed = true;
		}
		LastAt = Now;
		return false;
	}

private:
	// The five classes side by side on West Mesa, clear of Crew A, facing the camera, crews
	// alternating, in one landscape picture (Lineup.png).
	template <typename FAt>
	bool Lineup(UWorld* World, APlayerController* Player, double Now, const FAt& At)
	{
		static const EKineticHarness Classes[] = {EKineticHarness::Rigger, EKineticHarness::Breaker, EKineticHarness::Climber,
		                                          EKineticHarness::Doubler, EKineticHarness::Scrapper};
		if (LineupStage == 0)
		{
			for (int32 I = 0; I < 5; ++I)
			{
				const FTransform Where(FRotator(0.0, 90.0, 0.0), At(FVector(-78.0 + 2.0 * I, -35.0, 9.0)));  // Facing layout south
				AKineticCharacter* Who = World->SpawnActorDeferred<AKineticCharacter>(AKineticCharacter::StaticClass(), Where, nullptr, nullptr,
				                                                                      ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
				Who->Harness = Classes[I];
				Who->FinishSpawning(Where);
				Who->SetCrew(I % 2);
			}
			LineupStage = 1;
			LastAt = Now;
			return false;
		}
		if (Now - LastAt < 3.0)
		{
			return false;  // Land, settle into Idle
		}
		if (LineupStage == 1)
		{
			Camera->SetActorLocationAndRotation(At(FVector(-74.0, -44.5, 9.4)), (At(FVector(-74.0, -35.0, 9.0)) - At(FVector(-74.0, -44.5, 9.4))).Rotation());
			Camera->GetCameraComponent()->SetFieldOfView(55.0f);
			Player->SetViewTarget(Camera);
			LineupStage = 2;
			LastAt = Now - 1.0;
			return false;
		}
		if (LineupStage == 2)
		{
			const FString File = FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir() / TEXT("Kinetic/Shots/Lineup.png"));
			Player->ConsoleCommand(FString::Printf(TEXT("HighResShot 2400x1350 filename=\"%s\""), *File));  // The game's view, not the editor's
			Test->AddInfo(FString::Printf(TEXT("shot %s"), *File));
			LineupStage = 3;
			LastAt = Now;
			return false;
		}
		if (LineupStage == 3)
		{
			// A close look at the Breaker's hands (the takes carry no fingers: the retarget curls them).
			Camera->SetActorLocationAndRotation(At(FVector(-76.0, -37.6, 9.05)), (At(FVector(-76.0, -35.0, 8.85)) - At(FVector(-76.0, -37.6, 9.05))).Rotation());
			Camera->GetCameraComponent()->SetFieldOfView(40.0f);
			LineupStage = 4;
			LastAt = Now - 1.0;
			return false;
		}
		if (LineupStage == 4)
		{
			const FString File = FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir() / TEXT("Kinetic/Shots/Lineup_Hands.png"));
			Player->ConsoleCommand(FString::Printf(TEXT("HighResShot 2400x1350 filename=\"%s\""), *File));
			Test->AddInfo(FString::Printf(TEXT("shot %s"), *File));
			LineupStage = 5;
			LastAt = Now;
			return false;
		}
		if (LineupStage == 5)
		{
			// And its head: the face, the hair and the collar.
			Camera->SetActorLocationAndRotation(At(FVector(-75.6, -36.4, 9.75)), (At(FVector(-76.0, -35.0, 9.62)) - At(FVector(-75.6, -36.4, 9.75))).Rotation());
			Camera->GetCameraComponent()->SetFieldOfView(35.0f);
			LineupStage = 6;
			LastAt = Now - 1.0;
			return false;
		}
		if (LineupStage == 6)
		{
			const FString File = FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir() / TEXT("Kinetic/Shots/Lineup_Head.png"));
			Player->ConsoleCommand(FString::Printf(TEXT("HighResShot 2400x1350 filename=\"%s\""), *File));
			Test->AddInfo(FString::Printf(TEXT("shot %s"), *File));
			LineupStage = 7;
			LastAt = Now;
			return false;
		}
		return Now - LastAt > 3.0;
	}

	FAutomationTestBase* Test;
	int32 Next = 0;
	int32 LineupStage = 0;
	double LastAt = 0.0;
	bool bHeld = false, bAimed = false;
	ACameraActor* Camera = nullptr;
};

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKineticYardShotsTest, "Kinetic.Shots.Yard",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FKineticYardShotsTest::RunTest(const FString& Parameters)
{
	if (!FApp::CanEverRender())
	{
		AddInfo(TEXT("no rendering (-nullrhi): no shots"));
		return true;
	}
	PlayMode(TEXT("Scrap"), true);
	if (!AutomationOpenMap(YardMap))
	{
		AddError(TEXT("the Yard grey-box did not open (make yard-greybox)"));
		return false;
	}
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForPlay(this));
	ADD_LATENT_AUTOMATION_COMMAND(FShotsOfTheYard(this));
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
