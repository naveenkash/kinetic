// Hang and Set lines (bible/05 sections 3.2, 3.5, 6.2a, 10) through HangSolver.h, and the
// active-line contract: free inverse masses, thresholds decided on the step the force acts.
#include "SimFixtures.h"

#include <cmath>
#include <vector>

using namespace Kinetic;
using namespace Kinetic::Test;

namespace
{
constexpr FVec3 Origin{0, 0, 0};
constexpr double G = 9.81;

FSimBody Weight(const FVec3& Position, double MassKg)
{
	FSimBody Body = ObjectAt(Position, MassKg);
	Body.bGravity = true;
	return Body;
}

double LineLength(const FReferenceSim& Sim, const FSimLine& Line)
{
	return (Sim.Body(Line.BodyB).Position - Sim.Body(Line.BodyA).Position).Length();
}

double RestLength(const FSimLine& Line) { return Line.State.L0 + Line.State.SlipAccum + Line.State.ReelAccum; }

// A load dropped onto a Climber line at Speed; returns how far the line slipped.
double JoltSlip(double DropSpeed, double Dt)
{
	FReferenceSim Sim(Tables(), Dt);
	const int Anchor = Sim.AddBody(StaticAt({0, 0, 30}));
	FSimBody Load = Weight({0, 0, 25}, 90.0);
	Load.Velocity = {0, 0, -DropSpeed};
	const int Body = Sim.AddBody(Load);
	const int Line = Sim.AddLine(LineOf(EHarnessClass::Climber, Anchor, ESeat::Static, Body, ESeat::Body));
	Sim.Engage(Line, ELineMode::Hang);
	RunFor(Sim, 3.0);
	return Sim.Line(Line).State.SlipAccum;
}
} // namespace

// The load a Hang line reads is m·g hanging still and m·v²/L swinging.
KTEST(HangLoadReadsWeightAndSwing)
{
	FReferenceSim Sim(Tables(), StepSeconds);
	const int Pivot = Sim.AddBody(StaticAt(Origin));
	FSimBody Ball = ObjectAt({4, 0, 0}, 90.0);
	Ball.Velocity = {0, 12, 0};  // No gravity: a pure swing
	const int Swinging = Sim.AddBody(Ball);
	const int Line = Sim.AddLine(LineOf(EHarnessClass::Rigger, Pivot, ESeat::Static, Swinging, ESeat::Body));
	Sim.Engage(Line, ELineMode::Hang);
	RunFor(Sim, 1.0);
	KCHECK_NEAR(Sim.Line(Line).State.AppliedForce, 90.0 * 12.0 * 12.0 / 4.0, 0.03 * 3240.0);
	KCHECK_NEAR(LineLength(Sim, Sim.Line(Line)), 4.0, 0.01);
	KCHECK_NEAR(Sim.Body(Swinging).Velocity.Length(), 12.0, 0.1);
}

// A pendulum's bottom load is m·g + m·v²/L: 90 kg dropped from level on 4 m reads 3·m·g,
// under a Climber's 4 kN, so it holds.
KTEST(PendulumBottomLoadIsThreeTimesWeight)
{
	FReferenceSim Sim(Tables(), StepSeconds);
	const int Pivot = Sim.AddBody(StaticAt({0, 0, 10}));
	const int Bob = Sim.AddBody(Weight({4, 0, 10}, 90.0));
	const int Line = Sim.AddLine(LineOf(EHarnessClass::Climber, Pivot, ESeat::Static, Bob, ESeat::Body));
	Sim.Engage(Line, ELineMode::Hang);
	RunFor(Sim, 1.5);
	KCHECK_NEAR(Sim.Line(Line).MaxAppliedForce, 3.0 * 90.0 * G, 0.04 * 3.0 * 90.0 * G);
	KCHECK((Sim.Line(Line).EventsSeen & LineEvents::Slipping) == 0);
}

// 3.2: over the Hang load the line slips. It holds the Hang load while it pays out, as fast
// as the load goes (a clutch: open item 7a).
KTEST(HangSlipsAtTheHangLoad)
{
	for (const double Mass : {500.0, 2000.0})  // 4.9 kN and 19.6 kN on a Climber's 4 kN line
	{
		FReferenceSim Sim(Tables(), StepSeconds);
		const int Anchor = Sim.AddBody(StaticAt({0, 0, 100}));
		const int Load = Sim.AddBody(Weight({0, 0, 80}, Mass));
		const int Line = Sim.AddLine(LineOf(EHarnessClass::Climber, Anchor, ESeat::Static, Load, ESeat::Body));
		Sim.Engage(Line, ELineMode::Hang);
		RunFor(Sim, 1.0);
		const double Creep = (Mass * G - 4000.0) / Mass;  // m/s² the load falls at while slipping
		KCHECK((Sim.Line(Line).EventsSeen & LineEvents::Slipping) != 0);
		KCHECK_NEAR(Sim.Line(Line).State.AppliedForce, 4000.0, 1.0);
		KCHECK_NEAR(Sim.Line(Line).MaxAppliedForce, 4000.0, 1.0);  // Never more than the Hang load
		KCHECK_NEAR(Sim.Line(Line).State.SlipAccum, 0.5 * Creep, 0.03 * Creep);
		KCHECK_NEAR(Sim.Body(Load).Velocity.Z, -Creep, 0.01 * Creep);
		KCHECK_NEAR(LineLength(Sim, Sim.Line(Line)), RestLength(Sim.Line(Line)), 1e-6);
	}
}

// A jolt slips the line until it can hold again: the slip is the drop's v²/2a at the Hang
// load, at any tick rate.
KTEST(JoltSlipDependsOnTheLoadNotTheTickRate)
{
	const double Hold = (4000.0 - 90.0 * G) / 90.0;  // m/s² the line decelerates the load at
	for (const double Drop : {3.0, 8.0, 20.0, 45.0})
	{
		const double Expected = Drop * Drop / (2.0 * Hold);
		const double At60 = JoltSlip(Drop, StepSeconds);
		const double At120 = JoltSlip(Drop, StepSeconds / 2.0);
		// The catch is first order in the step: within a step's travel of the drop.
		KCHECK_NEAR(At60, Expected, 0.03 * Expected + Drop * StepSeconds);
		KCHECK_NEAR(At120, Expected, 0.03 * Expected + Drop * StepSeconds / 2.0);
	}
	KCHECK_NEAR(JoltSlip(0.5, StepSeconds), 0.0, 1e-9);  // 90 kg at 0.5 m/s: held
}

