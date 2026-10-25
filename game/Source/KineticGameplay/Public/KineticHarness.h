// The five harness classes (bible/05 section 2.2) as an engine enum, so a pawn, a URL option
// and the editor can name one. KineticCore's Kinetic::EHarnessClass is the source of truth;
// the two are kept in the same order.
#pragma once

#include "CoreMinimal.h"
#include "Kinetic/Tables.h"

#include "KineticHarness.generated.h"

UENUM(BlueprintType)
enum class EKineticHarness : uint8
{
	Climber,
	Scrapper,
	Rigger,
	Doubler,
	Breaker
};

namespace KineticHarness
{
inline Kinetic::EHarnessClass ToCore(EKineticHarness Harness) { return static_cast<Kinetic::EHarnessClass>(Harness); }
inline EKineticHarness FromCore(Kinetic::EHarnessClass Class) { return static_cast<EKineticHarness>(Class); }

// "Breaker", "breaker" and so on; false if no class has that name.
KINETICGAMEPLAY_API bool Parse(const FString& Name, EKineticHarness& Out);
KINETICGAMEPLAY_API FString Name(EKineticHarness Harness);
}  // namespace KineticHarness
