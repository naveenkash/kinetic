#include "KineticHUD.h"

#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "KineticCharacter.h"
#include "KineticGameMode.h"
#include "KineticSpoolWorld.h"

namespace
{
const TCHAR* PhaseName(Kinetic::EScrapPhase Phase)
{
	switch (Phase)
	{
	case Kinetic::EScrapPhase::Reset: return TEXT("RESET");
	case Kinetic::EScrapPhase::Drop: return TEXT("DROP");
	case Kinetic::EScrapPhase::Live: return TEXT("LIVE");
	case Kinetic::EScrapPhase::SuddenDeath: return TEXT("SUDDEN DEATH");
	case Kinetic::EScrapPhase::RoundEnd: return TEXT("ROUND OVER");
	case Kinetic::EScrapPhase::MatchOver: return TEXT("MATCH OVER");
	}
	return TEXT("");
}
}  // namespace

void AKineticHUD::DrawMatch()
{
	// The match: round, score, clock and phase across the top; the Marshal's calls and the
	// Stalls down the left.
	const AKineticGameMode* GameMode = GetWorld()->GetAuthGameMode<AKineticGameMode>();
	const FKineticScrap* Scrap = GameMode ? GameMode->Scrap() : nullptr;
	if (!Scrap)
	{
		return;
	}
	const Kinetic::FScrapMatch& M = Scrap->Match();
	const int32 Mine = Scrap->HumanSlot() == INDEX_NONE ? 0 : Scrap->TeamOf(Scrap->HumanSlot());
	const double Clock = FMath::Max(0.0, M.ClockRemaining());
	const int32 Minutes = static_cast<int32>(Clock) / 60;
	const int32 Seconds = static_cast<int32>(Clock) % 60;
	const FString Top = FString::Printf(TEXT("Round %d    %s %d  -  %d %s    %d:%02d    %s"), M.Round() + 1,
	                                    Scrap->HumanSlot() == INDEX_NONE ? TEXT("Crew A") : TEXT("Your crew"), M.Wins(Mine),
	                                    M.Wins(1 - Mine), Scrap->HumanSlot() == INDEX_NONE ? TEXT("Crew B") : TEXT("Their crew"),
	                                    Minutes, Seconds, PhaseName(M.Phase()));
	const FLinearColor Text(0.95f, 0.92f, 0.85f);
	float W = 0.0f, H = 0.0f;
	GetTextSize(Top, W, H, GEngine->GetMediumFont());
	DrawText(Top, Text, (static_cast<float>(Canvas->ClipX) - W) * 0.5f, 16.0f, GEngine->GetMediumFont());
	const FString Standing = FString::Printf(TEXT("Standing  %d  -  %d"), M.StandingCount(Mine), M.StandingCount(1 - Mine));
	GetTextSize(Standing, W, H, GEngine->GetSmallFont());
	DrawText(Standing, Text, (static_cast<float>(Canvas->ClipX) - W) * 0.5f, 42.0f, GEngine->GetSmallFont());
	float Y = 80.0f;
	for (const FString& Line : Scrap->Feed())
	{
		DrawText(Line, FLinearColor(0.85f, 0.85f, 0.8f), 24.0f, Y, GEngine->GetSmallFont());
		Y += 18.0f;
	}
}

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
	DrawMatch();
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
			const AKineticGameMode* GameMode = GetWorld()->GetAuthGameMode<AKineticGameMode>();
			Banner = GameMode && GameMode->Scrap()
			             ? FString::Printf(TEXT("STALLED by %s   out until the reset"), Causes[static_cast<int>(P->StallCause)])
			             : FString::Printf(TEXT("STALLED by %s   back in %.0f s"), Causes[static_cast<int>(P->StallCause)],
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
