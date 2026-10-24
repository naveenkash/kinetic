// The line solver against bible/05's worked examples (sections 3.2 to 3.4, 4.3, 5.3, 10)
// and the test matrix in spool-and-line-implementation.md section 8.
#include "SimFixtures.h"

using namespace Kinetic;
using namespace Kinetic::Test;

namespace
{
constexpr FVec3 Origin{0, 0, 0};

double DistanceMoved(const FReferenceSim& Sim, int Body, const FVec3& From)
{
	return (Sim.Body(Body).Position - From).Length();
}
} // namespace

// 4.3: a braced Breaker Hauls a 130 kg Rigger from 15 m.
KTEST(PanicWindowReferenceCase)
{
	const FPanicRun Edge = RunHaulPanic(Tables(), EHarnessClass::Breaker, EHarnessClass::Rigger, 15.0, StepSeconds, 5.0);
	KCHECK(Edge.FeetOffAt.has_value() && Edge.DistanceAt.has_value());
	KCHECK_NEAR(*Edge.FeetOffAt, 0.26, StepSeconds);   // Ramp passes 650 N: 5 m/s² foot-hold
	KCHECK_NEAR(*Edge.DistanceAt, 2.2, 0.1);           // Over the edge
	KCHECK_NEAR(Edge.SpeedAtDistance, 15.0, 1.0);      // At roughly 15 m/s

	const FPanicRun AtOne = RunHaulPanic(Tables(), EHarnessClass::Breaker, EHarnessClass::Rigger, 2.5, StepSeconds, 5.0);
	KCHECK_NEAR(*AtOne.DistanceAt, 1.0, 0.1);          // "About 2.5 m" at 1.0 s
	const FPanicRun AtOneSix = RunHaulPanic(Tables(), EHarnessClass::Breaker, EHarnessClass::Rigger, 8.0, StepSeconds, 5.0);
	KCHECK_NEAR(*AtOneSix.DistanceAt, 1.6, 0.1);       // "About 8 m" at 1.6 s
}

KTEST(BracedBreakerTakesNoneOfItsOwnHaul)
{
	FReferenceSim Sim(Tables(), StepSeconds);
	const int Breaker = Sim.AddBody(PlayerAt(Origin, EHarnessClass::Breaker, true, true));
	const int Rigger = Sim.AddBody(PlayerAt({15, 0, 0}, EHarnessClass::Rigger, true));
	Sim.Engage(Sim.AddLine(LineOf(EHarnessClass::Breaker, Breaker, ESeat::OwnPlate, Rigger, ESeat::OtherPlate)),
	           ELineMode::Haul);
	RunFor(Sim, 1.5);
	KCHECK_NEAR(DistanceMoved(Sim, Breaker, Origin), 0.0, 1e-12);
	KCHECK(Sim.Body(Breaker).Footing == EFooting::Braced);
	KCHECK(DistanceMoved(Sim, Rigger, {15, 0, 0}) > 3.0);
}

// 3.3: a 220 kg Breaker and a 90 kg Climber close at a 71:29 split.
KTEST(HaulSplitsByInverseMass)
{
	FReferenceSim Sim(Tables(), StepSeconds);
	const int Heavy = Sim.AddBody(PlayerAt(Origin, EHarnessClass::Breaker, false));
	const int Light = Sim.AddBody(PlayerAt({12, 0, 0}, EHarnessClass::Climber, false));
	Sim.Engage(Sim.AddLine(LineOf(EHarnessClass::Breaker, Heavy, ESeat::OwnPlate, Light, ESeat::OtherPlate)),
	           ELineMode::Haul);
	RunFor(Sim, 0.8);
	const double HeavyMoved = DistanceMoved(Sim, Heavy, Origin);
	const double LightMoved = DistanceMoved(Sim, Light, {12, 0, 0});
	KCHECK_NEAR(LightMoved / (HeavyMoved + LightMoved), 220.0 / 310.0, 0.005);
}