// A standing player holding a Hang line to a falling container is pulled with the Hang load
// at most, never more (round 2: an unlimited arrest yanked them to 8.7 m/s in one step).
KTEST(AJoltNeverPullsHarderThanTheHangLoad)
{
	FReferenceSim Sim(Tables(), StepSeconds);
	const int Climber = Sim.AddBody(PlayerAt({0, 0, 10}, EHarnessClass::Climber, true));
	FSimBody Container = Weight({3, 0, 10}, 2000.0);
	Container.Velocity = {0, 0, -14};
	const int Load = Sim.AddBody(Container);
	const int Line = Sim.AddLine(LineOf(EHarnessClass::Climber, Climber, ESeat::OwnPlate, Load, ESeat::Body));
	Sim.Engage(Line, ELineMode::Hang);
	for (int I = 0; I < 30; ++I)
	{
		Sim.Step();
		KCHECK(Sim.Line(Line).State.AppliedForce <= 4000.0 * (1.0 + 1e-9));
	}
	KCHECK(Sim.Body(Climber).Velocity.Length() <= 4000.0 / 90.0 * 0.5 + 1e-6);
}

// Two lines catching a dropped load share it symmetrically, whatever order they are solved in.
KTEST(SymmetricLinesCatchSymmetrically)
{
	FReferenceSim Sim(Tables(), StepSeconds);
	const int Left = Sim.AddBody(StaticAt({-3, 0, 30}));
	const int Right = Sim.AddBody(StaticAt({3, 0, 30}));
	FSimBody Dropped = Weight({0, 0, 26}, 2000.0);
	Dropped.Velocity = {0, 0, -12};
	const int Load = Sim.AddBody(Dropped);
	const int First = Sim.AddLine(LineOf(EHarnessClass::Rigger, Left, ESeat::Static, Load, ESeat::Body));
	const int Second = Sim.AddLine(LineOf(EHarnessClass::Rigger, Right, ESeat::Static, Load, ESeat::Body));
	Sim.Engage(First, ELineMode::Hang);
	Sim.Engage(Second, ELineMode::Hang);
	RunFor(Sim, 3.0);
	KCHECK_NEAR(Sim.Body(Load).Position.X, 0.0, 1e-6);
	KCHECK_NEAR(Sim.Line(First).State.SlipAccum, Sim.Line(Second).State.SlipAccum, 1e-6);
	KCHECK_NEAR(Sim.Line(First).MaxAppliedForce, Sim.Line(Second).MaxAppliedForce, 1.0);
	KCHECK(Sim.Line(First).MaxAppliedForce <= 12000.0 * (1.0 + 1e-9));
}

// A chain of light links under a heavy load converges: lengths at rest, the load at rest.
KTEST(ChainsOfLightAndHeavyBodiesHold)
{
	for (const int Links : {2, 7})
	{
		FReferenceSim Sim(Tables(), StepSeconds);
		int Above = Sim.AddBody(StaticAt({0, 0, 40}));
		std::vector<int> Lines;
		for (int I = 1; I <= Links + 1; ++I)
		{
			const bool bLoad = I == Links + 1;
			const int Body = Sim.AddBody(Weight({0, 0, 40.0 - 2.0 * I}, bLoad ? 1000.0 : (Links == 2 ? 12.0 : 5.0)));
			Lines.push_back(Sim.AddLine(LineOf(EHarnessClass::Rigger, Above, ESeat::Body, Body, ESeat::Body)));
			Sim.Engage(Lines.back(), ELineMode::Hang);
			Above = Body;
		}
		RunFor(Sim, 4.0);
		for (const int Line : Lines)
		{
			KCHECK_NEAR(LineLength(Sim, Sim.Line(Line)), 2.0, 1e-4);
			KCHECK((Sim.Line(Line).EventsSeen & LineEvents::Slipping) == 0);
		}
		KCHECK(std::abs(Sim.Body(Above).Velocity.Z) < 1e-3);
		KCHECK_NEAR(Sim.Line(Lines.front()).State.AppliedForce, (1000.0 + Links * (Links == 2 ? 12.0 : 5.0)) * G, 5.0);
	}
}

// A short, fast line keeps its speed and reads its true load: a Bola rod, and a light body on
// a short rope.
KTEST(ShortFastLinesKeepTheirSwing)
{
	FReferenceSim Bola(Tables(), StepSeconds);
	FSimBody Left = ObjectAt({-0.5, 0, 0}, 20.0);
	Left.Velocity = {0, 15, 0};
	FSimBody Right = ObjectAt({0.5, 0, 0}, 20.0);
	Right.Velocity = {0, -15, 0};
	const int A = Bola.AddBody(Left);
	const int B = Bola.AddBody(Right);
	FSimLine Rod = LineOf(EHarnessClass::Rigger, A, ESeat::Body, B, ESeat::Body);
	Rod.State.bRodMode = true;
	const int Line = Bola.AddLine(Rod);
	Bola.Engage(Line, ELineMode::Hang);
	Bola.Step();  // The first step starts with no turn yet: half the centripetal impulse
	for (int I = 0; I < 120; ++I)
	{
		Bola.Step();
		KCHECK_NEAR(Bola.Body(A).Velocity.Length(), 15.0, 0.3);
		KCHECK_NEAR(Bola.Line(Line).State.AppliedForce, 10.0 * 30.0 * 30.0 / 1.0, 0.03 * 9000.0);
		KCHECK_NEAR(LineLength(Bola, Bola.Line(Line)), 1.0, 0.01);
	}

	FReferenceSim Short(Tables(), StepSeconds);
	const int Pivot = Short.AddBody(StaticAt(Origin));
	FSimBody Fast = ObjectAt({0.6, 0, 0}, 5.0);
	Fast.Velocity = {0, 20, 0};
	const int Body = Short.AddBody(Fast);
	const int Rope = Short.AddLine(LineOf(EHarnessClass::Climber, Pivot, ESeat::Static, Body, ESeat::Body));
	Short.Engage(Rope, ELineMode::Hang);
	Short.Step();
	for (int I = 0; I < 120; ++I)
	{
		Short.Step();
		KCHECK_NEAR(Short.Body(Body).Velocity.Length(), 20.0, 0.4);
		KCHECK_NEAR(Short.Line(Rope).State.AppliedForce, 5.0 * 400.0 / 0.6, 0.03 * 3333.0);
	}
}

