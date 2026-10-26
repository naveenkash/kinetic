// Claim with Tag-Out (docs/proposals/claim-tag-out.md, v0.2; everything here is PROPOSED): two
// crews, one beacon each, one capture ends the match. Nobody is roped: every rigger wears a
// three-pip lockout code on the shoulders and back, and a crew that reads it and calls it locks
// that rigger out. This is the mode on the server, engine-free:
// - codes, re-rolled on every drop and at a tag locker (2);
// - which plates a viewer can read, by cone, range and line of sight (2.2, 10.1), and the crew
//   read board the server fills from exactly those reveals (2.4);
// - the Call: enter three pips, send at a target in range, hold the 0.6 s lock, and the clean,
//   lost and bad outcomes with their cooldowns (3);
// - the tag lockers' 3 s re-stamp (2.5);
// - the Claim: beacons on their stands, the grab, the carrier, drops, returns, the plant, the
//   regulation clock, the reserve timers and overtime with its closest-approach decision (5);
// - hiding your code (6): the grease rag and the dust bag, sun glare off a plate and the sun in a
//   caller's eyes, and heat haze.
// The game feeds it positions, what each rigger can see (its own line traces), Stalls from the
// arena and the riggers' inputs, and reads the state and the events back.
//
// Choices the proposal leaves open (PROPOSED, here):
// - A clean call Stalls the target (open question 1 takes the Stall, not the 2 s freeze).
// - Your own beacon need not be home to plant (open question 2).
// - Closest approach is the walkable route to the pad's edge when the game reports one
//   (ReportRoute, from its navmesh), else straight to it.
// - A grease rag smears the next clean plate in the order back, left, right: the back is the
//   plate a chaser reads.
// - Glare: the plates tilt up 20 degrees (shoulders and back), and a plate flares for a viewer
//   within 10 degrees of the sun's reflection off it. The dust and the haze are spheres.
// - A bad call leaks the lowest-numbered plate the target crew does not have yet, not a random
//   one: the match stays deterministic without a random source per call.
#pragma once

#include "Kinetic/Api.h"
#include "Kinetic/MatchEvents.h"
#include "Kinetic/Vec3.h"

#include <array>
#include <cstdint>
#include <optional>
#include <vector>

