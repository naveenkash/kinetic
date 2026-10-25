// Kinematics, force volumes and tells (docs/tech/kinematics-and-force-volumes.md; bible/05 6.6,
// 6.7 and 11), checked against the Yard's timetable and Gantry cycle
// (docs/arenas/the-yard/events-and-moving-parts.md) and the 6.6 worked examples.
//
// Time bases: kinematics run on match time e, seconds since "live", as the Yard document
// writes them; MidMatchEventTime runs on the round clock, which starts with the Drop (see the
// open inconsistency in kinematics-and-force-volumes.md section 4).
#include "Kinetic/ForceVolumes.h"
#include "Kinetic/Kinematics.h"
#include "Kinetic/Tells.h"
#include "SimFixtures.h"

#include <algorithm>
#include <cmath>

using namespace Kinetic;
using namespace Kinetic::Test;

namespace
{
constexpr double Pi = 3.14159265358979323846;
constexpr double HaulerSpeed = 12.0;  // m/s

// A South trench crossing, X -80 to +80 at Y -30: 13.3 s nose-in to nose-out.
FPolylinePath SouthTrench() { return FPolylinePath({{-80, -30, 0}, {80, -30, 0}}); }

// The Yard's South timetable in Scrap: every 45 s from e 10 until Shift Change resets the
// timetables at e 80 (trains not yet departed are cancelled), then e 83 and every 22.5 s, and
// from e 140 back to 45 s counted from the last train (e 128).
FTimetable SouthScrap() { return FTimetable({{10, 45, 80}, {83, 22.5, 140}, {173, 45}}); }

// The Gantry's 120 s cycle as bridge X, trolley Y and load height.
FCycle Gantry(double PhaseAtLive)
{
	return FCycle({{0, {-50, 14, 3}},
	               {2, {-50, 14, 3}},
	               {10, {-50, 14, 30}},
	               {70, {45, 0, 30}},
	               {80, {45, 0, 12}},
	               {85, {45, 0, 34}},
	               {120 - 1e-6, {-50, 14, 34}}},
	              120.0, PhaseAtLive);
}

// 14 m by 5 m behind the nose, the load riding at the trench lip.
const FBox Train{{7.0, 2.5, 1.5}, {-7.0, 0.0, 0.0}};

// The first time in [From, To) the run's box crosses segment A-B, or -1.
double FirstCut(const FRailRun& Run, double From, double To, const FVec3& A, const FVec3& B)
{
	for (double T = From; T < To; T += StepSeconds)
	{
		const auto Poses = Run.SweepPoses(T, T + StepSeconds);
		if (Poses && SweptSever(Train, Poses->first, Poses->second, 4, A, B))
		{
			return T;
		}
	}
	return -1.0;
}
} // namespace

KTEST(HaulersRunTheirTimetable)
{
	const FRailRun South(SouthTrench(), HaulerSpeed, SouthScrap());
	KCHECK_NEAR(South.NoseOutSeconds(), 160.0 / 12.0, 1e-9);  // 13.3 s nose-in to nose-out
	KCHECK_NEAR(South.ClearSeconds(), 14.5, 1e-9);            // 14.5 s until the tail clears
	KCHECK(!South.IsRunning(9.99) && !South.PoseAt(9.99));    // In the plenum: no pose at all
	KCHECK(South.IsRunning(10.0));
	KCHECK_NEAR(South.PoseAt(14.2)->Position.X, -80 + 4.2 * 12, 1e-9);  // About X -30 at t0 + 4.2
	KCHECK(South.IsRunning(10 + 14.0));                                 // The tail still in the trench
	KCHECK_NEAR(South.PoseAt(10 + 14.0)->Position.X, -80 + 14.0 * 12, 1e-9);
	KCHECK(!South.IsRunning(10 + 14.6));
	KCHECK_NEAR(South.VelocityAt(12.0).X, 12.0, 1e-9);
	KCHECK(South.VelocityAt(30.0).Length() == 0.0);
	KCHECK(South.IsRunning(55.0) && !South.IsRunning(54.9));
}

