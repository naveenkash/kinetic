// A line-movable object (bible/05 section 6): a generated mesh simulated by Chaos with its
// spec's mass, and the placement that decides when a line starts it. Loose props simulate from
// the start; racked, stacked and bolted ones stay put until an active pull meets their
// break-free threshold (DT_BreakFree), then simulate. Fixed scenery and rated loads never move
// under a line (rated loads commit through their timeline, a later phase).
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Kinetic/MassRules.h"

#include "KineticProp.generated.h"

class UStaticMeshComponent;

// Kinetic::EPlacement, in the same order.
UENUM(BlueprintType)
enum class EKineticPlacement : uint8
{
	Loose,
	Rack,
	Stack,
	Bolted,
	Rated,
	HangingHouse
};

UCLASS()
class KINETICGAMEPLAY_API AKineticProp : public AActor
{
	GENERATED_BODY()

public:
	AKineticProp();

	UPROPERTY(VisibleAnywhere, Category = "Kinetic")
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Kinetic")
	float MassKg = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Kinetic")
	EKineticPlacement Placement = EKineticPlacement::Loose;

	// Heavy on a bare floor, water tanks, the gas manifold: no line moves it (6.1a).
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Kinetic")
	bool bFixedScenery = false;

	// The rules' view of it (KineticCore's FObjectBody).
	Kinetic::FObjectBody Body() const;

	// The placement threshold was met: it simulates from now on (the lurch).
	void BreakFree();
	bool IsBrokenFree() const { return bBrokenFree; }

	// Whether Chaos is moving it now.
	bool IsSimulating() const;

	// Applies the mass and the placement's starting state. Called at BeginPlay, and by the
	// level builder after setting the properties.
	void ApplyBody();

	// Back where the level put it, at rest and in its placement (a round's reset, game-modes 2).
	void ResetToAuthored();

protected:
	virtual void BeginPlay() override;

private:
	bool bBrokenFree = false;
	FTransform Authored;
};