// A slipping rope never pushes: a load just over the Hang load starts slipping from rest no
// faster than free fall.
KTEST(ASlippingRopeNeverPushes)
{
	FReferenceSim Sim(Tables(), StepSeconds);
	const int Anchor = Sim.AddBody(StaticAt({0, 0, 20}));
	const int Load = Sim.AddBody(Weight({0, 0, 15}, 410.0));  // 4.02 kN on a 4 kN line
	const int Line = Sim.AddLine(LineOf(EHarnessClass::Climber, Anchor, ESeat::Static, Load, ESeat::Body));
	Sim.Engage(Line, ELineMode::Hang);
	Sim.Step();
	KCHECK(Sim.Body(Load).Velocity.Z >= -G * StepSeconds);
	KCHECK(Sim.Body(Load).Velocity.Z < 0.0);
	KCHECK((Sim.Line(Line).EventsSeen & LineEvents::Slipping) != 0);
}

// An overloaded rod in compression gives way inward; it never lengthens or throws the load.
KTEST(AnOverloadedRodGivesWayInward)
{
	FReferenceSim Sim(Tables(), StepSeconds);
	const int Anchor = Sim.AddBody(StaticAt(Origin));
	FSimBody Ball = ObjectAt({5, 0, 0}, 50.0);
	Ball.Velocity = {-30, 0, 0};
	const int Moving = Sim.AddBody(Ball);
	FSimLine Rod = LineOf(EHarnessClass::Climber, Anchor, ESeat::Static, Moving, ESeat::Body);
	Rod.State.bRodMode = true;
	const int Line = Sim.AddLine(Rod);
	Sim.Engage(Line, ELineMode::Hang);
	for (int I = 0; I < 30; ++I)
	{
		Sim.Step();
		KCHECK(RestLength(Sim.Line(Line)) <= 5.0 + 1e-9);
		KCHECK(Sim.Body(Moving).Velocity.X <= 1e-9);
	}
	KCHECK(Sim.Line(Line).State.SlipAccum < 0.0);
}

// 6.2a: Hang loads add. Two Rigger lines hold 1,300 kg (12.8 kN against 24 kN) without
// slipping and share it; 2,600 kg (25.5 kN) slips both.
KTEST(TwoHangLinesShareTheLoad)
{
	for (const double Mass : {1300.0, 2600.0})
	{
		FReferenceSim Sim(Tables(), StepSeconds);
		const int Left = Sim.AddBody(StaticAt({-1, 0, 30}));
		const int Right = Sim.AddBody(StaticAt({1, 0, 30}));
		const int Load = Sim.AddBody(Weight({0, 0, 25}, Mass));
		const int First = Sim.AddLine(LineOf(EHarnessClass::Rigger, Left, ESeat::Static, Load, ESeat::Body));
		const int Second = Sim.AddLine(LineOf(EHarnessClass::Rigger, Right, ESeat::Static, Load, ESeat::Body));
		Sim.Engage(First, ELineMode::Hang);
		Sim.Engage(Second, ELineMode::Hang);
		RunFor(Sim, 1.0);
		const bool bOver = Mass * G * std::sqrt(26.0) / 5.0 > 24000.0;
		const uint32_t Slipped = (Sim.Line(First).EventsSeen | Sim.Line(Second).EventsSeen) & LineEvents::Slipping;
		KCHECK((Slipped != 0) == bOver);
		if (!bOver)
		{
			const double Each = Mass * G / 2.0 * std::sqrt(26.0) / 5.0;  // Along each angled line
			KCHECK_NEAR(Sim.Line(First).State.AppliedForce, Each, 0.01 * Each);
			KCHECK_NEAR(Sim.Line(Second).State.AppliedForce, Each, 0.01 * Each);
			KCHECK_NEAR(Sim.Body(Load).Position.Z, 25.0, 0.005);
		}
	}
}

// The Gallows: a Rigger reels a grounded 220 kg Breaker up off their feet at 1.0 m/s.
KTEST(GallowsHoistsAGroundedPlayer)
{
	FReferenceSim Sim(Tables(), StepSeconds);
	const int Anchor = Sim.AddBody(StaticAt({0, 0, 10}));
	const int Breaker = Sim.AddBody(PlayerAt({0, 0, 4}, EHarnessClass::Breaker, true));
	const int Line = Sim.AddLine(LineOf(EHarnessClass::Rigger, Anchor, ESeat::Static, Breaker, ESeat::OtherPlate));
	Sim.Engage(Line, ELineMode::Hang);
	RunFor(Sim, 0.5);
	KCHECK_NEAR(Sim.Body(Breaker).Position.Z, 4.0, 1e-9);  // Hanging slack-free, still standing
	KCHECK(Sim.Body(Breaker).Player.bGrounded);
	Sim.SetReelInput(Line, -1);
	RunFor(Sim, 2.0);
	KCHECK_NEAR(Sim.Line(Line).State.ReelAccum, -2.0, 0.02);
	KCHECK_NEAR(Sim.Body(Breaker).Position.Z, 6.0, 0.03);
	KCHECK_NEAR(LineLength(Sim, Sim.Line(Line)), RestLength(Sim.Line(Line)), 1e-6);
	KCHECK(!Sim.Body(Breaker).Player.bGrounded);
	KCHECK((Sim.Line(Line).EventsSeen & LineEvents::Slipping) == 0);
	Sim.SetReelInput(Line, 0);
	RunFor(Sim, 1.0);
	KCHECK_NEAR(Sim.Line(Line).State.AppliedForce, 220.0 * G, 5.0);  // Hanging
	KCHECK_NEAR(Sim.Body(Breaker).Position.Z, 6.0, 0.03);
}