KTEST(HaulersFaceTheirWay)
{
	// The Center trench runs east to west: the box behind the nose is behind it, to the east.
	const FRailRun Center(FPolylinePath({{80, 0, 0}, {-80, 0, 0}}), HaulerSpeed, FTimetable({{25, 45}}));
	KCHECK_NEAR(std::abs(Center.PoseAt(26.0)->Angle), Pi, 1e-12);
	const double Cut = FirstCut(Center, 25.0, 40.0, {40, -6, 1}, {40, 6, 1});  // A line across at X 40
	KCHECK(Cut > 0.0);
	KCHECK_NEAR(Center.PoseAt(Cut)->Position.X, 40.0, 0.25);  // The nose reaches it: e 25 + 40 / 12
}

KTEST(AHeadingWrapTurnsTheShortWay)
{
	// Westbound with a slight kink: the heading crosses ±π between steps. The box must not spin
	// round the nose: a line 10 m ahead is cut only when the nose reaches it.
	const FRailRun Kinked(FPolylinePath({{80, 0, 0}, {0, 0.7, 0}, {-80, 0, 0}}), HaulerSpeed, FTimetable({{0, 45}}));
	const double Cut = FirstCut(Kinked, 0.0, 15.0, {-10, -6, 1}, {-10, 6, 1});
	KCHECK(Cut > 0.0);
	KCHECK_NEAR(Kinked.PoseAt(Cut)->Position.X, -10.0, 0.25);
	const FPose Mid = Lerp({{}, Up, 3.1}, {{}, Up, -3.1}, 0.5);
	KCHECK_NEAR(std::abs(Mid.Angle), Pi, 1e-9);
}

KTEST(ShiftChangeResetsTheTimetables)
{
	const FTimetable South = SouthScrap();
	KCHECK(South.DeparturesBetween(0, 200) == std::vector<double>({10, 55, 83, 105.5, 128, 173}));
	KCHECK_NEAR(*South.LastDepartureAt(100.0), 83.0, 1e-9);
	KCHECK(!South.LastDepartureAt(9.0));
	// All three trenches during Shift Change: a train somewhere every 7.5 s.
	const FTimetable Center({{25, 45, 80}, {90.5, 22.5, 140}});
	const FTimetable North({{40, 45, 80}, {98, 22.5, 140}});
	std::vector<double> All;
	for (const FTimetable* T : {&South, &Center, &North})
	{
		for (const double D : T->DeparturesBetween(80, 140))
		{
			All.push_back(D);
		}
	}
	std::sort(All.begin(), All.end());
	for (size_t I = 1; I < All.size(); ++I)
	{
		KCHECK_NEAR(All[I] - All[I - 1], 7.5, 1e-9);
	}
}

KTEST(TimetableEdges)
{
	// A segment ending on its own grid: the departure at Until is cancelled, both ways of asking.
	const FTimetable T({{10, 45, 100}});
	KCHECK(T.DeparturesBetween(0, 200) == std::vector<double>({10, 55}));
	KCHECK_NEAR(*T.LastDepartureAt(101.0), 55.0, 1e-12);
	KCHECK_NEAR(*T.LastDepartureAt(55.0 - 1e-8), 10.0, 1e-12);  // Never a departure after T
	KCHECK_NEAR(*T.LastDepartureAt(55.0), 55.0, 1e-12);         // Exactly on it counts
	const FRailRun Run(SouthTrench(), HaulerSpeed, T);
	KCHECK(!Run.IsRunning(101.0));
	// A grid point that floating point puts a hair under T (0.3 / 0.1 is 2.9999999999999996).
	KCHECK_NEAR(*FTimetable({{0, 0.1}}).LastDepartureAt(0.3), 0.3, 1e-12);
	// A long grid before Until is not walked a point at a time (and is still right).
	KCHECK_NEAR(*FTimetable({{0, 0.01, 1}}).LastDepartureAt(1000.0), 0.99, 1e-9);
	// Departures closer than a train clears: a step across the new departure is not a sweep.
	const FRailRun Close(SouthTrench(), HaulerSpeed, FTimetable({{0, 10}}));
	KCHECK(!Close.SweepPoses(9.99, 10.01));
}

