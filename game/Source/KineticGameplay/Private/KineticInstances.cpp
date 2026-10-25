#include "KineticInstances.h"

#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"

AKineticInstances::AKineticInstances()
{
	PrimaryActorTick.bCanEverTick = false;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent->SetMobility(EComponentMobility::Static);
}

int32 AKineticInstances::AddInstances(UStaticMesh* Mesh, const TArray<FTransform>& Transforms)
{
	if (!Mesh || Transforms.IsEmpty())
	{
		return 0;
	}
	UHierarchicalInstancedStaticMeshComponent* Type = nullptr;
	for (UHierarchicalInstancedStaticMeshComponent* Existing : Types)
	{
		if (Existing && Existing->GetStaticMesh() == Mesh)
		{
			Type = Existing;
		}
	}
	if (!Type)
	{
		Type = NewObject<UHierarchicalInstancedStaticMeshComponent>(this, Mesh->GetFName(), RF_Transactional);
		Type->SetStaticMesh(Mesh);
		Type->SetMobility(EComponentMobility::Static);
		Type->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Type->SetCanEverAffectNavigation(false);
		Type->SetupAttachment(RootComponent);
		AddInstanceComponent(Type);  // Saved with the level
		Type->RegisterComponent();
		Types.Add(Type);
	}
	Type->AddInstances(Transforms, false, true);
	return Type->GetInstanceCount();
}

int32 AKineticInstances::InstanceCount() const
{
	int32 Count = 0;
	for (const UHierarchicalInstancedStaticMeshComponent* Type : Types)
	{
		Count += Type ? Type->GetInstanceCount() : 0;
	}
	return Count;
}
