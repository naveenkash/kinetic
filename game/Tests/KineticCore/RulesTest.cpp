// Mass rules, Spool tension and release impulses (bible/05 sections 3.3, 3.7, 3.8, 4.2,
// 5.3, 5.4, 6.1 to 6.3, 6.6).
#include "Kinetic/MassRules.h"
#include "Kinetic/Spool.h"
#include "TestHarness.h"

#include <cmath>

using namespace Kinetic;
using namespace Kinetic::Test;

namespace
{
constexpr double G = 9.81;

EFooting FootingUnder(EHarnessClass Victim, double ForceN, bool bBraced)
{
	const FPlayerBody Body = PlayerBodyFor(Tables().Harness(Victim), bBraced, true);
	return ResolvePlayerUnderLine(Body, {ForceN, 0, 0}, false, G).Footing;
}
} // namespace

// 5.3: "a Breaker's 1,000 N takes every unbraced class off its feet except another Breaker."
KTEST(BreakerLineTakesEveryoneButABreakerOffTheirFeet)
{
	for (const EHarnessClass Class : AllHarnessClasses)
	{
		const EFooting Expected = Class == EHarnessClass::Breaker ? EFooting::Skid : EFooting::Airborne;
		KCHECK(FootingUnder(Class, 1000.0, false) == Expected);
	}
}

// 5.3: "A Scrapper's 400 N takes only a Climber off its feet; everyone else skids."
KTEST(ScrapperLineTakesOnlyAClimberOffItsFeet)
{
	for (const EHarnessClass Class : AllHarnessClasses)
	{
		const EFooting Expected = Class == EHarnessClass::Climber ? EFooting::Airborne : EFooting::Skid;
		KCHECK(FootingUnder(Class, 400.0, false) == Expected);
	}
}

// 5.3 and 5.4: overspin takes an unbraced Breaker off its feet; braced, it never moves; a braced
// Rigger holds a Breaker's line but not a Doubler's full overspin.
KTEST(BraceThresholds)
{
	KCHECK(FootingUnder(EHarnessClass::Breaker, 1200.0, false) == EFooting::Airborne);
	KCHECK(FootingUnder(EHarnessClass::Breaker, 1200.0, true) == EFooting::Braced);
	KCHECK(FootingUnder(EHarnessClass::Breaker, 4400.0, true) == EFooting::Braced);  // 20 m/s², inclusive
	KCHECK(FootingUnder(EHarnessClass::Rigger, 1000.0, true) == EFooting::Braced);
	const FPlayerLineResponse Overspun =
	    ResolvePlayerUnderLine(PlayerBodyFor(Tables().Harness(EHarnessClass::Rigger), true, true), {1200, 0, 0}, false, G);
	KCHECK(Overspun.bBraceBroken && Overspun.Footing == EFooting::Airborne);
	// Only Riggers and Breakers can brace.
	KCHECK(!PlayerBodyFor(Tables().Harness(EHarnessClass::Climber), true, true).bBraced);
}

KTEST(SkidDragFollowsTheGround)
{
	const FPlayerBody Rigger = PlayerBodyFor(Tables().Harness(EHarnessClass::Rigger), false, true);
	const FPlayerLineResponse R = ResolvePlayerUnderLine(Rigger, {520, 0, 100}, false, G);
	KCHECK(R.Footing == EFooting::Skid);
	KCHECK_NEAR(R.DragAcceleration.X, 4.0, 1e-12);
	KCHECK_NEAR(R.DragAcceleration.Z, 0.0, 1e-12);
	KCHECK_NEAR(R.InvMass, 1.0 / 130.0, 1e-15);  // A Hang line moves a skidding player
}

KTEST(TheKickAlwaysLaunchesAnUnbracedGroundedPlayer)
{
	const FPlayerBody Breaker = PlayerBodyFor(Tables().Harness(EHarnessClass::Breaker), false, true);
	KCHECK(ResolvePlayerUnderLine(Breaker, {10, 0, 0}, true, G).Footing == EFooting::Airborne);
}

KTEST(AnUpwardPullThatBeatsWeightHoists)
{
	const FPlayerBody Breaker = PlayerBodyFor(Tables().Harness(EHarnessClass::Breaker), false, true);
	KCHECK(ResolvePlayerUnderLine(Breaker, {0, 0, 800}, false, G).Footing == EFooting::Skid);   // Under weight
	KCHECK(ResolvePlayerUnderLine(Breaker, {0, 0, 2200}, false, G).Footing == EFooting::Hoisted);
}