// A Hang line that lifts less than a player's weight leaves them standing.
KTEST(AHangLineUnderWeightDoesNotLift)
{
	FReferenceSim Sim(Tables(), StepSeconds);
	const int Climber = Sim.AddBody(PlayerAt(Origin, EHarnessClass::Climber, true));
	const int Load = Sim.AddBody(Weight({0, 0, 4}, 30.0));  // On a rod above: pushes down
	FSimLine Rod = LineOf(EHarnessClass::Climber, Climber, ESeat::OwnPlate, Load, ESeat::Body);
	Rod.State.bRodMode = true;
	Sim.Engage(Sim.AddLine(Rod), ELineMode::Hang);
	RunFor(Sim, 1.0);
	KCHECK(Sim.Body(Climber).Player.bGrounded);
	KCHECK_NEAR(Sim.Body(Climber).Position.Z, 0.0, 1e-9);
	KCHECK_NEAR(Sim.Body(Load).Position.Z, 4.0, 1e-3);
}

// Anchor Drop: a 500 kg load off an edge on a grounded Rigger's Hang line drags the Rigger.
KTEST(AnchorDropDragsAGroundedPlayer)
{
	FReferenceSim Sim(Tables(), StepSeconds);
	const int Rigger = Sim.AddBody(PlayerAt(Origin, EHarnessClass::Rigger, true));
	const int Load = Sim.AddBody(Weight({3, 0, 0}, 500.0));  // Just past the edge, no floor
	const int Line = Sim.AddLine(LineOf(EHarnessClass::Breaker, Rigger, ESeat::OtherPlate, Load, ESeat::Body));
	Sim.Engage(Line, ELineMode::Hang);
	RunFor(Sim, 0.8);
	KCHECK(Sim.Body(Rigger).Position.X > 1.0);
	KCHECK(Sim.Body(Rigger).Position.Z >= -1e-9);  // The floor holds them up
	KCHECK(Sim.Body(Rigger).Footing == EFooting::Airborne);  // Over foot-hold: off their feet
	KCHECK_NEAR(LineLength(Sim, Sim.Line(Line)), RestLength(Sim.Line(Line)), 0.01);
}

// Overboard countered: a teammate's Hang line to a floor anchor holds the victim against the
// Jacker's 600 N push.
KTEST(AHangLineHoldsAnOverboardVictim)
{
	FReferenceSim Sim(Tables(), StepSeconds);
	const int Jacker = Sim.AddBody(PlayerAt(Origin, EHarnessClass::Rigger, true, true));
	const int Victim = Sim.AddBody(PlayerAt({1, 0, 0}, EHarnessClass::Rigger, true));
	const int Anchor = Sim.AddBody(StaticAt({-2, 0, 0}));
	Sim.Engage(Sim.AddLine(LineOf(EHarnessClass::Rigger, Jacker, ESeat::OwnPlate, Victim, ESeat::OtherPlate)),
	           ELineMode::Jack);
	const int Hold = Sim.AddLine(LineOf(EHarnessClass::Climber, Anchor, ESeat::Static, Victim, ESeat::OtherPlate));
	Sim.Engage(Hold, ELineMode::Hang);
	RunFor(Sim, 2.0);
	KCHECK(std::abs(Sim.Body(Victim).Position.X - 1.0) < 0.01);
	KCHECK_NEAR(Sim.Line(Hold).State.AppliedForce, 600.0, 1.0);
}

// 6.2: a Jack starts a placed object; the Kick and the push are tested against break-free.
KTEST(AJackStartsAPlacedObject)
{
	FReferenceSim Sim(Tables(), StepSeconds);
	const int Anchor = Sim.AddBody(StaticAt(Origin));
	const int Cylinder = Sim.AddBody(ObjectAt({1, 0, 0}, 45.0, EPlacement::Rack));
	Sim.Engage(Sim.AddLine(LineOf(EHarnessClass::Rigger, Anchor, ESeat::OwnPlate, Cylinder, ESeat::Body)),
	           ELineMode::Jack);
	RunFor(Sim, 0.5);
	KCHECK(Sim.Body(Cylinder).bBrokenFreeLatched);
	KCHECK(Sim.Body(Cylinder).Position.X > 2.0);
}

// Static on both ends the Jack still pushes at T, so the manifold reads its 600 N trigger.
KTEST(AJackBetweenStaticEndsAppliesTension)
{
	FReferenceSim Sim(Tables(), StepSeconds);
	const int Wall = Sim.AddBody(StaticAt(Origin));
	FSimBody Manifold = ObjectAt({2, 0, 0}, 400.0);
	Manifold.Object.bFixedScenery = true;
	const int Fixed = Sim.AddBody(Manifold);
	const int Line = Sim.AddLine(LineOf(EHarnessClass::Breaker, Wall, ESeat::OwnPlate, Fixed, ESeat::Body));
	Sim.Engage(Line, ELineMode::Jack);
	RunFor(Sim, 0.5);
	KCHECK_NEAR(Sim.Line(Line).MaxAppliedForce, 1000.0, 1e-9);
	KCHECK(Sim.Line(Line).MaxAppliedForce >= Tables().Volatile(EVolatile::GasManifold).TriggerJackN);
	KCHECK_NEAR(Sim.Body(Fixed).Position.X, 2.0, 0.0);
}

// 3.3: the Kick against a brace is computed on the braced player's free mass and breaks it.
KTEST(TheKickBreaksABrace)
{
	FReferenceSim Sim(Tables(), StepSeconds);
	const int Rigger = Sim.AddBody(PlayerAt(Origin, EHarnessClass::Rigger, true, true));
	const int Wall = Sim.AddBody(StaticAt({-1, 0, 0}));
	Sim.Engage(Sim.AddLine(LineOf(EHarnessClass::Rigger, Rigger, ESeat::OwnPlate, Wall, ESeat::Static)),
	           ELineMode::Jack);
	RunFor(Sim, 0.3 + StepSeconds);
	KCHECK(!Sim.Body(Rigger).Player.bBraced);
	KCHECK(Sim.Body(Rigger).Footing == EFooting::Airborne);
	KCHECK_NEAR(Sim.Body(Rigger).Velocity.Length(), 12.0, 0.5);
}