KTEST(HaulGoesSlackPastTheClosingCap)
{
	FReferenceSim Sim(Tables(), StepSeconds);
	const int Anchor = Sim.AddBody(StaticAt(Origin));
	const int Crate = Sim.AddBody(ObjectAt({19, 0, 0}, 30.0));
	const int Line = Sim.AddLine(LineOf(EHarnessClass::Breaker, Anchor, ESeat::OwnPlate, Crate, ESeat::Body));
	Sim.Engage(Line, ELineMode::Haul);
	double MaxSpeed = 0.0;
	for (int I = 0; I < 90; ++I)
	{
		Sim.Step();
		MaxSpeed = std::max(MaxSpeed, Sim.Body(Crate).Velocity.Length());
	}
	KCHECK(MaxSpeed <= 25.5);
	KCHECK((Sim.Line(Line).EventsSeen & LineEvents::SlackPastCap) != 0);
}

KTEST(HaulStopsWhenTheDogsMeet)
{
	FReferenceSim Sim(Tables(), StepSeconds);
	const int Anchor = Sim.AddBody(StaticAt(Origin));
	const int Barrel = Sim.AddBody(ObjectAt({3, 0, 0}, 100.0));
	const int Line = Sim.AddLine(LineOf(EHarnessClass::Rigger, Anchor, ESeat::OwnPlate, Barrel, ESeat::Body));
	Sim.Engage(Line, ELineMode::Haul);
	TimeUntil(Sim, 3.0, [&] { return (Sim.Line(Line).EventsSeen & LineEvents::DogsMet) != 0; });
	KCHECK((Sim.Line(Line).EventsSeen & LineEvents::DogsMet) != 0);
	KCHECK(Sim.Line(Line).State.Mode == ELineMode::Slack);  // The Haul ends; it does not pull back
	const double X = Sim.Body(Barrel).Position.X;
	const double Speed = Sim.Body(Barrel).Velocity.Length();
	RunFor(Sim, 1.0);
	KCHECK_NEAR(Sim.Line(Line).State.AppliedForce, 0.0, 0.0);
	KCHECK_NEAR(Sim.Body(Barrel).Position.X, X - Speed * 1.0, 1e-6);  // Coasts on, unpulled
}

// 3.2 and spool-and-line 8: a self-dog Jack to a wall launches a Climber to V within 0.3 s.
KTEST(KickLaunchesAClimberOffAWall)
{
	FReferenceSim Sim(Tables(), StepSeconds);
	const int Climber = Sim.AddBody(PlayerAt(Origin, EHarnessClass::Climber, true));
	const int Wall = Sim.AddBody(StaticAt({-1, 0, 0}));
	const int Line = Sim.AddLine(LineOf(EHarnessClass::Climber, Climber, ESeat::OwnPlate, Wall, ESeat::Static));
	Sim.Engage(Line, ELineMode::Jack);
	RunFor(Sim, 0.3 + StepSeconds);
	KCHECK_NEAR(Sim.Body(Climber).Velocity.Length(), 18.0, 0.5);
	KCHECK(Sim.Body(Climber).Footing == EFooting::Airborne);
	KCHECK((Sim.Line(Line).EventsSeen & LineEvents::KickActive) != 0);
	const double ForceCap = 1.5 * 90.0 * 18.0 / 0.3;
	KCHECK(Sim.Line(Line).MaxAppliedForce <= ForceCap + 1e-9);
	// "A Climber launched at 18 m/s at 45° travels about 33 m": v²/g on flat ground.
	KCHECK_NEAR(18.0 * 18.0 / Tables().Globals().Gravity, 33.0, 0.1);
}

