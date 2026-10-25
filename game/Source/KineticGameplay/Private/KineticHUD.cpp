#include "KineticHUD.h"

#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "KineticCharacter.h"
#include "KineticSpoolWorld.h"

void AKineticHUD::DrawHUD()
{
	Super::DrawHUD();
	if (!Canvas)
	{
		return;
	}
	const float X = static_cast<float>(Canvas->ClipX) * 0.5f;
	const float Y = static_cast<float>(Canvas->ClipY) * 0.5f;
	const FLinearColor Mark(0.95f, 0.85f, 0.55f, 0.9f);
	DrawLine(X - 8.0f, Y, X - 3.0f, Y, Mark, 1.5f);
	DrawLine(X + 3.0f, Y, X + 8.0f, Y, Mark, 1.5f);
	DrawLine(X, Y - 8.0f, X, Y - 3.0f, Mark, 1.5f);
	DrawLine(X, Y + 3.0f, X, Y + 8.0f, Mark, 1.5f);
	const AKineticCharacter* Character = Cast<AKineticCharacter>(GetOwningPawn());
	if (!Character)
	{
		return;
	}
	const UKineticSpoolWorld* Spool = GetWorld()->GetSubsystem<UKineticSpoolWorld>();
	const FKineticSpoolPlayer* P = Spool ? Spool->PlayerState(Character) : nullptr;
	const FVector Velocity = Character->GetCharacterMovement()->Velocity;
	const double Speed = FVector(Velocity.X, Velocity.Y, 0.0).Size() / 100.0;
	const TCHAR* State = Character->IsClimbing() ? TEXT("climbing")
	                     : Character->GetCharacterMovement()->IsFalling() ? TEXT("airborne")
	                     : Character->IsUnderLine() ? TEXT("dragged")
	                     : P && P->bBraced ? TEXT("braced")
	                     : Character->IsSprinting() ? TEXT("sprinting")
	                                                : TEXT("on foot");
	const auto ModeName = [](Kinetic::ELineMode Mode) {
		switch (Mode)
		{
		case Kinetic::ELineMode::Haul: return TEXT("HAUL");
		case Kinetic::ELineMode::Jack: return TEXT("JACK");
		case Kinetic::ELineMode::Hang: return TEXT("HANG");
		case Kinetic::ELineMode::Set: return TEXT("SET");
		default: return TEXT("slack");
		}
	};
	FString Spooled = P ? FString::Printf(TEXT("Spool: %s"), ModeName(P->Preselect)) : FString();
	if (Spool)
	{
		for (const FKineticLine& L : Spool->Lines())
		{
			if (L.Owner.Get() == Character && !L.bSet)
			{
				Spooled += L.IsLive() ? FString::Printf(TEXT("   line %s %.0f N"), ModeName(L.State.Mode), L.State.AppliedForce)
				                      : FString(TEXT("   Dog out"));
			}
		}
	}
	const FLinearColor Text(0.9f, 0.9f, 0.85f);
	const float Bottom = static_cast<float>(Canvas->ClipY);
	DrawText(FString::Printf(TEXT("%s   %.1f m/s   %s"), *KineticHarness::Name(Character->Harness), Speed, State), Text, 24.0f,
	         Bottom - 48.0f, GEngine->GetMediumFont());
	DrawText(Spooled, Text, 24.0f, Bottom - 76.0f, GEngine->GetMediumFont());
	// A hit, for a moment; a Stall until the Hooks bring them back.
	if (P && Spool)
	{
		const double Now = Spool->Now();
		FString Banner;
		if (!P->bStanding)
		{
			const TCHAR* Causes[] = {TEXT("an object"), TEXT("the world"), TEXT("a moving part"), TEXT("a player"),
			                         TEXT("a crush"), TEXT("a blast"), TEXT("a ring-out")};
			Banner = FString::Printf(TEXT("STALLED by %s   back in %.0f s"), Causes[static_cast<int>(P->StallCause)],
			                         FMath::Max(0.0, UKineticSpoolWorld::RespawnSeconds - (Now - P->StalledAt)));
		}
		else if (Now - P->LastOutcomeAt < 1.0 && P->LastOutcome != Kinetic::EOutcome::None)
		{
			Banner = P->LastOutcome == Kinetic::EOutcome::Flip ? TEXT("FLIPPED") : TEXT("STAGGERED");
		}
		if (!Banner.IsEmpty())
		{
			DrawText(Banner, FLinearColor(1.0f, 0.55f, 0.2f), X - 120.0f, Y + 60.0f, GEngine->GetLargeFont());
		}
	}
}
