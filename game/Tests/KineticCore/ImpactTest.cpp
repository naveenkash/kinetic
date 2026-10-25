// Impact classification (bible/05 sections 6.4, 7.1 to 7.3; player-physics section 4) and
// the Cutter (4.1, 4.2).
#include "Kinetic/Cutter.h"
#include "Kinetic/Impact.h"
#include "Kinetic/Spool.h"
#include "TestHarness.h"

#include <cmath>
#include <numbers>

using namespace Kinetic;
using namespace Kinetic::Test;

namespace
{
constexpr double Dt = 1.0 / 60.0;

// An object hitting the player from the side: none of its weight on the player.
FContact ObjectContact(int OtherId, double Speed, double ImpulseNs)
{
	return {OtherId, EContactKind::Object, Speed, ImpulseNs, 0.0, ESurface::Hard, {1, 0, 0}};
}

EOutcome ObjectHit(double MassKg, double Speed)
{
	return Classify(EImpactTable::ObjectVsPlayer, ImpactImpulse(MassKg, Speed), ESurface::Hard, Tables());
}

FVec3 Heading(double Degrees)
{
	const double R = Degrees * std::numbers::pi / 180.0;
	return {std::cos(R), std::sin(R), 0.0};
}
} // namespace

// 7.1's impulse examples.
KTEST(BibleImpulseExamples)
{
	KCHECK(ObjectHit(45, 25) == EOutcome::Flip);    // 1,125 N·s: a Hauled cylinder at the cap
	KCHECK(ObjectHit(45, 27) == EOutcome::Stall);   // 1,215 N·s off a Jack push
	KCHECK(ObjectHit(180, 7) == EOutcome::Stall);   // A drum at 7 m/s, 1,260
	KCHECK(ObjectHit(12, 100) == EOutcome::Stall);  // A plank needs 100 m/s
	KCHECK(ObjectHit(2000, 0.6) == EOutcome::Stall);  // A container does not need speed
	KCHECK(ObjectHit(20, 10) == EOutcome::None);
	// The cut recoil never staggers (4.2).
	const double MaxRecoil = CutRecoilNs(1200.0, Tables().Globals());
	KCHECK(Classify(EImpactTable::ObjectVsPlayer, MaxRecoil, ESurface::Hard, Tables()) == EOutcome::None);
	// The full overspin snap Stalls (3.7).
	const double Snap = SnapImpulseNs(1200.0, Tables().Globals());
	KCHECK(Classify(EImpactTable::ObjectVsPlayer, Snap, ESurface::Hard, Tables()) == EOutcome::Stall);
}

KTEST(SpeedThresholdsAndSurfaces)
{
	KCHECK(Classify(EImpactTable::PlayerVsStatic, 12.0, ESurface::Hard, Tables()) == EOutcome::Stall);
	KCHECK(Classify(EImpactTable::PlayerVsStatic, 11.99, ESurface::Hard, Tables()) == EOutcome::Flip);
	KCHECK(Classify(EImpactTable::PlayerVsStatic, 12.0, ESurface::Wood, Tables()) == EOutcome::Flip);   // Needs 14.4
	KCHECK(Classify(EImpactTable::PlayerVsStatic, 30.0, ESurface::DeepWater, Tables()) == EOutcome::Flip);
	KCHECK(Classify(EImpactTable::PlayerVsStatic, 19.0, ESurface::StallGel, Tables()) == EOutcome::None);  // Stagger at 20
	KCHECK(Classify(EImpactTable::PlayerVsKinematic, 9.0, ESurface::Hard, Tables()) == EOutcome::Stall);
	KCHECK(Classify(EImpactTable::PlayerVsKinematic, 6.0, ESurface::Hard, Tables()) == EOutcome::Flip);
	KCHECK(Classify(EImpactTable::HangLineHit, 3.0, ESurface::Hard, Tables()) == EOutcome::Stagger);
	KCHECK(Classify(EImpactTable::HangLineHit, 40.0, ESurface::Hard, Tables()) == EOutcome::Flip);  // Never a Stall
}

