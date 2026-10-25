// Every Spool line in a level, solved once per step by KineticCore's rules (bible/05 sections
// 3 to 6; docs/tech/spool-and-line-implementation.md). The step is the browser client's, with
// engine bodies in place of its boxes:
//
//   1. Dogs in flight sweep the level and seat: a player's Catch Plate, a prop, or the world.
//   2. Lines engage their mode once both Dogs seat, switch after the class delay, time out.
//   3. A trial solve sees which placed props an active pull breaks free (6.2a).
//   4. Haul and Jack forces (LineSolver) go to props as impulses and to players by footing:
//      braced, skidding along the ground, or airborne and hoisted (MassRules).
//   5. Hang and Set lines are solved together over every body (HangSolver); the result goes
//      back to Chaos bodies and character movement as velocities, net of the gravity the
//      engine adds over the step.
//
// It runs one step per frame: the project runs at a fixed 60 Hz (Config/DefaultEngine.ini), the
// rules' step. Props are point masses at their centre of mass, as in the rules; a line is drawn
// to where its Dog seated. Networking, the flight body, impacts and kinematic seats come later.
#pragma once

#include "CoreMinimal.h"
#include "Kinetic/Impact.h"
#include "Kinetic/LineSolver.h"
#include "Kinetic/MassRules.h"
#include "Kinetic/MatchEvents.h"
#include "Subsystems/WorldSubsystem.h"

#include "KineticSpoolWorld.generated.h"

class AKineticCharacter;
class AKineticLineRenderer;
class AKineticProp;

namespace Kinetic
{
class FKineticTables;
}

enum class EKineticSeat : uint8
{
	None,
	Static,    // The world: a point
	Prop,      // An AKineticProp
	Plate,     // Another player's Catch Plate
	OwnPlate,  // The owner's own (a self-dog)
};

struct FKineticLineEnd
{
	EKineticSeat Seat = EKineticSeat::None;
	FVector Point = FVector::ZeroVector;  // Static: the world point; Prop: where it seated, prop-local (cm)
	TWeakObjectPtr<AKineticProp> Prop;
	TWeakObjectPtr<AKineticCharacter> Player;
	Kinetic::EPlate Plate = Kinetic::EPlate::None;
};

struct FKineticDog
{
	FVector Position = FVector::ZeroVector;  // cm
	FVector Velocity = FVector::ZeroVector;  // cm/s
	double Travelled = 0.0;                  // m
};

enum class EKineticLineEnded : uint8
{
	Released,
	Cut,
	DogsMet,
	Expired,
	Missed,
	RanOut
};

struct FKineticLine
{
	int32 Id = 0;
	TWeakObjectPtr<AKineticCharacter> Owner;
	FKineticLineEnd A, B;
	TOptional<FKineticDog> FlyingA, FlyingB;
	Kinetic::FLineState State;
	Kinetic::FLineParams Params;
	double TensionN = 0.0;
	Kinetic::ELineMode Preselect = Kinetic::ELineMode::Haul;
	Kinetic::ELineMode PendingMode = Kinetic::ELineMode::Slack;  // A switch waiting out the class delay
	double PendingAt = -1.0;
	double CreatedAt = 0.0;
	bool bSet = false;  // A released Hang: a set line until it expires
	double SetExpiresAt = 0.0;

	bool IsLive() const { return A.Seat != EKineticSeat::None && B.Seat != EKineticSeat::None; }
};

// What the Spool needs to remember of each player between steps.
struct FKineticSpoolPlayer
{
	Kinetic::ELineMode Preselect = Kinetic::ELineMode::Haul;  // The mode a new line engages in
	bool bSelfDogNext = false;
	bool bUnderLine = false;  // A Haul or Jack acted on them last step: their input is ignored
	Kinetic::EFooting Footing = Kinetic::EFooting::Skid;
	FVector LastHangForce = FVector::ZeroVector;  // N (not cm), from last step's Hang solve
	bool bBraceInput = false;
	bool bBraced = false;
	double BraceHeld = 0.0;
	bool bDrawn = false;
	double DrawnAt = -1.0;
	double NextSwingAt = -1.0;
	// Hits (player-physics 4): the Impact Monitor, and the outcome it last gave.
	TSharedPtr<Kinetic::FImpactMonitor> Monitor;
	double StunnedUntil = -1.0;
	Kinetic::EOutcome LastOutcome = Kinetic::EOutcome::None;
	double LastOutcomeAt = -1.0;
	bool bStanding = true;  // False once Stalled, until the Hooks bring them back
	Kinetic::EStallCause StallCause = Kinetic::EStallCause::WorldImpact;
	double StalledAt = -1.0;
};

