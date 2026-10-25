// Bots (game-modes-and-match-flow section 9, characters/npcs/bot-crew.md): the cut reflex and
// what it does to the panic window, the backfill bot's rig choice and its limits, and the
// tutorial Probationers' schedule.
#include "Kinetic/Bots.h"
#include "Kinetic/Cutter.h"
#include "Kinetic/Spool.h"
#include "Kinetic/ReferenceSim.h"
#include "SimFixtures.h"

#include <algorithm>
#include <cmath>

using namespace Kinetic;
using namespace Kinetic::Test;

namespace
{
constexpr double Frame = 1.0 / 60.0;

// Ticks a defense at 60 Hz from Start until Seconds have passed: when it drew, and each swing.
struct FTimeline
{
	double DrawAt = -1.0;
	std::vector<std::pair<int, double>> Swings;
};

FTimeline Run(FBotDefense& D, double Start, double Seconds)
{
	FTimeline T;
	for (int I = 0; I <= static_cast<int>(Seconds / Frame); ++I)
	{
		const double Now = Start + I * Frame;
		const FBotDefense::FActions A = D.Tick(Now);
		if (A.bDraw && T.DrawAt < 0.0)
		{
			T.DrawAt = Now;
		}
		if (A.Swing)
		{
			T.Swings.push_back({*A.Swing, Now});
		}
	}
	return T;
}
} // namespace

KTEST(BotsReplayFromTheMatchSeed)
{
	FBotRandom A(42, 3), B(42, 3), C(42, 4), D(43, 3);
	bool bSame = true, bOtherSlot = false, bOtherSeed = false;
	for (int I = 0; I < 100; ++I)
	{
		const double X = A.Uniform();
		bSame = bSame && X == B.Uniform();
		bOtherSlot = bOtherSlot || X != C.Uniform();
		bOtherSeed = bOtherSeed || X != D.Uniform();
		KCHECK(X >= 0.0 && X < 1.0);
	}
	KCHECK(bSame && bOtherSlot && bOtherSeed);
	KCHECK(BotCallsign(EBotProfile::Backfill, 1) == "O-01");
	KCHECK(BotCallsign(EBotProfile::Probationer, 3) == "P-03");
}

namespace
{
FBotDefense DefenseFor(EBotProfile Profile, EHarnessClass Class, uint64_t Seed = 1)
{
	return FBotDefense(Profile, Class, ECutterVariant::Standard, Tables(), FBotRandom(Seed, 0));
}
} // namespace

KTEST(BackfillBotsCutBetweenOneAndOnePointSixSeconds)
{
	FBotDefense D = DefenseFor(EBotProfile::Backfill, EHarnessClass::Rigger, 7);
	KCHECK_NEAR(D.Warmup(), CutterWarmupSeconds(ECutterVariant::Standard, EHarnessClass::Rigger, Tables()), 1e-12);
	const int N = 4000;
	double Sum = 0.0, Low = 1e9, High = -1e9;
	int BelowMid = 0;
	for (int I = 0; I < N; ++I)
	{
		D.Seated(I, 10.0);
		const FBotCutPlan P = D.Plans().back();
		const double Delay = P.SwingAt - P.SeatedAt;
		Sum += Delay, Low = std::min(Low, Delay), High = std::max(High, Delay);
		BelowMid += Delay < 1.3 ? 1 : 0;
		KCHECK_NEAR(P.SwingAt - P.DrawAt, D.Warmup(), 1e-12);  // The warm-up ends at the swing
		D.Ended(I);
	}
	KCHECK(Low >= 1.0 && High < 1.6);
	KCHECK(Low < 1.01 && High > 1.59);  // The whole range is used
	KCHECK_NEAR(Sum / N, 1.3, 0.01);
	KCHECK_NEAR(static_cast<double>(BelowMid) / N, 0.5, 0.03);  // Uniform
	// A re-seat of the same line starts over, with a fresh draw.
	D.Seated(1, 20.0);
	D.Seated(1, 21.0);
	KCHECK(D.Plans().size() == 1 && D.Plans()[0].SeatedAt == 21.0);
}

