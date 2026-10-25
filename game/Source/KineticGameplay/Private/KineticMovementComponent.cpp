#include "KineticMovementComponent.h"

#include "Components/CapsuleComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kinetic/Tables.h"
#include "EngineUtils.h"
#include "KineticCharacter.h"
#include "KineticLadder.h"
#include "KineticRulesSubsystem.h"

void UKineticMovementComponent::HandleImpact(const FHitResult& Hit, float TimeSlice, const FVector& MoveDelta)
{
	if (AKineticCharacter* Character = Cast<AKineticCharacter>(CharacterOwner))
	{
		Character->RecordContact(Hit, Velocity, FVector::ZeroVector);
	}
	Super::HandleImpact(Hit, TimeSlice, MoveDelta);
}

bool UKineticMovementComponent::CanWalkOffLedges() const
{
	const AKineticCharacter* Character = Cast<AKineticCharacter>(CharacterOwner);
	if (!Super::CanWalkOffLedges() || !Character || !Character->WalksOwnWay() || !UpdatedComponent)
	{
		return Super::CanWalkOffLedges();
	}
	// The drop that lands at the Stall speed against the world (DT_ImpactThresholds).
	const UKineticRulesSubsystem* Rules = GEngine ? GEngine->GetEngineSubsystem<UKineticRulesSubsystem>() : nullptr;
	const Kinetic::FKineticTables* Tables = Rules ? Rules->Tables() : nullptr;
	const std::optional<double> Stall = Tables ? Tables->Impact(Kinetic::EImpactTable::PlayerVsStatic).Stall : std::nullopt;
	const double G = -GetGravityZ();
	if (!Stall || G <= 0.0)
	{
		return true;
	}
	const double LethalCm = (*Stall * 100.0) * (*Stall * 100.0) / (2.0 * G);
	const FVector Ahead = Velocity.GetSafeNormal2D();
	if (Ahead.IsNearlyZero())
	{
		return true;
	}
	const UCapsuleComponent* Capsule = CharacterOwner->GetCapsuleComponent();
	const FVector Feet = UpdatedComponent->GetComponentLocation() - FVector(0.0, 0.0, Capsule->GetScaledCapsuleHalfHeight());
	// Over the top of a ladder: let them go, and the ladder catches them (AKineticCharacter::UpdateLadder).
	for (TActorIterator<AKineticLadder> It(GetWorld()); It; ++It)
	{
		if (FMath::Abs(Feet.Z - It->TopZ()) < 40.0 && FVector::Dist2D(Feet, It->ClimbAt(It->TopZ())) < 150.0)
		{
			return true;
		}
	}
	const FVector From = Feet + Ahead * (Capsule->GetScaledCapsuleRadius() + 30.0) + FVector(0.0, 0.0, 50.0);
	FHitResult Below;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(KineticLedgeGuard), false, CharacterOwner);
	const bool bFloor = GetWorld()->LineTraceSingleByChannel(Below, From, From - FVector(0.0, 0.0, 50.0 + LethalCm * 0.95),
	                                                        UpdatedComponent->GetCollisionObjectType(), Params);
	return bFloor;
}
