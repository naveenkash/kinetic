// The browser simulation: walking and the drop, Dogs and lines, the Cutter's panic-window
// test, bots cutting free, and whole matches of bots.
#include "Session.h"
#include "World.h"

#include "TestHarness.h"

#include <cmath>

using namespace Kinetic;
using namespace Kinetic::Web;
using namespace Kinetic::Test;

namespace
{
constexpr double Pi = 3.14159265358979323846;

// A world of the Yard with players at given feet positions, every player still.
struct FScene
{
	FWorld World;
	std::vector<FPlayerInput> Inputs;

	FScene() : World(Tables(), MakeYard()) {}

	int Add(int Team, EHarnessClass Class, const FVec3& Feet, double Yaw = 0.0)
	{
		const int I = World.AddPlayer(static_cast<int>(Inputs.size()), Team, Class);
		FWorldPlayer& P = World.Player(I);
		P.Position = Feet + FVec3{0, 0, PlayerHalfHeight};
		P.Yaw = Yaw;
		FPlayerInput In;
		In.Yaw = Yaw;
		Inputs.push_back(In);
		return I;
	}

	void Run(double Seconds)
	{
		for (int K = 0; K < static_cast<int>(std::lround(Seconds / FWorld::Dt)); ++K)
		{
			World.Step(Inputs);
			for (FPlayerInput& In : Inputs)
			{
				In.bFire = In.bSelfDog = In.bRelease = In.bSwing = false;
				In.Mode = 0;
			}
		}
	}

	// Aim a player at a point (yaw and pitch from their eye).
	void Aim(int I, const FVec3& Point)
	{
		const FVec3 D = Point - World.EyeOf(I);
		Inputs[static_cast<size_t>(I)].Yaw = std::atan2(D.Y, D.X);
		Inputs[static_cast<size_t>(I)].Pitch = std::atan2(D.Z, std::sqrt(D.X * D.X + D.Y * D.Y));
	}
};
} // namespace

KTEST(TheYardHasFloorsATrenchAndBridges)
{
	const FWorld W(Tables(), MakeYard());
	KCHECK(W.HasFloorAt({-20, 0, 0}) && W.HasFloorAt({20, 0, 0}));
	KCHECK(!W.HasFloorAt({0, 0, 0}));                       // The trench
	KCHECK(W.HasFloorAt({0, 13, 0}) && W.HasFloorAt({0, -13, 0}));  // The bridges
	KCHECK(!W.HasFloorAt({33.5, 0, 0}));                    // Past the edge
	for (const int Side : {0, 1})
	{
		for (const FVec3& S : W.Arena().Spawns[Side])
		{
			KCHECK(W.HasFloorAt(S));
		}
	}
}

KTEST(APlayerWalksAtRunSpeedAndFallsOffTheEdge)
{
	FScene S;
	const int P = S.Add(0, EHarnessClass::Rigger, {-26, 0, 0}, Pi);  // Facing -x, the west edge
	S.Run(0.2);
	KCHECK(S.World.Players()[0].bGrounded && std::abs(S.World.Players()[0].Position.Z - PlayerHalfHeight) < 0.02);
	S.Inputs[static_cast<size_t>(P)].MoveForward = 1.0;
	S.Run(0.5);
	KCHECK_NEAR(std::sqrt(S.World.Players()[0].Velocity.X * S.World.Players()[0].Velocity.X), Tables().Harness(EHarnessClass::Rigger).RunSpeed, 0.05);
	S.Run(3.0);  // Off the west edge at x = -32
	bool bRingOut = false;
	for (const FWorldEvent& E : S.World.TakeEvents())
	{
		bRingOut = bRingOut || (E.Kind == FWorldEvent::EKind::Stall && E.Cause == EStallCause::RingOut);
	}
	KCHECK(bRingOut && !S.World.Players()[0].bStanding);
}

KTEST(WalkingIntoAWallIsNotAHit)
{
	FScene S;
	S.Add(0, EHarnessClass::Rigger, {-18, -9, 0}, Pi / 2);  // Toward the container at y = -7.25
	S.Inputs[0].MoveForward = 1.0;
	S.Inputs[0].bSprint = true;
	S.Run(2.0);
	KCHECK(S.World.Players()[0].bStanding && S.World.Time() > S.World.Players()[0].StunnedUntil);
	KCHECK(S.World.Players()[0].Position.Y < -7.25 - PlayerHalfWidth + 0.01);
}