// spool-and-line 8: between two free bodies the Kick reaches relative V, split by inverse mass.
KTEST(KickBetweenFreeBodiesSplitsByInverseMass)
{
	FReferenceSim Sim(Tables(), StepSeconds);
	const int Heavy = Sim.AddBody(ObjectAt(Origin, 220.0));
	const int Light = Sim.AddBody(ObjectAt({2, 0, 0}, 90.0));
	Sim.Engage(Sim.AddLine(LineOf(EHarnessClass::Climber, Heavy, ESeat::Body, Light, ESeat::Body)), ELineMode::Jack);
	RunFor(Sim, 0.3 + StepSeconds);
	const double HeavySpeed = Sim.Body(Heavy).Velocity.Length();
	const double LightSpeed = Sim.Body(Light).Velocity.Length();
	KCHECK(HeavySpeed + LightSpeed >= 18.0 - 0.5);
	const double KickShare = 90.0 / 310.0;  // The heavy end's share of the Kick
	KCHECK_NEAR(HeavySpeed * 220.0, LightSpeed * 90.0, 0.03 * HeavySpeed * 220.0);  // Equal momentum
	KCHECK(HeavySpeed / (HeavySpeed + LightSpeed) <= KickShare + 0.01);
}

// 3.2 and 10 (Overboard): no Kick against another player's plate; a 600 N push skids a
// 130 kg Rigger at 4.6 m/s², about two seconds to cross nine meters.
KTEST(KickInterlockMakesOverboardASkid)
{
	FReferenceSim Sim(Tables(), StepSeconds);
	const int Jacker = Sim.AddBody(PlayerAt(Origin, EHarnessClass::Rigger, true, true));
	const int Victim = Sim.AddBody(PlayerAt({1, 0, 0}, EHarnessClass::Rigger, true));
	const int Line = Sim.AddLine(LineOf(EHarnessClass::Rigger, Jacker, ESeat::OwnPlate, Victim, ESeat::OtherPlate));
	Sim.Engage(Line, ELineMode::Jack);
	const double Crossed = TimeUntil(Sim, 4.0, [&] { return DistanceMoved(Sim, Victim, {1, 0, 0}) >= 9.0; });
	KCHECK(Crossed > 1.9 && Crossed < 2.3);
	KCHECK((Sim.Line(Line).EventsSeen & LineEvents::KickActive) == 0);
	KCHECK(Sim.Line(Line).MaxAppliedForce <= 600.0 + 1e-9);
	KCHECK(Sim.Body(Victim).Footing == EFooting::Skid);
	KCHECK_NEAR(DistanceMoved(Sim, Jacker, Origin), 0.0, 1e-12);
}

KTEST(JackReleasesAtTheClassLineLength)
{
	FReferenceSim Sim(Tables(), StepSeconds);
	const int Anchor = Sim.AddBody(StaticAt(Origin));
	const int Barrel = Sim.AddBody(ObjectAt({1, 0, 0}, 100.0));
	const int Line = Sim.AddLine(LineOf(EHarnessClass::Scrapper, Anchor, ESeat::OwnPlate, Barrel, ESeat::Body));
	Sim.Engage(Line, ELineMode::Jack);
	double MaxSpeed = 0.0;
	TimeUntil(Sim, 5.0, [&] {
		MaxSpeed = std::max(MaxSpeed, Sim.Body(Barrel).Velocity.Length());
		return Sim.Line(Line).State.Mode == ELineMode::Slack;
	});
	KCHECK((Sim.Line(Line).EventsSeen & LineEvents::AutoRelease) != 0);
	KCHECK(DistanceMoved(Sim, Barrel, Origin) >= Tables().Harness(EHarnessClass::Scrapper).LineLength);
	KCHECK(MaxSpeed <= 30.5);
}