// 6.2 and 6.2a: inclusive thresholds, forces add as vectors, Hang never tests break-free.
KTEST(BreakFreeIsInclusiveAndAddsAsVectors)
{
	const FObjectBody Stacked{2000.0, EPlacement::Stack};
	KCHECK(ResolveObjectUnderLines(Stacked, {400, 0, 0}, false, Tables()).InvMass == 0.0);
	KCHECK(ResolveObjectUnderLines(Stacked, {500, 0, 0}, false, Tables()).bBreaksFreeNow);
	KCHECK(ResolveObjectUnderLines(Stacked, {400, 400, 0}, false, Tables()).bBreaksFreeNow);  // 566 N
	KCHECK(!ResolveObjectUnderLines(Stacked, FVec3{400, 0, 0} + FVec3{-400, 0, 0}, false, Tables()).bBreaksFreeNow);  // Opposed lines cancel
	KCHECK(ResolveObjectUnderLines(FObjectBody{100.0, EPlacement::Bolted}, {800, 0, 0}, false, Tables()).bBreaksFreeNow);
	KCHECK(ResolveObjectUnderLines(Stacked, {}, true, Tables()).InvMass == 0.0);
	KCHECK(ResolveObjectUnderLines(FObjectBody{100.0, EPlacement::Loose}, {}, true, Tables()).InvMass > 0.0);
	FObjectBody Fixed{900.0, EPlacement::Loose};
	Fixed.bFixedScenery = true;
	KCHECK(ResolveObjectUnderLines(Fixed, {5000, 0, 0}, false, Tables()).InvMass == 0.0);
}

// v0.2.1: a Breaker's 1,000 N starts a rated load and a Doubler's 800 N a bolted one, on any
// axis, though a pull built as Axis × force can measure a hair under the threshold.
KTEST(BreakFreeHoldsOnEveryAxis)
{
	int Missed = 0;
	for (int I = 1; I <= 1000; ++I)
	{
		const FVec3 Axis = FVec3{std::sin(I * 1.3), std::cos(I * 0.7), std::sin(I * 2.9)}.SafeNormal();
		Missed += MeetsBreakFree((Axis * 1000.0).Length(), Tables().BreakFreeN(EPlacement::Rated)) ? 0 : 1;
		Missed += MeetsBreakFree((Axis * 800.0).Length(), Tables().BreakFreeN(EPlacement::Bolted)) ? 0 : 1;
	}
	KCHECK(Missed == 0);
	KCHECK(!MeetsBreakFree(999.99, Tables().BreakFreeN(EPlacement::Rated)));
}

// 6.2a: a rated load commits after 0.6 s of active pull at 1,000 N on its axis.
KTEST(RatedLoadsCommitAfterTheHold)
{
	constexpr double Dt = 1.0 / 60.0;
	FRatedCommit Column;
	for (int I = 0; I < 35; ++I)
	{
		KCHECK(!Column.Update(1000.0, Dt, Tables()));
	}
	KCHECK(Column.Update(1000.0, Dt, Tables()));  // The 36th step: 0.6 s

	FRatedCommit Weak;
	for (int I = 0; I < 120; ++I)
	{
		Weak.Update(999.0, Dt, Tables());
	}
	KCHECK(!Weak.IsCommitted());

	FRatedCommit Interrupted;
	for (int I = 0; I < 30; ++I)
	{
		Interrupted.Update(1000.0, Dt, Tables());
	}
	Interrupted.Update(0.0, Dt, Tables());
	KCHECK_NEAR(Interrupted.HeldSeconds(), 0.0, 0.0);
}

KTEST(KinematicAttachmentsPullFreeAtTheRatedThreshold)
{
	FObjectBody Load{4000.0, EPlacement::Rated};
	Load.bRated = true;
	Load.bKinematicAttachment = true;
	KCHECK(!ResolveObjectUnderLines(Load, {999, 0, 0}, false, Tables()).bReleasedFromPath);
	KCHECK(ResolveObjectUnderLines(Load, {1000, 0, 0}, false, Tables()).bReleasedFromPath);
	FObjectBody Column{40000.0, EPlacement::Rated};
	Column.bRated = true;
	KCHECK(ResolveObjectUnderLines(Column, {5000, 0, 0}, false, Tables()).InvMass == 0.0);
}

