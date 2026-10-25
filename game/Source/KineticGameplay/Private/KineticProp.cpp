#include "KineticProp.h"

#include "Components/StaticMeshComponent.h"

static_assert(static_cast<int>(EKineticPlacement::HangingHouse) == static_cast<int>(Kinetic::EPlacement::HangingHouse),
              "EKineticPlacement must follow Kinetic::EPlacement");

AKineticProp::AKineticProp()
{
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetCollisionProfileName(TEXT("PhysicsActor"));
	Mesh->SetMobility(EComponentMobility::Movable);
	Mesh->SetNotifyRigidBodyCollision(true);  // A prop striking a player reports its impulse
	RootComponent = Mesh;
}

Kinetic::FObjectBody AKineticProp::Body() const
{
	Kinetic::FObjectBody Out;
	Out.MassKg = MassKg;
	Out.Placement = static_cast<Kinetic::EPlacement>(Placement);
	Out.bFixedScenery = bFixedScenery;
	Out.bRated = Placement == EKineticPlacement::Rated;
	Out.bBrokenFree = bBrokenFree;
	return Out;
}

void AKineticProp::ApplyBody()
{
	Mesh->SetMassOverrideInKg(NAME_None, MassKg, true);
	const bool bSimulate = !bFixedScenery && (Placement == EKineticPlacement::Loose || bBrokenFree);
	Mesh->SetSimulatePhysics(bSimulate);
}

void AKineticProp::BeginPlay()
{
	Super::BeginPlay();
	ApplyBody();
}

void AKineticProp::BreakFree()
{
	if (bBrokenFree)
	{
		return;
	}
	bBrokenFree = true;
	ApplyBody();
}

bool AKineticProp::IsSimulating() const { return Mesh->IsSimulatingPhysics(); }