UCLASS()
class KINETICGAMEPLAY_API UKineticSpoolWorld : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	static constexpr double Dt = 1.0 / 60.0;

	// The Spool's buttons (bible/05 section 3.1, the browser client's controls).
	void FireDog(AKineticCharacter* Player);            // Dog A, then Dog B; a third press releases and fires a new A
	void SelfDogNext(AKineticCharacter* Player);        // The next Dog goes into your own chest plate, and fires it
	void SelectMode(AKineticCharacter* Player, Kinetic::ELineMode Mode);  // Haul, Jack or Hang
	void Release(AKineticCharacter* Player);            // A Hang becomes a set line; Haul and Jack vanish
	void SetBrace(AKineticCharacter* Player, bool bHeld);  // Held: crouch to brace (Rigger, Breaker)
	void SetCutterDrawn(AKineticCharacter* Player, bool bDrawn);
	void Swing(AKineticCharacter* Player);              // Once warm: cut the nearest line in the cone

	const TArray<FKineticLine>& Lines() const { return LineList; }
	const FKineticSpoolPlayer* PlayerState(const AKineticCharacter* Player) const;
	double Now() const { return Clock; }

	// A player's Catch Plate, in the world (cm).
	static FVector PlatePosition(const AKineticCharacter* Player, Kinetic::EPlate Plate);
	// Where a seated end is for the rules (cm): a plate, a prop's centre of mass, a point.
	FVector EndPosition(const FKineticLineEnd& End) const;
	// Where a line is drawn to (cm): the plate, or the point on the prop the Dog seated at.
	FVector DrawnEndPosition(const FKineticLine& Line, bool bSideA) const;
	// Where the Dog is (cm): a plate, the point on a prop it seated at, a point. Haul and Jack
	// act between the Dogs, so the Dogs can meet (05 3.2).
	FVector DogPosition(const FKineticLineEnd& End) const;
	// What the Hang solve moves (cm): a player's centre, a prop's centre of mass, a point.
	FVector BodyPosition(const FKineticLineEnd& End) const;

	// The rules step, public for tests; Tick calls it once per frame.
	void Step();

	// Takes a player out of the round (05 7.4): the Hooks lift them off, their lines go. In the
	// walk-the-arena mode they are back at Spawn A after RespawnSeconds.
	void Stall(AKineticCharacter* Player, Kinetic::EStallCause Cause);
	static constexpr double RespawnSeconds = 3.0;
	static constexpr double KillZCm = -600.0;  // The Well's kill volume, and anything off the edge (layout.md)

	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
	const Kinetic::FKineticTables* Tables() const;
	FKineticSpoolPlayer& StateOf(AKineticCharacter* Player);
	int32 ActiveLineOf(const AKineticCharacter* Player) const;
	void EndLine(int32 Index, EKineticLineEnded Why);
	void ReleaseActive(AKineticCharacter* Player);
	void Engage(FKineticLine& Line, Kinetic::ELineMode Mode);
	TOptional<FKineticLineEnd> SweepDog(const AKineticCharacter* Owner, const FVector& From, const FVector& To,
	                                    FVector& Hit) const;
	void UpdateDogs();
	void UpdateLines();
	void Simulate();
	void Impacts();
	void HangLineContacts();
	void Respawn(AKineticCharacter* Player);
	Kinetic::FLineEndpoint EndpointOf(const FKineticLineEnd& End, bool bHeld) const;

	TArray<FKineticLine> LineList;
	TMap<TWeakObjectPtr<AKineticCharacter>, FKineticSpoolPlayer> Players;
	int32 NextLineId = 1;
	double Clock = 0.0;

	UPROPERTY()
	TObjectPtr<AKineticLineRenderer> Renderer;
};