KTEST(TheGantryCycle)
{
	// Scrap: phase 72 at live, so the first pit drop (c = 80) lands at e 8.
	const FCycle Scrap = Gantry(72.0);
	KCHECK_NEAR(Scrap.CycleTimeAt(8.0), 80.0, 1e-9);
	KCHECK_NEAR(Scrap.CycleTimeAt(48.0), 0.0, 1e-9);  // Pick at e 48
	const FCycle C = Gantry(0.0);
	KCHECK_NEAR(C.PositionAt(40.0).X, -50 + 95.0 * 30 / 60, 1e-9);   // Over Marker Flat at c 40
	KCHECK_NEAR(C.VelocityAt(40.0).X, 95.0 / 60.0, 1e-6);            // About 1.6 m/s east
	KCHECK_NEAR(C.PositionAt(160.0).X, C.PositionAt(40.0).X, 1e-9);  // It repeats
	KCHECK_NEAR(C.PositionAt(78.0).Z, 30 - 18 * 0.8, 1e-9);          // Lowering toward the drop
	KCHECK(C.CycleTimeAt(-1e-20) < 120.0);                           // [0, Period), always
	// Two keys at one time are a step, kept in their authored order.
	const FCycle Step({{0, {0, 0, 0}}, {5, {0, 0, 0}}, {5, {10, 0, 0}}, {10, {10, 0, 0}}}, 10.0, 0.0);
	KCHECK_NEAR(Step.PositionAt(4.99).X, 0.0, 1e-9);
	KCHECK_NEAR(Step.PositionAt(5.01).X, 10.0, 1e-9);
}

KTEST(RingAndHullRotations)
{
	const FSpin Ring{{0, 0, 0}, Up, 120.0};
	KCHECK_NEAR(Ring.AngleAt(30.0), Pi / 2, 1e-12);
	const FPose OnRing = Ring.PoseOf({{100, 0, 0}, Up, 0.0}, 30.0);
	KCHECK_NEAR(OnRing.Position.X, 0.0, 1e-9);
	KCHECK_NEAR(OnRing.Position.Y, 100.0, 1e-9);
	const FRock Hull;
	KCHECK_NEAR(Hull.AngleAt(22.5), 6.0 * Pi / 180.0, 1e-12);  // Full lean a quarter cycle in
	KCHECK_NEAR(Hull.AngleAt(45.0), 0.0, 1e-12);
	KCHECK_NEAR(Hull.AngleAt(67.5), -6.0 * Pi / 180.0, 1e-12);
	const FPose Tilted = Hull.PoseOf({{0, 10, 0}, Hull.Axis, 0.0}, 22.5);
	KCHECK_NEAR(Tilted.Position.Z, 10 * std::sin(6.0 * Pi / 180.0), 1e-9);  // The +Y side rises
}

KTEST(AHaulerSeversLinesItPassesThrough)
{
	const FRailRun South(SouthTrench(), HaulerSpeed, SouthScrap());
	const FVec3 BridgeA{0, -36, 1}, BridgeB{0, -24, 1};  // A Hang line across the trench at X 0
	const double CutAt = FirstCut(South, 10.0, 25.0, BridgeA, BridgeB);
	// Cut as the nose reaches the bridge at X 0, 80 m in: e 10 + 6.7 s.
	KCHECK(CutAt > 0.0);
	KCHECK_NEAR(South.PoseAt(CutAt)->Position.X, 0.0, 0.25);
	// A set line along the lip, 0.3 m outside: the 5 m load inside a 6 m trench clears it.
	KCHECK(FirstCut(South, 10.0, 25.0, {0, -27.2, 1}, {20, -27.2, 1}) < 0.0);
	// Where along the segment it crosses: the point the line enters the box.
	const std::optional<double> Along = SegmentHitsBox(Train, {{7, -30, 0}, Up, 0.0}, BridgeA, BridgeB);
	KCHECK(Along && *Along > 0.25 && *Along < 0.5);
}

