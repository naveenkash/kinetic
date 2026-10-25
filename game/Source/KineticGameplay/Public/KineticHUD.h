// The grey-box HUD: a crosshair, the class, speed and state, the Spool, hits, and in a Scrap
// match the round, score, clock and the Marshal's calls, until a UMG HUD replaces it.
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

private:
	void DrawMatch();
};