// v0.2.1: a Doubler's 800 N starts a bolted object on any axis.
KTEST(BreakFreeHoldsOnAnyAxis)
{
	for (const FVec3 Where : {FVec3{5, 0, 0}, FVec3{1, 2, 2}, FVec3{3, 1, 7}, FVec3{-2, 5, 1}})
	{
		FReferenceSim Sim(Tables(), StepSeconds);
		const int Anchor = Sim.AddBody(StaticAt(Origin));
		const int Bolted = Sim.AddBody(ObjectAt(Where, 100.0, EPlacement::Bolted));
		Sim.Engage(Sim.AddLine(LineOf(EHarnessClass::Doubler, Anchor, ESeat::OwnPlate, Bolted, ESeat::Body)),
		           ELineMode::Haul);
		RunFor(Sim, 1.0);
		KCHECK(Sim.Body(Bolted).bBrokenFreeLatched);
	}
}

// 6.7: a Lift counterweight pulled off its path is a simulated body from then on.
KTEST(AReleasedAttachmentStaysReleased)
{
	FReferenceSim Sim(Tables(), StepSeconds);
	const int Anchor = Sim.AddBody(StaticAt({10, 0, 20}));
	FSimBody Counterweight = Weight({0, 0, 20}, 4000.0);
	Counterweight.Object.bRated = true;
	Counterweight.Object.bKinematicAttachment = true;
	const int Body = Sim.AddBody(Counterweight);
	const int Line = Sim.AddLine(LineOf(EHarnessClass::Breaker, Anchor, ESeat::OwnPlate, Body, ESeat::Body));
	Sim.Engage(Line, ELineMode::Haul);
	RunFor(Sim, 0.5);
	KCHECK(Sim.Body(Body).bBrokenFreeLatched);
	Sim.SetTension(Line, 0.0);
	const double Z = Sim.Body(Body).Position.Z;
	RunFor(Sim, 1.0);
	KCHECK(Z - Sim.Body(Body).Position.Z > 4.0);  // Falling, not frozen on the path
}

// A slipping line that reaches the end of its filament lets go; a Reel stops at the class
// line length and never shortens the line. No body passes the hard velocity cap after the
// Hang solve.
KTEST(TheLineLengthBoundsSlipAndReel)
{
	FReferenceSim Sim(Tables(), StepSeconds);
	const int Anchor = Sim.AddBody(StaticAt({0, 0, 100}));
	const int Load = Sim.AddBody(Weight({0, 0, 50.05}, 3000.0));  // Engaged near the end: no exploit
	const int Line = Sim.AddLine(LineOf(EHarnessClass::Climber, Anchor, ESeat::Static, Load, ESeat::Body));
	Sim.Engage(Line, ELineMode::Hang);
	RunFor(Sim, 1.0);
	KCHECK(Sim.Line(Line).State.Mode == ELineMode::Slack);
	KCHECK((Sim.Line(Line).EventsSeen & LineEvents::RanOut) != 0);
	KCHECK(Sim.Line(Line).MaxAppliedForce <= 4000.0 * (1.0 + 1e-9));

	FReferenceSim Reel(Tables(), StepSeconds);
	const int Top = Reel.AddBody(StaticAt({0, 0, 100}));
	const int Held = Reel.AddBody(Weight({0, 0, 52}, 200.0));  // L0 = 48 m on a 50 m line
	const int Winch = Reel.AddLine(LineOf(EHarnessClass::Climber, Top, ESeat::Static, Held, ESeat::Body));
	Reel.Engage(Winch, ELineMode::Hang);
	Reel.SetReelInput(Winch, 1);
	double PreviousRest = RestLength(Reel.Line(Winch));
	double MaxSpeed = 0.0;
	for (int I = 0; I < 240; ++I)
	{
		Reel.Step();
		const double Rest = RestLength(Reel.Line(Winch));
		KCHECK(Rest <= 50.0 + 1e-9);
		KCHECK(Rest >= PreviousRest - 1e-9);
		PreviousRest = Rest;
		MaxSpeed = std::max(MaxSpeed, Reel.Body(Held).Velocity.Length());
	}
	KCHECK(Reel.Line(Winch).State.Mode == ELineMode::Hang);
	KCHECK_NEAR(RestLength(Reel.Line(Winch)), 50.0, 1e-9);
	KCHECK_NEAR(LineLength(Reel, Reel.Line(Winch)), 50.0, 1e-4);
	KCHECK(MaxSpeed <= 50.0 + 1e-9);
}

// A winch cannot gain on ends that cannot move, so a placed object freed later does not jump.
KTEST(AReelOnAnImmovableEndDoesNothing)
{
	FReferenceSim Sim(Tables(), StepSeconds);
	const int Anchor = Sim.AddBody(StaticAt(Origin));
	const int Cylinder = Sim.AddBody(ObjectAt({5, 0, 0}, 45.0, EPlacement::Rack));
	const int Line = Sim.AddLine(LineOf(EHarnessClass::Rigger, Anchor, ESeat::Static, Cylinder, ESeat::Body));
	Sim.Engage(Line, ELineMode::Hang);
	Sim.SetReelInput(Line, -1);
	RunFor(Sim, 3.0);
	KCHECK_NEAR(RestLength(Sim.Line(Line)), 5.0, 1e-12);
	const int Haul = Sim.AddLine(LineOf(EHarnessClass::Rigger, Anchor, ESeat::OwnPlate, Cylinder, ESeat::Body));
	Sim.Engage(Haul, ELineMode::Haul);
	double X = Sim.Body(Cylinder).Position.X;
	for (int I = 0; I < 60; ++I)
	{
		Sim.Step();
		KCHECK(std::abs(Sim.Body(Cylinder).Position.X - X) < 0.2);
		X = Sim.Body(Cylinder).Position.X;
	}
}

