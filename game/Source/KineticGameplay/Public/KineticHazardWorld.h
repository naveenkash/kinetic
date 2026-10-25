// Volatiles, blasts and force volumes in a level (bible/05 6.4 and 6.6; docs/tech/
// kinematics-and-force-volumes.md section 2), on KineticCore's rules:
// - A volatile prop (AKineticProp::Volatile) arms when it takes an impact over its class's
//   trigger speed (DT_Volatiles: propane 8 m/s, a fuel drum 10, a demolition charge 6), or is in
//   another blast. It hisses, and goes off no sooner than 0.3 s later (FTellGate, 05 11). A gas
//   manifold arms on a Jack over 600 N seated in it, and shrieks for 1.0 s.
// - A blast is judged per player by distance (ClassifyBlast: a Stall, flip or stagger), and
//   throws loose props with its impulse, falling linearly to nothing at its radius. A fuel drum
//   leaves a fire patch for 6 s; a manifold vents a 6 m gas jet for 3 s.
// - Force volumes push players and loose props by a_ref × 100 / m, capped at 2 × a_ref
//   (BodyAcceleration); they never Stall. Entering a fire patch staggers.
// - A demolition charge sheds the ladder cage beside it (the Yard's north leg).
// Sounds are the generated cues in /Game/Audio (Tools/unreal/import_audio.py); with none
// imported the hazards run silent.
#pragma once

#include "CoreMinimal.h"
#include "Kinetic/ForceVolumes.h"
#include "Kinetic/Tells.h"
#include "Subsystems/WorldSubsystem.h"

#include "KineticHazardWorld.generated.h"

class AKineticProp;
class AKineticCharacter;
class UStaticMeshComponent;

UCLASS()
class KINETICGAMEPLAY_API UKineticHazardWorld : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	static constexpr double Dt = 1.0 / 60.0;

	// Arms a volatile: its tell now, the blast once the lead has passed.
	void Arm(AKineticProp* Volatile);
	// A blast of a volatile's class at a point (a demolition charge, a drum, a cylinder).
	void Blast(AKineticProp* Source, const FVector& At);
	// A radial shove (the Gantry load landing: 3 m, 12 m/s² for 0.5 s).
	void Shove(const FVector& Centre, double RadiusM, double ARef, double Seconds);
	// The round's reset: volumes out, every arm called off.
	void ResetRound();

	// Plays a generated cue (/Game/Audio/SW_<Name>), arena-wide or at a point.
	static void PlayCue(UWorld* World, const TCHAR* Name, const FVector* At = nullptr);

	double Now() const { return Clock; }
	int32 BlastsSeen() const { return Blasts; }
	bool IsBurning(const FVector& At) const;

	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

private:
	struct FArmed
	{
		TWeakObjectPtr<AKineticProp> Prop;
	};
	struct FFire
	{
		Kinetic::FFirePatch Patch;
		double LitAt = 0.0;
		TWeakObjectPtr<UStaticMeshComponent> Flames;
	};
	struct FJet
	{
		Kinetic::FGasJet Jet;
		double TriggeredAt = 0.0;
		TWeakObjectPtr<UStaticMeshComponent> Vapour;
	};
	struct FFlash
	{
		TWeakObjectPtr<UStaticMeshComponent> Mesh;
		double Until = 0.0;
	};
	struct FShove
	{
		FVector Centre;
		double RadiusM = 3.0;
		double ARef = 12.0;
		double Until = 0.0;
	};

	void WatchVolatiles();
	void UpdateArmed();
	void PushBodies();
	void Ignite(const FVector& At);
	void Vent(AKineticProp* Manifold);
	void ShedLadderNear(const FVector& At);
	UStaticMeshComponent* Effect(const TCHAR* Mesh, const FVector& At, const FRotator& Rotation, const FVector& Scale,
	                             const FLinearColor& Colour);
	void PushPlayer(AKineticCharacter& Player, const Kinetic::FVec3& ARef);

	double Clock = 0.0;
	int32 Blasts = 0;
	Kinetic::FTellGate Gate;
	TArray<FArmed> Armed;
	TArray<FFire> Fires;
	TArray<FJet> Jets;
	TArray<FShove> Shoves;
	TArray<FFlash> Flashes;
	TMap<TWeakObjectPtr<AKineticProp>, FVector> LastVelocity;  // cm/s, per volatile
	TSet<TWeakObjectPtr<AKineticCharacter>> InFire;            // Who was in a fire patch last step

	UPROPERTY()
	TObjectPtr<AActor> Effects;  // Holds the flames and vapour meshes
};
