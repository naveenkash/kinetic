// Draws every Spool line and Dog in flight as plain meshes: a thin cylinder per line, coloured
// by its mode (Slack grey, Haul orange, Jack cyan, Hang yellow, Set white), and a small sphere
// per Dog. The Niagara ribbon of docs/tech/spool-and-line-implementation.md section 6 replaces
// it with the line's VFX.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "KineticLineRenderer.generated.h"

class UMaterialInstanceDynamic;
class UStaticMesh;
class UStaticMeshComponent;
class UKineticSpoolWorld;

UCLASS(Transient, NotPlaceable)
class KINETICGAMEPLAY_API AKineticLineRenderer : public AActor
{
	GENERATED_BODY()

public:
	AKineticLineRenderer();

	void Draw(const UKineticSpoolWorld& Spool);

private:
	UStaticMeshComponent* Piece(TArray<TObjectPtr<UStaticMeshComponent>>& Pool, int32 Index, UStaticMesh* Mesh);

	UPROPERTY()
	TArray<TObjectPtr<UStaticMeshComponent>> LinePieces;
	UPROPERTY()
	TArray<TObjectPtr<UStaticMeshComponent>> DogPieces;
	UPROPERTY()
	TArray<TObjectPtr<UMaterialInstanceDynamic>> ModeMaterials;  // By Kinetic::ELineMode
	UPROPERTY()
	TObjectPtr<UStaticMesh> Cylinder;
	UPROPERTY()
	TObjectPtr<UStaticMesh> Sphere;
};
