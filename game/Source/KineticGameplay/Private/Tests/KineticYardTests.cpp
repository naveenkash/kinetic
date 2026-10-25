// Plays the Yard grey-box in the editor: the character spawns on West Mesa (Spawn A), stands,
// runs at its class's run speed and jumps its class's jump height (bible/05 section 2.2).
// Needs the level built (make yard-greybox) and the props imported (make import-props).
#include "Engine/Engine.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "KineticCharacter.h"
#include "KineticRulesSubsystem.h"
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
}  // namespace

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