KTEST(ADespawnSeversNothing)
{
	const FRailRun South(SouthTrench(), HaulerSpeed, SouthScrap());
	// A bridge laid at X -45 after the train has passed is never cut, through the despawn and
	// while the next train is parked; a line across the trench mouth is not cut while parked.
	KCHECK(FirstCut(South, 20.0, 54.0, {-45, -36, 1}, {-45, -24, 1}) < 0.0);
	KCHECK(FirstCut(South, 26.0, 54.0, {-94, -30, 1}, {-80.5, -30, 1}) < 0.0);
	KCHECK(!South.SweepPoses(24.4, 24.6));  // Across the tail clearing: nothing to sweep
	KCHECK(!South.SweepPoses(54.9, 55.1));  // Across a spawn: nothing to sweep
}

KTEST(SubstepsCatchFastThinSweeps)
{
	// A 0.3 m box moving 1 m a step past a line: one sample a step misses it, four catch it.
	const FBox Thin{{0.15, 0.15, 0.15}, {}};
	const FPose From{{0, 0, 0}, Up, 0.0};
	const FPose To{{1, 0, 0}, Up, 0.0};
	const FVec3 A{0.4, -1, 0}, B{0.4, 1, 0};
	KCHECK(!SweptSever(Thin, From, To, 1, A, B));
	KCHECK(SweptSever(Thin, From, To, 4, A, B).has_value());
}

KTEST(TheHaulSledRatchets)
{
	FHaulSled Sled(FPolylinePath({{0, 0, 0}, {0, 0, 72}}));  // Up the Spine
	const FVec3 ClimberPull{0, 0, 400};
	for (int I = 0; I < 600; ++I)
	{
		Sled.Step(ClimberPull, StepSeconds);
	}
	KCHECK_NEAR(Sled.Speed(), (400.0 - 200.0) / 4000.0 * 10.0, 1e-9);  // 0.05 m/s², 10 s
	// Pulled back down: the sled slows to a stop and never runs back.
	double Peak = Sled.Distance();
	for (int I = 0; I < 600; ++I)
	{
		Sled.Step({0, 0, -3000}, StepSeconds);
		KCHECK(Sled.Distance() >= Peak);
		Peak = Sled.Distance();
	}
	KCHECK(Sled.Speed() == 0.0);
	const double Held = Sled.Distance();
	Sled.Step({0, 0, -3000}, StepSeconds);
	KCHECK(Sled.Distance() == Held);
	FHaulSled Crew(FPolylinePath({{0, 0, 0}, {0, 0, 72}}));
	FHaulSled Solo(FPolylinePath({{0, 0, 0}, {0, 0, 72}}));
	for (int I = 0; I < 600; ++I)
	{
		Crew.Step({0, 0, 1200}, StepSeconds);
		Solo.Step(ClimberPull, StepSeconds);
	}
	KCHECK_NEAR(Crew.Distance() / Solo.Distance(), 5.0, 1e-6);  // Three lines: five times the net force
	KCHECK(Crew.TeethPassed() == static_cast<int>(Crew.Distance() / 0.25));
	Crew.Reset();
	KCHECK(Crew.Distance() == 0.0 && Crew.TeethPassed() == 0);
}

KTEST(TheClientClockSyncsAndSmooths)
{
	FServerClock Clock;
	KCHECK_NEAR(Clock.Now(5000.0), 5000.0, 1e-12);  // No estimate yet: local time, remembered nowhere
	Clock.Sample(100.0, 0.080, 5000.1);
	KCHECK_NEAR(Clock.Now(5001.0), 100.04 + 0.9, 1e-9);  // Synced at once, not frozen at 5000
	// 10 ms off (offset -4900.05 against -4900.06): a tenth of it is smoothed in, no jump.
	Clock.Sample(101.0, 0.080, 5001.09);
	KCHECK_NEAR(Clock.Now(5001.09), 5001.09 - 4900.06 + 0.001, 1e-9);
}