KTEST(TheDefenseDrawsThenSwingsOnTime)
{
	// A Probationer (a Breaker) cuts at 1.4 s: the draw at 1.4 s less its 0.30 s warm-up.
	FBotDefense D = DefenseFor(EBotProfile::Probationer, EHarnessClass::Breaker);
	KCHECK_NEAR(D.Warmup(), 0.30, 1e-12);
	const FCutterTuning& Cutter = Tables().Cutter(ECutterVariant::Standard);
	KCHECK_NEAR(D.SwingCycle(), Cutter.Swing + Cutter.Recovery, 1e-12);
	D.Seated(5, 2.0);
	const FTimeline T = Run(D, 2.0, 1.45);
	KCHECK_NEAR(T.DrawAt, 2.0 + 1.4 - 0.30, Frame);
	KCHECK(T.Swings.size() == 1 && T.Swings[0].first == 5);
	KCHECK_NEAR(T.Swings[0].second, 3.4, Frame);
	D.Ended(5);  // The cut landed
	KCHECK(D.Plans().empty() && Run(D, 3.45, 2.0).Swings.empty());
}

KTEST(AMissedSwingIsTriedAgain)
{
	// The swing at 1.4 s does not end the line (aimed off, or refused): the bot swings again as
	// soon as it has recovered, 0.45 s on, until the line ends.
	FBotDefense D = DefenseFor(EBotProfile::Probationer, EHarnessClass::Breaker);
	D.Seated(3, 0.0);
	const FTimeline T = Run(D, 0.0, 2.4);
	KCHECK(T.Swings.size() == 3);
	KCHECK_NEAR(T.Swings[0].second, 1.4, Frame);
	KCHECK_NEAR(T.Swings[1].second - T.Swings[0].second, D.SwingCycle(), Frame);
	KCHECK_NEAR(T.Swings[2].second - T.Swings[1].second, D.SwingCycle(), Frame);
	D.Ended(3);
	KCHECK(Run(D, 2.5, 2.0).Swings.empty());
}

KTEST(OneCutterCutsTwoLinesInTurn)
{
	FBotDefense D = DefenseFor(EBotProfile::Probationer, EHarnessClass::Breaker);
	D.Seated(1, 0.0);
	D.Seated(2, 0.0);
	FTimeline T;
	for (int I = 0; I <= 180 && T.Swings.size() < 2; ++I)
	{
		const double Now = I * Frame;
		if (const std::optional<int> Swing = D.Tick(Now).Swing)
		{
			T.Swings.push_back({*Swing, Now});
			D.Ended(*Swing);  // Each cut lands
		}
	}
	KCHECK(T.Swings.size() == 2 && T.Swings[0].first != T.Swings[1].first);
	KCHECK_NEAR(T.Swings[0].second, 1.4, Frame);
	// The second waits for the swing and the recovery, and no longer: the server allows it then.
	const double Gap = T.Swings[1].second - T.Swings[0].second;
	KCHECK(Gap >= D.SwingCycle() - 1e-9 && Gap < D.SwingCycle() + Frame - 1e-9);
	// From every start frame, the second swing is never a frame later than the cycle.
	for (int Start = 0; Start < 600; ++Start)
	{
		FBotDefense E = DefenseFor(EBotProfile::Probationer, EHarnessClass::Breaker);
		const double Seat = Start * Frame;
		E.Seated(1, Seat);
		E.Seated(2, Seat);
		std::vector<double> At;
		for (int I = Start; I <= Start + 180 && At.size() < 2; ++I)
		{
			if (const std::optional<int> Swing = E.Tick(I * Frame).Swing)
			{
				At.push_back(I * Frame);
				E.Ended(*Swing);
			}
		}
		KCHECK(At.size() == 2 && At[1] - At[0] < E.SwingCycle() + Frame - 1e-9);
	}
}

KTEST(ALineThatEndsIsNotCut)
{
	FBotDefense D = DefenseFor(EBotProfile::Probationer, EHarnessClass::Breaker);
	D.Seated(1, 0.0);
	const FTimeline Before = Run(D, 0.0, 1.2);  // Drawn at 1.1 s
	KCHECK(Before.DrawAt > 0.0 && Before.Swings.empty());
	D.Ended(1);  // A teammate cut it
	KCHECK(Run(D, 1.2, 2.0).Swings.empty());
	// A new seat later starts a fresh draw: the Cutter was sheathed.
	D.Seated(2, 5.0);
	const FTimeline After = Run(D, 5.0, 1.5);
	KCHECK_NEAR(After.DrawAt, 5.0 + 1.1, Frame);
	KCHECK(After.Swings.size() == 1 && After.Swings[0].first == 2);
}

