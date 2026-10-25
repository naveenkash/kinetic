// A climbable ladder: the mesa rungs, the trench rungs, the portal cage ladders and the mast
// ladders. A character inside the volume pressing toward it climbs at its class's ladder speed
// (bible/05 section 2.3: 2.0 m/s, the Breaker 1.6), and steps off at the top.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "KineticLadder.generated.h"

class UBoxComponent;

UCLASS()
class KINETICGAMEPLAY_API AKineticLadder : public AActor
{
	GENERATED_BODY()

public:
	AKineticLadder();

	// The volume a climber can be in, and which way is "into" the ladder (toward the wall).
	UPROPERTY(VisibleAnywhere, Category = "Kinetic")
	TObjectPtr<UBoxComponent> Volume;

	UPROPERTY(EditAnywhere, Category = "Kinetic")
	FVector IntoWall = FVector::ForwardVector;

	// The world height a climber steps off at.
	float TopZ() const;
};
