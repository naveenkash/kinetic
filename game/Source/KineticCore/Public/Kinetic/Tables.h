// Data tables loaded from the CSV files in Content/Core/Tables, the same files Unreal imports
// as data tables. Values are SI. Tools/ci/check_tables.py keeps them equal to bible/05.
#pragma once

#include "Kinetic/Api.h"

#include <array>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>

namespace Kinetic
{
enum class EHarnessClass : uint8_t { Climber, Scrapper, Rigger, Doubler, Breaker };
inline constexpr std::array AllHarnessClasses{EHarnessClass::Climber, EHarnessClass::Scrapper, EHarnessClass::Rigger,
                                              EHarnessClass::Doubler, EHarnessClass::Breaker};
KINETICCORE_API std::string_view ToString(EHarnessClass Class);

enum class ECutterVariant : uint8_t { Standard, Hot, Long };
enum class ESurface : uint8_t { Hard, Wood, Scrap, DeepWater, StallGel };
enum class EPlacement : uint8_t { Loose, Rack, Stack, Bolted, Rated, HangingHouse };
enum class EVolatile : uint8_t { Propane, FuelDrum, DemoCharge, GasManifold };
enum class EImpactTable : uint8_t { PlayerVsStatic, PlayerVsKinematic, ObjectVsPlayer, PlayerVsPlayerFloor, HangLineHit };

// DT_HarnessClass (bible/05 sections 2.2, 3.4, 5.3, 5.4).
struct FHarnessTuning
{
	double MassKg = 0, RunSpeed = 0, SprintSpeed = 0, JumpHeight = 0, MantleHeight = 0, AirControl = 0, CrouchSpeed = 0;
	bool bCanBrace = false, bCanPerch = false;
	double FootHold = 0, BracedFootHold = 0;
	double LineLength = 0, TensionN = 0, OverspinTensionN = 0, KickLaunchSpeedV = 0, HangLoadN = 0, DogSpeed = 0;
	double ModeSwitch = 0, Recall = 0;
	int SetLines = 0, ActiveLines = 0;
	double SetLineLife = 0, HangStaggerSeconds = 0, CutterWarmupOffset = 0;
	double SmallPropSpeedMul = 1, AirborneFireSpeedKeep = 1, LadderSpeed = 0;

	// The Scrapper bonus is a 30% faster travel and arrival speed, implemented as the square of
	// the speed multiplier on line force (1.3² = 1.69×), bible/05 section 6.1.
	double SmallPropForceMul() const { return SmallPropSpeedMul * SmallPropSpeedMul; }
};

struct FCutterTuning
{
	double Warmup = 0, Swing = 0, Recovery = 0, Reach = 0, ConeDeg = 0;
};

// Stagger, flip and Stall thresholds; a missing value means that outcome does not exist for the event.
struct FThresholds
{
	std::optional<double> Stagger, Flip, Stall;
};

struct FCrushRule
{
	double ForceN = 0, Seconds = 0;
};

struct FVolatileTuning
{
	double TriggerSpeed = 0, TriggerJackN = 0, BlastRadius = 0, BlastImpulseNs = 0, BlastStallRadius = 0,
	       BlastFlipRadius = 0;
};

struct FMassTierBounds
{
	double LightUpperKg = 0, MediumUpperKg = 0, HeavyUpperKg = 0;
};

// DT_SpoolGlobals (project-structure section 4.1, shared constants).
struct FSpoolGlobals
{
	double HaulRamp = 0, KickTime = 0, InterlockRamp = 0, KickForceCapMul = 0, ReelRate = 0, ReelHoldThreshold = 0;
	int KinematicSeverSubsteps = 0;
	double HardVelocityCap = 0, HaulCloseCap = 0, JackSeparateCap = 0;
	double SnapImpulseBase = 0, SnapReferenceForce = 0, CutRecoilPer1000N = 0;
	double SmallPropBonusMassCap = 0, ManifoldJackTrigger = 0, DogsMeet = 0, DogRadius = 0, SlackTimeout = 0;
	double SlipDistance = 0, SlipInterval = 0;
	double OverspinRamp = 0, OverspinOctave = 0, OverspinSnap = 0, OverspinLaunch = 0, OverspinStagger = 0, OverspinCooldown = 0;
	double DoublerSplit = 0, DoublerRestore = 0, SetLineFade = 0, SetLineProximity = 0, RatedCommitHold = 0;
	double DogMissRecall = 0, ForceVolumeReferenceMassKg = 0, ForceVolumeCapMul = 0, Gravity = 0;
};

class FTableError : public std::runtime_error
{
public:
	using std::runtime_error::runtime_error;
};

// Every table, loaded once. Throws FTableError naming the file, row and column of any problem.
class KINETICCORE_API FKineticTables
{
public:
	static FKineticTables LoadFromDirectory(const std::filesystem::path& Directory);

	const FHarnessTuning& Harness(EHarnessClass Class) const { return HarnessRows[static_cast<size_t>(Class)]; }
	const FCutterTuning& Cutter(ECutterVariant Variant) const { return CutterRows[static_cast<size_t>(Variant)]; }
	const FThresholds& Impact(EImpactTable Table) const { return ImpactRows[static_cast<size_t>(Table)]; }
	double SurfaceMultiplier(ESurface Surface) const { return SurfaceRows[static_cast<size_t>(Surface)]; }
	double BreakFreeN(EPlacement Placement) const { return BreakFreeRows[static_cast<size_t>(Placement)]; }
	const FVolatileTuning& Volatile(EVolatile Kind) const { return VolatileRows[static_cast<size_t>(Kind)]; }
	const FCrushRule& Crush() const { return CrushRule; }
	double LedgeGrabMaxSpeed() const { return LedgeGrabMax; }
	const FMassTierBounds& MassTiers() const { return TierBounds; }
	const FSpoolGlobals& Globals() const { return Spool; }

private:
	std::array<FHarnessTuning, 5> HarnessRows{};
	std::array<FCutterTuning, 3> CutterRows{};
	std::array<FThresholds, 5> ImpactRows{};
	std::array<double, 5> SurfaceRows{};
	std::array<double, 6> BreakFreeRows{};
	std::array<FVolatileTuning, 4> VolatileRows{};
	FCrushRule CrushRule{};
	double LedgeGrabMax = 0;
	FMassTierBounds TierBounds{};
	FSpoolGlobals Spool{};
};
} // namespace Kinetic