namespace
{
// A bench Haul from 15 m: the victim's distance dragged and speed at each frame from the seat.
struct FDragTrace
{
	std::vector<double> Dragged, Speed;
};

FDragTrace TraceHaul(EHarnessClass Attacker, EHarnessClass Victim)
{
	FReferenceSim Sim(Tables(), StepSeconds);
	const int A = Sim.AddBody(PlayerAt({0, 0, 0}, Attacker, true, true));
	const int B = Sim.AddBody(PlayerAt({15, 0, 0}, Victim, true));
	Sim.Engage(Sim.AddLine(LineOf(Attacker, A, ESeat::OwnPlate, B, ESeat::OtherPlate)), ELineMode::Haul);
	FDragTrace T{{0.0}, {0.0}};
	for (int I = 0; I < 120; ++I)
	{
		Sim.Step();
		T.Dragged.push_back(15.0 - Sim.Body(B).Position.X);
		T.Speed.push_back(-Sim.Body(B).Velocity.X);
	}
	return T;
}

// The share of backfill bot cuts after which the victim stops short of the edge, if a cut
// victim slows at their own foot-hold (05 5.3; the stop after a cut is open-items 7c).
double SurvivalStoppingAtFootHold(EHarnessClass Attacker, EHarnessClass Victim, int Trials)
{
	const FDragTrace T = TraceHaul(Attacker, Victim);
	const double Stop = Tables().Harness(Victim).FootHold;
	int Survived = 0;
	for (int Seed = 0; Seed < Trials; ++Seed)
	{
		FBotDefense D = DefenseFor(EBotProfile::Backfill, Victim, static_cast<uint64_t>(Seed));
		D.Seated(1, 0.0);
		for (size_t F = 1; F < T.Dragged.size(); ++F)
		{
			if (D.Tick(static_cast<double>(F) * Frame).Swing)
			{
				const double V = T.Speed[F];
				Survived += T.Dragged[F] + V * V / (2.0 * Stop) < 15.0 ? 1 : 0;
				break;
			}
		}
	}
	return static_cast<double>(Survived) / Trials;
}
} // namespace

KTEST(BackfillBotsCutBeforeEveryBenchEdge)
{
	// 05 4.3's drag for every attacker and victim class: a Haul from 15 m reaches the edge at
	// 1.85 s at the soonest, after the latest bot cut at 1.6 s. So a bot's cut always lands
	// before the victim reaches the edge.
	double Soonest = 1e9;
	for (const EHarnessClass Attacker : AllHarnessClasses)
	{
		for (const EHarnessClass Victim : AllHarnessClasses)
		{
			const FPanicRun Run = RunHaulPanic(Tables(), Attacker, Victim, 15.0, StepSeconds, 10.0);
			KCHECK(Run.DistanceAt.has_value());
			Soonest = std::min(Soonest, Run.DistanceAt.value_or(0.0));
		}
	}
	KCHECK_NEAR(Soonest, 1.85, 0.01);
	KCHECK(Soonest > BackfillCutMaxSeconds);
	// But the cut ends the pull, not the slide: the Climber cut at 1.6 s has been dragged 11 m
	// and is doing 15.5 m/s, so it needs about 30 m/s² to stop in the last 4 m.
	const FDragTrace T = TraceHaul(EHarnessClass::Breaker, EHarnessClass::Climber);
	KCHECK_NEAR(T.Dragged[96], 11.0, 0.1);
	KCHECK_NEAR(T.Speed[96], 15.5, 0.1);
	KCHECK_NEAR(T.Speed[96] * T.Speed[96] / (2.0 * (15.0 - T.Dragged[96])), 29.6, 0.5);
}

KTEST(BotSurvivalDependsOnTheStopAfterTheCut)
{
	// If a cut victim slows at their foot-hold, the backfill window gives: 05 4.3's own case (a
	// Breaker on a Rigger) about 73%; a Breaker on a Climber never; a Climber's Haul always.
	KCHECK_NEAR(SurvivalStoppingAtFootHold(EHarnessClass::Breaker, EHarnessClass::Rigger, 2000), 0.73, 0.04);
	KCHECK(SurvivalStoppingAtFootHold(EHarnessClass::Breaker, EHarnessClass::Climber, 500) == 0.0);
	KCHECK(SurvivalStoppingAtFootHold(EHarnessClass::Climber, EHarnessClass::Breaker, 500) == 1.0);
	// Over all 25 pairings, about 85%.
	double Sum = 0.0;
	for (const EHarnessClass Attacker : AllHarnessClasses)
	{
		for (const EHarnessClass Victim : AllHarnessClasses)
		{
			Sum += SurvivalStoppingAtFootHold(Attacker, Victim, 400);
		}
	}
	KCHECK_NEAR(Sum / 25.0, 0.85, 0.03);
}