KTEST(ADogSeatsInAWallAndAHaulDragsACrate)
{
	FScene S;
	const int P = S.Add(0, EHarnessClass::Breaker, {6, -10, 0}, Pi / 2);
	S.Run(0.1);
	// The crate at (6, -4); Dog A into it, Dog B into the container face at (10, 2.75).
	S.Inputs[0].Mode = 1;
	S.Aim(P, {6, -4, 0.5});
	S.Inputs[0].bFire = true;
	S.Run(0.4);
	KCHECK(S.World.Lines().size() == 1 && S.World.Lines()[0].A.Seat == EDogSeat::Crate);
	S.Aim(P, {10, 2.75, 1.5});
	S.Inputs[0].bFire = true;
	S.Run(0.4);
	KCHECK(S.World.Lines().size() == 1 && S.World.Lines()[0].bLive());
	KCHECK(S.World.Lines()[0].B.Seat == EDogSeat::Static && S.World.Lines()[0].State.Mode == ELineMode::Haul);
	size_t Crate = 0;
	for (size_t C = 0; C < S.World.Crates().size(); ++C)
	{
		if ((S.World.Crates()[C].Position - FVec3{6, -4, 0.5}).Length() < 0.1)
		{
			Crate = C;
		}
	}
	KCHECK(S.World.Lines()[0].A.Crate == static_cast<int>(Crate));
	const FVec3 Before = S.World.Crates()[Crate].Position;
	S.Run(1.0);
	const FVec3 After = S.World.Crates()[Crate].Position;
	KCHECK((After - Before).Length() > 1.0);  // Dragged toward the container
	KCHECK((After - FVec3{10, 2.75, 1.5}).Length() < (Before - FVec3{10, 2.75, 1.5}).Length());
}

KTEST(AHaulAcrossTheTrenchIsARingOut)
{
	// A braced Breaker on the east bank Hauls a Rigger on the west bank toward the east
	// trench pylon: across the trench, and in.
	FScene S;
	const int B = S.Add(0, EHarnessClass::Breaker, {6, -2, 0}, Pi);
	const int R = S.Add(1, EHarnessClass::Rigger, {-6, -2, 0}, 0.0);
	S.Inputs[static_cast<size_t>(B)].bBrace = true;
	S.Run(0.5);
	KCHECK(S.World.Players()[static_cast<size_t>(B)].bBraced);
	S.Inputs[static_cast<size_t>(B)].Mode = 1;
	S.Aim(B, S.World.Players()[static_cast<size_t>(R)].Position);
	S.Inputs[static_cast<size_t>(B)].bFire = true;
	S.Run(0.3);
	KCHECK(S.World.Lines().size() == 1 && S.World.Lines()[0].A.Seat == EDogSeat::Plate);
	S.Aim(B, {2.3, -5, 1.5});
	S.Inputs[static_cast<size_t>(B)].bFire = true;
	S.Run(6.0);
	bool bRingOut = false;
	for (const FWorldEvent& E : S.World.TakeEvents())
	{
		bRingOut = bRingOut || (E.Kind == FWorldEvent::EKind::Stall && E.Player == R && E.Cause == EStallCause::RingOut);
	}
	KCHECK(bRingOut);
	KCHECK(S.World.Lines().empty());  // The line on a Stalled body ends
}

