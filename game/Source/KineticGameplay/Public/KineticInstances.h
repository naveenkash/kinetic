// Scenery repeated by the thousand (the port's containers, cranes, light towers and bollards,
// art/port): one hierarchical instanced mesh per type, filled in the editor by
// build_yard_greybox.py from art/out/port/instances.json and saved with the level. No collision:
// it is backdrop.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "KineticInstances.generated.h"

class UHierarchicalInstancedStaticMeshComponent;
class UStaticMesh;

UCLASS()
class KINETICGAMEPLAY_API AKineticInstances : public AActor
{
	GENERATED_BODY()

public:
	AKineticInstances();

	// Places the mesh at each transform (cm, in the world), adding to what it already has.
	UFUNCTION(BlueprintCallable, Category = "Kinetic")
	int32 AddInstances(UStaticMesh* Mesh, const TArray<FTransform>& Transforms);

	UFUNCTION(BlueprintCallable, Category = "Kinetic")
	int32 InstanceCount() const;

private:
	UPROPERTY(VisibleAnywhere, Category = "Kinetic")
	TArray<TObjectPtr<UHierarchicalInstancedStaticMeshComponent>> Types;
};