namespace
{
FBotView RiggerAt(const FVec3& Position)
{
	FBotView V;
	V.Slot = 0;
	V.Team = 0;
	V.Class = EHarnessClass::Rigger;  // 40 m line
	V.Position = Position;
	return V;
}
} // namespace

KTEST(BotsNeverHaulATeammateOrUseMapCables)
{
	KCHECK(!BotMayUse({EBotProfile::Backfill, ELineMode::Haul, true}));
	KCHECK(BotMayUse({EBotProfile::Backfill, ELineMode::Jack, true}));  // A Slingshot off a teammate is fine
	KCHECK(BotMayUse({EBotProfile::Backfill, ELineMode::Haul, false}));
	KCHECK(!BotMayUse({EBotProfile::Backfill, ELineMode::Slack, false, true}));
	KCHECK(!BotMayUse({EBotProfile::Probationer, ELineMode::Jack, false}));
	KCHECK(!BotMayUse({EBotProfile::Probationer, ELineMode::Haul, false, false, true}));
	KCHECK(BotMayUse({EBotProfile::Backfill, ELineMode::Haul, false, false, true}));
	KCHECK(BotOverspinHeldSeconds == 0.0);
	KCHECK(OverspinStage(Tables().Harness(EHarnessClass::Doubler), Tables().Globals(), BotOverspinHeldSeconds) ==
	       EOverspinStage::None);

	// Only teammates about, with an edge anchor right there: nothing to do.
	FBotView V = RiggerAt({0, 0, 0});
	V.Others = {{1, 0, {10, 0, 0}, {}}, {2, 0, {12, 2, 0}, {}}};
	V.Anchors = {{{20, 0, 0}, true}};
	KCHECK(ChooseRig(V, Tables()).Rig == EBotRig::None);
	// An enemy further off than a teammate is still the one targeted.
	V.Others.push_back({3, 1, {25, 0, 0}, {}});
	V.Anchors = {{{35, 0, 0}, true}};
	const FBotIntent I = ChooseRig(V, Tables());
	KCHECK(I.Rig == EBotRig::HaulToAnchor && I.TargetSlot == 3 && I.Mode == ELineMode::Haul);
	// A Stalled enemy is no target.
	V.Others.back().bStanding = false;
	KCHECK(ChooseRig(V, Tables()).Rig == EBotRig::None);
}

