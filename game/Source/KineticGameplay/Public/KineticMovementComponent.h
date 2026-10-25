// Character movement that tells its character about every blocking hit while the velocity
// that made it is still intact (the hit events Unreal dispatches after a move see the velocity
// the move left, which for a landing is zero). The Impact Monitor judges those contacts.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "KineticMovementComponent.generated.h"

UCLASS()
class KINETICGAMEPLAY_API UKineticMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

protected:
	virtual void HandleImpact(const FHitResult& Hit, float TimeSlice, const FVector& MoveDelta) override;
};