// 7.2: a Breaker hitting a Climber at 10 m/s Stalls the Climber and flips the Breaker.
KTEST(MassMattersOnBothEnds)
{
	KCHECK(ClassifyPlayerVsPlayer(220.0, 10.0, Tables()) == EOutcome::Stall);  // The Climber's side
	KCHECK(ClassifyPlayerVsPlayer(90.0, 10.0, Tables()) == EOutcome::Flip);    // The Breaker's side
	KCHECK(ClassifyPlayerVsPlayer(90.0, 14.0, Tables()) == EOutcome::Stall);   // The symmetric floor
	KCHECK(ClassifyPlayerVsPlayer(90.0, 13.0, Tables()) == EOutcome::Flip);
}

KTEST(ImpactMonitorSumsPerBodyOnly)
{
	FImpactMonitor Same(Tables());
	const std::vector<FContact> OneBody{ObjectContact(1, 3, 700), ObjectContact(1, 3, 600)};
	KCHECK(Same.EndStep(OneBody, 0.0, Dt) == EOutcome::Stall);

	// A body's speed is its fastest contact point, not the first one reported.
	FImpactMonitor Points(Tables());
	KCHECK(Points.EndStep({{4, EContactKind::Static, 3, 0}, {4, EContactKind::Static, 12, 0}}, 0.0, Dt) ==
	       EOutcome::Stall);

	FImpactMonitor Two(Tables());
	const std::vector<FContact> TwoBodies{ObjectContact(1, 3, 700), ObjectContact(2, 3, 600)};
	KCHECK(Two.EndStep(TwoBodies, 0.0, Dt) == EOutcome::Flip);
}

// 7.2: the threshold against the world is speed, whatever the player's mass; a Breaker's
// solver impulse against a wall is 220 kg times the speed and must not decide it.
KTEST(StaticAndKinematicHitsAreJudgedOnSpeedAlone)
{
	FImpactMonitor Wall(Tables());
	KCHECK(Wall.EndStep({{1, EContactKind::Static, 5.5, 220 * 5.5}}, 0.0, Dt) == EOutcome::Stagger);
	FImpactMonitor HardWall(Tables());
	KCHECK(HardWall.EndStep({{1, EContactKind::Static, 11.9, 220 * 11.9}}, 0.0, Dt) == EOutcome::Flip);
	FImpactMonitor Rigger(Tables());
	KCHECK(Rigger.EndStep({{1, EContactKind::Static, 9.3, 130 * 9.3}}, 0.0, Dt) == EOutcome::Flip);
	FImpactMonitor Gantry(Tables());
	KCHECK(Gantry.EndStep({{1, EContactKind::Kinematic, 5.5, 220 * 5.5}}, 0.0, Dt) == EOutcome::Stagger);
	FImpactMonitor Landing(Tables());  // A 10 cm drop: under the stagger speed, control regained (5.1)
	KCHECK(Landing.EndStep({{1, EContactKind::Static, 1.4, 220 * 1.4}}, 0.0, Dt) == EOutcome::None);
}

// A heavy object resting on a player is a support force, not a hit, at any step rate; the
// same container arriving at 0.6 m/s Stalls (7.1).
KTEST(AHeavyLoadRestingOnAPlayerIsNotAHit)
{
	for (const double Step : {1.0 / 60.0, 1.0 / 30.0})
	{
		FImpactMonitor Monitor(Tables());
		const double Support = 2000.0 * Tables().Globals().Gravity * Step;
		for (int I = 0; I < 60; ++I)
		{
			const FContact Resting{5, EContactKind::Object, 0.0, Support, 2000.0, ESurface::Hard, {0, 0, -1}};
			KCHECK(Monitor.EndStep({Resting}, I * Step, Step) == EOutcome::None);
		}
		FImpactMonitor Dropped(Tables());
		const FContact OnTop{5, EContactKind::Object, 0.6, 2000.0 * 0.6 + Support, 2000.0, ESurface::Hard, {0, 0, -1}};
		KCHECK(Dropped.EndStep({OnTop}, 0.0, Step) == EOutcome::Stall);
		// Arriving sideways on rollers, the floor carries its weight: nothing is subtracted.
		FImpactMonitor Rolled(Tables());
		const FContact Side{5, EContactKind::Object, 0.6, 2000.0 * 0.6, 2000.0, ESurface::Hard, {1, 0, 0}};
		KCHECK(Rolled.EndStep({Side}, 0.0, Step) == EOutcome::Stall);
	}
}

