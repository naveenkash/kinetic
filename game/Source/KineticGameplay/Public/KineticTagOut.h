// One Claim with Tag-Out match in a level (docs/proposals/claim-tag-out.md; everything here is
// PROPOSED): two crews of four, no Spool lines, KineticCore's FTagOutMatch for the codes, the
// board, the calls, the beacons and the clock, and bots for every slot a human does not play.
// AKineticGameMode owns it and ticks it once a frame, before the Spool's step.
//
// - The crews: slot 0 is the player (their class), a Climber, a Scrapper and a Breaker; crew 1 is
//   a Doubler, a Climber, a Scrapper and a Breaker. With no human a bot plays slot 0 and the
//   player's view follows it.
// - The site (section 9): each crew's home pad at its spawn (Spawn_A, Spawn_B) with the beacon's
//   stand at its centre and a tag locker beside it, and the two map lockers on NC2 and SC4. All
//   grey-box shapes in the crew colours.
// - Reading (10.1): every rigger's eyes are traced to the plates in front of them ten times a
//   second; what passes the cone, the range and the line of sight goes on their crew's board. The
//   digits on a plate are drawn only while the player's own eyes could read it (with the 10.1
//   pop-in margin), so the screen never shows a pip the board would not.
// - The Call goes at the enemy whose torso is under the crosshair (within 40 m, in sight), and the
//   lock holds while the caller keeps them within 12 degrees of the crosshair and in sight.
// - The Cutter (4): a swing at an enemy within its reach and cone staggers them 0.3 s and spins
//   them 70 degrees, the way that turns their back toward the swinger's left. A rigger just spun
//   cannot be spun again for 1.5 s, so two Cutters cannot hold someone in a stagger.
// - A thrown beacon flies about 12 m after a 1.0 s wind-up, and a dropped one falls to the ground.
// - The bots: the Climber runs for the enemy beacon, the Breaker anchors the pad and walks a
//   dropped beacon home, and the rest hunt: they get round an enemy to read them, call when the
//   board has the whole code (and sometimes gamble on two of three), turn to face a caller to
//   break the lock, re-stamp when the other crew has two of their pips, and swing the Cutter up
//   close.
#pragma once

#include "CoreMinimal.h"
#include "Kinetic/Bots.h"
#include "Kinetic/TagOut.h"
#include "KineticHarness.h"
#include "KineticScrap.h"

class AKineticBotController;
class AKineticCharacter;
class APlayerController;
class AStaticMeshActor;
class UMaterialInstanceDynamic;
class UTextRenderComponent;
class UKineticSpoolWorld;

// What the watched rigger's eyes have of another rigger right now.
struct FKineticTagSight
{
	bool bVisible = false;  // In front of them and in sight
	uint8 Plates = 0;       // The plates they could read (bit i is Kinetic::ETagPlate i)
};

class KINETICGAMEPLAY_API FKineticTagOut
{
public:
	struct FSettings
	{
		EKineticHarness HumanClass = EKineticHarness::Rigger;
		bool bHumanPlays = true;
		Kinetic::FTagOutRules Rules;
		uint64 Seed = 1;
	};

	FKineticTagOut(UWorld& InWorld, const FSettings& InSettings);
	~FKineticTagOut();

	void AddHuman(APlayerController* Player);
	// One 60 Hz step of the match, before the Spool's.
	void Tick();
	// A new match from the drop (after one is over, or a test's shorter rules).
	void StartMatch();
	void SetRules(const Kinetic::FTagOutRules& Rules) { Settings.Rules = Rules; }

	const Kinetic::FTagOutMatch& Match() const { return *TheMatch; }
	const TArray<FString>& Feed() const { return FeedLines; }
	int32 HumanSlot() const { return Settings.bHumanPlays ? 0 : INDEX_NONE; }
	// Whose eyes the plates and the HUD use: the player's, or the bot in slot 0 they watch.
	int32 ViewSlot() const { return 0; }
	int32 SlotCount() const { return Seats.Num(); }
	int32 TeamOf(int32 Slot) const { return Slot < CrewSize ? 0 : 1; }
	AKineticCharacter* Rigger(int32 Slot) const;
	int32 SlotOf(const AKineticCharacter* Character) const;
	FString Who(int32 Slot) const;
	const FKineticTagSight& Sight(int32 Slot) const { return Seats[Slot].Sight; }
	bool StripOpen(int32 Slot) const { return Seats[Slot].bStripOpen; }
	// Seconds left on a beacon throw's wind-up, or -1.
	double ThrowWindup(int32 Slot) const;
	// Where things are, in the world (cm).
	FVector PadCm(int32 Team) const { return Pads[Team]; }
	FVector BeaconCm(int32 Team) const;
	FVector ChestCm(int32 Slot) const;
	FKineticRoundTimes RoundTimes() const;