KTEST(TheClientClockSlewsAStep)
{
	FServerClock Clock;
	Clock.Sample(100.0, 0.0, 0.0);  // Offset 100
	double Previous = Clock.Now(0.0);
	double Converged = -1.0;
	// From local 1 s the server is 300 ms ahead; a sample every 0.5 s, a frame every 10 ms.
	for (int Frame = 1; Frame <= 400; ++Frame)
	{
		const double Local = Frame * 0.01;
		if (Frame % 50 == 0)
		{
			Clock.Sample(Local + 100.0 + (Local >= 1.0 ? 0.3 : 0.0), 0.0, Local);
		}
		const double Now = Clock.Now(Local);
		KCHECK(Now >= Previous);
		KCHECK(Now - Previous <= 0.01 + 0.3 * 0.01 / FServerClock::ResnapSeconds + 1e-9);  // Slewed, never jumped
		if (Converged < 0.0 && std::abs(Now - (Local + 100.3)) < 1e-6)
		{
			Converged = Local;
		}
		Previous = Now;
	}
	KCHECK(Converged > 0.0 && Converged <= 1.5 + FServerClock::ResnapSeconds + 1e-9);  // Two samples, then 0.5 s
}

KTEST(TheClientClockIgnoresASpikeAndNeverRunsBack)
{
	FServerClock Clock;
	Clock.Sample(100.0, 0.0, 0.0);
	Clock.Sample(1.0 + 100.4, 0.0, 1.0);  // One late packet
	Clock.Sample(1.5 + 100.0, 0.0, 1.5);  // Then back to normal
	KCHECK_NEAR(Clock.Now(1.5), 101.5, 1e-9);
	// The server is 300 ms behind from now on: the estimate slows to meet it, never backward.
	double Previous = Clock.Now(2.0);
	for (int Frame = 201; Frame <= 600; ++Frame)
	{
		const double Local = Frame * 0.01;
		if (Frame % 50 == 0)
		{
			Clock.Sample(Local + 99.7, 0.0, Local);
		}
		const double Now = Clock.Now(Local);
		KCHECK(Now >= Previous);
		Previous = Now;
	}
	KCHECK_NEAR(Clock.Now(6.0), 6.0 + 99.7, 1e-6);
}

KTEST(ForceVolumesScaleByMassAndCap)
{
	const FSpoolGlobals& Globals = Tables().Globals();
	const FUpdraft Updraft;
	const FVec3 Core = Updraft.ARefAt({0, 0, 10}, 0.0);
	KCHECK_NEAR(Core.Z, 14.0, 1e-12);
	// 6.6: a 90 kg Climber 15.6 m/s², a 220 kg Breaker 6.4, a 2,000 kg container 0.7. A 45 kg
	// cylinder would feel 31 but is capped at 2 × a_ref, 28.
	KCHECK_NEAR(BodyAcceleration(Core, 90, EVolumeTarget::Player, Globals).Z, 14.0 * 100 / 90, 1e-9);
	KCHECK_NEAR(BodyAcceleration(Core, 220, EVolumeTarget::Player, Globals).Z, 6.36, 0.01);
	KCHECK_NEAR(BodyAcceleration(Core, 2000, EVolumeTarget::LooseObject, Globals).Z, 0.7, 1e-9);
	KCHECK_NEAR(BodyAcceleration(Core, 45, EVolumeTarget::LooseObject, Globals).Z, 28.0, 1e-9);
	KCHECK_NEAR(BodyAcceleration(Core, 45, EVolumeTarget::LineHeldObject, Globals).Z, 28.0, 1e-9);
	for (const EVolumeTarget T : {EVolumeTarget::PlacedObject, EVolumeTarget::RatedObject, EVolumeTarget::CableHung,
	                              EVolumeTarget::Kinematic})
	{
		KCHECK(BodyAcceleration(Core, 45, T, Globals).Length() == 0.0);
	}
	KCHECK_NEAR(Updraft.ARefAt({30, 0, 10}, 0.0).Z, 7.0, 1e-9);  // Half way to 60 m
	KCHECK(Updraft.ARefAt({61, 0, 10}, 0.0).Z == 0.0);
	FUpdraft Banded;
	Banded.Band = {0.0, 60.0};
	KCHECK(Banded.ARefAt({0, 0, 61}, 0.0).Length() == 0.0);  // Above its height band
}