// Only the share of an object's weight that bears down on the player is support: none for an
// object pushing up from below, and the body's normal is its contact points' normals summed.
KTEST(SupportIsTheWeightTheContactCarries)
{
	const double Step = 1.0 / 30.0;
	const double Support = 2000.0 * Tables().Globals().Gravity * Step;  // 654 N·s
	FImpactMonitor Below(Tables());
	KCHECK(Below.EndStep({{5, EContactKind::Object, 0.5, 310.0, 2000.0, ESurface::Hard, {0, 0, 1}}}, 0.0, Step) ==
	       EOutcome::Stagger);

	const double Share = Support / std::sqrt(2.0) * 0.5;  // Two points: (1,0,0) and (0,0,−1), summed
	const double Each = Share + 125.0;
	FImpactMonitor Corner(Tables());
	KCHECK(Corner.EndStep({{5, EContactKind::Object, 0.5, Each, 2000.0, ESurface::Hard, {1, 0, 0}},
	                       {5, EContactKind::Object, 0.5, Each, 2000.0, ESurface::Hard, {0, 0, -1}}},
	                      0.0, Step) == EOutcome::None);  // 250 N·s once the support is removed
}

KTEST(SupportShareByDirection)
{
	KCHECK_NEAR(SupportShare({0, 0, -1}), 1.0, 1e-12);
	KCHECK_NEAR(SupportShare({0, 0, -5}), 1.0, 1e-12);  // Any length
	KCHECK_NEAR(SupportShare({1, 0, 0}), 0.0, 1e-12);
	KCHECK_NEAR(SupportShare({0, 0, 1}), 0.0, 1e-12);
	KCHECK_NEAR(SupportShare({1, 0, -1}), std::sqrt(0.5), 1e-12);
	KCHECK_NEAR(SupportShare({}), 1.0, 0.0);  // Missing: counted as bearing down
}

// Player-physics 4.2's resting limits never matter: no threshold, on any surface, is under them.
KTEST(RestingContactsCanNeverClassify)
{
	const double MinMul = Tables().SurfaceMultiplier(ESurface::Hard);
	for (const ESurface Surface : {ESurface::Hard, ESurface::Wood, ESurface::Scrap, ESurface::DeepWater, ESurface::StallGel})
	{
		KCHECK(Tables().SurfaceMultiplier(Surface) >= MinMul);
	}
	for (const EImpactTable Table : {EImpactTable::PlayerVsStatic, EImpactTable::PlayerVsKinematic,
	                                 EImpactTable::PlayerVsPlayerFloor})
	{
		KCHECK(*Tables().Impact(Table).Stagger * MinMul >= FImpactMonitor::RestingSpeed);
	}
	KCHECK(*Tables().Impact(EImpactTable::ObjectVsPlayer).Stagger * MinMul >= FImpactMonitor::RestingImpulse);
	FImpactMonitor Monitor(Tables());
	KCHECK(Monitor.EndStep({{3, EContactKind::Static, 0.9, 40}, ObjectContact(4, 0.9, 49)}, 0.0, Dt) ==
	       EOutcome::None);
}

