// The Yard's moving parts and events (docs/arenas/the-yard/events-and-moving-parts.md), run from
// the round's clock: e is seconds since "live" (FKineticScrap's round times; in the
// walk-the-arena mode, since play began, with the mid-match event where the default Scrap rules
// put it). Every pose is a function of e, as kinematics-and-force-volumes.md section 1 asks.
// - Haulers (1): three 14 m trains at 12 m/s, South and North west to east from e 10 and 40,
//   Center east to west from e 25, every 45 s; a klaxon 3 s before each nose-in. A train severs
//   the lines it passes, hits whoever is in the trench at its own speed, carries whoever rides
//   it, and sweeps its trench's plank bridges on the first pass.
// - The Gantry (2): the bridge, trolley and magnet on the 120 s cycle, picking a hull plate
//   (2,800 kg) at the Pick Stack and dropping it into the press at c 80, with a 2 s drop tone. A
//   Breaker's Haul of 1,000 N strips the plate off the magnet after a 1.0 s groan.
// - Shift Change (3), at the mid-match event: the Gantry releases its plate wherever it is (it
//   lands as fixed scenery, with a 3 m debris shove), then works the East Pick Stack at speed
//   for 60 s; the Haulers re-phase to 22.5 s for 60 s, then 45 s from each trench's last train.
// - Lockout (5), at sudden death: two curtains advance from X ±80 at 1 m/s from T+5 to ±10 by
//   T+75, severing lines, shoving props, pushing riggers and crushing anyone pinned (0.5 s);
//   the Haulers run every 22.5 s staggered 7.5 s, and the Gantry parks at X 0.
// - The press ram on its 20 s cycle, the trench lamps (dim amber, bright 24 m ahead of a nose
//   and at the entry end from the klaxon, red through Shift Change and Lockout), and the reset (7).
//
// - Rated loads (4), each holding still until active pull along its axis has held 1,000 N for
//   the commit hold (FRatedCommit), then a 1.0 s groan, then its timeline: a hull section (R1 to
//   R4, the Plow) slides off its skids into the nearest trench, where the next train carries
//   it as a wall to the plenum; a pulled shack column drops the control shack 10.5 m onto the
//   North Dock (whoever is on it lands at 14.3 m/s); the press counterweight swings north twice
//   over 6 s and lands at (45, +14) as a block.
//
// Simplifications (PROPOSED): each pick takes a fresh plate (the stacks' live tops are not
// modelled as separate bodies); after Shift Change the Gantry cycles the East Pick Stack every
// 35 s for 60 s, then every 60 s; the corner masts' lights are not built, so Lockout does not
// darken them.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Kinetic/Bots.h"
#include "Kinetic/Kinematics.h"
#include "Kinetic/MassRules.h"

#include "KineticYardDirector.generated.h"

class AKineticCharacter;
class AKineticKinematic;
class AKineticProp;
class UStaticMeshComponent;

UCLASS()
class KINETICGAMEPLAY_API AKineticYardDirector : public AActor
{
	GENERATED_BODY()

public:
	AKineticYardDirector();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	// For bots (Caboose) and tests: the trains whose klaxon has sounded or that are running.
	std::vector<Kinetic::FBotHauler> HaulersForBots() const;
	// e, and whether a trench's train is in the arena now.
	double Elapsed() const { return E; }
	bool IsHaulerRunning(int32 Trench) const;
	AKineticKinematic* Hauler(int32 Trench) const;
	AKineticKinematic* Gantry() const { return Bridge; }
	AKineticKinematic* Curtain(int32 Side) const;
	bool IsCarrying() const;
	int32 DropsSeen() const { return Drops; }
	// A plank bridge breaks (a train's first pass, a Breaker landing on it).
	void Fracture(AActor* Plank);