KTEST(TheUpdraftGustsOnItsCycle)
{
	FUpdraft Updraft;
	Updraft.Gust.Keys = {{0, 0}, {20, 0}, {22, 1}, {30, 1}, {32, 0}};
	KCHECK(Updraft.ARefAt({0, 0, 0}, 10.0).Z == 0.0);
	KCHECK_NEAR(Updraft.ARefAt({0, 0, 0}, 21.0).Z, 7.0, 1e-9);
	KCHECK_NEAR(Updraft.ARefAt({0, 0, 0}, 65.0).Z, 14.0, 1e-9);  // The next cycle
	KCHECK(Updraft.GustStarts(0, 100) == std::vector<double>({20, 60}));
	// A gust that rises across the cycle's wrap: interpolated through it, and its start found.
	FUpdraft Wrapped;
	Wrapped.Gust.Keys = {{0, 1}, {8, 1}, {10, 0}, {38, 0}};
	KCHECK_NEAR(Wrapped.ARefAt({0, 0, 0}, 39.0).Z, 7.0, 1e-9);  // Half way from 0 at 38 to 1 at 40
	KCHECK(Wrapped.GustStarts(0, 100) == std::vector<double>({38, 78}));
	// Keys starting after 0: before the first key the curve is still coming from the last.
	FUpdraft Late;
	Late.Gust.Keys = {{10, 0}, {20, 1}, {30, 0.5}};
	KCHECK_NEAR(Late.ARefAt({0, 0, 0}, 5.0).Z, 14.0 * 0.5 * (1.0 - 15.0 / 20.0), 1e-9);  // From 0.5 at 30 to 0 at 50
}

KTEST(ThePulseShovesThenInhales)
{
	const FPulse Pulse;
	const FVec3 P{60, 0, 0};  // A quarter of the way to the Rim
	KCHECK(Pulse.ARefAt(P, 54.9).Length() == 0.0);
	KCHECK_NEAR(Pulse.ARefAt(P, 55.2).X, 20.0 * 0.75, 1e-9);  // Outward
	KCHECK_NEAR(Pulse.ARefAt(P, 55.7).X, -8.0 * 0.75, 1e-9);  // Inward
	KCHECK(Pulse.ARefAt(P, 56.1).Length() == 0.0);
	KCHECK_NEAR(Pulse.ARefAt(P, 130.1).X, 15.0, 1e-9);        // Every 75 s
	KCHECK(Pulse.ARefAt({240, 0, 0}, 55.2).Length() == 0.0);   // Nothing at the Rim
	KCHECK(Pulse.ARefAt({60, 0, 80}, 55.2).Length() == 0.0);   // Nor above Z 70
	KCHECK(Pulse.ARefAt({0, 0, 0}, 55.2).Length() == 0.0);     // At the axis: no direction, no NaN
}

KTEST(JetsFireAndWater)
{
	const FGasJet Jet{{0, 0, 0}, {1, 0, 0}};
	// A 100 kg body from rest reaches 15 m/s in 3 s, then the jet stops pushing it.
	double V = 0.0;
	for (double T = 0.0; T < 3.0; T += StepSeconds)
	{
		V += Jet.ARefAt({3, 0, 0}, {V, 0, 0}, T, 0.0).X * StepSeconds;
	}
	KCHECK_NEAR(V, 15.0, 0.1);
	KCHECK(Jet.ARefAt({3, 0, 0}, {15.0, 0, 0}, 1.0, 0.0).Length() == 0.0);
	KCHECK(Jet.ARefAt({3, 2, 0}, {}, 1.0, 0.0).Length() == 0.0);  // Outside the cone
	KCHECK(Jet.ARefAt({3, 0, 0}, {}, 3.1, 0.0).Length() == 0.0);  // After 3 s
	const FFirePatch Fire{{0, 0, 0}};
	KCHECK(Fire.Contains({2, 0, 0}, 1.0, 0.0) && !Fire.Contains({2, 0, 0}, 6.1, 0.0));
	KCHECK(!Fire.Contains({2, 0, 30}, 1.0, 0.0));  // A catwalk overhead is not in it
	KCHECK_NEAR(Fire.ARefAt({2, 0, 0}, {}, 1.0, 0.0).X, 8.0, 1e-12);
	KCHECK(Fire.ARefAt({2, 0, 0}, {4, 0, 0}, 1.0, 0.0).Length() == 0.0);  // Already moving away at 4 m/s
	const FWaterRush Rush{{0, 0, 0}, {10, 10, 5}, {1, 0, 0}};
	KCHECK(Rush.ARefAt({5, 5, 1}, 5.9, 0.0).Length() == 0.0);
	KCHECK_NEAR(Rush.ARefAt({5, 5, 1}, 6.0, 0.0).X, 12.0, 1e-12);
	KCHECK(Rush.ARefAt({5, 5, 1}, 10.0, 0.0).Length() == 0.0);
}