// Reeled in to the Dogs-meet length, the load settles there instead of jittering.
KTEST(ReelingInStopsCleanlyAtTheDogs)
{
	FReferenceSim Sim(Tables(), StepSeconds);
	const int Anchor = Sim.AddBody(StaticAt({0, 0, 10}));
	const int Load = Sim.AddBody(Weight({0, 0, 8}, 100.0));
	const int Line = Sim.AddLine(LineOf(EHarnessClass::Rigger, Anchor, ESeat::Static, Load, ESeat::Body));
	Sim.Engage(Line, ELineMode::Hang);
	Sim.SetReelInput(Line, -1);
	RunFor(Sim, 3.0);
	KCHECK_NEAR(RestLength(Sim.Line(Line)), 0.5, 1e-9);
	for (int I = 0; I < 60; ++I)
	{
		Sim.Step();
		KCHECK_NEAR(LineLength(Sim, Sim.Line(Line)), 0.5, 1e-4);
		KCHECK(std::abs(Sim.Body(Load).Velocity.Z) < 0.01);
	}
}

// Engaged with the Dogs together, the line takes the Dogs-meet length: the load hangs from it
// and its speed does not build up unseen.
KTEST(AHangEngagedAtZeroLengthHangsAtTheDogs)
{
	FReferenceSim Sim(Tables(), StepSeconds);
	const int Anchor = Sim.AddBody(StaticAt({0, 0, 10}));
	const int Load = Sim.AddBody(Weight({0, 0, 10}, 100.0));
	const int Line = Sim.AddLine(LineOf(EHarnessClass::Rigger, Anchor, ESeat::Static, Load, ESeat::Body));
	Sim.Engage(Line, ELineMode::Hang);
	RunFor(Sim, 3.0);
	KCHECK_NEAR(Sim.Line(Line).State.L0, 0.5, 1e-12);
	KCHECK_NEAR(Sim.Body(Load).Position.Z, 10.0 - RestLength(Sim.Line(Line)), 1e-6);
	KCHECK(RestLength(Sim.Line(Line)) < 0.51);  // The catch after a 0.5 m drop slips a few mm
	KCHECK(Sim.Body(Load).Velocity.Length() < 0.01);
	KCHECK_NEAR(Sim.Line(Line).State.AppliedForce, 100.0 * G, 1.0);
}

// 6.2 and 6.7: the Kick counts toward break-free at T, so a Climber's Kick starts a racked
// cylinder (200 N) but not a stack (500 N), a bolted object (800 N) or a Lift counterweight
// (1,000 N).
KTEST(TheKickCountsTowardBreakFreeAtT)
{
	const auto Kicked = [](EPlacement Placement, bool bAttachment) {
		FReferenceSim Sim(Tables(), StepSeconds);
		const int Wall = Sim.AddBody(StaticAt(Origin));
		FSimBody Target = ObjectAt({1, 0, 0}, bAttachment ? 4000.0 : 100.0, Placement);
		Target.Object.bRated = bAttachment;
		Target.Object.bKinematicAttachment = bAttachment;
		const int Body = Sim.AddBody(Target);
		Sim.Engage(Sim.AddLine(LineOf(EHarnessClass::Climber, Wall, ESeat::OwnPlate, Body, ESeat::Body)),
		           ELineMode::Jack);
		RunFor(Sim, 0.5);
		return Sim.Body(Body).bBrokenFreeLatched;
	};
	KCHECK(Kicked(EPlacement::Rack, false));
	KCHECK(!Kicked(EPlacement::Bolted, false));
	KCHECK(!Kicked(EPlacement::Stack, false));  // 400 N against a 500 N stack
	KCHECK(!Kicked(EPlacement::Loose, true));
}

// 6.1 and 6.2: the Scrapper bonus is speed on small props, not force against placement: a
// Scrapper's 400 N Haul does not start a stacked 60 kg crate (500 N), though it moves a loose one faster.
KTEST(TheScrapperBonusDoesNotStartAStack)
{
	FReferenceSim Sim(Tables(), StepSeconds);
	const int Anchor = Sim.AddBody(StaticAt(Origin));
	const int Crate = Sim.AddBody(ObjectAt({5, 0, 0}, 60.0, EPlacement::Stack));
	const int Line = Sim.AddLine(LineOf(EHarnessClass::Scrapper, Anchor, ESeat::OwnPlate, Crate, ESeat::Body));
	Sim.Engage(Line, ELineMode::Haul);
	RunFor(Sim, 1.0);
	KCHECK(!Sim.Body(Crate).bBrokenFreeLatched);
	KCHECK_NEAR(Sim.Line(Line).MaxAppliedForce, 400.0 * 1.69, 1e-9);
}

// Set engaged from Slack takes the current length; it does not snap to a stale one.
KTEST(SetCapturesTheCurrentLength)
{
	FReferenceSim Sim(Tables(), StepSeconds);
	const int Anchor = Sim.AddBody(StaticAt(Origin));
	const int Crate = Sim.AddBody(ObjectAt({5, 0, 0}, 50.0));
	const int Line = Sim.AddLine(LineOf(EHarnessClass::Climber, Anchor, ESeat::Static, Crate, ESeat::Body));
	Sim.Engage(Line, ELineMode::Set);
	Sim.Step();
	KCHECK_NEAR(Sim.Body(Crate).Position.X, 5.0, 1e-9);
	KCHECK_NEAR(Sim.Line(Line).State.L0, 5.0, 1e-12);
}

