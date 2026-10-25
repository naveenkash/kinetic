#include "KineticLadder.h"

#include "Components/BoxComponent.h"

AKineticLadder::AKineticLadder()
{
	Volume = CreateDefaultSubobject<UBoxComponent>(TEXT("Volume"));
	Volume->SetCollisionProfileName(TEXT("Trigger"));
	Volume->SetBoxExtent(FVector(40.0, 40.0, 200.0));
	RootComponent = Volume;
}

float AKineticLadder::TopZ() const
{
	return Volume->GetComponentLocation().Z + Volume->GetScaledBoxExtent().Z;
}