KTEST(TellLeadsAreChecked)
{
	std::vector<FTellEvent> Yard = {
	    {"South_Departure", ETellSystem::HaulerApproach, 10, 45, 80, 3.0, 3.0},
	    {"Gantry_Drop", ETellSystem::GantryDrop, 8, 120, 1e300, 2.0, 2.0},
	    {"Shift_Change", ETellSystem::MidMatchEvent, 80, 0, 1e300, 5.0, 5.0},  // The Yard's e 80
	};
	KCHECK(TimelineProblems(Yard).empty());
	Yard[0].LeadAudio = 2.5;  // A Hauler klaxon at 2.5 s
	Yard.push_back({"Gust", ETellSystem::ForceVolume, 0, 0.5, 1e300, 1.0, 1.0});
	const std::vector<std::string> Problems = TimelineProblems(Yard);
	KCHECK(Problems.size() == 2);
	KCHECK(Problems[0] == "South_Departure: lead under the hauler_approach minimum");
	KCHECK(Problems[1] == "Gust: repeats before its own cue");
	KCHECK(Occurrences(Yard[0], 0, 100) == std::vector<double>({10, 55}));
}

KTEST(TheSchedulerFiresEachCueOnce)
{
	FTellScheduler Scheduler({{"South_Departure", ETellSystem::HaulerApproach, 10, 45, 1e300, 3.0, 2.0}});
	int Audio = 0;
	int Visual = 0;
	double AudioAt = -1.0;
	for (double T = 0.0; T < 60.0; T += StepSeconds)
	{
		for (const FCue& C : Scheduler.Advance(T))
		{
			(C.Kind == ECueKind::Audio ? Audio : Visual) += 1;
			if (C.Kind == ECueKind::Audio && AudioAt < 0)
			{
				AudioAt = T;
			}
		}
	}
	KCHECK(Audio == 2 && Visual == 2);  // e 10 and e 55
	KCHECK_NEAR(AudioAt, 7.0, StepSeconds + 1e-9);  // 3 s ahead
	// A client that joins 1 s before a train hears its klaxon at once; one that joins after
	// the train has gone hears nothing for it.
	FTellScheduler Late({{"South_Departure", ETellSystem::HaulerApproach, 10, 45, 1e300, 3.0, 3.0}});
	KCHECK(Late.Advance(9.0).size() == 2);
	FTellScheduler After({{"South_Departure", ETellSystem::HaulerApproach, 10, 45, 1e300, 3.0, 3.0}});
	KCHECK(After.Advance(10.5).empty());
	KCHECK(After.Advance(10.4).empty());  // Time never runs backward
	KCHECK(After.Advance(52.0).size() == 2);
}

KTEST(TheSchedulerDropsCuesAFrameGapSkipped)
{
	// Running, then a 6 s hitch spans both a klaxon and its train: the late klaxon is dropped.
	FTellScheduler S({{"South_Departure", ETellSystem::HaulerApproach, 10, 45, 1e300, 3.0, 3.0}});
	KCHECK(S.Advance(5.0).empty());
	KCHECK(S.Advance(11.0).empty());
	KCHECK(S.Advance(52.5).size() == 2);  // The next train's cue fires as normal
}