KTEST(ChooseRigWorksTheBaseKit)
{
	FBotView V = RiggerAt({0, 0, 0});
	V.Others = {{1, 1, {20, 0, 0}, {}}};
	V.Anchors = {{{20, 12, 0}, false}, {{20, -6, 0}, true}, {{-10, 0, 0}, false}};

	// An edge anchor first, even with a shorter drag than the wall.
	FBotIntent I = ChooseRig(V, Tables());
	KCHECK(I.Rig == EBotRig::HaulToAnchor && I.TargetSlot == 1);
	KCHECK(I.DogA.X == 20 && I.DogB.Y == -6 && I.Mode == ELineMode::Haul);
	// An edge anchor too close to the enemy is no rig. The wall behind the bot would drag the
	// enemy through it: the wall beside them instead.
	V.Anchors[1].Point = {20, -3, 0};
	I = ChooseRig(V, Tables());
	KCHECK(I.Rig == EBotRig::HaulToAnchor && I.DogB.Y == 12);
	V.Anchors[0].Point = {20, 3, 0};
	KCHECK(ChooseRig(V, Tables()).Rig == EBotRig::None);  // Nothing left worth a Haul
	V.Anchors[0].Point = {20, 12, 0};

	// A Hauler arriving within 1.5 s with the enemy beside its track: Caboose before anything.
	V.Haulers = {{{0, 5, 0}, {40, 5, 0}, 0.0, 40.0}};  // Track along y = 5; the train at x = 0 now, 40 m/s
	I = ChooseRig(V, Tables());
	// Dog B goes to the track beside the enemy, 20 m along from the bot's end of it.
	KCHECK(I.Rig == EBotRig::Caboose && I.TargetSlot == 1 && I.DogB.X == 20 && I.DogB.Y == 5);
	KCHECK(I.Mode == ELineMode::Haul);
	KCHECK_NEAR(V.Haulers[0].ArrivesAt(V.Others[0].Position), 0.5, 1e-12);  // 20 m along at 40 m/s
	V.Haulers[0].ArrivesInSeconds = 1.2;  // At the stretch in 1.2 s, beside them in 1.7 s: too early
	KCHECK(ChooseRig(V, Tables()).Rig == EBotRig::HaulToAnchor);
	V.Haulers[0].ArrivesInSeconds = -0.6;  // Already past them
	KCHECK(ChooseRig(V, Tables()).Rig == EBotRig::HaulToAnchor);
	V.Haulers[0].ArrivesInSeconds = -0.4;  // Past the start of the stretch, not yet beside them
	KCHECK(ChooseRig(V, Tables()).Rig == EBotRig::Caboose);
	V.Haulers[0] = {{0, 12, 0}, {40, 12, 0}, 0.0, 40.0};  // A track 12 m from the enemy
	KCHECK(ChooseRig(V, Tables()).Rig == EBotRig::HaulToAnchor);
	V.Haulers.clear();

	// An enemy heading for a doorway, with only walls to Haul to: a Clothesline.
	V.Doorways = {{{24, -1, 1.2}, {24, 1, 1.2}}};
	V.Others[0].Velocity = {3, 0, 0};
	I = ChooseRig(V, Tables());
	KCHECK(I.Rig == EBotRig::Clothesline && I.Mode == ELineMode::Hang && I.bReleaseAfterSet);
	KCHECK(I.DogA.Y == -1 && I.DogB.Y == 1);
	V.Others[0].Velocity = {-3, 0, 0};  // Walking away from it
	KCHECK(ChooseRig(V, Tables()).Rig == EBotRig::HaulToAnchor);

	// Out of range: Slingshot off the wall behind, toward the enemy.
	V.Doorways.clear();
	V.Others[0].Position = {60, 0, 0};
	I = ChooseRig(V, Tables());
	KCHECK(I.Rig == EBotRig::Slingshot && I.bSelfDogA && I.Mode == ELineMode::Jack && I.DogB.X == -10);
	V.Anchors = {{{20, 12, 0}, false}};  // No wall behind
	KCHECK(ChooseRig(V, Tables()).Rig == EBotRig::None);

	// Busy: a line already live, or a Dog in its own plate.
	V.Others[0].Position = {20, 0, 0};
	KCHECK(ChooseRig(V, Tables()).Rig == EBotRig::HaulToAnchor);
	V.bLineOnSelf = true;
	KCHECK(ChooseRig(V, Tables()).Rig == EBotRig::None);
	V.bLineOnSelf = false, V.bLineLive = true;
	KCHECK(ChooseRig(V, Tables()).Rig == EBotRig::None);
}

namespace
{
// Ticks the crew at 60 Hz from Start for Seconds with a fixed view; the first tick each bot
// got each task.
struct FCrewLog
{
	std::array<std::vector<std::pair<EProbationerTask, double>>, 3> Tasks;
	std::array<double, 3> MissM{0, 0, 0};
	double FirstAt(int Bot, EProbationerTask Task) const
	{
		for (const auto& [T, At] : Tasks[static_cast<size_t>(Bot)])
		{
			if (T == Task) return At;
		}
		return -1.0;
	}
};

FCrewLog Drive(FProbationerCrew& Crew, FProbationerView V, double Start, double Seconds)
{
	FCrewLog Log;
	for (int I = 0; I <= static_cast<int>(Seconds / Frame); ++I)
	{
		V.Now = Start + I * Frame;
		const auto Orders = Crew.Tick(V);
		for (size_t B = 0; B < 3; ++B)
		{
			auto& L = Log.Tasks[B];
			if (L.empty() || L.back().first != Orders[B].Task)
			{
				L.push_back({Orders[B].Task, V.Now});
			}
			if (Orders[B].Task == EProbationerTask::FireDog)
			{
				Log.MissM[B] = Orders[B].AimMissM;
			}
		}
	}
	return Log;
}
} // namespace