KTEST(TheCutterMustLookAtTheLine)
{
	// A Breaker Hauls a Rigger who has their back to the line. Looking away, the swing finds
	// nothing; turned to it, the swing cuts (05 4.2).
	FScene S;
	const int B = S.Add(0, EHarnessClass::Breaker, {-10, 12, 0}, 0.0);
	const int R = S.Add(1, EHarnessClass::Rigger, {5, 12, 0}, 0.0);  // Facing +x, away from the Breaker
	S.Inputs[static_cast<size_t>(B)].bBrace = true;
	S.Run(0.5);
	S.Inputs[static_cast<size_t>(B)].Mode = 1;
	S.Aim(B, S.World.Players()[static_cast<size_t>(R)].Position);
	S.Inputs[static_cast<size_t>(B)].bFire = true;
	S.Run(0.4);
	KCHECK(S.World.Lines().size() == 1 && S.World.Lines()[0].A.Plate == EPlate::Back);
	S.Aim(B, {-20, 12, 0.5});  // The floor behind the Breaker
	S.Inputs[static_cast<size_t>(B)].bFire = true;
	S.Inputs[static_cast<size_t>(R)].bDraw = true;
	S.Run(0.4);
	KCHECK(S.World.Lines().size() == 1 && S.World.Lines()[0].bLive());
	S.Inputs[static_cast<size_t>(R)].bSwing = true;
	S.Run(1.0 / 60.0);
	KCHECK(S.World.Lines().size() == 1);  // Looking away: no cut
	S.Run(0.5);
	S.Aim(R, S.World.Players()[static_cast<size_t>(B)].Position);
	S.Inputs[static_cast<size_t>(R)].bSwing = true;
	S.Run(1.0 / 60.0);
	KCHECK(S.World.Lines().empty());
}

KTEST(ABotCutsALineOffItsPlate)
{
	// A braced Breaker Hauls a bot Rigger from 15 m. The bot's defense draws, faces along the
	// line and cuts it between 1.0 and 1.6 s after the seat, before the 15 m drag ends.
	for (const uint64_t Seed : {1ull, 2ull, 3ull, 4ull, 5ull})
	{
		FScene S;
		const int B = S.Add(0, EHarnessClass::Breaker, {-24, 0, 0}, 0.0);
		const int R = S.Add(1, EHarnessClass::Rigger, {-9, 0, 0}, 0.0);  // Back to the Breaker
		FBotBrain Brain(R, Seed, Tables(), EHarnessClass::Rigger);
		S.Inputs[static_cast<size_t>(B)].bBrace = true;
		S.Run(0.5);
		S.Inputs[static_cast<size_t>(B)].Mode = 1;
		S.Aim(B, S.World.Players()[static_cast<size_t>(R)].Position);
		S.Inputs[static_cast<size_t>(B)].bFire = true;
		std::vector<FWorldEvent> Events;
		double SeatedAt = -1.0, CutAt = -1.0, LiveAt = -1.0;
		for (int K = 0; K < 240 && CutAt < 0.0; ++K)
		{
			if (K == 20)
			{
				S.Aim(B, {-29, 0, 0});
				S.Inputs[static_cast<size_t>(B)].bFire = true;  // Dog B into the floor behind
			}
			S.Inputs[static_cast<size_t>(R)] = Brain.Think(S.World, Events);
			S.World.Step(S.Inputs);
			for (FPlayerInput& In : S.Inputs)
			{
				In.bFire = false;
				In.Mode = 0;
			}
			Events = S.World.TakeEvents();
			for (const FWorldEvent& E : Events)
			{
				if (E.Kind == FWorldEvent::EKind::Seat && E.Target == R)
				{
					SeatedAt = S.World.Time();
				}
				if (E.Kind == FWorldEvent::EKind::LineEnded && E.Ended == ELineEnded::Cut)
				{
					CutAt = S.World.Time();
				}
			}
			if (LiveAt < 0.0 && !S.World.Lines().empty() && S.World.Lines()[0].bLive())
			{
				LiveAt = S.World.Time();
			}
		}
		KCHECK(SeatedAt > 0.0 && CutAt > 0.0);
		KCHECK(CutAt - SeatedAt >= 1.0 - 1e-9 && CutAt - SeatedAt <= 1.6 + 2.0 / 60.0);
		KCHECK(S.World.Players()[static_cast<size_t>(R)].bStanding);
	}
}

KTEST(BotMatchesPlayToTheEndTheSameWayTwice)
{
	FSession A(Tables(), 7, EHarnessClass::Rigger, false);
	FSession B(Tables(), 7, EHarnessClass::Rigger, false);
	const FPlayerInput Idle;
	int Ticks = 0;
	while (A.MatchesFinished() == 0 && Ticks < 60 * 60 * 20)
	{
		A.Step(Idle);
		B.Step(Idle);
		++Ticks;
	}
	KCHECK(A.MatchesFinished() == 1);  // A whole best-of-5 among bots, inside 20 minutes
	KCHECK(A.SnapshotJson() == B.SnapshotJson());  // Seeded: the same match
	KCHECK(A.StallsSeen() >= 3);
	KCHECK(A.Match().Wins(0) == 3 || A.Match().Wins(1) == 3);
	const std::string Arena = A.ArenaJson();
	KCHECK(Arena.front() == '{' && Arena.find("\"solids\"") != std::string::npos);
}

