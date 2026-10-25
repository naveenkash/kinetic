// Plays the Yard grey-box in the editor: the character spawns on West Mesa (Spawn A), stands,
// runs at its class's run speed and jumps its class's jump height (bible/05 section 2.2).
// Needs the level built (make yard-greybox) and the props imported (make import-props).
#include "Camera/CameraComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"
#include "KineticCharacter.h"
#include "KineticProp.h"
#include "KineticRulesSubsystem.h"
#include "KineticSpoolWorld.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"

#if WITH_EDITOR && WITH_DEV_AUTOMATION_TESTS

#include "Editor.h"
#include "Tests/AutomationEditorCommon.h"

namespace
{
const TCHAR* YardMap = TEXT("/Game/Arenas/YRD_Yard/L_YRD_Greybox");

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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKineticYardSpoolTest, "Kinetic.Yard.Spool",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FKineticYardSpoolTest::RunTest(const FString& Parameters)
{
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
