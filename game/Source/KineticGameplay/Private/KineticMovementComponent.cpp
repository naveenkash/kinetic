#include "KineticMovementComponent.h"

#include "KineticCharacter.h"

void UKineticMovementComponent::HandleImpact(const FHitResult& Hit, float TimeSlice, const FVector& MoveDelta)
{
	if (AKineticCharacter* Character = Cast<AKineticCharacter>(CharacterOwner))
	{
		Character->RecordContact(Hit, Velocity, FVector::ZeroVector);
	}
	Super::HandleImpact(Hit, TimeSlice, MoveDelta);
}
