// Claim with Tag-Out (docs/proposals/claim-tag-out.md): codes, reading plates, the crew board,
// the Call and its outcomes, the tag lockers, the beacons, the reserve and the clock.
#include "Kinetic/TagOut.h"
#include "TestHarness.h"

#include <set>

using namespace Kinetic;

namespace
{
constexpr double Tick = 1.0 / 60.0;
const std::array<FVec3, 2> Pads{FVec3{-68.0, -39.0, 8.0}, FVec3{68.0, 39.0, 8.0}};
const std::vector<FVec3> Lockers{FVec3{-6.0, 21.0, 8.0}, FVec3{28.0, -21.0, 8.0}};

// Two crews of two: slots 0 and 1 on team 0, 2 and 3 on team 1.
std::vector<FMatchPlayer> Crews()
{
	std::vector<FMatchPlayer> Roster;
	for (int Slot = 0; Slot < 4; ++Slot)
	{
		FMatchPlayer Player;
		Player.Slot = Slot;
		Player.Team = Slot / 2;
		Roster.push_back(Player);
	}
	return Roster;
}

FTagPose At(const FVec3& Chest, const FVec3& Forward = {1.0, 0.0, 0.0})
{
	FTagPose Pose;
	Pose.Chest = Chest;
	Pose.Forward = Forward;
	Pose.Right = {-Forward.Y, Forward.X, 0.0};  // Z up, Y right of X
	return Pose;
}

void Place(FTagOutMatch& Match, int Slot, const FVec3& Chest, double Speed = 0.0)
{
	Match.SetPose(Slot, At(Chest), Speed, false);
}

void Run(FTagOutMatch& Match, double Seconds, std::vector<bool> InView = std::vector<bool>(4, true))
{
	for (int I = 0, N = static_cast<int>(std::lround(Seconds / Tick)); I < N; ++I)
	{
		Match.Step(Tick, InView);
	}
}

// A match past its drop, everyone standing in the middle of the yard.
FTagOutMatch Live(uint64_t Seed = 7)
{
	FTagOutMatch Match(Crews(), Pads, Lockers, Seed);
	Run(Match, Match.Rules().DropSeconds);
	for (int Slot = 0; Slot < 4; ++Slot)
	{
		Place(Match, Slot, {Slot * 5.0, 0.0, 8.0});
	}
	Match.TakeEvents();
	return Match;
}

void Enter(FTagOutMatch& Match, int Caller, const FTagCode& Code)
{
	for (uint8_t Pip : Code.Pips)
	{
		Match.EnterPip(Caller, Pip);
	}
}

bool Has(const std::vector<FTagEventRecord>& Events, ETagEvent Kind, int Slot = NoPlayer)
{
	for (const FTagEventRecord& Event : Events)
	{
		if (Event.Kind == Kind && (Slot == NoPlayer || Event.Slot == Slot))
		{
			return true;
		}
	}
	return false;
}
} // namespace

KTEST(TagOutCodesAreSeededPipsFromOneToFour)
{
	std::set<int> Seen;
	for (int Slot = 0; Slot < 8; ++Slot)
	{
		for (int Stamp = 1; Stamp < 32; ++Stamp)
		{
			const FTagCode Code = FTagOutMatch::RollCode(11, Slot, Stamp);
			KCHECK(Code == FTagOutMatch::RollCode(11, Slot, Stamp));
			for (uint8_t Pip : Code.Pips)
			{
				KCHECK(Pip >= 1 && Pip <= 4);
				Seen.insert(Pip);
			}
		}
	}
	KCHECK(Seen.size() == 4);
	KCHECK(!(FTagOutMatch::RollCode(11, 0, 1) == FTagOutMatch::RollCode(12, 0, 1)) ||
	       !(FTagOutMatch::RollCode(11, 0, 2) == FTagOutMatch::RollCode(12, 0, 2)));
}

KTEST(TagOutPlatesReadInTheirConeAndRange)
{
	const FTagOutRules Rules;
	const FTagPose Target = At({0.0, 0.0, 1.4});  // Facing +X: the back faces -X, the left shoulder -Y
	const std::array<bool, PlateCount> Clear{true, true, true};
	constexpr uint8_t Left = 1, Back = 2, Right = 4;
	KCHECK(ReadablePlates({-10.0, 0.0, 1.4}, false, Target, Clear, Rules) == Back);
	KCHECK(ReadablePlates({-7.0, -7.0, 1.4}, false, Target, Clear, Rules) == (Left | Back));  // 45° off both
	KCHECK(ReadablePlates({0.0, 10.0, 1.4}, false, Target, Clear, Rules) == Right);
	KCHECK(ReadablePlates({10.0, 0.0, 1.4}, false, Target, Clear, Rules) == 0);  // The chest is blank
	KCHECK(ReadablePlates({-25.0, 0.0, 1.4}, false, Target, Clear, Rules) == 0);  // Past the naked eye
	KCHECK(ReadablePlates({-25.0, 0.0, 1.4}, true, Target, Clear, Rules) == Back);
	KCHECK(ReadablePlates({-49.0, 0.0, 1.4}, true, Target, Clear, Rules) == Back);
	KCHECK(ReadablePlates({-51.0, 0.0, 1.4}, true, Target, Clear, Rules) == 0);
	KCHECK(ReadablePlates({-38.0, 0.0, 1.4}, false, Target, Clear, Rules, true) == Back);  // The flag panel
	KCHECK(ReadablePlates({-10.0, 0.0, 1.4}, false, Target, {true, false, true}, Rules) == 0);  // Blocked
}