namespace Kinetic
{
struct FTagOutRules
{
	// Reading (2.2).
	double PlateConeDegrees = 55.0;
	double NakedRangeM = 20.0;
	double LevelRangeM = 50.0;
	double LevelRaiseSeconds = 0.4;
	double CarrierBackRangeM = 40.0;  // The flag panel's doubled back pip (5.3)
	// The Call (3).
	double CallRangeM = 40.0;
	double LockSeconds = 0.6;
	double CleanCooldown = 1.0;
	double LostCooldown = 1.5;
	double BadCooldown = 2.0;
	double BadCallRevealSeconds = 3.0;
	// Tag lockers (2.5).
	double RestampSeconds = 3.0;
	double RestampHearM = 30.0;
	double StillSpeedMs = 0.25;  // Moving faster than this breaks a hold that needs standing still
	// The Claim (5).
	double DropSeconds = 6.0;
	double RegulationSeconds = 480.0;  // 8:00
	double OvertimeSeconds = 180.0;    // Up to 3:00
	double MidEventRemaining = 240.0;  // 4:00: the arena's mid-match event
	double GrabSeconds = 0.5;
	double ReturnSeconds = 2.0;
	double PlantSeconds = 1.0;
	double AutoReturnSeconds = 30.0;
	double UseRangeM = 1.6;           // Touching the stand, the beacon
	double PadRadiusM = 2.0;          // A 4 m circle
	double CarrierSprintScale = 0.9;
	double CarrierLadderScale = 0.75;
	double FlashSeconds = 3.0;
	double KillZ = -6.0;              // A beacon below this (the Press pit) goes home
	// Hiding your code (6).
	int GadgetsPerDrop = 2;
	double GreaseApplySeconds = 1.0;
	double GreaseSeconds = 12.0;
	double DustRadiusM = 3.0;         // A 6 m cloud
	double DustSeconds = 8.0;
	double PlateTiltDegrees = 20.0;
	double GlareDegrees = 10.0;
	double SunLockDegrees = 12.0;     // No lock on a target this close to the sun in the caller's view
	double HazeRangeM = 12.0;         // Through heat haze, nothing beyond this reads
	// Reserve: 6 s for the first 3:00 elapsed, 9 s until 6:00, 12 s to the end of regulation,
	// 15 s in overtime.
	std::array<double, 4> ReserveSeconds{6.0, 9.0, 12.0, 15.0};
	std::array<double, 2> ReserveSteps{180.0, 360.0};
};

// The plates that carry pips, in code order: left shoulder, back, right shoulder (2.1).
enum class ETagPlate : uint8_t { LeftShoulder, Back, RightShoulder };
inline constexpr int PlateCount = 3;

// A lockout code: three pips from 1 to 4, written left, back, right.
struct FTagCode
{
	std::array<uint8_t, PlateCount> Pips{1, 1, 1};
	bool operator==(const FTagCode& O) const { return Pips == O.Pips; }
};

// A rigger's pose for reading: where the plates are, which way the body faces, and the eyes.
struct FTagPose
{
	FVec3 Chest;                    // m
	FVec3 Eye;                      // m: where they read and call from
	FVec3 Forward{1.0, 0.0, 0.0};   // Horizontal unit vectors
	FVec3 Right{0.0, 1.0, 0.0};      // Unreal's axes: X forward, Y right, Z up
};

// The world direction each plate faces.
KINETICCORE_API FVec3 PlateFacing(const FTagPose& Pose, ETagPlate Plate);

// Which of the target's plates the viewer could read (bit i is plate i): inside the plate's
// cone, in range (the sighting level's when raised), and with LineOfSight[i] (the game's trace
// from the viewer's eye to plate i).
KINETICCORE_API uint8_t ReadablePlates(const FVec3& ViewerEye, bool bLevelRaised, const FTagPose& Target,
                                       const std::array<bool, PlateCount>& LineOfSight, const FTagOutRules& Rules,
                                       bool bCarrier = false);

// The direction a plate faces, tilted up as the plates are (for glare).
KINETICCORE_API FVec3 TiltedPlateNormal(const FTagPose& Pose, ETagPlate Plate, const FTagOutRules& Rules);
// Whether the segment from A to B passes within Radius of Centre.
KINETICCORE_API bool SegmentNearPoint(const FVec3& A, const FVec3& B, const FVec3& Centre, double Radius);

enum class ETagPhase : uint8_t { Drop, Live, Overtime, Over };

// The one gadget slot (6): grease rags or dust bags.
enum class ETagGadget : uint8_t { Grease, Dust };

struct FTagCloud
{
	FVec3 At;
	double Until = 0.0;
	int Slot = -1;  // Who cut the bag
};

// Hot steel the game reports each step: the press frame, a Hauler's exhaust.
struct FTagHaze
{
	FVec3 At;
	double RadiusM = 3.0;
};

enum class ETagEvent : uint8_t
{
	Live,
	MidEvent,       // The arena's mid-match event (the Yard's Shift Change)
	Overtime,       // The arena's sudden-death event runs
	Revealed,       // Slot's crew was shown Other's plate Plate
	CallSent,       // Slot called Other: the lock starts (Other hears the tick)
	TaggedOut,      // Slot's clean call locked Other out
	LostLock,       // Slot's call on Other lost its lock
	BadCall,        // Slot's call on Other was wrong: Plate of Slot's code leaked to Other's crew
	Restamped,      // Slot re-stamped at a locker (Other is the locker)
	Stalled,        // Slot is down (the arena, or a call)
	Respawned,      // Slot drops back in at their pad, re-coded
	BeaconGrabbed,  // Slot lifted Team's beacon
	BeaconDropped,  // Team's beacon fell where Slot went down
	BeaconReturned, // Team's beacon went home (Slot returned it, or NoPlayer: on its own)
	Planted,        // Slot planted Team's beacon: Slot's crew wins
	MatchOver,      // Team won, or NoPlayer: a draw
	Smearing,       // Slot started greasing Plate
	Smeared,        // Slot's Plate is smeared for GreaseSeconds
	DustCloud,      // Slot cut a dust bag
};

struct FTagEventRecord
{
	ETagEvent Kind = ETagEvent::Live;
	double Time = 0.0;  // s since the start of the Drop
	int Slot = NoPlayer;
	int Other = NoPlayer;
	int Team = NoPlayer;
	ETagPlate Plate = ETagPlate::Back;
};

// What a crew knows of one enemy's code.
struct KINETICCORE_API FTagRead
{
	std::array<std::optional<uint8_t>, PlateCount> Pips{};
	std::array<double, PlateCount> Seen{};  // When each pip was last shown (s)
	bool bStale = false;  // The rigger re-stamped or went down since: the pips are an old code
	int Known() const;    // Pips of the current code
};

enum class EBeaconState : uint8_t { OnStand, Carried, Dropped };

struct FTagBeacon
{
	int Team = 0;               // Whose beacon: the other crew captures it
	EBeaconState State = EBeaconState::OnStand;
	FVec3 Stand;                // m, at its home pad's centre
	FVec3 At;                   // Where it is now
	int Carrier = NoPlayer;
	double DroppedAt = 0.0;
	double ClosestToEnemyPadM = 1e9;  // The capturing crew's best approach (5.6)
	bool bRouted = false;             // The game reports the walkable route: no straight-line guess
};

enum class ECallState : uint8_t { Idle, Locking };

struct FTagRigger
{
	int Slot = 0;
	int Team = 0;
	EHarnessClass Harness = EHarnessClass::Rigger;
	FTagCode Code;
	bool bStanding = false;
	double ReserveUntil = 0.0;
	FTagPose Pose;
	double SpeedMs = 0.0;
	bool bLevelRaised = false;
	double LevelUpAt = -1.0;            // When the sighting level finished raising
	// The call.
	std::vector<uint8_t> Entered;       // Pips entered so far (up to 3)
	ECallState Call = ECallState::Idle;
	int CallTarget = NoPlayer;
	double LockElapsed = 0.0;
	double CallCooldownUntil = 0.0;
	double ExposedUntil = 0.0;          // A bad call shows the caller to the target's crew
	// The gadget.
	ETagGadget Gadget = ETagGadget::Grease;
	int GadgetsLeft = 0;
	std::array<double, PlateCount> SmearedUntil{};
	double SmearingUntil = -1.0;        // A grease rag going on
	ETagPlate SmearPlate = ETagPlate::Back;
	// A hold on Use: what it is working on and for how long.
	double UseHeld = 0.0;
	int UseOn = -1;                     // UseBeacon + team, UseLocker + index, or UsePlant
	bool bUsing = false;                // Held this step
};

class KINETICCORE_API FTagOutMatch
{
public:
	FTagOutMatch(const std::vector<FMatchPlayer>& Roster, const std::array<FVec3, 2>& Pads, const std::vector<FVec3>& Lockers,
	             uint64_t Seed, FTagOutRules Rules = {});