	// For tests.
	int32 MatchesFinished() const { return Finished; }
	int32 CallsSent() const { return Calls; }
	int32 TagOuts() const { return Tagged; }
	int32 BadCalls() const { return Bad; }
	int32 Reveals() const { return Revealed; }
	int32 Grabs() const { return Grabbed; }
	int32 Restamps() const { return Restamped; }
	int32 Swings() const { return Swung; }

	static constexpr int32 CrewSize = 4;
	static constexpr double RestartSeconds = 8.0;
	static constexpr double ThrowWindupSeconds = 1.0;
	static constexpr double RevealHz = 10.0;
	static constexpr double CrosshairHoldDeg = 12.0;
	static constexpr double CutterStaggerSeconds = 0.3;
	static constexpr double CutterSpinDeg = 70.0;
	static constexpr double CutterSpinGuardSeconds = 1.5;  // A spun rigger cannot be spun again this soon

private:
	enum class ERole : uint8 { Runner, Anchor, Hunter };

	struct FSeat
	{
		TWeakObjectPtr<AKineticCharacter> Character;
		TWeakObjectPtr<AKineticBotController> Brain;
		TWeakObjectPtr<UTextRenderComponent> Plates[Kinetic::PlateCount];
		FKineticTagSight Sight;
		bool bStripOpen = false;
		bool bLevel = false;
		bool bCarrying = false;
		double ThrowAt = -1.0;
		double NextSwingAt = 0.0;
		double SpunUntil = 0.0;
		// The bot.
		double BotSwingAt = 0.0;
		ERole Role = ERole::Hunter;
		TOptional<Kinetic::FBotRandom> Random;
		int32 ReactTo = INDEX_NONE;
		double ReactAt = 0.0;
		bool bWillReact = false;
		double WatchUntil = 0.0;
		double MoveUntil = 0.0;
		double GambleAt = 0.0;
		double FlankSide = 1.0;
	};

	struct FBeaconProp
	{
		TWeakObjectPtr<AStaticMeshActor> Body;
		TWeakObjectPtr<AStaticMeshActor> Reflector;
		TWeakObjectPtr<UTextRenderComponent> Flag;
		FVector At = FVector::ZeroVector;  // cm
		FVector Velocity = FVector::ZeroVector;
		bool bFlying = false;
	};

	void FindSite();
	void BuildSite();
	void SpawnRiggers();
	void PlaceAtPad(int32 Slot);
	void Say(FString Text);
	FString Code(const Kinetic::FTagCode& Code) const;
	UKineticSpoolWorld* Spool() const;
	const Kinetic::FKineticTables* Tables() const;
	AStaticMeshActor* Shape(const TCHAR* Mesh, const FVector& At, const FVector& Scale, const FLinearColor& Colour,
	                        UMaterialInstanceDynamic** OutMaterial = nullptr);
	FVector Ground(const FVector& Above, double Fallback) const;

	void Poses();
	void Reveal();
	bool Clear(const FVector& From, const FVector& To, const AActor* Viewer, const AActor* Target) const;
	bool InCrosshair(int32 Caller, int32 Target, double MaxDeg) const;
	int32 UnderCrosshair(int32 Caller) const;
	void Inputs();
	void Swing(int32 Slot);
	void Throw(int32 Slot);
	void Think(int32 Slot);
	void Events();
	void Beacons();
	void Show();

	UWorld& World;
	FSettings Settings;
	TArray<Kinetic::FMatchPlayer> Roster;
	TArray<FSeat> Seats;
	TUniquePtr<Kinetic::FTagOutMatch> TheMatch;
	TArray<FString> FeedLines;
	FVector Pads[2];
	FRotator PadFacing[2];
	TArray<FVector> LockerCm;
	TArray<TWeakObjectPtr<AStaticMeshActor>> Site;
	TArray<TWeakObjectPtr<UMaterialInstanceDynamic>> LockerLights;
	TArray<double> LockerRedUntil;
	FBeaconProp BeaconProps[2];
	double MatchOverAt = -1.0;
	double NextRevealAt = 0.0;
	double SirenAt = -1.0;
	double MidMatchAt = -1.0;
	double OvertimeAt = -1.0;
	int32 Started = 0;
	int32 Finished = 0;
	int32 Calls = 0;
	int32 Tagged = 0;
	int32 Bad = 0;
	int32 Revealed = 0;
	int32 Grabbed = 0;
	int32 Restamped = 0;
	int32 Swung = 0;
	uint64 MatchSeed = 1;
};