KTEST(ImpactMonitorDebouncesRepeatHits)
{
	FImpactMonitor Monitor(Tables());
	KCHECK(Monitor.EndStep({ObjectContact(7, 2, 400)}, 0.0, Dt) == EOutcome::Stagger);
	KCHECK(Monitor.EndStep({ObjectContact(7, 2, 450)}, 0.1, Dt) == EOutcome::None);   // Debounced
	KCHECK(Monitor.EndStep({ObjectContact(7, 2, 800)}, 0.15, Dt) == EOutcome::Flip);  // Twice the last
	KCHECK(Monitor.EndStep({ObjectContact(7, 2, 450)}, 0.45, Dt) == EOutcome::Stagger);
	KCHECK(Monitor.EndStep({ObjectContact(7, 2, 1300)}, 0.46, Dt) == EOutcome::Stall);

	// A Stall is never debounced, even when it is not twice the last hit.
	FImpactMonitor Escalate(Tables());
	KCHECK(Escalate.EndStep({ObjectContact(6, 2, 700)}, 0.0, Dt) == EOutcome::Flip);
	KCHECK(Escalate.EndStep({ObjectContact(6, 2, 1250)}, 0.1, Dt) == EOutcome::Stall);

	// Player hits are debounced on m_other·v, not on a solver impulse the caller may leave 0.
	FImpactMonitor Shove(Tables());
	const FContact Breaker{8, EContactKind::Player, 2.0, 0.0, 220.0};
	KCHECK(Shove.EndStep({Breaker}, 0.0, Dt) == EOutcome::Stagger);  // 440 N·s
	KCHECK(Shove.EndStep({Breaker}, 0.05, Dt) == EOutcome::None);
	KCHECK(Shove.EndStep({Breaker}, 0.1, Dt) == EOutcome::None);

	// Wall hits are debounced on speed.
	FImpactMonitor Bounce(Tables());
	KCHECK(Bounce.EndStep({{9, EContactKind::Static, 6.0, 780}}, 0.0, Dt) == EOutcome::Stagger);
	KCHECK(Bounce.EndStep({{9, EContactKind::Static, 7.0, 4000}}, 0.1, Dt) == EOutcome::None);
}

KTEST(ImpactMonitorSwallowsTheHandoffSettle)
{
	FImpactMonitor Monitor(Tables());
	Monitor.NotifyHandoff();
	KCHECK(Monitor.EndStep({{1, EContactKind::Static, 6, 130 * 6}}, 0.0, Dt) == EOutcome::None);
	KCHECK(Monitor.EndStep({{2, EContactKind::Static, 9, 130 * 9}}, 0.02, Dt) == EOutcome::Flip);
	KCHECK(Monitor.EndStep({{3, EContactKind::Static, 6, 130 * 6}}, 0.04, Dt) == EOutcome::Stagger);  // Settle over
}

// 7.1 and player-physics 4.3: 8 kN or more from opposing sides for 0.5 s, with a kinematic or
// a Heavy body on one side, summed per body.
KTEST(CrushNeedsOpposingForceHeldHalfASecond)
{
	const auto Holds = [](const std::vector<FCrushContact>& Contacts) {
		FCrushDetector Detector(Tables());
		bool bCrushed = false;
		for (int I = 0; I < 60; ++I)
		{
			bCrushed = Detector.Update(Contacts, Dt) || bCrushed;
		}
		return bCrushed;
	};
	const std::vector<FCrushContact> Pinned{{1, {0, 0, 1}, 9000, false}, {2, {0, 0, -1}, 9000, true}};
	FCrushDetector Press(Tables());
	for (int I = 0; I < 29; ++I)
	{
		KCHECK(!Press.Update(Pinned, Dt));
	}
	KCHECK(Press.Update(Pinned, Dt));  // 30 steps: 0.5 s

	KCHECK(Holds({{1, {0, 0, 1}, 8000, false}, {2, {0, 0, -1}, 8000, true}}));   // Inclusive
	KCHECK(!Holds({{1, {0, 0, 1}, 9000, false}, {2, {0, 0, -1}, 9000, false}}));  // No kinematic or Heavy
	KCHECK(!Holds({{1, {0, 0, 1}, 7999, true}, {2, {0, 0, -1}, 9000, true}}));    // One side too weak
	KCHECK(!Holds({{1, {0, 0, 1}, 7999, true}, {2, {0, 0, -1}, 7999, true}}));
	KCHECK(!Holds({{1, {0, 0, 1}, 9000, true}, {2, {0.2, 0, 1}, 9000, true}}));   // Same side
	KCHECK(!Holds({{1, {0, 0, 1}, 9000, true}, {1, {0, 0, -1}, 9000, true}}));    // One body
	// 12 kN per side over two manifold points each.
	KCHECK(Holds({{1, {0, 0, 1}, 6000, false}, {1, {0.1, 0, 1}, 6000, false},
	              {2, {0, 0, -1}, 6000, true}, {2, {-0.1, 0, -1}, 6000, true}}));

	FCrushDetector Decay(Tables());
	for (int I = 0; I < 20; ++I)
	{
		Decay.Update(Pinned, Dt);
	}
	Decay.Update({}, Dt);
	KCHECK_NEAR(Decay.Seconds(), 18.0 * Dt, 1e-12);  // Lapses decay at twice the rate
}

