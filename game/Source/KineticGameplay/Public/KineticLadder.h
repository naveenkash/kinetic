// A climbable ladder: the mesa rungs, the trench rungs, the portal cage ladders and the mast
// ladders. A character inside the volume pressing toward it climbs at its class's ladder speed
// (bible/05 section 2.3: 2.0 m/s, the Breaker 1.6), and steps off at the top.
//
// It is also a navigation link, both ways, from the foot of the ladder to where the climber
// steps off, so a bot's path can use it (AKineticBotController climbs it).
#pragma once

#include "AI/Navigation/NavRelevantInterface.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NavLinkHostInterface.h"

#include "KineticLadder.generated.h"

class UBoxComponent;

UCLASS()
class KINETICGAMEPLAY_API AKineticLadder : public AActor, public INavLinkHostInterface, public INavRelevantInterface
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
	// Where a climber stands to start up (the foot, outside the wall) and where they step off
	// (on top, past the wall's face), in the world.
	FVector Foot() const;
	FVector StepOff() const;
	// Where a climber holds on at a height: against the wall, outside it.
	FVector ClimbAt(double Z) const;

	virtual void GetNavigationData(FNavigationRelevantData& Data) const override;
	virtual FBox GetNavigationBounds() const override;
	virtual bool IsNavigationRelevant() const override { return true; }
	virtual bool GetNavigationLinksClasses(TArray<TSubclassOf<UNavLinkDefinition>>& OutClasses) const override { return false; }
	virtual bool GetNavigationLinksArray(TArray<FNavigationLink>& OutLink, TArray<FNavigationSegmentLink>& OutSegments) const override;

private:
	FNavigationLink Link() const;
};