KTEST(TagOutTheBoardHoldsWhatTheCrewWasShown)
{
	FTagOutMatch Match = Live();
	const FTagCode Code = Match.Rigger(2).Code;
	Match.Show(0, 2, 2);  // The back
	KCHECK(Match.Read(0, 2).Known() == 1);
	KCHECK(Match.Read(0, 2).Pips[1] == Code.Pips[1]);
	KCHECK(!Match.Read(0, 2).Pips[0].has_value());
	Match.Show(1, 2, 1);  // A crewmate's look goes on the same board
	KCHECK(Match.Read(0, 2).Known() == 2);
	KCHECK(Match.Read(1, 2).Known() == 0);  // Nothing on the other crew's board
	Match.Show(2, 3, 7);
	KCHECK(Match.Read(1, 3).Known() == 0);  // Crewmates are not read
	const auto Events = Match.TakeEvents();
	KCHECK(Has(Events, ETagEvent::Revealed, 2));
	Match.Show(0, 2, 2);
	KCHECK(!Has(Match.TakeEvents(), ETagEvent::Revealed));  // Seen again: the age moves, no event
}

KTEST(TagOutACleanCallLocksTheTargetOut)
{
	FTagOutMatch Match = Live();
	const FTagCode Old = Match.Rigger(2).Code;
	KCHECK(!Match.SendCall(0, 2, true));  // Nothing entered
	Enter(Match, 0, Old);
	KCHECK(!Match.SendCall(0, 1, true));  // A crewmate
	KCHECK(Match.SendCall(0, 2, true));
	KCHECK(Match.CalledBy(2) == 0);
	Match.EnterPip(0, 1);  // Locked in while the lock holds
	KCHECK(Match.Rigger(0).Entered.size() == 3);
	Run(Match, 0.5);
	KCHECK(Match.Rigger(2).bStanding);
	Run(Match, 0.1);
	KCHECK(!Match.Rigger(2).bStanding);
	const auto Events = Match.TakeEvents();
	KCHECK(Has(Events, ETagEvent::CallSent, 0));
	KCHECK(Has(Events, ETagEvent::TaggedOut, 0));
	KCHECK(Has(Events, ETagEvent::Stalled, 2));
	KCHECK_NEAR(Match.Rigger(0).CallCooldownUntil - Match.Time(), 1.0, 1e-6);
	KCHECK(Match.Rigger(0).Entered.empty());
	// They drop back in at their pad after the reserve, on a new code.
	Run(Match, 5.9);
	KCHECK(!Match.Rigger(2).bStanding);
	Run(Match, 0.2);
	KCHECK(Match.Rigger(2).bStanding);
	KCHECK(!(Match.Rigger(2).Code == Old));
	KCHECK_NEAR(Match.Rigger(2).Pose.Chest.X, Pads[1].X, 1e-9);
}

KTEST(TagOutALockLostOutOfSightCostsTheCooldown)
{
	FTagOutMatch Match = Live();
	Enter(Match, 0, Match.Rigger(2).Code);
	KCHECK(Match.SendCall(0, 2, true));
	Run(Match, 0.3);
	Run(Match, Tick, {false, true, true, true});  // The target ducks behind a container
	KCHECK(Has(Match.TakeEvents(), ETagEvent::LostLock, 0));
	KCHECK(Match.Rigger(2).bStanding);
	KCHECK(Match.Rigger(0).Entered.size() == 3);  // The pips stay up for another go
	KCHECK(!Match.SendCall(0, 2, true));
	Run(Match, 1.4);
	KCHECK(!Match.SendCall(0, 2, true));
	Run(Match, 0.15);
	KCHECK(Match.SendCall(0, 2, true));
	// Walking out of the 40 m range loses it as well.
	Place(Match, 2, {45.0, 0.0, 8.0});
	Run(Match, Tick);
	KCHECK(Has(Match.TakeEvents(), ETagEvent::LostLock, 0));
	Run(Match, 2.0);
	KCHECK(!Match.SendCall(0, 2, true));  // Too far to send at all
}