namespace
{
// Fires a two-Dog line for player I: A at PointA, B at PointB, in Mode (1 Haul, 2 Jack, 3 Hang).
void Rig(FScene& S, int I, int Mode, const FVec3& PointA, const FVec3& PointB)
{
	S.Inputs[static_cast<size_t>(I)].Mode = Mode;
	S.Aim(I, PointA);
	S.Inputs[static_cast<size_t>(I)].bFire = true;
	S.Run(0.4);
	S.Aim(I, PointB);
	S.Inputs[static_cast<size_t>(I)].bFire = true;
	S.Run(0.4);
}
} // namespace

KTEST(ALineUnderFootHoldStillDragsAStandingPlayer)
{
	// A Scrapper's 400 N on a 130 kg Rigger is 3.1 m/s², under the Rigger's 5 m/s² foot-hold:
	// a skid, which overrides the Rigger's own movement (05 5.1), even walking away.
	FScene S;
	const int Scrapper = S.Add(0, EHarnessClass::Scrapper, {-20, 0, 0}, 0.0);
	const int Rigger = S.Add(1, EHarnessClass::Rigger, {-10, 0, 0}, 0.0);
	S.Run(0.1);
	Rig(S, Scrapper, 1, S.World.Players()[static_cast<size_t>(Rigger)].Position, {-30, 0, 0});
	KCHECK(S.World.Lines().size() == 1 && S.World.Lines()[0].bLive());
	S.Inputs[static_cast<size_t>(Rigger)].MoveForward = 1.0;  // Walking away, +x: no use
	const double X0 = S.World.Players()[static_cast<size_t>(Rigger)].Position.X;
	S.Run(2.0);
	const FWorldPlayer& R = S.World.Players()[static_cast<size_t>(Rigger)];
	KCHECK(R.bGrounded && R.Footing == EFooting::Skid);
	KCHECK(X0 - R.Position.X > 3.0);  // Dragged toward the Dog in the floor, not walking off
}

KTEST(ASlingshotCarries)
{
	// A Climber self-Dogs, puts Dog B in the container behind and Jacks: the Kick launches them
	// at V = 18 m/s and nothing slows them but gravity and landing.
	FScene S;
	const int C = S.Add(0, EHarnessClass::Climber, {-18, -10, 0}, -Pi / 2);
	S.Run(0.1);
	S.Inputs[0].Mode = 2;
	S.Inputs[0].bSelfDog = true;
	S.Run(0.1);
	S.Aim(C, {-18, -7.25, 1.2});  // The container's face behind (+y side of the Climber)
	S.Inputs[0].bFire = true;
	double Peak = 0.0;
	for (int K = 0; K < 36; ++K)
	{
		S.Run(1.0 / 60.0);
		Peak = std::max(Peak, std::sqrt(S.World.Players()[0].Velocity.X * S.World.Players()[0].Velocity.X +
		                                S.World.Players()[0].Velocity.Y * S.World.Players()[0].Velocity.Y));
	}
	KCHECK(Peak > 12.0);
	KCHECK(S.World.Players()[0].bStanding && S.World.Players()[0].Position.Y < -14.0);  // Over 4 m in 0.6 s
	// Nothing but the wall at y = -20 stops them: at that speed, a Stall (05 7.1).
	S.Run(1.5);
	KCHECK(!S.World.Players()[0].bStanding);
}

