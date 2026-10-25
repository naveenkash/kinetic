#include "KineticLineRenderer.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "KineticCharacter.h"
#include "KineticSpoolWorld.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
constexpr double LineRadiusCm = 1.2;
constexpr double DogRadiusCm = 6.0;
}  // namespace

AKineticLineRenderer::AKineticLineRenderer()
{
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	Cylinder = CylinderMesh.Object;
	Sphere = SphereMesh.Object;
}

UStaticMeshComponent* AKineticLineRenderer::Piece(TArray<TObjectPtr<UStaticMeshComponent>>& Pool, int32 Index, UStaticMesh* Mesh)
{
	while (Pool.Num() <= Index)
	{
		UStaticMeshComponent* Made = NewObject<UStaticMeshComponent>(this);
		Made->SetStaticMesh(Mesh);
		Made->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Made->SetCastShadow(false);
		Made->SetupAttachment(RootComponent);
		Made->RegisterComponent();
		Pool.Add(Made);
	}
	UStaticMeshComponent* Out = Pool[Index];
	Out->SetVisibility(true);
	return Out;
}

void AKineticLineRenderer::Draw(const UKineticSpoolWorld& Spool)
{
	if (ModeMaterials.Num() == 0 && Cylinder)
	{
		// The owner's team tint (characters/README.md: line glow is team-locked), dim while slack,
		// full under Haul or Jack; set lines are Bone. Per crew: Slack, Haul, Jack, Hang, Set.
		const float Strength[] = {0.35f, 1.0f, 1.0f, 0.8f, 1.0f};
		for (int32 Crew = 0; Crew < 2; ++Crew)
		{
			for (int32 Mode = 0; Mode < 5; ++Mode)
			{
				const FLinearColor Colour = Mode == 4 ? FLinearColor::FromSRGBColor(FColor(0xE8, 0xE6, 0xE0))
				                                      : AKineticCharacter::TeamTint(Crew) * Strength[Mode];
				UMaterialInstanceDynamic* Mid = UMaterialInstanceDynamic::Create(Cylinder->GetMaterial(0), this);
				Mid->SetVectorParameterValue(TEXT("Color"), Colour);
				ModeMaterials.Add(Mid);
			}
		}
	}
	int32 Lines = 0, Dogs = 0;
	for (const FKineticLine& Line : Spool.Lines())
	{
		const FVector A = Spool.DrawnEndPosition(Line, true);
		const FVector B = Line.FlyingB.IsSet() || Line.B.Seat != EKineticSeat::None
		                      ? Spool.DrawnEndPosition(Line, false)
		                      : (Line.Owner.IsValid() ? UKineticSpoolWorld::PlatePosition(Line.Owner.Get(), Kinetic::EPlate::Chest) : A);
		const FVector Span = B - A;
		const double Length = Span.Size();
		if (Length > 1.0)
		{
			UStaticMeshComponent* Seg = Piece(LinePieces, Lines++, Cylinder);
			Seg->SetWorldLocationAndRotation((A + B) * 0.5, FRotationMatrix::MakeFromZ(Span / Length).Rotator());
			Seg->SetWorldScale3D(FVector(LineRadiusCm / 50.0, LineRadiusCm / 50.0, Length / 100.0));
			const int32 Mode = Line.bSet ? 4 : Line.IsLive() ? static_cast<int32>(Line.State.Mode) : 0;
			const int32 Crew = Line.Owner.IsValid() ? FMath::Clamp(Line.Owner->Crew, 0, 1) : 0;
			if (ModeMaterials.IsValidIndex(Crew * 5 + Mode))
			{
				Seg->SetMaterial(0, ModeMaterials[Crew * 5 + Mode]);
			}
		}
		for (const TOptional<FKineticDog>* Dog : {&Line.FlyingA, &Line.FlyingB})
		{
			if (Dog->IsSet())
			{
				UStaticMeshComponent* Ball = Piece(DogPieces, Dogs++, Sphere);
				Ball->SetWorldLocation((*Dog)->Position);
				Ball->SetWorldScale3D(FVector(DogRadiusCm / 50.0));
			}
		}
	}
	for (int32 I = Lines; I < LinePieces.Num(); ++I)
	{
		LinePieces[I]->SetVisibility(false);
	}
	for (int32 I = Dogs; I < DogPieces.Num(); ++I)
	{
		DogPieces[I]->SetVisibility(false);
	}
}