KTEST(TagOutABadCallLeaksAPipAndShowsTheCaller)
{
	FTagOutMatch Match = Live();
	FTagCode Wrong = Match.Rigger(2).Code;
	Wrong.Pips[2] = static_cast<uint8_t>(Wrong.Pips[2] % 4 + 1);
	Match.Show(2, 0, 1);  // Team 1 already has the caller's left shoulder
	Enter(Match, 0, Wrong);
	KCHECK(Match.SendCall(0, 2, true));
	Run(Match, 0.6);
	const auto Events = Match.TakeEvents();
	KCHECK(Has(Events, ETagEvent::BadCall, 0));
	KCHECK(Match.Rigger(2).bStanding);
	KCHECK(Match.Read(1, 0).Known() == 2);
	KCHECK(Match.Read(1, 0).Pips[1] == Match.Rigger(0).Code.Pips[1]);  // The lowest plate they lacked
	KCHECK(Match.Exposed(0));
	KCHECK_NEAR(Match.Rigger(0).CallCooldownUntil - Match.Time(), 2.0, 1e-6);
	Run(Match, 3.1);
	KCHECK(!Match.Exposed(0));
}

KTEST(TagOutTheFourthPipStartsTheCallOver)
{
	FTagOutMatch Match = Live();
	Match.EnterPip(0, 1);
	Match.EnterPip(0, 2);
	Match.EnterPip(0, 3);
	Match.EnterPip(0, 4);
	KCHECK(Match.Rigger(0).Entered.size() == 1 && Match.Rigger(0).Entered[0] == 4);
	Match.EnterPip(0, 5);
	KCHECK(Match.Rigger(0).Entered.size() == 1);
	Match.ClearCall(0);
	KCHECK(Match.Rigger(0).Entered.empty());
}

KTEST(TagOutALockerRestampsAStillRigger)
{
	FTagOutMatch Match = Live();
	const FTagCode Old = Match.Rigger(0).Code;
	Match.Show(2, 0, 7);
	KCHECK(Match.Read(1, 0).Known() == 3);
	Place(Match, 0, Lockers[0]);
	for (int I = 0; I < 120; ++I)  // 2 s, then a step off
	{
		Match.HoldUse(0, Tick);
		Match.Step(Tick, {});
	}
	Match.SetPose(0, At(Lockers[0]), 3.0, false);
	Match.HoldUse(0, Tick);
	Match.Step(Tick, {});
	KCHECK(Match.Rigger(0).UseHeld == 0.0);
	Match.SetPose(0, At(Lockers[0]), 0.0, false);
	for (int I = 0; I < 179; ++I)
	{
		Match.HoldUse(0, Tick);
		Match.Step(Tick, {});
	}
	KCHECK(Match.Rigger(0).Code == Old);
	Match.HoldUse(0, Tick);
	Match.Step(Tick, {});
	KCHECK(!(Match.Rigger(0).Code == Old));
	KCHECK(Has(Match.TakeEvents(), ETagEvent::Restamped, 0));
	KCHECK(Match.Read(1, 0).bStale);
	KCHECK(Match.Read(1, 0).Known() == 0);
	Match.Show(2, 0, 2);  // The first look at the new code wipes the old one
	KCHECK(!Match.Read(1, 0).bStale && Match.Read(1, 0).Known() == 1);
}

KTEST(TagOutTheBeaconIsGrabbedDroppedReturnedAndPlanted)
{
	FTagOutMatch Match = Live();
	// Slot 0 lifts team 1's beacon off its stand.
	Place(Match, 0, Pads[1]);
	for (int I = 0; I < 29; ++I)
	{
		Match.HoldUse(0, Tick);
		Match.Step(Tick, {});
	}
	KCHECK(Match.Beacon(1).State == EBeaconState::OnStand);
	Match.HoldUse(0, Tick);
	Match.Step(Tick, {});
	KCHECK(Match.Beacon(1).State == EBeaconState::Carried && Match.Carrying(0) == 1);
	// Stalled on the way home: it lies where they went down.
	const FVec3 Fell{20.0, 10.0, 8.0};
	Place(Match, 0, Fell);
	Match.Step(Tick, {});
	Match.Stall(0);
	KCHECK(Match.Beacon(1).State == EBeaconState::Dropped);
	KCHECK_NEAR(Match.Beacon(1).At.X, Fell.X, 1e-9);
	KCHECK(Has(Match.TakeEvents(), ETagEvent::BeaconDropped, 0));
	// Its owner walks it home with a 2 s hold.
	Place(Match, 2, Fell);
	for (int I = 0; I < 120; ++I)
	{
		Match.HoldUse(2, Tick);
		Match.Step(Tick, {});
	}
	KCHECK(Match.Beacon(1).State == EBeaconState::OnStand);
	KCHECK(Has(Match.TakeEvents(), ETagEvent::BeaconReturned, 2));
	// Slot 1 takes it all the way and plants it in team 0's pad.
	Place(Match, 1, Pads[1]);
	for (int I = 0; I < 30; ++I)
	{
		Match.HoldUse(1, Tick);
		Match.Step(Tick, {});
	}
	KCHECK(Match.Carrying(1) == 1);
	Place(Match, 1, Pads[0] + FVec3{1.0, 0.0, 0.0});
	for (int I = 0; I < 60; ++I)
	{
		Match.HoldUse(1, Tick);
		Match.Step(Tick, {});
	}
	KCHECK(Match.Phase() == ETagPhase::Over);
	KCHECK(Match.Winner() == 0);
	const auto Events = Match.TakeEvents();
	KCHECK(Has(Events, ETagEvent::Planted, 1));
	KCHECK(Has(Events, ETagEvent::MatchOver));
}