KTEST(TheProbationersFireHaulAndHandOver)
{
	KCHECK_NEAR(FProbationerCrew::RecallSeconds, Tables().Harness(EHarnessClass::Breaker).Recall, 1e-12);
	FProbationerCrew Crew;
	FProbationerView V;
	V.DistanceToPlayer = {20, 22, 30};
	FCrewLog L = Drive(Crew, V, 0.0, 1.0);
	KCHECK(L.Tasks[0].size() == 1 && L.Tasks[0][0].first == EProbationerTask::WalkRoute);

	// At 15 m P-01 winds up for 1 s, then fires once and waits on the Dog.
	V.DistanceToPlayer = {15, 18, 30};
	L = Drive(Crew, V, 1.0, 2.0);
	KCHECK_NEAR(L.FirstAt(0, EProbationerTask::WindUp), 1.0, 1e-9);
	KCHECK_NEAR(L.FirstAt(0, EProbationerTask::FireDog), 2.0, Frame);
	KCHECK(L.MissM[0] == 0.0);
	KCHECK(L.Tasks[0].back().first == EProbationerTask::AwaitSeat);
	KCHECK(L.FirstAt(1, EProbationerTask::WindUp) < 0.0);  // Only the lead fires

	// The seat: brace 0.3 s, then Haul.
	Crew.Seated(0, 3.05);
	L = Drive(Crew, V, 3.05, 1.0);
	KCHECK_NEAR(L.FirstAt(0, EProbationerTask::Haul), 3.35, Frame);

	// Cut: the look and the recall, 1.4 s; no Dog for 6 s; P-02 steps forward.
	Crew.Cut(0, 4.1);
	KCHECK(Crew.Lead() == 1);
	V.DistanceToPlayer = {12, 14, 30};
	L = Drive(Crew, V, 4.1, 3.0);
	KCHECK(L.Tasks[0][0].first == EProbationerTask::Recall);
	KCHECK_NEAR(L.FirstAt(0, EProbationerTask::WalkRoute), 5.5, Frame);
	KCHECK_NEAR(L.FirstAt(1, EProbationerTask::WindUp), 4.1, 1e-9);
	KCHECK(L.FirstAt(0, EProbationerTask::WindUp) < 0.0);

	// P-02's Dog misses: a recall, then it may fire again.
	Crew.Missed(1, 7.2);
	L = Drive(Crew, V, 7.2, 1.5);
	KCHECK_NEAR(L.FirstAt(1, EProbationerTask::WindUp), 8.1, Frame);

	// A Probationer on the marker holds it.
	FProbationerCrew Hold;
	FProbationerView H;
	H.bOnMarker = {false, true, true};
	L = Drive(Hold, H, 0.0, 0.5);
	KCHECK(L.Tasks[1][0].first == EProbationerTask::HoldMarker && L.Tasks[0][0].first == EProbationerTask::WalkRoute);
}

KTEST(TheAvalancheAndTheCabooseComeOnceEach)
{
	FProbationerCrew Crew;
	FProbationerView V;
	V.PlayerToStackM = 5.0;
	FCrewLog L = Drive(Crew, V, 0.0, 5.0);
	KCHECK_NEAR(L.FirstAt(1, EProbationerTask::AvalancheWindUp), 0.0, 1e-9);
	KCHECK_NEAR(L.FirstAt(1, EProbationerTask::Avalanche), 2.0, Frame);
	KCHECK(Crew.AvalancheUsed());
	KCHECK(L.Tasks[1].size() == 3 && L.Tasks[1].back().first == EProbationerTask::Haul);  // On the stack's line
	Crew.Ended(1, 5.0);
	L = Drive(Crew, V, 5.0, 3.0);
	KCHECK(L.FirstAt(1, EProbationerTask::AvalancheWindUp) < 0.0);  // Once
	KCHECK(L.Tasks[1].back().first == EProbationerTask::WalkRoute);

	V.bKlaxon = true;
	V.PlayerToTrenchM = 12.0;
	L = Drive(Crew, V, 8.0, 1.0);
	KCHECK(L.FirstAt(2, EProbationerTask::Caboose) < 0.0);  // Too far from the trench
	V.PlayerToTrenchM = 9.0;
	L = Drive(Crew, V, 9.0, 2.0);
	KCHECK_NEAR(L.FirstAt(2, EProbationerTask::Caboose), 9.0, 1e-9);
	KCHECK(L.Tasks[2].size() == 2 && L.Tasks[2].back().first == EProbationerTask::Haul && Crew.CabooseUsed());
	Crew.Ended(2, 11.0);
	L = Drive(Crew, V, 11.0, 2.0);
	KCHECK(L.FirstAt(2, EProbationerTask::Caboose) < 0.0);  // Once
}