// A line straight above a standing body takes none of the weight the floor carries: a Climber
// under a Gallows line stays on their feet until it is reeled, and a crate on the floor under a
// taut line reads no load.
KTEST(AVerticalLineOverTheFloorCarriesNothingUntilItLifts)
{
	FReferenceSim Sim(Tables(), StepSeconds);
	const int Anchor = Sim.AddBody(StaticAt({0, 0, 10}));
	const int Climber = Sim.AddBody(PlayerAt({0, 0, 4}, EHarnessClass::Climber, true));
	const int Line = Sim.AddLine(LineOf(EHarnessClass::Rigger, Anchor, ESeat::Static, Climber, ESeat::OtherPlate));
	Sim.Engage(Line, ELineMode::Hang);
	RunFor(Sim, 1.0);
	KCHECK(Sim.Body(Climber).Player.bGrounded);
	KCHECK(Sim.Body(Climber).Footing == EFooting::Skid);
	KCHECK(Sim.Line(Line).State.AppliedForce < 1.0);
	Sim.SetReelInput(Line, -1);
	RunFor(Sim, 1.0);
	KCHECK(!Sim.Body(Climber).Player.bGrounded);
	KCHECK_NEAR(Sim.Body(Climber).Position.Z, 5.0, 0.05);

	FReferenceSim Crate(Tables(), StepSeconds);
	const int Top = Crate.AddBody(StaticAt({0, 0, 10}));
	FSimBody Box = Weight({0, 0, 0}, 1000.0);
	Box.FloorZ = 0.0;
	const int Body = Crate.AddBody(Box);
	const int Taut = Crate.AddLine(LineOf(EHarnessClass::Climber, Top, ESeat::Static, Body, ESeat::Body));
	Crate.Engage(Taut, ELineMode::Hang);
	RunFor(Crate, 1.0);
	KCHECK(Crate.Line(Taut).State.AppliedForce < 1.0);
	KCHECK((Crate.Line(Taut).EventsSeen & LineEvents::Slipping) == 0);
}

// 3.2: a self-dog Jack off an object that holds is a launch, as off a wall: the player gets V.
KTEST(AKickOffAnObjectThatHoldsGivesTheWholeOfV)
{
	FReferenceSim Sim(Tables(), StepSeconds);
	const int Climber = Sim.AddBody(PlayerAt(Origin, EHarnessClass::Climber, true));
	const int Beam = Sim.AddBody(ObjectAt({-1, 0, 0}, 100.0, EPlacement::Bolted));  // 800 N: a Climber cannot start it
	Sim.Engage(Sim.AddLine(LineOf(EHarnessClass::Climber, Climber, ESeat::OwnPlate, Beam, ESeat::Body)),
	           ELineMode::Jack);
	RunFor(Sim, 0.3 + StepSeconds);
	KCHECK_NEAR(Sim.Body(Climber).Velocity.Length(), 18.0, 0.5);
	KCHECK(!Sim.Body(Beam).bBrokenFreeLatched);
	KCHECK_NEAR(Sim.Body(Beam).Position.X, -1.0, 0.0);
}

// A line that lets go reads no load from the next step.
KTEST(ALineThatLetsGoReadsZero)
{
	FReferenceSim Sim(Tables(), StepSeconds);
	const int Anchor = Sim.AddBody(StaticAt({0, 0, 100}));
	const int Load = Sim.AddBody(Weight({0, 0, 50.05}, 3000.0));
	const int Line = Sim.AddLine(LineOf(EHarnessClass::Climber, Anchor, ESeat::Static, Load, ESeat::Body));
	Sim.Engage(Line, ELineMode::Hang);
	TimeUntil(Sim, 2.0, [&] { return Sim.Line(Line).State.Mode == ELineMode::Slack; });
	Sim.Step();
	KCHECK_NEAR(Sim.Line(Line).State.AppliedForce, 0.0, 0.0);
}

// Drift is removed at most 5 cm per line per step: a rod engaged shorter than the Dogs-meet
// length opens gradually instead of jumping.
KTEST(DriftIsNeverRemovedInOneJump)
{
	FReferenceSim Sim(Tables(), StepSeconds);
	const int Left = Sim.AddBody(ObjectAt({0, 0, 0}, 20.0));
	const int Right = Sim.AddBody(ObjectAt({0.1, 0, 0}, 20.0));
	FSimLine Rod = LineOf(EHarnessClass::Rigger, Left, ESeat::Body, Right, ESeat::Body);
	Rod.State.bRodMode = true;
	const int Line = Sim.AddLine(Rod);
	Sim.Engage(Line, ELineMode::Hang);
	double Length = LineLength(Sim, Sim.Line(Line));
	for (int I = 0; I < 20; ++I)
	{
		Sim.Step();
		const double Next = LineLength(Sim, Sim.Line(Line));
		KCHECK(std::abs(Next - Length) <= 0.05 + 1e-9);
		Length = Next;
	}
	KCHECK_NEAR(Length, 0.5, 1e-6);
}

// The Scrapper bonus stays out of the Jack push's break-free force too: the post-Kick push of
// 400 N does not start a 500 N stack.
KTEST(TheScrapperBonusDoesNotStartAStackWithAPush)
{
	FReferenceSim Sim(Tables(), StepSeconds);
	const int Wall = Sim.AddBody(StaticAt(Origin));
	const int Crate = Sim.AddBody(ObjectAt({1, 0, 0}, 60.0, EPlacement::Stack));
	const int Line = Sim.AddLine(LineOf(EHarnessClass::Scrapper, Wall, ESeat::OwnPlate, Crate, ESeat::Body));
	Sim.Engage(Line, ELineMode::Jack);
	RunFor(Sim, 1.0);
	KCHECK(!Sim.Body(Crate).bBrokenFreeLatched);
	KCHECK_NEAR(Sim.Line(Line).MaxAppliedForce, 400.0 * 1.69, 1e-9);
}

namespace
{
FSimBody KinematicAt(const FVec3& Position, const FVec3& Velocity)
{
	FSimBody Body;
	Body.Kind = ESimBodyKind::Kinematic;
	Body.Position = Position;
	Body.Velocity = Velocity;
	return Body;
}
} // namespace

// 5.4 and 6.7: a kinematic dragging a Hang line moves whatever is on the other end. A braced
// Breaker's brace breaks under the Hang load and they are towed (the Caboose).
KTEST(AKinematicBreaksABraceThroughAHangLine)
{
	FReferenceSim Sim(Tables(), StepSeconds);
	const int Breaker = Sim.AddBody(PlayerAt(Origin, EHarnessClass::Breaker, true, true));
	const int Hauler = Sim.AddBody(KinematicAt({10, 0, 0}, {4, 0, 0}));
	const int Line = Sim.AddLine(LineOf(EHarnessClass::Rigger, Hauler, ESeat::Body, Breaker, ESeat::OtherPlate));
	Sim.Engage(Line, ELineMode::Hang);
	RunFor(Sim, 3.0);
	KCHECK(!Sim.Body(Breaker).Player.bBraced);
	KCHECK(Sim.Body(Breaker).Position.X > 5.0);
	KCHECK(LineLength(Sim, Sim.Line(Line)) <= 40.0 + 1e-6);
}