KTEST(SetLinesRetireTheOldestAndExpire)
{
	// A Rigger has two set lines (12 s); a third retires the oldest. A third Dog press on a live
	// Hang line also leaves it as a set line.
	FScene S;
	const int R = S.Add(0, EHarnessClass::Rigger, {-10, 12, 0}, 0.0);
	S.Run(0.1);
	Rig(S, R, 3, {-16, 9.5, 0}, {-12, 9.5, 0});
	KCHECK(S.World.Lines().size() == 1 && S.World.Lines()[0].State.Mode == ELineMode::Hang);
	const int First = S.World.Lines()[0].Id;
	S.Inputs[0].bRelease = true;
	S.Run(0.1);
	KCHECK(S.World.Lines().size() == 1 && S.World.Lines()[0].bSet);
	Rig(S, R, 3, {-16, 14.5, 0}, {-12, 14.5, 0});
	S.Aim(R, {-16, 16, 0});
	S.Inputs[0].bFire = true;  // The third press: a set line, and a new Dog A
	S.Run(0.1);
	int Set = 0;
	for (const FWorldLine& L : S.World.Lines()) Set += L.bSet ? 1 : 0;
	KCHECK(Set == 2);
	S.Run(0.4);
	S.Aim(R, {-12, 16, 0});
	S.Inputs[0].bFire = true;
	S.Run(0.4);
	S.Inputs[0].bRelease = true;  // A third set line: the first retires
	S.Run(0.1);
	Set = 0;
	bool bFirst = false;
	for (const FWorldLine& L : S.World.Lines())
	{
		Set += L.bSet ? 1 : 0;
		bFirst = bFirst || L.Id == First;
	}
	KCHECK(Set == 2 && !bFirst);
	S.Run(Tables().Harness(EHarnessClass::Rigger).SetLineLife + 0.5);
	KCHECK(S.World.Lines().empty());
}

KTEST(AModeSwitchWaitsTheClassDelay)
{
	FScene S;
	const int B = S.Add(0, EHarnessClass::Breaker, {6, -10, 0}, Pi / 2);
	S.Run(0.1);
	Rig(S, B, 1, {6, -4, 0.5}, {10, 2.75, 1.5});
	KCHECK(S.World.Lines()[0].State.Mode == ELineMode::Haul);
	S.Inputs[0].Mode = 3;
	S.Run(1.0 / 60.0);
	KCHECK(S.World.Lines()[0].State.Mode == ELineMode::Haul);  // Not yet
	S.Run(Tables().Harness(EHarnessClass::Breaker).ModeSwitch + 2.0 / 60.0);
	KCHECK(S.World.Lines()[0].State.Mode == ELineMode::Hang);
	// Asking for Jack, then back to Hang, calls the switch off.
	S.Inputs[0].Mode = 2;
	S.Run(1.0 / 60.0);
	S.Inputs[0].Mode = 3;
	S.Run(Tables().Harness(EHarnessClass::Breaker).ModeSwitch + 2.0 / 60.0);
	KCHECK(S.World.Lines()[0].State.Mode == ELineMode::Hang);
}

KTEST(AHangLineIsSolidToPlayers)
{
	// A Rigger strings a Clothesline across the doorway at 1.2 m; a Climber walking into it is
	// stopped, and one sprinting at 9 m/s is flipped over it (05 3.5).
	FScene S;
	const int R = S.Add(0, EHarnessClass::Rigger, {-16, -13, 0}, 0.0);
	const int Walker = S.Add(1, EHarnessClass::Climber, {-8, -13, 0}, Pi);
	S.Run(0.1);
	Rig(S, R, 3, {-12, -14, 1.2}, {-12, -12, 1.2});
	S.Inputs[static_cast<size_t>(R)].bRelease = true;
	S.Run(0.1);
	KCHECK(S.World.Lines().size() == 1 && S.World.Lines()[0].bSet);
	S.Inputs[static_cast<size_t>(Walker)].MoveForward = 0.3;  // 2.25 m/s: under the stagger speed
	S.Run(3.0);
	KCHECK(S.World.Players()[static_cast<size_t>(Walker)].Position.X > -12.0);  // Held at the line
	S.Inputs[static_cast<size_t>(Walker)].MoveForward = 0.0;
	S.World.Player(Walker).Position = {-6, -13, PlayerHalfHeight};
	S.Run(0.5);
	S.Inputs[static_cast<size_t>(Walker)].MoveForward = 1.0;
	S.Inputs[static_cast<size_t>(Walker)].bSprint = true;
	bool bFlipped = false;
	for (int K = 0; K < 120 && !bFlipped; ++K)
	{
		S.World.Step(S.Inputs);
		for (const FWorldEvent& E : S.World.TakeEvents())
		{
			bFlipped = bFlipped || (E.Kind == FWorldEvent::EKind::Flip && E.Player == Walker);
		}
	}
	KCHECK(bFlipped);
	S.Run(1.5);
	KCHECK(S.World.Players()[static_cast<size_t>(Walker)].Position.X < -12.0);  // Over to the far side
}