	static constexpr double HaulerSpeed = 12.0;   // m/s
	static constexpr double TrainLengthM = 14.0;
	static constexpr double GantryCycle = 120.0;

private:
	struct FTrench
	{
		double YM = 0.0;        // Layout Y of the trench's centre line
		double First = 0.0;     // First nose-in, e
		double Offset = 0.0;    // Nose-in after Shift Change's payload
		double Stagger = 0.0;   // In Lockout
		bool bEastward = true;
		const TCHAR* Klaxon = TEXT("KlaxonLow");
	};
	struct FLoad
	{
		TWeakObjectPtr<AKineticProp> Prop;
		bool bOnMagnet = true;
		bool bLanded = false;
		double StripCuedAt = -1.0;
		FVector LastVelocity = FVector::ZeroVector;
		double StillFor = 0.0;
	};
	enum class ERated : uint8 { Plow, ShackColumn, Counterweight };
	struct FRated
	{
		TObjectPtr<AKineticKinematic> Body = nullptr;
		ERated Kind = ERated::Plow;
		Kinetic::FRatedCommit Commit;
		FVector Rest = FVector::ZeroVector;  // Unreal cm
		FVector Axis = FVector::ZeroVector;  // The authored pull axis (unit), or zero for any
		int32 Trench = -1;                   // The Plow's trench
		FVector Lip = FVector::ZeroVector;   // Where its slide ends, over the trench (cm)
		double GroanAt = -1.0;
		double FiredAt = -1.0;
		bool bCarried = false;
		bool bGone = false;
	};
	struct FLamp
	{
		TObjectPtr<UStaticMeshComponent> Mesh;
		int32 Trench = 0;
		double XM = 0.0;  // Layout X
		int32 State = -1;
	};

	void ReadClock();
	void ResetRound();
	Kinetic::FRailRun RunFor(int32 Trench) const;
	void UpdateHaulers();
	void UpdateGantry();
	void UpdateCurtains();
	void UpdateRam();
	void UpdateLamps();
	void Strike(AKineticKinematic& K);
	void BuildRated();
	void UpdateRated();
	FVector PullOn(const AKineticKinematic& Body) const;
	void Pick();
	void Release(bool bByShiftChange);
	void WatchLoads();
	FVector GantryTarget(double& OutHoistSpeed) const;

	// The round's clock.
	double Now = 0.0, E = -100.0, PrevE = -100.0;
	double ESiren = -1.0, EMid = -1.0, EPlannedMid = 69.0, ELock = -1.0;
	int32 Resets = -1;
	bool bMidFired = false, bSirenFired = false, bLockFired = false;

	FTrench Trenches[3];
	UPROPERTY()
	TObjectPtr<AKineticKinematic> Trains[3];
	UPROPERTY()
	TObjectPtr<AKineticKinematic> Bridge;  // The beam and cab, along X
	UPROPERTY()
	TObjectPtr<AKineticKinematic> Magnet;  // The trolley's magnet, along X and Y and up and down
	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> Cable;
	UPROPERTY()
	TObjectPtr<AKineticKinematic> Curtains[2];
	UPROPERTY()
	TObjectPtr<AActor> Ram;
	FVector RamRest = FVector::ZeroVector;
	UPROPERTY()
	TObjectPtr<AActor> LampHolder;
	TArray<FLamp> Lamps;
	TArray<TWeakObjectPtr<AActor>> Planks;
	TArray<FRated> Rated;
	UPROPERTY()
	TObjectPtr<AKineticKinematic> Shack;
	FVector ShackRest = FVector::ZeroVector;
	double ShackFallAt = -1.0;
	bool bShackDown = false;
	TMap<TWeakObjectPtr<AKineticCharacter>, double> Pinned;

	FVector GantryAt = FVector::ZeroVector;
	double XAtMid = -15.0;  // Where Shift Change caught the bridge  // Layout metres: bridge X, trolley Y, magnet underside Z
	TArray<FLoad> Loads;
	int32 Drops = 0;
	double LastCycleC = -1.0, LastEastU = -1.0;
};
