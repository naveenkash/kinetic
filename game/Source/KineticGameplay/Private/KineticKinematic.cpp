#include "KineticKinematic.h"

#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "KineticCharacter.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
UStaticMesh* CubeMesh()
{
	static UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
	return Cube;
}
}  // namespace

AKineticKinematic::AKineticKinematic()
{
	PrimaryActorTick.bCanEverTick = false;
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	Root->SetMobility(EComponentMobility::Movable);
	RootComponent = Root;
}

UStaticMeshComponent* AKineticKinematic::AddBox(FName Name, const FVector& CentreCm, const FVector& SizeCm, const FLinearColor& Colour,
                                                bool bSevers)
{
	UStaticMeshComponent* Part = NewObject<UStaticMeshComponent>(this, Name);
	Part->SetMobility(EComponentMobility::Movable);
	Part->SetStaticMesh(CubeMesh());
	Part->SetupAttachment(Root);
	Part->SetRelativeLocation(CentreCm);
	Part->SetRelativeScale3D(SizeCm / 100.0);  // The engine cube is 1 m
	Part->SetCollisionProfileName(TEXT("BlockAll"));
	Part->SetCanEverAffectNavigation(false);  // It moves: bots path round the scenery
	Part->RegisterComponent();
	if (UMaterialInstanceDynamic* M = Part->CreateDynamicMaterialInstance(0))
	{
		M->SetVectorParameterValue(TEXT("Color"), Colour);
	}
	Parts.Add(Part);
	if (bSevers)
	{
		BoxMin = BoxMin.ComponentMin(CentreCm - SizeCm * 0.5);
		BoxMax = BoxMax.ComponentMax(CentreCm + SizeCm * 0.5);
		const FVector Half = (BoxMax - BoxMin) * 0.5 / 100.0;
		const FVector Centre = (BoxMax + BoxMin) * 0.5 / 100.0;
		Box.HalfExtents = {Half.X, Half.Y, Half.Z};
		Box.Center = {Centre.X, Centre.Y, Centre.Z};
		bHasBox = true;
	}
	return Part;
}

UStaticMeshComponent* AKineticKinematic::AddLook(UStaticMesh* Mesh, const TArray<FName>& Shown)
{
	if (!Mesh)
	{
		return nullptr;
	}
	Look = NewObject<UStaticMeshComponent>(this, TEXT("Look"));
	Look->SetMobility(EComponentMobility::Movable);
	Look->SetStaticMesh(Mesh);
	Look->SetupAttachment(Root);
	Look->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Look->SetCanEverAffectNavigation(false);
	Look->RegisterComponent();
	for (UStaticMeshComponent* Part : Parts)
	{
		if (!Shown.Contains(Part->GetFName()))
		{
			Part->SetVisibility(false);
		}
	}
	return Look;
}

Kinetic::FPose AKineticKinematic::PoseOf(const FTransform& T)
{
	Kinetic::FPose P;
	const FVector L = T.GetLocation() / 100.0;
	P.Position = {L.X, L.Y, L.Z};
	P.Axis = Kinetic::Up;
	P.Angle = FMath::DegreesToRadians(T.Rotator().Yaw);
	return P;
}

void AKineticKinematic::MoveTo(const FVector& Location, const FRotator& Rotation, double Dt)
{
	if (!bActive)
	{
		// Back from the plenum: in place first, so the move is not swept across the arena.
		bActive = true;
		SetActorHiddenInGame(false);
		SetActorEnableCollision(true);
		SetActorLocationAndRotation(Location, Rotation, false, nullptr, ETeleportType::TeleportPhysics);
		Before = GetActorTransform();
		VelocityCm = FVector::ZeroVector;
		return;
	}
	Before = GetActorTransform();
	VelocityCm = Dt > 0.0 ? (Location - Before.GetLocation()) / Dt : FVector::ZeroVector;
	SetActorLocationAndRotation(Location, Rotation, false, nullptr, ETeleportType::None);
}

void AKineticKinematic::Park(const FVector& Location)
{
	if (!bActive && GetActorLocation().Equals(Location))
	{
		return;
	}
	bActive = false;
	VelocityCm = FVector::ZeroVector;
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	SetActorLocation(Location, false, nullptr, ETeleportType::TeleportPhysics);
	Before = GetActorTransform();
}

void AKineticKinematic::SetAffectsNavigation(bool bAffects)
{
	for (UStaticMeshComponent* Part : Parts)
	{
		Part->SetCanEverAffectNavigation(bAffects);
	}
}

bool AKineticKinematic::Overlaps(const AKineticCharacter& Character) const
{
	if (!bActive)
	{
		return false;
	}
	const UCapsuleComponent* Capsule = Character.GetCapsuleComponent();
	const FCollisionShape Shape = FCollisionShape::MakeCapsule(Capsule->GetScaledCapsuleRadius(), Capsule->GetScaledCapsuleHalfHeight());
	for (const UStaticMeshComponent* Part : Parts)
	{
		if (Part->IsCollisionEnabled() &&
		    const_cast<UStaticMeshComponent*>(Part)->OverlapComponent(Character.GetActorLocation(), Character.GetActorQuat(), Shape))
		{
			return true;
		}
	}
	return false;
}