	const FTagOutRules& Rules() const { return Config; }
	ETagPhase Phase() const { return CurrentPhase; }
	double Time() const { return Clock; }
	// Regulation's remaining time (s), or overtime's once it runs.
	double Remaining() const;
	int Winner() const { return WinningTeam; }

	const std::vector<FTagRigger>& Riggers() const { return Seats; }
	const FTagRigger& Rigger(int Slot) const { return Seats[Slot]; }
	const FTagBeacon& Beacon(int Team) const { return Beacons[Team]; }
	const FVec3& Pad(int Team) const { return HomePads[Team]; }
	const std::vector<FVec3>& Lockers() const { return LockerAt; }
	// What Team's crew knows of Slot's code.
	const FTagRead& Read(int Team, int Slot) const { return Board[Team][Slot]; }
	bool LockersOpen() const { return CurrentPhase == ETagPhase::Live; }
	bool CanCall() const { return CurrentPhase == ETagPhase::Live || CurrentPhase == ETagPhase::Overtime; }
	bool CanMove(int Slot) const { return CanCall() && Seats[Slot].bStanding; }
	// The sighting level is up and steady (raised for LevelRaiseSeconds).
	bool LevelReady(int Slot) const;
	// A bad call has shown Slot's position to the crew it called.
	bool Exposed(int Slot) const { return Seats[Slot].ExposedUntil > Clock; }
	// The team whose beacon Slot carries, or NoPlayer.
	int Carrying(int Slot) const;
	bool Smeared(int Slot, ETagPlate Plate) const { return Seats[Slot].SmearedUntil[static_cast<int>(Plate)] > Clock; }
	// Of the plates the viewer's eyes reach (cone, range, line of sight), the ones that read:
	// not smeared, not through dust, not through haze past 12 m, not flaring.
	uint8_t Legible(int Viewer, int Target, uint8_t Plates) const;
	// The sun's reflection off Target's Plate points at Viewer.
	bool Glares(int Viewer, int Target, ETagPlate Plate) const;
	// Target is within SunLockDegrees of the sun in Caller's view.
	bool SunBehind(int Caller, int Target) const;
	bool ThroughDust(const FVec3& From, const FVec3& To) const;
	const std::vector<FTagCloud>& Clouds() const { return Dust; }
	// Who is calling Slot right now (NoPlayer if nobody): Slot's harness ticks toward them.
	int CalledBy(int Slot) const;

