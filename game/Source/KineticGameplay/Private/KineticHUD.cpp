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

void AKineticHUD::DrawTagOut()
{
	const AKineticGameMode* GameMode = GetWorld()->GetAuthGameMode<AKineticGameMode>();
	const FKineticTagOut* Tag = GameMode ? GameMode->TagOut() : nullptr;
	if (!Tag)
	{
		return;
	}
	using namespace Kinetic;
	const FTagOutMatch& M = Tag->Match();
	const FTagOutRules& Rules = M.Rules();
	const int32 Me = Tag->ViewSlot();
	const int32 Mine = Tag->TeamOf(Me);
	const FTagRigger& R = M.Rigger(Me);
	const double Now = M.Time();
	const float CX = static_cast<float>(Canvas->ClipX) * 0.5f;
	const float CY = static_cast<float>(Canvas->ClipY) * 0.5f;
	const float Bottom = static_cast<float>(Canvas->ClipY);
	const FLinearColor Text(0.95f, 0.92f, 0.85f);
	const FLinearColor Amber(1.0f, 0.75f, 0.25f);
	const FLinearColor Warn(1.0f, 0.35f, 0.2f);
	float W = 0.0f, H = 0.0f;
	const auto Centred = [&](const FString& Line, float Y, UFont* Font, const FLinearColor& Colour) {
		GetTextSize(Line, W, H, Font);
		DrawText(Line, Colour, CX - W * 0.5f, Y, Font);
	};
	const auto Pips = [](const FTagRead& Read) {
		FString Out;
		for (int32 I = 0; I < PlateCount; ++I)
		{
			Out += I > 0 ? TEXT(" · ") : TEXT("");
			Out += Read.Pips[I] ? FString::FromInt(*Read.Pips[I]) : FString(TEXT("?"));
		}
		return Out;
	};

	// The clock and the beacons across the top.
	const double Clock = M.Remaining();
	const TCHAR* Phase = M.Phase() == ETagPhase::Drop       ? TEXT("DROP")
	                     : M.Phase() == ETagPhase::Live     ? TEXT("LIVE")
	                     : M.Phase() == ETagPhase::Overtime ? TEXT("OVERTIME")
	                                                        : TEXT("MATCH OVER");
	const double Shown = M.Phase() == ETagPhase::Drop ? Rules.DropSeconds - Now : Clock;
	Centred(FString::Printf(TEXT("TAG-OUT    %d:%02d    %s"), static_cast<int32>(Shown) / 60, static_cast<int32>(Shown) % 60, Phase), 16.0f,
	        GEngine->GetMediumFont(), Text);
	const auto BeaconLine = [&](int32 Team) {
		const FTagBeacon& B = M.Beacon(Team);
		const FString Whose = Team == Mine ? TEXT("Your beacon") : TEXT("Their beacon");
		switch (B.State)
		{
		case EBeaconState::OnStand: return Whose + TEXT(": home");
		case EBeaconState::Carried: return Whose + TEXT(": carried by ") + Tag->Who(B.Carrier);
		case EBeaconState::Dropped:
		default:
			return Whose + FString::Printf(TEXT(": down, home in %.0f s"), FMath::Max(0.0, Rules.AutoReturnSeconds - (Now - B.DroppedAt)));
		}
	};
	Centred(BeaconLine(Mine) + TEXT("      ") + BeaconLine(1 - Mine), 42.0f, GEngine->GetSmallFont(), Text);
	float Y = 80.0f;
	for (const FString& Line : Tag->Feed())
	{
		DrawText(Line, FLinearColor(0.85f, 0.85f, 0.8f), 24.0f, Y, GEngine->GetSmallFont());
		Y += 18.0f;
	}

	// Over each enemy in sight: what the crew knows of their code, and how old it is. A caller who
	// made a bad call on this crew shows through walls for 3 s.
	APlayerController* PC = GetOwningPlayerController();
	for (int32 Slot = 0; Slot < Tag->SlotCount(); ++Slot)
	{
		if (Tag->TeamOf(Slot) == Mine || !M.Rigger(Slot).bStanding)
		{
			continue;
		}
		const bool bExposed = M.Exposed(Slot);
		if (!Tag->Sight(Slot).bVisible && !bExposed)
		{
			continue;
		}
		FVector2D Screen;
		if (!PC || !PC->ProjectWorldLocationToScreen(Tag->ChestCm(Slot) + FVector(0.0, 0.0, 70.0), Screen))
		{
			continue;
		}
		const FTagRead& Read = M.Read(Mine, Slot);
		double Newest = -1.0;
		for (int32 I = 0; I < PlateCount; ++I)
		{
			Newest = Read.Pips[I] ? FMath::Max(Newest, Read.Seen[I]) : Newest;
		}
		FString Label = Read.bStale ? FString(TEXT("? · ? · ?")) : Pips(Read);
		if (!Read.bStale && Newest >= 0.0)
		{
			Label += FString::Printf(TEXT("  %.0fs"), Now - Newest);
		}
		if (bExposed)
		{
			Label = TEXT("BAD CALL  ") + Label;
		}
		GetTextSize(Label, W, H, GEngine->GetSmallFont());
		DrawText(Label, Read.bStale ? FLinearColor(0.5f, 0.5f, 0.5f) : bExposed ? Warn : Amber, static_cast<float>(Screen.X) - W * 0.5f,
		         static_cast<float>(Screen.Y) - H, GEngine->GetSmallFont());
	}

	// Being called: the tick, and an edge marker toward the caller.
	const int32 Caller = M.CalledBy(Me);
	if (Caller != NoPlayer)
	{
		const APawn* Pawn = GetOwningPawn();
		const FVector ViewDir = PC ? PC->GetControlRotation().Vector() : FVector::ForwardVector;
		const FVector To = (Tag->ChestCm(Caller) - (Pawn ? Pawn->GetActorLocation() : FVector::ZeroVector)).GetSafeNormal2D();
		const double Angle = FMath::Atan2(FVector::CrossProduct(ViewDir.GetSafeNormal2D(), To).Z, FVector::DotProduct(ViewDir.GetSafeNormal2D(), To));
		const float MX = CX + static_cast<float>(FMath::Sin(Angle)) * 220.0f;
		const float MY = CY - static_cast<float>(FMath::Cos(Angle)) * 220.0f;
		DrawText(TEXT("<>"), Warn, MX - 10.0f, MY - 10.0f, GEngine->GetLargeFont());
		Centred(TEXT("BEING CALLED"), CY - 120.0f, GEngine->GetLargeFont(), Warn);
	}

	// The call strip, the lock, the cooldown and any hold, above the class line.
	FString Strip = TEXT("CALL  ");
	for (int32 I = 0; I < PlateCount; ++I)
	{
		Strip += I > 0 ? TEXT(" · ") : TEXT("");
		Strip += I < static_cast<int32>(R.Entered.size()) ? FString::FromInt(R.Entered[I]) : FString(TEXT("_"));
	}
	if (R.Call == ECallState::Locking)
	{
		Strip += FString::Printf(TEXT("    LOCKING %s  %3.0f%%"), *Tag->Who(R.CallTarget), 100.0 * R.LockElapsed / Rules.LockSeconds);
	}
	else if (Now < R.CallCooldownUntil)
	{
		Strip += FString::Printf(TEXT("    cooldown %.1f s"), R.CallCooldownUntil - Now);
	}
	else if (R.Entered.size() == PlateCount)
	{
		Strip += TEXT("    F to send at the crosshair");
	}
	Centred(Strip, Bottom - 132.0f, GEngine->GetMediumFont(), Tag->StripOpen(Me) || R.Call == ECallState::Locking ? Amber : Text);
	FString Hold;
	const int32 Use = R.UseOn;
	if (Use == FTagOutMatch::UsePlant)
	{
		Hold = FString::Printf(TEXT("PLANTING %3.0f%%"), 100.0 * R.UseHeld / Rules.PlantSeconds);
	}
	else if (Use >= FTagOutMatch::UseLocker && Use < FTagOutMatch::UsePlant)
	{
		Hold = FString::Printf(TEXT("RE-STAMPING %3.0f%%  hold still"), 100.0 * R.UseHeld / Rules.RestampSeconds);
	}
	else if (Use == FTagOutMatch::UseBeacon + Mine)
	{
		Hold = FString::Printf(TEXT("RETURNING %3.0f%%"), 100.0 * R.UseHeld / Rules.ReturnSeconds);
	}
	else if (Use == FTagOutMatch::UseBeacon + 1 - Mine)
	{
		Hold = FString::Printf(TEXT("GRABBING %3.0f%%"), 100.0 * R.UseHeld / Rules.GrabSeconds);
	}
	else if (Tag->ThrowWindup(Me) >= 0.0)
	{
		Hold = FString::Printf(TEXT("THROWING in %.1f s"), Tag->ThrowWindup(Me));
	}
	else if (M.Carrying(Me) != NoPlayer)
	{
		Hold = TEXT("CARRYING: hold E on your pad to plant, G to throw");
	}
	if (!Hold.IsEmpty())
	{
		Centred(Hold, CY + 90.0f, GEngine->GetMediumFont(), Amber);
	}
	FString Own = FString::Printf(TEXT("Your code  %d · %d · %d"), R.Code.Pips[0], R.Code.Pips[1], R.Code.Pips[2]);
	const int32 Leaked = M.Read(1 - Mine, Me).Known();
	if (Leaked > 0)
	{
		Own += FString::Printf(TEXT("   (a bad call gave them %d)"), Leaked);
	}
	if (M.LevelReady(Me))
	{
		Own += TEXT("   LEVEL");
	}
	DrawText(Own, Text, 24.0f, Bottom - 104.0f, GEngine->GetMediumFont());
	if (!R.bStanding)
	{
		Centred(FString::Printf(TEXT("LOCKED OUT   back in %.0f s"), FMath::Max(0.0, R.ReserveUntil - Now)), CY + 60.0f, GEngine->GetLargeFont(),
		        Warn);
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
	DrawTagOut();
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
	const AKineticGameMode* Mode = GetWorld()->GetAuthGameMode<AKineticGameMode>();
	if (Mode && Mode->TagOut())
	{
		return;  // No Spool, and the Tag-Out panel has the rest
	}
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