// A placed object, which a Hang line never starts (6.2), holds: the line pays out at its Hang
// load behind the kinematic until it runs out, never stretching past its length.
KTEST(AKinematicPaysOutAHangLineOnAPlacedObject)
{
	FReferenceSim Sim(Tables(), StepSeconds);
	const int Cylinder = Sim.AddBody(ObjectAt(Origin, 45.0, EPlacement::Rack));
	const int Hauler = Sim.AddBody(KinematicAt({10, 0, 0}, {4, 0, 0}));
	const int Line = Sim.AddLine(LineOf(EHarnessClass::Rigger, Hauler, ESeat::Body, Cylinder, ESeat::Body));
	Sim.Engage(Line, ELineMode::Hang);
	RunFor(Sim, 1.0);
	KCHECK((Sim.Line(Line).EventsSeen & LineEvents::Slipping) != 0);
	KCHECK_NEAR(Sim.Line(Line).State.AppliedForce, 12000.0, 1e-6);
	KCHECK_NEAR(RestLength(Sim.Line(Line)), LineLength(Sim, Sim.Line(Line)), 1e-6);
	RunFor(Sim, 10.0);
	KCHECK(Sim.Line(Line).State.Mode == ELineMode::Slack);
	KCHECK((Sim.Line(Line).EventsSeen & LineEvents::RanOut) != 0);
	KCHECK_NEAR(Sim.Body(Cylinder).Position.X, 0.0, 0.0);
}

// Lines between the same two bodies share the load in proportion to their Hang loads, and a
// load under their combined Hang load does not creak.
KTEST(ParallelLinesShareByHangLoad)
{
	FReferenceSim Sim(Tables(), StepSeconds);
	const int Anchor = Sim.AddBody(StaticAt({0, 0, 30}));
	const int Load = Sim.AddBody(Weight({0, 0, 25}, 1300.0));  // 12.75 kN on 4 + 12 kN
	const int Climber = Sim.AddLine(LineOf(EHarnessClass::Climber, Anchor, ESeat::Static, Load, ESeat::Body));
	const int Rigger = Sim.AddLine(LineOf(EHarnessClass::Rigger, Anchor, ESeat::Static, Load, ESeat::Body));
	Sim.Engage(Climber, ELineMode::Hang);
	Sim.Engage(Rigger, ELineMode::Hang);
	RunFor(Sim, 1.0);
	const double Total = 1300.0 * G;
	KCHECK_NEAR(Sim.Line(Climber).State.AppliedForce, Total * 4.0 / 16.0, 1.0);
	KCHECK_NEAR(Sim.Line(Rigger).State.AppliedForce, Total * 12.0 / 16.0, 1.0);
	KCHECK(((Sim.Line(Climber).EventsSeen | Sim.Line(Rigger).EventsSeen) & LineEvents::Slipping) == 0);
}

namespace
{
// A small portable generator, so the networks are the same on every standard library.
class FLcg
{
public:
	explicit FLcg(uint64_t Seed) : State(Seed * 2862933555777941757ull + 3037000493ull) {}
	double Unit()
	{
		State = State * 6364136223846793005ull + 1442695040888963407ull;
		return static_cast<double>(State >> 11) / 9007199254740992.0;  // [0, 1)
	}

private:
	uint64_t State;
};
} // namespace

// Random networks of ropes and rods, Hang loads from every class, reels, floors, statics and
// masses from 5 to 2,000 kg: the bounded solve always finishes exactly (never at its pivot
// limit), nothing goes non-finite, and nothing passes the hard velocity cap.
KTEST(RandomNetworksSolveExactly)
{
	const EHarnessClass Classes[] = {EHarnessClass::Climber, EHarnessClass::Scrapper, EHarnessClass::Rigger,
	                                 EHarnessClass::Doubler, EHarnessClass::Breaker};
	for (uint64_t Seed = 1; Seed <= 200; ++Seed)
	{
		FLcg Rng(Seed);
		FReferenceSim Sim(Tables(), StepSeconds);
		std::vector<int> Bodies;
		for (int I = 0; I < 8; ++I)
		{
			const FVec3 Where{Rng.Unit() * 8 - 4, Rng.Unit() * 8 - 4, Rng.Unit() * 6};
			if (Rng.Unit() < 0.2)
			{
				Bodies.push_back(Sim.AddBody(StaticAt(Where + FVec3{0, 0, 6})));
				continue;
			}
			FSimBody Body = Weight(Where, 5.0 * std::exp(Rng.Unit() * std::log(400.0)));
			if (Rng.Unit() < 0.5)
			{
				Body.FloorZ = 0.0;
			}
			Body.Velocity = {Rng.Unit() * 20 - 10, Rng.Unit() * 20 - 10, Rng.Unit() * 20 - 10};
			Bodies.push_back(Sim.AddBody(Body));
		}
		std::vector<int> Lines;
		for (int I = 0; I < 10; ++I)
		{
			const int A = static_cast<int>(Rng.Unit() * 8);
			const int B = (A + 1 + static_cast<int>(Rng.Unit() * 7)) % 8;
			FSimLine Line = LineOf(Classes[static_cast<int>(Rng.Unit() * 5)], Bodies[A], ESeat::Body, Bodies[B], ESeat::Body);
			Line.State.bRodMode = Rng.Unit() < 0.3;
			Lines.push_back(Sim.AddLine(Line));
			Sim.Engage(Lines.back(), ELineMode::Hang);
			if (Rng.Unit() < 0.3)
			{
				Sim.SetReelInput(Lines.back(), Rng.Unit() < 0.5 ? -1 : 1);
			}
		}
		RunFor(Sim, 2.0);
		for (const int Line : Lines)
		{
			KCHECK((Sim.Line(Line).EventsSeen & LineEvents::SolverLimit) == 0);
		}
		for (const int Body : Bodies)
		{
			const double Speed = Sim.Body(Body).Velocity.Length();
			KCHECK(std::isfinite(Sim.Body(Body).Position.Length()) && Speed <= 50.0 + 1e-9);
		}
	}
}
