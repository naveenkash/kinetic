// A kinematic (bible/05 6.7; docs/tech/kinematics-and-force-volumes.md section 1): a body that
// moves on its arena's schedule, never under a line. A Hauler, the Gantry's bridge and magnet,
// a Lockout curtain, the press ram. Its schedule moves it (AKineticYardDirector); this is the
// body:
// - its parts are grey-box boxes made in code, coloured, blocking everything; a machine mesh
//   can be drawn over them (AddLook), the boxes then only colliding;
// - it keeps its velocity, so a hit on a player is judged at its speed (the Impact Monitor's
//   kinematic table), and a Dog seated in it rides with it;
// - its sever box is what the sweep test runs against every line it passes (05 3.5); a line
//   with a Dog seated in it is not severed by it.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Kinetic/Kinematics.h"

#include "KineticKinematic.generated.h"

class AKineticCharacter;
class UStaticMesh;
class UStaticMeshComponent;

UCLASS()
class KINETICGAMEPLAY_API AKineticKinematic : public AActor
{
	GENERATED_BODY()

public:
	AKineticKinematic();

	// A box part in the actor's frame (cm), in a grey-box colour. The sever box grows to hold it
	// unless bSevers is false (a hoist cable, a lamp).
	UStaticMeshComponent* AddBox(FName Name, const FVector& CentreCm, const FVector& SizeCm, const FLinearColor& Colour,
	                             bool bSevers = true);

	// The machine drawn over the boxes (art/machines): a mesh in the actor's frame with no
	// collision; the boxes stop drawing and go on colliding. The parts named in Shown stay drawn
	// (a headlamp, a cable). Null (not imported): the boxes stay as they are.
	UStaticMeshComponent* AddLook(UStaticMesh* Mesh, const TArray<FName>& Shown = {});

	// Moves it to a pose this step (dt s): velocity from the move, no sweep, so Chaos pushes what
	// it meets and characters standing on it ride.
	void MoveTo(const FVector& Location, const FRotator& Rotation, double Dt);
	// Out of the arena (a Hauler in its plenum): hidden, no collision, no velocity.
	void Park(const FVector& Location);
	bool IsActive() const { return bActive; }

	FVector Velocity() const { return VelocityCm; }  // cm/s
	// The rules' view: the pose before and after the last move (metres, a yaw about Up), and the
	// sever box in the actor's frame (metres).
	Kinetic::FPose PoseBefore() const { return PoseOf(Before); }
	Kinetic::FPose PoseNow() const { return PoseOf(GetActorTransform()); }
	const Kinetic::FBox& SeverBox() const { return Box; }
	bool HasSeverBox() const { return bHasBox; }

	// Whether a character's capsule overlaps a part now.
	bool Overlaps(const AKineticCharacter& Character) const;

	// Scenery that only moves on a rated commit (a hull section, the shack) is in the navmesh.
	void SetAffectsNavigation(bool bAffects);

	// Hits on players: on, at the kinematic's speed. A curtain pushes and crushes instead.
	UPROPERTY(EditAnywhere, Category = "Kinetic")
	bool bHitsPlayers = true;

private:
	static Kinetic::FPose PoseOf(const FTransform& T);

	UPROPERTY()
	TObjectPtr<USceneComponent> Root;
	UPROPERTY()
	TArray<TObjectPtr<UStaticMeshComponent>> Parts;
	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> Look;

	FTransform Before;
	FVector VelocityCm = FVector::ZeroVector;
	Kinetic::FBox Box;
	FVector BoxMin = FVector(TNumericLimits<double>::Max()), BoxMax = FVector(-TNumericLimits<double>::Max());
	bool bHasBox = false;
	bool bActive = true;
};
