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
	Mesh->SetCanEverAffectNavigation(false);  // It moves: bots path round the scenery, not round props
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
	Authored = GetActorTransform();
	ApplyBody();
}

void AKineticProp::ResetToAuthored()
{
	Mesh->SetSimulatePhysics(false);
	SetActorTransform(Authored, false, nullptr, ETeleportType::ResetPhysics);
	bBrokenFree = false;
	ApplyBody();
	if (Mesh->IsSimulatingPhysics())
	{
		Mesh->SetPhysicsLinearVelocity(FVector::ZeroVector);
		Mesh->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
		Mesh->PutRigidBodyToSleep();
	}
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