	// Each step, before Step: where each rigger is (m), facing, speed, and the sighting level.
	void SetPose(int Slot, const FTagPose& Pose, double SpeedMs, bool bLevelRaised);
	// What a viewer can read this step (the game's cone, range and line-of-sight result, from
	// ReadablePlates): the pips go on the viewer's crew board.
	void Show(int Viewer, int Target, uint8_t Plates);

	// The call strip: enter a pip (1 to 4; a fourth clears and starts over), clear, and send at a
	void EnterPip(int Caller, int Pip);
	void ClearCall(int Caller);
	// target the game found in the crosshair and in sight (the range is checked here).
	bool SendCall(int Caller, int Target, bool bInSight);
	// Hold Use this step (released when not called): grab, return, plant, re-stamp, whichever
	// the rigger is standing at.
	void HoldUse(int Slot, double Dt);

	// The rigger's gadget (the count refills to GadgetsPerDrop at each drop).
	void SetLoadout(int Slot, ETagGadget Gadget);
	// The gadget: a grease rag starts its 1 s on the next clean plate; a dust bag is cut open at
	// At (the game calls this at the end of the Cutter swing that opens it). False if it cannot.
	bool UseGadget(int Slot, const FVec3& At = {});
	// The sun (a unit vector toward it) and the hot steel, each step.
	void SetSun(const FVec3& ToSun) { Sun = ToSun; }
	void SetHaze(std::vector<FTagHaze> Zones) { Haze = std::move(Zones); }
	// The walkable route from Team's carried beacon to its carrier's pad edge (m), from the
	// game's navmesh: the closest approach counts this, not the straight line.
	void ReportRoute(int Team, double Metres);

	// The arena downed a rigger (a fall, a Hauler, the press).
	void Stall(int Slot);
	// The carrier lets the beacon go at the end of the throw's wind-up.
	void Throw(int Slot);
	// Where a dropped or thrown beacon lies now (the game simulates it as a prop).
	void SetBeaconAt(int Team, const FVec3& At);

	// Advances the clock: the lock on every call (CallTargetInView[caller]: the target is still
	// in the caller's sight), the reserves, the beacon timers, the phase.
	void Step(double Dt, const std::vector<bool>& CallTargetInView);

	std::vector<FTagEventRecord> TakeEvents();

	static FTagCode RollCode(uint64_t Seed, int Slot, int Stamp);

	static constexpr int UseBeacon = 0;
	static constexpr int UseLocker = 100;
	static constexpr int UsePlant = 200;

private:
	void Emit(ETagEvent Kind, int Slot = NoPlayer, int Other = NoPlayer, int Team = NoPlayer, ETagPlate Plate = ETagPlate::Back);
	void Recode(int Slot);
	void DropIn(int Slot);
	void Down(int Slot);
	void Reveal(int Team, int Target, ETagPlate Plate);
	void ClearBoardFor(int Slot);
	void ResolveCall(int Caller);
	void SendHome(int Team, int Returner);
	double ReserveSecondsNow() const;
	bool OnPad(int Slot, int PadTeam) const;
	bool Touching(int Slot, const FVec3& At) const;
	void End(int Team);

	FTagOutRules Config;
	std::vector<FTagRigger> Seats;
	std::array<FVec3, 2> HomePads;
	std::vector<FVec3> LockerAt;
	std::array<FTagBeacon, 2> Beacons;
	std::array<std::vector<FTagRead>, 2> Board;
	std::vector<int> Stamps;
	uint64_t MatchSeed = 0;
	ETagPhase CurrentPhase = ETagPhase::Drop;
	double Clock = 0.0;
	bool bMidEventFired = false;
	int WinningTeam = NoPlayer;
	std::vector<FTagEventRecord> Events;
	FVec3 Sun{0.0, 0.0, 1.0};
	std::vector<FTagHaze> Haze;
	std::vector<FTagCloud> Dust;
};
} // namespace Kinetic