KTEST(ThePlayerFacesTheOtherSideAfterAReset)
{
	FSession Session(Tables(), 5, EHarnessClass::Rigger, true);
	FPlayerInput In;
	In.Yaw = 1.0;
	Session.Step(In);
	const int Resets = Session.World().ResetCount();
	KCHECK(Resets >= 1);
	// Within the hold, input yaw is ignored: the spawn facing stands for the client to adopt.
	const double Spawn = Session.World().Players()[0].Yaw;
	KCHECK(Spawn == 0.0 || std::abs(Spawn - Pi) < 1e-9);
	// After the hold, the player's yaw is theirs again.
	for (int K = 0; K < 30; ++K)
	{
		Session.Step(In);
	}
	KCHECK(std::abs(Session.World().Players()[0].Yaw - 1.0) < 1e-9);
}

KTEST(HangLinesBlockTheStunnedAndNotFromTheFloor)
{
	FScene S;
	const int R = S.Add(0, EHarnessClass::Rigger, {-16, -13, 0}, 0.0);
	const int Walker = S.Add(1, EHarnessClass::Climber, {-8, -13, 0}, Pi);
	S.Run(0.1);
	Rig(S, R, 3, {-12, -14, 1.2}, {-12, -12, 1.2});
	S.Inputs[static_cast<size_t>(R)].bRelease = true;
	S.Run(0.1);
	// Stunned for another reason and in flight at 4 m/s (over the stagger speed): still held
	// by the line, and not hit again.
	FWorldPlayer& W = S.World.Player(Walker);
	W.StunnedUntil = S.World.Time() + 5.0;
	W.Position = {-10.8, -13, PlayerHalfHeight + 0.6};
	W.Velocity = {-4.0, 0, 0};
	W.bGrounded = false;
	W.Footing = EFooting::Airborne;
	S.World.TakeEvents();
	bool bHitAgain = false;
	for (int K = 0; K < 60; ++K)
	{
		S.World.Step(S.Inputs);
		for (const FWorldEvent& E : S.World.TakeEvents())
		{
			bHitAgain = bHitAgain || ((E.Kind == FWorldEvent::EKind::Stagger || E.Kind == FWorldEvent::EKind::Flip) && E.Player == Walker);
		}
	}
	KCHECK(S.World.Players()[static_cast<size_t>(Walker)].Position.X > -12.0 + PlayerHalfWidth - 0.05);
	KCHECK(S.World.Players()[static_cast<size_t>(Walker)].Position.X < -11.0);  // It reached the line
	KCHECK(!bHitAgain);
	// A line in the floor does not stop anyone.
	FScene F;
	const int Q = F.Add(0, EHarnessClass::Rigger, {-10, 12, 0}, 0.0);
	const int Crosser = F.Add(1, EHarnessClass::Climber, {-14, 7, 0}, Pi / 2);
	F.Run(0.1);
	Rig(F, Q, 3, {-16, 9.5, 0}, {-12, 9.5, 0});
	F.Inputs[static_cast<size_t>(Q)].bRelease = true;
	F.Run(0.1);
	F.Inputs[static_cast<size_t>(Crosser)].MoveForward = 0.3;
	F.Run(3.0);
	KCHECK(F.World.Players()[static_cast<size_t>(Crosser)].Position.Y > 10.5);
}

KTEST(LookingAroundKeepsAPlayerFromIdlingOut)
{
	// Only looking around, for 90 s, is input: no idle leaver, no no-contest.
	FSession Session(Tables(), 9, EHarnessClass::Rigger, true);
	FPlayerInput In;
	for (int K = 0; K < 90 * 60; ++K)
	{
		In.Yaw = 0.3 * std::sin(K * 0.01);
		Session.Step(In);
	}
	KCHECK(!Session.Match().Result().bNoContest);
	KCHECK(Session.Match().Phase() != EScrapPhase::MatchOver || Session.MatchesFinished() > 0);
	KCHECK(Session.SnapshotJson().find("standing still") == std::string::npos);
}
