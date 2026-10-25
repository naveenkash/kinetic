// The grey-box HUD: a crosshair and the class, speed and state, until the UMG HUD of the match
// modes replaces it.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"

#include "KineticHUD.generated.h"

UCLASS()
class KINETICGAMEPLAY_API AKineticHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;
};
