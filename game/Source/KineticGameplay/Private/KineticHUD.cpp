#include "KineticHUD.h"

#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "KineticCharacter.h"

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
	const FVector Velocity = Character->GetCharacterMovement()->Velocity;
	const double Speed = FVector(Velocity.X, Velocity.Y, 0.0).Size() / 100.0;
	const TCHAR* State = Character->IsClimbing() ? TEXT("climbing")
	                     : Character->GetCharacterMovement()->IsFalling() ? TEXT("airborne")
	                     : Character->IsSprinting() ? TEXT("sprinting")
	                                                : TEXT("on foot");
	const FString Line = FString::Printf(TEXT("%s   %.1f m/s   %s"), *KineticHarness::Name(Character->Harness), Speed, State);
	DrawText(Line, FLinearColor(0.9f, 0.9f, 0.85f), 24.0f, static_cast<float>(Canvas->ClipY) - 48.0f, GEngine->GetMediumFont());
}