KTEST(MassTiersAndTheScrapperBonusTargets)
{
	const FMassTierBounds& B = Tables().MassTiers();
	KCHECK(TierOf(29.9, false, B) == EMassTier::Light);
	KCHECK(TierOf(30.0, false, B) == EMassTier::Medium);
	KCHECK(TierOf(299.0, false, B) == EMassTier::Medium);
	KCHECK(TierOf(300.0, false, B) == EMassTier::Heavy);
	KCHECK(TierOf(3000.0, false, B) == EMassTier::Heavy);
	KCHECK(TierOf(3001.0, false, B) == EMassTier::Rated);
	KCHECK(TierOf(500.0, true, B) == EMassTier::Rated);
	KCHECK(IsSmallPropBonusTarget(45.0, Tables()));    // Propane cylinder: yes
	KCHECK(IsSmallPropBonusTarget(12.0, Tables()));    // Plank
	KCHECK(IsSmallPropBonusTarget(90.0, Tables()));    // A scrapped Climber (7.4)
	KCHECK(!IsSmallPropBonusTarget(100.0, Tables()));  // Under 100 kg only
	KCHECK(!IsSmallPropBonusTarget(180.0, Tables()));  // Fuel drum: no
}

// 6.6: a body feels a_ref × 100 / m, capped at 2 × a_ref.
KTEST(ForceVolumesScaleWithMassAndCap)
{
	const FSpoolGlobals& Globals = Tables().Globals();
	KCHECK_NEAR(ForceVolumeAcceleration(10.0, 90.0, Globals), 1000.0 / 90.0, 1e-12);
	KCHECK_NEAR(ForceVolumeAcceleration(10.0, 220.0, Globals), 1000.0 / 220.0, 1e-12);
	KCHECK_NEAR(ForceVolumeAcceleration(10.0, 10.0, Globals), 20.0, 1e-12);
}

// 3.7 and 3.8: overspin toward 1,200 N over 1.0 s; the Doubler's split and restore.
KTEST(DoublerTensionOverspinAndSplit)
{
	const FHarnessTuning& Doubler = Tables().Harness(EHarnessClass::Doubler);
	const FSpoolGlobals& Globals = Tables().Globals();
	KCHECK_NEAR(LineTension(Doubler, Globals, {}), 800.0, 1e-9);
	KCHECK_NEAR(LineTension(Doubler, Globals, {0.5}), 1000.0, 1e-9);
	KCHECK_NEAR(LineTension(Doubler, Globals, {1.0}), 1200.0, 1e-9);
	KCHECK_NEAR(LineTension(Doubler, Globals, {1.5}), 1200.0, 1e-9);
	KCHECK_NEAR(LineTension(Doubler, Globals, {0.0, true}), 480.0, 1e-9);
	KCHECK_NEAR(LineTension(Doubler, Globals, {1.0, true}), 720.0, 1e-9);
	KCHECK_NEAR(LineTension(Doubler, Globals, {0.0, false, 0.15}), 640.0, 1e-9);
	KCHECK_NEAR(LineTension(Doubler, Globals, {0.0, false, 0.3}), 800.0, 1e-9);
	KCHECK_NEAR(HangLoadWithOverspin(Doubler, Globals, 1.0), 12000.0, 1e-9);

	const FHarnessTuning& Breaker = Tables().Harness(EHarnessClass::Breaker);
	KCHECK_NEAR(LineTension(Breaker, Globals, {1.0, true}), 1000.0, 1e-9);  // No overspin, one line
	KCHECK(OverspinStage(Breaker, Globals, 2.0) == EOverspinStage::None);
}

KTEST(OverspinStages)
{
	const FHarnessTuning& Doubler = Tables().Harness(EHarnessClass::Doubler);
	const FSpoolGlobals& Globals = Tables().Globals();
	KCHECK(OverspinStage(Doubler, Globals, 0.0) == EOverspinStage::None);
	KCHECK(OverspinStage(Doubler, Globals, 0.5) == EOverspinStage::Escalating);
	KCHECK(OverspinStage(Doubler, Globals, 1.0) == EOverspinStage::Octave);
	KCHECK(OverspinStage(Doubler, Globals, 1.79) == EOverspinStage::Octave);
	KCHECK(OverspinStage(Doubler, Globals, 1.8) == EOverspinStage::Snapped);
}

// 3.7 and 4.2: the snap is 1,500 N·s × F / 1,200 N; a cut recoils 150 N·s per 1,000 N.
KTEST(SnapAndCutImpulses)
{
	const FSpoolGlobals& Globals = Tables().Globals();
	KCHECK_NEAR(SnapImpulseNs(1200.0, Globals), 1500.0, 1e-9);
	KCHECK_NEAR(SnapImpulseNs(600.0, Globals), 750.0, 1e-9);
	KCHECK_NEAR(CutRecoilNs(1000.0, Globals), 150.0, 1e-9);
	KCHECK_NEAR(CutRecoilNs(1200.0, Globals), 180.0, 1e-9);
}