KTEST(ABotOnALineDoesNotFireAnother)
{
	// P-01 is cut, so P-02 leads; P-02 makes its Avalanche and stays on the stack's line, so it
	// does not fire at the player (a new Dog A would drop the stack) until that line ends.
	FProbationerCrew Crew;
	FProbationerView V;
	V.DistanceToPlayer = {30, 30, 30};
	V.PlayerToStackM = 5.0;
	Crew.Cut(0, 0.0);
	FCrewLog L = Drive(Crew, V, 0.0, 2.5);
	KCHECK(Crew.Lead() == 1 && L.FirstAt(1, EProbationerTask::Avalanche) > 0.0);
	V.DistanceToPlayer = {30, 10, 30};
	V.PlayerToStackM = 1e9;
	L = Drive(Crew, V, 2.5, 3.0);
	KCHECK(L.FirstAt(1, EProbationerTask::WindUp) < 0.0 && L.Tasks[1][0].first == EProbationerTask::Haul);
	Crew.Ended(1, 5.5);
	L = Drive(Crew, V, 5.5, 2.0);
	KCHECK_NEAR(L.FirstAt(1, EProbationerTask::WindUp), 5.5 + FProbationerCrew::RecallSeconds, Frame);
}

KTEST(ACutProbationerWaitsSixSecondsEvenWhenTheLeadComesBack)
{
	// All three cut in turn: the lead comes back to P-01 within its 6 s, and it waits them out.
	FProbationerCrew Crew;
	FProbationerView V;
	V.DistanceToPlayer = {10, 10, 10};
	Crew.Cut(0, 0.0);
	Crew.Cut(1, 0.1);
	Crew.Cut(2, 0.2);
	KCHECK(Crew.Lead() == 0);
	const FCrewLog L = Drive(Crew, V, 0.3, 7.0);
	KCHECK_NEAR(L.FirstAt(0, EProbationerTask::WindUp), FProbationerCrew::RefireAfterCutSeconds, Frame);
}

KTEST(ADownedProbationerPassesTheLead)
{
	FProbationerCrew Crew;
	FProbationerView V;
	V.DistanceToPlayer = {10, 10, 10};
	V.bAlive = {false, true, true};
	const FCrewLog L = Drive(Crew, V, 0.0, 0.5);
	KCHECK(Crew.Lead() == 1 && L.Tasks[0][0].first == EProbationerTask::Down);
	KCHECK_NEAR(L.FirstAt(1, EProbationerTask::WindUp), 0.0, 1e-9);
}

KTEST(AfterTwoScrapsTheProbationersMissOnPurpose)
{
	FProbationerCrew Crew;
	FProbationerView V;
	V.DistanceToPlayer = {10, 10, 10};
	V.PlayerScrapped = 2;
	V.PlayerToStackM = 3.0;
	V.bKlaxon = true;
	V.PlayerToTrenchM = 3.0;
	const FCrewLog L = Drive(Crew, V, 0.0, 3.0);
	KCHECK_NEAR(L.FirstAt(0, EProbationerTask::FireDog), 1.0, Frame);
	KCHECK(L.MissM[0] == FProbationerCrew::MissM);
	KCHECK(L.FirstAt(1, EProbationerTask::AvalancheWindUp) < 0.0 && L.FirstAt(2, EProbationerTask::Caboose) < 0.0);

	// A second scrap during the Avalanche's wind-up calls it off: it could be a third.
	FProbationerCrew Late;
	FProbationerView W;
	W.PlayerScrapped = 1;
	W.PlayerToStackM = 3.0;
	FCrewLog M = Drive(Late, W, 0.0, 1.0);
	KCHECK(M.FirstAt(1, EProbationerTask::AvalancheWindUp) == 0.0);
	W.PlayerScrapped = 2;
	M = Drive(Late, W, 1.0, 3.0);
	KCHECK(M.FirstAt(1, EProbationerTask::Avalanche) < 0.0 && M.FirstAt(1, EProbationerTask::AvalancheWindUp) < 0.0);
}