// 6.4: propane's authored radii; impulse falls off; deep water halves it.
KTEST(BlastsByDistance)
{
	KCHECK(ClassifyBlast(EVolatile::Propane, 1.5, false, Tables()).Outcome == EOutcome::Stall);
	KCHECK(ClassifyBlast(EVolatile::Propane, 3.0, false, Tables()).Outcome == EOutcome::Flip);
	KCHECK(ClassifyBlast(EVolatile::Propane, 4.1, false, Tables()).Outcome == EOutcome::None);
	const FBlastResult Wet = ClassifyBlast(EVolatile::Propane, 1.5, true, Tables());
	KCHECK_NEAR(Wet.ImpulseNs, 1500.0 * (1.0 - 1.5 / 4.0) * 0.5, 1e-9);
	KCHECK(Wet.Outcome == EOutcome::Stagger);
	KCHECK(ClassifyBlast(EVolatile::FuelDrum, 0.0, false, Tables()).Outcome == EOutcome::Stall);
	KCHECK(ClassifyBlast(EVolatile::FuelDrum, 1.5, false, Tables()).Outcome == EOutcome::Flip);
	KCHECK(ClassifyBlast(EVolatile::GasManifold, 0.5, false, Tables()).Outcome == EOutcome::None);
}

// 4.1: base warm-up plus class offsets.
KTEST(CutterWarmups)
{
	KCHECK_NEAR(CutterWarmupSeconds(ECutterVariant::Standard, EHarnessClass::Rigger, Tables()), 0.17, 1e-12);
	KCHECK_NEAR(CutterWarmupSeconds(ECutterVariant::Hot, EHarnessClass::Scrapper, Tables()), 0.07, 1e-12);
	KCHECK_NEAR(CutterWarmupSeconds(ECutterVariant::Long, EHarnessClass::Breaker, Tables()), 0.36, 1e-12);
	KCHECK_NEAR(CutterWarmupSeconds(ECutterVariant::Standard, EHarnessClass::Climber, Tables()), 0.20, 1e-12);
}

// 4.2 and 5.5: a back-plate line needs a 150° turn to come inside the 70° cone.
KTEST(BackPlateLineNeedsTheTurn)
{
	const FCutterTuning& Standard = Tables().Cutter(ECutterVariant::Standard);
	const FVec3 Plate{-0.25, 0, 0};
	const FVec3 FarDog{-15, 0, 0};
	const auto Facing = [&](double Degrees) {
		return CanCut({{0, 0, 0}, Heading(Degrees), Standard.Reach, Standard.ConeDeg}, Plate, FarDog);
	};
	KCHECK(!Facing(0.0));
	KCHECK(!Facing(140.0));
	KCHECK(Facing(150.0));
	KCHECK(Facing(180.0));
	KCHECK(Facing(-150.0));
}

KTEST(CutNeedsReach)
{
	const FCutterTuning& Standard = Tables().Cutter(ECutterVariant::Standard);
	const FCutterTuning& Long = Tables().Cutter(ECutterVariant::Long);
	const FVec3 A{1.8, -5, 0};
	const FVec3 B{1.8, 5, 0};
	KCHECK(!CanCut({{0, 0, 0}, {1, 0, 0}, Standard.Reach, Standard.ConeDeg}, A, B));
	const std::optional<double> T = FindCutPoint({{0, 0, 0}, {1, 0, 0}, Long.Reach, Long.ConeDeg}, A, B);
	KCHECK(T.has_value() && std::abs(*T - 0.5) < 1e-6);  // The point nearest the blade
	KCHECK(!CanCut({{0, 0, 0}, {1, 0, 0}, 50.0, Standard.ConeDeg}, {-3, -5, 0}, {-3, 5, 0}));  // Behind
}
