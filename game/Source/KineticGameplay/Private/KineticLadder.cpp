#include "KineticLadder.h"

#include "AI/Navigation/NavigationRelevantData.h"
#include "AI/NavigationModifier.h"
#include "AI/NavigationSystemBase.h"
#include "Components/BoxComponent.h"
#include "NavigationSystemTypes.h"

namespace
{
constexpr double FootOutCm = 50.0;     // The foot point, out from the wall's face
constexpr double StepOffInCm = 90.0;   // Past the face on top: the character's step-off (KineticCharacter)
}

AKineticLadder::AKineticLadder()
{
	Volume = CreateDefaultSubobject<UBoxComponent>(TEXT("Volume"));
	Volume->SetCollisionProfileName(TEXT("Trigger"));
	Volume->SetBoxExtent(FVector(40.0, 40.0, 200.0));
	Volume->SetCanEverAffectNavigation(false);  // The volume is air; the link is the ladder
	RootComponent = Volume;
}

float AKineticLadder::TopZ() const
{
	return Volume->GetComponentLocation().Z + Volume->GetScaledBoxExtent().Z;
}

FVector AKineticLadder::Foot() const
{
	// The face is where the volume meets the wall: its far side along IntoWall.
	const FVector Into = FVector(IntoWall.X, IntoWall.Y, 0.0).GetSafeNormal();
	const FVector Extent = Volume->GetScaledBoxExtent();
	const double Depth = FMath::Abs(Into.X) * Extent.X + FMath::Abs(Into.Y) * Extent.Y;
	const FVector Centre = Volume->GetComponentLocation();
	const FVector Face = Centre + Into * Depth;
	return FVector(Face.X, Face.Y, Centre.Z - Extent.Z) - Into * FootOutCm;
}

FVector AKineticLadder::StepOff() const
{
	const FVector Into = FVector(IntoWall.X, IntoWall.Y, 0.0).GetSafeNormal();
	const FVector Base = Foot() + Into * (FootOutCm + StepOffInCm);
	return FVector(Base.X, Base.Y, TopZ());
}

FVector AKineticLadder::ClimbAt(double Z) const
{
	constexpr double ClimberRadiusCm = 40.0;
	const FVector Into = FVector(IntoWall.X, IntoWall.Y, 0.0).GetSafeNormal();
	const FVector Face = Foot() + Into * FootOutCm;
	return FVector(Face.X, Face.Y, Z) - Into * (ClimberRadiusCm + 10.0);  // The face is 5 cm inside the wall
}

FNavigationLink AKineticLadder::Link() const
{
	// Simple links are in the actor's frame.
	const FTransform& T = GetActorTransform();
	FNavigationLink L(T.InverseTransformPosition(Foot()), T.InverseTransformPosition(StepOff()));
	L.Direction = ENavLinkDirection::BothWays;
	L.SnapRadius = 60.0f;
	return L;
}

void AKineticLadder::SetUsable(bool bInUsable)
{
	if (bInUsable == bUsable)
	{
		return;
	}
	bUsable = bInUsable;
	Volume->SetCollisionEnabled(bUsable ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
	FNavigationSystem::UpdateActorData(*this);
}

void AKineticLadder::GetNavigationData(FNavigationRelevantData& Data) const
{
	if (bUsable)
	{
		NavigationHelper::ProcessNavLinkAndAppend(&Data.Modifiers, this, TArray<FNavigationLink>{Link()});
	}
}

FBox AKineticLadder::GetNavigationBounds() const
{
	FBox Box(ForceInit);
	Box += Foot();
	Box += StepOff();
	return Box.ExpandBy(50.0);
}

bool AKineticLadder::GetNavigationLinksArray(TArray<FNavigationLink>& OutLink, TArray<FNavigationSegmentLink>& OutSegments) const
{
	if (bUsable)
	{
		OutLink.Add(Link());
	}
	return bUsable;
}