KTEST(TheSchedulerTakesEventsMidMatch)
{
	FTellScheduler S({{"South_Departure", ETellSystem::HaulerApproach, 10, 45, 1e300, 3.0, 3.0}});
	KCHECK(S.Advance(7.0).size() == 2);  // The klaxon
	// The mid-match event is replicated at round start; added now, it fires its own cue once it
	// is due and nothing already fired fires again.
	const size_t Shift = S.AddEvent({"Shift_Change", ETellSystem::MidMatchEvent, 80, 0, 1e300, 5.0, 5.0});
	KCHECK(S.Advance(8.0).empty());
	int Fired = 0;
	for (double T = 8.0; T < 81.0; T += StepSeconds)
	{
		for (const FCue& C : S.Advance(T))
		{
			Fired += C.EventIndex == Shift ? 1 : 0;
		}
	}
	KCHECK(Fired == 2);
	// Added inside its own lead window, it fires at once, as a late join would.
	FTellScheduler Inside({});
	Inside.Advance(76.0);
	Inside.AddEvent({"Shift_Change", ETellSystem::MidMatchEvent, 80, 0, 1e300, 5.0, 5.0});
	KCHECK(Inside.Advance(77.0).size() == 2);
}

KTEST(AZeroLeadCueFiresAtItsMoment)
{
	FTellScheduler S({{"Seat", ETellSystem::DogSeated, 10, 0, 1e300, 0.0, 0.0}});
	KCHECK(S.Advance(9.99).empty());
	KCHECK(S.Advance(10.01).size() == 2);
	KCHECK(S.Advance(10.02).empty());
	// Added between frames, it still fires as its moment is crossed.
	FTellScheduler Added({});
	Added.Advance(9.99);
	Added.AddEvent({"Seat", ETellSystem::DogSeated, 10, 0, 1e300, 0.0, 0.0});
	KCHECK(Added.Advance(10.01).size() == 2);
	KCHECK(Added.Advance(10.02).empty());
}

KTEST(EventTellsHoldTheirConsequence)
{
	FTellGate Gate;
	KCHECK(!Gate.MayFire(ETellSystem::RatedLoadMoving, 1, 5.0));  // Never cued
	Gate.Cue(ETellSystem::RatedLoadMoving, 1, 5.0);
	Gate.Cue(ETellSystem::VolatileArmed, 7, 5.0);
	KCHECK(!Gate.MayFire(ETellSystem::RatedLoadMoving, 1, 5.99));
	KCHECK(Gate.MayFire(ETellSystem::RatedLoadMoving, 1, 6.0));   // 1.0 s after the groan
	KCHECK(!Gate.MayFire(ETellSystem::RatedLoadMoving, 2, 6.0));  // Another load was never groaned
	KCHECK(Gate.MayFire(ETellSystem::VolatileArmed, 7, 5.3));     // 0.3 s after the hiss
	KCHECK_NEAR(*Gate.FiresAt(ETellSystem::VolatileArmed, 7), 5.3, 1e-12);
	// The same load groans again next round: the new groan is what counts.
	Gate.Cue(ETellSystem::RatedLoadMoving, 1, 100.0);
	KCHECK(!Gate.MayFire(ETellSystem::RatedLoadMoving, 1, 100.5));
	KCHECK(Gate.MayFire(ETellSystem::RatedLoadMoving, 1, 101.0));
	Gate.Clear(ETellSystem::RatedLoadMoving, 1);
	KCHECK(!Gate.MayFire(ETellSystem::RatedLoadMoving, 1, 200.0));
	Gate.Reset();
	KCHECK(!Gate.MayFire(ETellSystem::VolatileArmed, 7, 200.0));
}

KTEST(MidMatchEventsFallOnTheRoundClock)
{
	// 05 9.2 and game-modes: the round clock starts with the Drop; the siren leads by 5 s.
	KCHECK_NEAR(MidMatchEventTime(EMatchMode::Scrap, 0, RoundSecondsOf(EMatchMode::Scrap)), 75.0, 1e-12);
	KCHECK_NEAR(MidMatchEventTime(EMatchMode::Claim, 0, RoundSecondsOf(EMatchMode::Claim)), 240.0, 1e-12);
	KCHECK_NEAR(MidMatchEventTime(EMatchMode::Haul, 240, RoundSecondsOf(EMatchMode::Haul)), 360.0, 1e-12);
	KCHECK_NEAR(MidMatchEventTime(EMatchMode::FreeScrap, 0, RoundSecondsOf(EMatchMode::FreeScrap)), 180.0, 1e-12);
	KCHECK(MinimumLead(ETellSystem::MidMatchEvent) == 5.0);
}