KTEST(TagOutALooseBeaconGoesHomeOnItsOwn)
{
	FTagOutMatch Match = Live();
	Place(Match, 0, Pads[1]);
	for (int I = 0; I < 30; ++I)
	{
		Match.HoldUse(0, Tick);
		Match.Step(Tick, {});
	}
	Match.Throw(0);
	KCHECK(Match.Beacon(1).State == EBeaconState::Dropped);
	Match.SetBeaconAt(1, {30.0, 0.0, 8.0});
	Run(Match, 29.9, {});
	KCHECK(Match.Beacon(1).State == EBeaconState::Dropped);
	Run(Match, 0.2, {});
	KCHECK(Match.Beacon(1).State == EBeaconState::OnStand);
	// Thrown into the press pit, it goes home at once.
	Place(Match, 0, Pads[1]);
	for (int I = 0; I < 30; ++I)
	{
		Match.HoldUse(0, Tick);
		Match.Step(Tick, {});
	}
	Match.Throw(0);
	Match.SetBeaconAt(1, {0.0, 0.0, -10.0});
	KCHECK(Match.Beacon(1).State == EBeaconState::OnStand);
}

KTEST(TagOutTheReserveLengthensWithTheClock)
{
	FTagOutMatch Match = Live();
	const double Start = Match.Time();
	Match.Stall(3);
	KCHECK_NEAR(Match.Rigger(3).ReserveUntil - Start, 6.0, 1e-6);
	Run(Match, 200.0, {});
	Match.Stall(3);
	KCHECK_NEAR(Match.Rigger(3).ReserveUntil - Match.Time(), 9.0, 1e-6);
	Run(Match, 200.0, {});
	Match.Stall(3);
	KCHECK_NEAR(Match.Rigger(3).ReserveUntil - Match.Time(), 12.0, 1e-6);
	Run(Match, 100.0, {});
	KCHECK(Match.Phase() == ETagPhase::Overtime);
	Match.Stall(3);
	KCHECK_NEAR(Match.Rigger(3).ReserveUntil - Match.Time(), 15.0, 1e-6);
}

KTEST(TagOutRegulationRunsIntoOvertimeAndClosestApproach)
{
	FTagOutMatch Match = Live();
	Run(Match, 239.0, {});
	KCHECK(!Has(Match.TakeEvents(), ETagEvent::MidEvent));
	KCHECK_NEAR(Match.Remaining(), 241.0, 1e-3);
	Run(Match, 1.0, {});
	KCHECK(Has(Match.TakeEvents(), ETagEvent::MidEvent));
	// Slot 2 takes team 0's beacon to within 30 m of home before overtime.
	Place(Match, 2, Pads[0]);
	for (int I = 0; I < 30; ++I)
	{
		Match.HoldUse(2, Tick);
		Match.Step(Tick, {});
	}
	Place(Match, 2, Pads[1] - FVec3{32.0, 0.0, 0.0});
	Run(Match, 240.0, {});
	KCHECK(Match.Phase() == ETagPhase::Overtime);
	KCHECK(!Match.LockersOpen());
	KCHECK(Has(Match.TakeEvents(), ETagEvent::Overtime));
	KCHECK(Match.Read(0, 2).Known() == 3);  // The Lockout puts the carrier's code up
	KCHECK_NEAR(Match.Beacon(0).ClosestToEnemyPadM, 30.0, 1e-6);
	Run(Match, 180.0, {});
	KCHECK(Match.Phase() == ETagPhase::Over);
	KCHECK(Match.Winner() == 1);
}

KTEST(TagOutNobodyCarryingIsADraw)
{
	FTagOutMatch Match = Live();
	Run(Match, 480.0 + 180.0 + 1.0, {});
	KCHECK(Match.Phase() == ETagPhase::Over);
	KCHECK(Match.Winner() == NoPlayer);
}