// 6.1: the Scrapper bonus is 1.69× line force on Light and Medium props under 100 kg, on Haul
// and the post-Kick push only; the Kick's V is unchanged.
KTEST(ScrapperBonusAppliesToSmallPropsOnly)
{
	FReferenceSim Sim(Tables(), StepSeconds);
	const int Anchor = Sim.AddBody(StaticAt(Origin));
	const int Cylinder = Sim.AddBody(ObjectAt({10, 0, 0}, 45.0));
	const int Drum = Sim.AddBody(ObjectAt({0, 10, 0}, 180.0));
	const int ToCylinder = Sim.AddLine(LineOf(EHarnessClass::Scrapper, Anchor, ESeat::OwnPlate, Cylinder, ESeat::Body));
	const int ToDrum = Sim.AddLine(LineOf(EHarnessClass::Scrapper, Anchor, ESeat::OwnPlate, Drum, ESeat::Body));
	Sim.Engage(ToCylinder, ELineMode::Haul);
	Sim.Engage(ToDrum, ELineMode::Haul);
	RunFor(Sim, 0.5);
	KCHECK_NEAR(Sim.Line(ToCylinder).MaxAppliedForce, 400.0 * 1.69, 1e-9);
	KCHECK_NEAR(Sim.Line(ToDrum).MaxAppliedForce, 400.0, 1e-9);

	FReferenceSim KickSim(Tables(), StepSeconds);
	const int Wall = KickSim.AddBody(StaticAt(Origin));
	const int Thrown = KickSim.AddBody(ObjectAt({1, 0, 0}, 45.0));
	KickSim.Engage(KickSim.AddLine(LineOf(EHarnessClass::Scrapper, Wall, ESeat::OwnPlate, Thrown, ESeat::Body)),
	               ELineMode::Jack);
	RunFor(KickSim, 0.3);
	KCHECK_NEAR(KickSim.Body(Thrown).Velocity.Length(), 14.0, 0.5);  // V, not 1.3 × V
}

KTEST(HangHoldsALoadUnderTheHangLoad)
{
	FReferenceSim Sim(Tables(), StepSeconds);
	const int Anchor = Sim.AddBody(StaticAt({0, 0, 10}));
	FSimBody Weight = ObjectAt({0, 0, 5}, 90.0);
	Weight.bGravity = true;
	const int Load = Sim.AddBody(Weight);
	const int Line = Sim.AddLine(LineOf(EHarnessClass::Climber, Anchor, ESeat::Static, Load, ESeat::Body));
	Sim.Engage(Line, ELineMode::Hang);
	RunFor(Sim, 2.0);
	KCHECK_NEAR((Sim.Body(Load).Position - Sim.Body(Anchor).Position).Length(), 5.0, 0.01);
	KCHECK_NEAR(Sim.Line(Line).State.AppliedForce, 90.0 * 9.81, 10.0);
	KCHECK((Sim.Line(Line).EventsSeen & LineEvents::Slipping) == 0);
}

// spool-and-line 8: a Rigger reels a 220 kg load up at 1.0 m/s under the 12 kN Hang load (a
// grounded player: GallowsHoistsAGroundedPlayer).
KTEST(ReelHoistsAtOneMeterPerSecond)
{
	FReferenceSim Sim(Tables(), StepSeconds);
	const int Anchor = Sim.AddBody(StaticAt({0, 0, 10}));
	FSimBody Weight = ObjectAt({0, 0, 4}, 220.0);
	Weight.bGravity = true;
	const int Load = Sim.AddBody(Weight);
	const int Line = Sim.AddLine(LineOf(EHarnessClass::Rigger, Anchor, ESeat::Static, Load, ESeat::Body));
	Sim.Engage(Line, ELineMode::Hang);
	RunFor(Sim, 0.5);
	const double Before = Sim.Body(Load).Position.Z;
	Sim.SetReelInput(Line, -1);
	RunFor(Sim, 2.0);
	KCHECK_NEAR(Sim.Line(Line).State.ReelAccum, -2.0, 0.02);
	KCHECK_NEAR(Sim.Body(Load).Position.Z - Before, 2.0, 0.05);
	KCHECK((Sim.Line(Line).EventsSeen & LineEvents::Slipping) == 0);
}

KTEST(ReelAgainstAnOverloadSlipsInstead)
{
	FReferenceSim Sim(Tables(), StepSeconds);
	const int Anchor = Sim.AddBody(StaticAt({0, 0, 30}));
	FSimBody Weight = ObjectAt({0, 0, 25}, 1300.0);  // 12.75 kN on a Rigger's 12 kN line
	Weight.bGravity = true;
	const int Load = Sim.AddBody(Weight);
	const int Line = Sim.AddLine(LineOf(EHarnessClass::Rigger, Anchor, ESeat::Static, Load, ESeat::Body));
	Sim.Engage(Line, ELineMode::Hang);
	Sim.SetReelInput(Line, -1);
	RunFor(Sim, 1.0);
	KCHECK_NEAR(Sim.Line(Line).State.ReelAccum, 0.0, 1e-12);  // No lift gained
	KCHECK((Sim.Line(Line).EventsSeen & LineEvents::Slipping) != 0);
}

KTEST(HangIsARopeUnlessRodMode)
{
	for (const bool bRod : {false, true})
	{
		FReferenceSim Sim(Tables(), StepSeconds);
		const int Anchor = Sim.AddBody(StaticAt(Origin));
		FSimBody Ball = ObjectAt({5, 0, 0}, 50.0);
		Ball.Velocity = {-3, 0, 0};  // Moving toward the anchor: compression
		const int Moving = Sim.AddBody(Ball);
		FSimLine Bola = LineOf(EHarnessClass::Rigger, Anchor, ESeat::Static, Moving, ESeat::Body);
		Bola.State.bRodMode = bRod;
		Sim.Engage(Sim.AddLine(Bola), ELineMode::Hang);
		RunFor(Sim, 0.5);
		const double Length = Sim.Body(Moving).Position.Length();
		KCHECK(bRod ? std::abs(Length - 5.0) < 0.01 : Length < 4.0);
	}
}

// 6.2: a Hang line never tests break-free; a Haul at 200 N or more frees a racked cylinder.
KTEST(PendulumFromARackHoldsAndAHaulFreesIt)
{
	FReferenceSim Sim(Tables(), StepSeconds);
	const int Cylinder = Sim.AddBody(ObjectAt({0, 0, 10}, 45.0, EPlacement::Rack));
	FSimBody Swinger = PlayerAt({4, 0, 7}, EHarnessClass::Climber, false);
	Swinger.bGravity = true;
	const int Climber = Sim.AddBody(Swinger);
	Sim.Engage(Sim.AddLine(LineOf(EHarnessClass::Climber, Climber, ESeat::OwnPlate, Cylinder, ESeat::Body)),
	           ELineMode::Hang);
	RunFor(Sim, 1.0);
	KCHECK_NEAR((Sim.Body(Cylinder).Position - FVec3{0, 0, 10}).Length(), 0.0, 1e-12);
	KCHECK(!Sim.Body(Cylinder).bBrokenFreeLatched);

	FReferenceSim HaulSim(Tables(), StepSeconds);
	const int Anchor = HaulSim.AddBody(StaticAt(Origin));
	const int Racked = HaulSim.AddBody(ObjectAt({5, 0, 0}, 45.0, EPlacement::Rack));
	HaulSim.Engage(HaulSim.AddLine(LineOf(EHarnessClass::Climber, Anchor, ESeat::OwnPlate, Racked, ESeat::Body)),
	               ELineMode::Haul);
	RunFor(HaulSim, 0.5);
	KCHECK(HaulSim.Body(Racked).bBrokenFreeLatched);
	KCHECK(HaulSim.Body(Racked).Position.X < 5.0);
}

KTEST(NoBodyExceedsTheHardVelocityCap)
{
	FReferenceSim Sim(Tables(), StepSeconds);
	FSimBody Debris = ObjectAt(Origin, 5.0);
	Debris.Velocity = {80, 0, 0};
	const int Fast = Sim.AddBody(Debris);
	Sim.Step();
	KCHECK(Sim.Body(Fast).Velocity.Length() <= 50.0 + 1e-9);
}

// The solver is deterministic: the same inputs give bit-identical results (risk 4).
KTEST(RunsAreBitIdentical)
{
	const FPanicRun First = RunHaulPanic(Tables(), EHarnessClass::Doubler, EHarnessClass::Climber, 12.0, StepSeconds, 5.0);
	const FPanicRun Second = RunHaulPanic(Tables(), EHarnessClass::Doubler, EHarnessClass::Climber, 12.0, StepSeconds, 5.0);
	KCHECK(First.DistanceAt == Second.DistanceAt && First.FeetOffAt == Second.FeetOffAt);
	KCHECK(First.SpeedAtDistance == Second.SpeedAtDistance);
}
