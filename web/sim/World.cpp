#include "World.h"

#include "Kinetic/Cutter.h"
#include "Kinetic/HangSolver.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace Kinetic::Web
{
namespace
{
constexpr double Inf = std::numeric_limits<double>::infinity();
constexpr double WalkAccel = 40.0;        // m/s², reaching run speed in about 0.15 s
constexpr double AirAccel = 8.0;          // m/s² at full air control (PROPOSED)
constexpr double LineStepOverM = 0.3;     // A Hang line lower than this above the feet is stepped over (PROPOSED)
constexpr double BraceStillSpeed = 0.5;   // m/s
constexpr double BraceSeconds = 0.3;      // 05 5.4
constexpr double StaggerSeconds = 0.5;    // PROPOSED for the prototype
constexpr double FlipSeconds = 1.2;
constexpr double CrateFriction = 6.0;     // 1/s on the floor
constexpr double GroundSnap = 0.05;       // m
constexpr double CrateHitSpeed = 2.0;     // m/s: a crate moving faster than this hits what it meets

FVec3 Flat(const FVec3& V) { return {V.X, V.Y, 0.0}; }

struct FSlabHit
{
	double T = Inf;
	FVec3 Normal;
};

// Where a segment From + (To − From)·t, t in [0, 1], enters a box, if it does.
FSlabHit SegmentBox(const FVec3& From, const FVec3& To, const FVec3& Min, const FVec3& Max)
{
	const FVec3 D = To - From;
	double Enter = 0.0, Exit = 1.0;
	FVec3 Normal;
	const double P[3] = {From.X, From.Y, From.Z};
	const double V[3] = {D.X, D.Y, D.Z};
	const double Lo[3] = {Min.X, Min.Y, Min.Z};
	const double Hi[3] = {Max.X, Max.Y, Max.Z};
	for (int Axis = 0; Axis < 3; ++Axis)
	{
		if (std::abs(V[Axis]) < 1e-12)
		{
			if (P[Axis] < Lo[Axis] || P[Axis] > Hi[Axis])
			{
				return {};
			}
			continue;
		}
		double T0 = (Lo[Axis] - P[Axis]) / V[Axis];
		double T1 = (Hi[Axis] - P[Axis]) / V[Axis];
		double Sign = -1.0;
		if (T0 > T1)
		{
			std::swap(T0, T1);
			Sign = 1.0;
		}
		if (T0 > Enter)
		{
			Enter = T0;
			Normal = {};
			(Axis == 0 ? Normal.X : Axis == 1 ? Normal.Y : Normal.Z) = Sign;
		}
		Exit = std::min(Exit, T1);
		if (Enter > Exit)
		{
			return {};
		}
	}
	return {Enter, Normal};
}

// The smallest push that separates box A from box B, and its axis, or nothing.
struct FPush
{
	FVec3 Normal;  // Pushes A out of B
	double Depth = 0.0;
};

std::optional<FPush> Overlap(const FVec3& CA, const FVec3& HA, const FVec3& CB, const FVec3& HB)
{
	const FVec3 D = CA - CB;
	const double PX = HA.X + HB.X - std::abs(D.X);
	const double PY = HA.Y + HB.Y - std::abs(D.Y);
	const double PZ = HA.Z + HB.Z - std::abs(D.Z);
	if (PX <= 0.0 || PY <= 0.0 || PZ <= 0.0)
	{
		return std::nullopt;
	}
	if (PZ <= PX && PZ <= PY)
	{
		return FPush{{0, 0, D.Z >= 0.0 ? 1.0 : -1.0}, PZ};
	}
	if (PX <= PY)
	{
		return FPush{{D.X >= 0.0 ? 1.0 : -1.0, 0, 0}, PX};
	}
	return FPush{{0, D.Y >= 0.0 ? 1.0 : -1.0, 0}, PY};
}

FVec3 HalfOf(const FSolid& S) { return (S.Max - S.Min) * 0.5; }
FVec3 CenterOf(const FSolid& S) { return (S.Max + S.Min) * 0.5; }
const FVec3 PlayerHalf{PlayerHalfWidth, PlayerHalfWidth, PlayerHalfHeight};

bool IsActive(ELineMode Mode) { return Mode == ELineMode::Haul || Mode == ELineMode::Jack; }
bool IsHang(ELineMode Mode) { return Mode == ELineMode::Hang || Mode == ELineMode::Set; }
} // namespace

FWorld::FWorld(const FKineticTables& InTables, FArena InArena) : Tables(InTables), ArenaData(std::move(InArena))
{
	for (const FCrateSpec& Spec : ArenaData.Crates)
	{
		FCrate C;
		C.Position = Spec.Center;
		C.Half = Spec.Half;
		C.MassKg = Spec.MassKg;
		C.Object.MassKg = Spec.MassKg;
		C.Object.Placement = Spec.Placement;
		C.bSmallPropBonus = IsSmallPropBonusTarget(Spec.MassKg, Tables);
		CrateList.push_back(C);
	}
}

int FWorld::AddPlayer(int Slot, int Team, EHarnessClass Class)
{
	FWorldPlayer P(Tables);
	P.Slot = Slot;
	P.Team = Team;
	P.Class = Class;
	PlayerList.push_back(std::move(P));
	LastLineForce.emplace_back();
	return static_cast<int>(PlayerList.size()) - 1;
}

void FWorld::Reset(const int SideOfTeam[2])
{
	LineList.clear();
	++Resets;
	HoldSpawnYawUntil = Now + SpawnYawHoldSeconds;
	int Used[2] = {0, 0};
	for (size_t I = 0; I < PlayerList.size(); ++I)
	{
		FWorldPlayer& P = PlayerList[I];
		const int Side = SideOfTeam[P.Team];
		const std::vector<FVec3>& Spawns = ArenaData.Spawns[Side];
		const FVec3 Feet = Spawns[static_cast<size_t>(Used[P.Team]++) % Spawns.size()];
		P.Position = Feet + FVec3{0, 0, PlayerHalfHeight};
		P.Velocity = {};
		P.Yaw = Side == 0 ? 0.0 : 3.14159265358979323846;  // Facing the other side
		P.Pitch = 0.0;
		P.bGrounded = true;
		P.bStanding = true;
		P.bBraced = false;
		P.BraceHeld = 0.0;
		P.Footing = EFooting::Skid;
		P.StunnedUntil = -1.0;
		P.bDrawn = false;
		P.NextSwingAt = -1.0;
		P.bSelfDogNext = false;
		P.Impacts.NotifyHandoff();
		LastLineForce[I] = {};
	}
	for (size_t I = 0; I < CrateList.size(); ++I)
	{
		const FCrateSpec& Spec = ArenaData.Crates[I];
		FCrate& C = CrateList[I];
		C.Position = Spec.Center;
		C.Velocity = {};
		C.Object.bBrokenFree = false;
	}
}

std::vector<FWorldEvent> FWorld::TakeEvents()
{
	std::vector<FWorldEvent> Out;
	Out.swap(Events);
	return Out;
}

FVec3 FWorld::EyeOf(int Player) const { return PlayerList.at(static_cast<size_t>(Player)).Position + FVec3{0, 0, EyeHeight}; }

FVec3 FWorld::Forward(int Player) const
{
	const FWorldPlayer& P = PlayerList.at(static_cast<size_t>(Player));
	return {std::cos(P.Pitch) * std::cos(P.Yaw), std::cos(P.Pitch) * std::sin(P.Yaw), std::sin(P.Pitch)};
}

FVec3 FWorld::PlatePosition(int Player, EPlate Plate) const
{
	const FWorldPlayer& P = PlayerList.at(static_cast<size_t>(Player));
	const FVec3 F{std::cos(P.Yaw), std::sin(P.Yaw), 0};
	const FVec3 R{std::sin(P.Yaw), -std::cos(P.Yaw), 0};
	switch (Plate)
	{
	case EPlate::Chest: return P.Position + F * 0.32 + FVec3{0, 0, 0.35};
	case EPlate::Back: return P.Position - F * 0.32 + FVec3{0, 0, 0.35};
	case EPlate::LeftShoulder: return P.Position - R * 0.38 + FVec3{0, 0, 0.6};
	case EPlate::RightShoulder: return P.Position + R * 0.38 + FVec3{0, 0, 0.6};
	case EPlate::None: break;
	}
	return P.Position;
}

FVec3 FWorld::EndPosition(const FLineEnd& End) const
{
	switch (End.Seat)
	{
	case EDogSeat::Static: return End.Point;
	case EDogSeat::Crate: return CrateList.at(static_cast<size_t>(End.Crate)).Position;
	case EDogSeat::Plate:
	case EDogSeat::OwnPlate: return PlatePosition(End.Player, End.Plate);
	case EDogSeat::None: break;
	}
	return End.Point;
}

FVec3 FWorld::DogOrEndPosition(const FWorldLine& Line, bool bSideA) const
{
	const std::optional<FDog>& Dog = bSideA ? Line.FlyingA : Line.FlyingB;
	if (Dog)
	{
		return Dog->Position;
	}
	return EndPosition(bSideA ? Line.A : Line.B);
}

double FWorld::CutterWarmup(int Player) const
{
	return CutterWarmupSeconds(ECutterVariant::Standard, PlayerList.at(static_cast<size_t>(Player)).Class, Tables);
}

double FWorld::SupportTop(const FVec3& Center, const FVec3& Half) const
{
	double Top = -Inf;
	const double Bottom = Center.Z - Half.Z;
	for (const FSolid& S : ArenaData.Solids)
	{
		const bool bUnder = Center.X + Half.X > S.Min.X && Center.X - Half.X < S.Max.X && Center.Y + Half.Y > S.Min.Y &&
		                    Center.Y - Half.Y < S.Max.Y;
		if (bUnder && S.Max.Z <= Bottom + 0.3)
		{
			Top = std::max(Top, S.Max.Z);
		}
	}
	return Top;
}

bool FWorld::HasFloorAt(const FVec3& Feet) const
{
	return SupportTop(Feet + FVec3{0, 0, PlayerHalfHeight}, {0.05, 0.05, PlayerHalfHeight}) > ArenaData.KillZ + 1.0;
}

int FWorld::ActiveLineOf(int Player) const
{
	for (size_t I = 0; I < LineList.size(); ++I)
	{
		if (LineList[I].Owner == Player && !LineList[I].bSet)
		{
			return static_cast<int>(I);
		}
	}
	return -1;
}

std::vector<int> FWorld::LinesOnPlayer(int Player) const
{
	std::vector<int> Out;
	for (const FWorldLine& L : LineList)
	{
		const bool bOnA = L.A.Seat == EDogSeat::Plate && L.A.Player == Player;
		const bool bOnB = L.B.Seat == EDogSeat::Plate && L.B.Player == Player;
		if ((bOnA || bOnB) && L.Owner != Player)
		{
			Out.push_back(L.Id);
		}
	}
	return Out;
}

void FWorld::EndLine(size_t LineIndex, ELineEnded Why)
{
	const FWorldLine& L = LineList.at(LineIndex);
	FWorldEvent E;
	E.Kind = FWorldEvent::EKind::LineEnded;
	E.Player = L.Owner;
	E.Line = L.Id;
	E.Ended = Why;
	Events.push_back(E);
	LineList.erase(LineList.begin() + static_cast<std::ptrdiff_t>(LineIndex));
}

void FWorld::FireDog(int Index)
{
	FWorldPlayer& P = PlayerList[static_cast<size_t>(Index)];
	const FHarnessTuning& H = Tables.Harness(P.Class);
	int Active = ActiveLineOf(Index);
	if (Active >= 0)
	{
		const FWorldLine& L = LineList[static_cast<size_t>(Active)];
		const bool bBFired = L.FlyingB || L.B.Seat != EDogSeat::None;
		if (bBFired)
		{
			// A third press releases the current line and launches a new Dog A (05 3.1).
			ReleaseActive(Index);
			Active = -1;
		}
	}
	const bool bSelf = P.bSelfDogNext;
	P.bSelfDogNext = false;
	FDog Dog;
	const FVec3 Dir = Forward(Index);
	Dog.Position = EyeOf(Index) + Dir * 0.5;
	Dog.Velocity = Dir * H.DogSpeed;
	FLineEnd Self;
	Self.Seat = EDogSeat::OwnPlate;
	Self.Player = Index;
	Self.Plate = EPlate::Chest;
	if (Active < 0)
	{
		FWorldLine L;
		L.Id = NextLineId++;
		L.Owner = Index;
		L.Params = FLineParams::FromHarness(H);
		L.TensionN = H.TensionN;
		L.Preselect = P.Preselect;
		L.CreatedAt = Now;
		if (bSelf)
		{
			L.A = Self;
		}
		else
		{
			L.FlyingA = Dog;
		}
		LineList.push_back(L);
		return;
	}
	FWorldLine& L = LineList[static_cast<size_t>(Active)];
	if (bSelf)
	{
		L.B = Self;
	}
	else
	{
		L.FlyingB = Dog;
	}
}

std::optional<FLineEnd> FWorld::SweepDog(int Owner, const FVec3& From, const FVec3& To, FVec3& Hit) const
{
	double BestT = Inf;
	std::optional<FLineEnd> Best;
	const double R = Tables.Globals().DogRadius;
	for (size_t I = 0; I < PlayerList.size(); ++I)
	{
		const FWorldPlayer& P = PlayerList[I];
		if (static_cast<int>(I) == Owner || !P.bStanding)
		{
			continue;
		}
		// A Dog that reaches the body anywhere seats in the plate nearest the impact (05 5.5).
		const FVec3 Pad{R, R, R};
		const FSlabHit S = SegmentBox(From, To, P.Position - PlayerHalf - Pad, P.Position + PlayerHalf + Pad);
		if (S.T < BestT)
		{
			BestT = S.T;
			FLineEnd E;
			E.Seat = EDogSeat::Plate;
			E.Player = static_cast<int>(I);
			Best = E;
		}
	}
	for (size_t I = 0; I < CrateList.size(); ++I)
	{
		const FCrate& C = CrateList[I];
		const FVec3 Pad{R, R, R};
		const FSlabHit S = SegmentBox(From, To, C.Position - C.Half - Pad, C.Position + C.Half + Pad);
		if (S.T < BestT)
		{
			BestT = S.T;
			FLineEnd E;
			E.Seat = EDogSeat::Crate;
			E.Crate = static_cast<int>(I);
			Best = E;
		}
	}
	for (const FSolid& Solid : ArenaData.Solids)
	{
		const FSlabHit S = SegmentBox(From, To, Solid.Min, Solid.Max);
		if (S.T < BestT)
		{
			BestT = S.T;
			FLineEnd E;
			E.Seat = EDogSeat::Static;
			Best = E;
		}
	}
	if (!Best)
	{
		return std::nullopt;
	}
	Hit = From + (To - From) * BestT;
	if (Best->Seat == EDogSeat::Static)
	{
		Best->Point = Hit;
	}
	if (Best->Seat == EDogSeat::Plate)
	{
		// The plate nearest the point of impact (05 5.5).
		double Nearest = Inf;
		for (const EPlate Plate : {EPlate::Chest, EPlate::Back, EPlate::LeftShoulder, EPlate::RightShoulder})
		{
			const double D = (PlatePosition(Best->Player, Plate) - Hit).Length();
			if (D < Nearest)
			{
				Nearest = D;
				Best->Plate = Plate;
			}
		}
	}
	return Best;
}

void FWorld::UpdateDogs()
{
	const FSpoolGlobals& G = Tables.Globals();
	for (size_t I = 0; I < LineList.size();)
	{
		FWorldLine& L = LineList[I];
		bool bEnded = false;
		for (const bool bSideA : {true, false})
		{
			std::optional<FDog>& Dog = bSideA ? L.FlyingA : L.FlyingB;
			if (!Dog)
			{
				continue;
			}
			const FVec3 From = Dog->Position;
			Dog->Velocity.Z -= G.Gravity * Dt;
			const FVec3 To = From + Dog->Velocity * Dt;
			Dog->Travelled += (To - From).Length();
			FVec3 Hit;
			const std::optional<FLineEnd> Seat = SweepDog(L.Owner, From, To, Hit);
			if (Seat)
			{
				(bSideA ? L.A : L.B) = *Seat;
				Dog.reset();
				FWorldEvent E;
				E.Kind = FWorldEvent::EKind::Seat;
				E.Player = L.Owner;
				E.Line = L.Id;
				E.Target = Seat->Seat == EDogSeat::Plate ? Seat->Player : -1;
				E.Plate = Seat->Plate;
				E.At = Hit;
				Events.push_back(E);
				continue;
			}
			Dog->Position = To;
			// Past the class line length, or out of the world: it falls harmless (05 3.1).
			if (Dog->Travelled > L.Params.LineLength || To.Z < ArenaData.KillZ)
			{
				bEnded = true;
				break;
			}
		}
		if (bEnded)
		{
			EndLine(I, ELineEnded::Missed);
			continue;
		}
		++I;
	}
}

void FWorld::UpdateLines()
{
	const FSpoolGlobals& G = Tables.Globals();
	for (size_t I = 0; I < LineList.size();)
	{
		FWorldLine& L = LineList[I];
		const auto OnStalled = [this](const FLineEnd& E) {
			return (E.Seat == EDogSeat::Plate || E.Seat == EDogSeat::OwnPlate) &&
			       !PlayerList[static_cast<size_t>(E.Player)].bStanding;
		};
		if (!PlayerList[static_cast<size_t>(L.Owner)].bStanding && !L.bSet)
		{
			EndLine(I, ELineEnded::BodyStalled);
			continue;
		}
		if (OnStalled(L.A) || OnStalled(L.B))
		{
			EndLine(I, ELineEnded::BodyStalled);
			continue;
		}
		if (L.bSet && Now >= L.SetExpiresAt)
		{
			EndLine(I, ELineEnded::Expired);
			continue;
		}
		const bool bAOnly = L.A.Seat != EDogSeat::None && !L.FlyingB && L.B.Seat == EDogSeat::None;
		if (bAOnly && Now - L.CreatedAt > G.SlackTimeout)
		{
			EndLine(I, ELineEnded::Released);  // Four seconds to fire B, or A recalls (05 3.1)
			continue;
		}
		if (L.bLive() && L.State.Mode == ELineMode::Slack && L.PendingAt < 0.0 && !L.bSet)
		{
			EngageMode(L.State, L.Preselect, Now, EndPosition(L.A), EndPosition(L.B), G);
		}
		if (L.PendingAt >= 0.0 && Now >= L.PendingAt)
		{
			EngageMode(L.State, L.PendingMode, Now, EndPosition(L.A), EndPosition(L.B), G);
			L.PendingAt = -1.0;
		}
		++I;
	}
}

void FWorld::ApplyInput(int Index, const FPlayerInput& In)
{
	FWorldPlayer& P = PlayerList[static_cast<size_t>(Index)];
	if (!P.bStanding)
	{
		return;
	}
	if (Now >= HoldSpawnYawUntil)
	{
		P.Yaw = In.Yaw;
		P.Pitch = std::clamp(In.Pitch, -1.45, 1.45);
	}
	if (Now < P.StunnedUntil || !P.bCanUseLines)
	{
		P.bDrawn = false;
		return;
	}
	const FHarnessTuning& H = Tables.Harness(P.Class);
	// The Cutter (05 4.1): out while held; a swing once warm and recovered.
	if (In.bDraw && !P.bDrawn)
	{
		P.bDrawn = true;
		P.DrawnAt = Now;
		FWorldEvent E;
		E.Kind = FWorldEvent::EKind::Draw;
		E.Player = Index;
		Events.push_back(E);
	}
	if (!In.bDraw)
	{
		P.bDrawn = false;
	}
	if (In.bSwing && P.bDrawn)
	{
		Swing(Index);
	}
	// The Spool.
	if (In.Mode >= 1 && In.Mode <= 3)
	{
		const ELineMode Mode = In.Mode == 1 ? ELineMode::Haul : In.Mode == 2 ? ELineMode::Jack : ELineMode::Hang;
		P.Preselect = Mode;
		const int Active = ActiveLineOf(Index);
		if (Active >= 0)
		{
			FWorldLine& L = LineList[static_cast<size_t>(Active)];
			L.Preselect = Mode;
			if (L.bLive() && L.State.Mode != Mode)
			{
				L.PendingMode = Mode;
				L.PendingAt = Now + H.ModeSwitch;
			}
			else if (L.bLive())
			{
				L.PendingAt = -1.0;  // Back to the mode it is in: the switch is called off
			}
		}
	}
	if (In.bSelfDog)
	{
		P.bSelfDogNext = true;
	}
	if (In.bFire || In.bSelfDog)
	{
		FireDog(Index);
	}
	if (In.bRelease)
	{
		ReleaseActive(Index);
	}
}

void FWorld::ReleaseActive(int Index)
{
	const int Active = ActiveLineOf(Index);
	if (Active < 0)
	{
		return;
	}
	const FWorldLine& L = LineList[static_cast<size_t>(Active)];
	if (!(L.bLive() && L.State.Mode == ELineMode::Hang))
	{
		EndLine(static_cast<size_t>(Active), ELineEnded::Released);
		return;
	}
	const FHarnessTuning& H = Tables.Harness(PlayerList[static_cast<size_t>(Index)].Class);
	int SetCount = 0;
	for (const FWorldLine& O : LineList)
	{
		SetCount += O.Owner == Index && O.bSet ? 1 : 0;
	}
	if (SetCount >= H.SetLines)
	{
		for (size_t J = 0; J < LineList.size(); ++J)
		{
			if (LineList[J].Owner == Index && LineList[J].bSet)
			{
				EndLine(J, ELineEnded::Expired);  // Setting another retires the oldest (05 3.6)
				break;
			}
		}
	}
	FWorldLine& Kept = LineList[static_cast<size_t>(ActiveLineOf(Index))];
	Kept.bSet = true;
	Kept.SetExpiresAt = Now + H.SetLineLife;
	Kept.PendingAt = -1.0;
	EngageMode(Kept.State, ELineMode::Set, Now, EndPosition(Kept.A), EndPosition(Kept.B), Tables.Globals());
}

void FWorld::Swing(int Index)
{
	FWorldPlayer& P = PlayerList[static_cast<size_t>(Index)];
	const FCutterTuning& Cutter = Tables.Cutter(ECutterVariant::Standard);
	if (Now < P.DrawnAt + CutterWarmup(Index) - 1e-9 || Now < P.NextSwingAt - 1e-9)
	{
		return;  // Still warming up, or recovering from the last swing
	}
	P.NextSwingAt = Now + Cutter.Swing + Cutter.Recovery;
	FWorldEvent E;
	E.Kind = FWorldEvent::EKind::Swing;
	E.Player = Index;
	const FCutQuery Q{EyeOf(Index), Forward(Index), Cutter.Reach, Cutter.ConeDeg};
	double Best = Inf;
	int BestLine = -1;
	for (size_t I = 0; I < LineList.size(); ++I)
	{
		const FWorldLine& L = LineList[I];
		if (!L.bLive())
		{
			continue;
		}
		const FVec3 A = EndPosition(L.A);
		const FVec3 B = EndPosition(L.B);
		const std::optional<double> T = FindCutPoint(Q, A, B);
		if (T)
		{
			const double D = (A + (B - A) * *T - Q.Eye).Length();
			if (D < Best)
			{
				Best = D;
				BestLine = static_cast<int>(I);
			}
		}
	}
	E.Line = BestLine >= 0 ? LineList[static_cast<size_t>(BestLine)].Id : -1;
	Events.push_back(E);
	if (BestLine >= 0)
	{
		EndLine(static_cast<size_t>(BestLine), ELineEnded::Cut);
	}
}

double FWorld::InvMassOfCrate(const FCrate& C) const { return CanEverMoveUnderALine(C.Object) ? 1.0 / C.MassKg : 0.0; }

FLineEndpoint FWorld::EndpointOf(const FLineEnd& End, bool bHeld) const
{
	FLineEndpoint E;
	switch (End.Seat)
	{
	case EDogSeat::Static:
		E.Seat = ESeat::Static;
		E.Position = End.Point;
		break;
	case EDogSeat::Crate:
	{
		const FCrate& C = CrateList[static_cast<size_t>(End.Crate)];
		E.Seat = ESeat::Body;
		E.Position = C.Position;
		E.Velocity = C.Velocity;
		E.InvMass = bHeld ? 0.0 : InvMassOfCrate(C);
		E.bSmallPropBonus = C.bSmallPropBonus;
		break;
	}
	case EDogSeat::Plate:
	case EDogSeat::OwnPlate:
	{
		const FWorldPlayer& P = PlayerList[static_cast<size_t>(End.Player)];
		E.Seat = End.Seat == EDogSeat::Plate ? ESeat::OtherPlate : ESeat::OwnPlate;
		E.Position = P.Position;
		E.Velocity = P.Velocity;
		E.InvMass = P.bStanding ? 1.0 / Tables.Harness(P.Class).MassKg : 0.0;
		break;
	}
	case EDogSeat::None:
		break;
	}
	return E;
}

void FWorld::StallPlayer(int Index, EStallCause Cause)
{
	FWorldPlayer& P = PlayerList[static_cast<size_t>(Index)];
	if (!P.bStanding)
	{
		return;
	}
	P.bStanding = false;
	P.bDrawn = false;
	FWorldEvent E;
	E.Kind = FWorldEvent::EKind::Stall;
	E.Player = Index;
	E.Cause = Cause;
	E.At = P.Position;
	Events.push_back(E);
	// The Hooks lift them out: off the board until the reset.
	P.Velocity = {};
	P.Position = {0, 0, -50};
}

void FWorld::Step(const std::vector<FPlayerInput>& Inputs)
{
	for (size_t I = 0; I < PlayerList.size() && I < Inputs.size(); ++I)
	{
		ApplyInput(static_cast<int>(I), Inputs[I]);
	}
	UpdateDogs();
	UpdateLines();
	Simulate(Inputs);
	UpdateLines();  // Lines on anyone Stalled this step end now
	Now += Dt;
}

void FWorld::Simulate(const std::vector<FPlayerInput>& Inputs)
{
	const FSpoolGlobals& G = Tables.Globals();
	const size_t NP = PlayerList.size();
	const size_t NB = NP + CrateList.size();
	const auto BodyOf = [NP](const FLineEnd& E) -> int {
		switch (E.Seat)
		{
		case EDogSeat::Crate: return static_cast<int>(NP) + E.Crate;
		case EDogSeat::Plate:
		case EDogSeat::OwnPlate: return E.Player;
		default: return -1;
		}
	};

	// A placed crate with nothing left under it (the crate below was pulled out) falls.
	for (FCrate& Crate : CrateList)
	{
		if (Crate.Object.Placement == EPlacement::Loose || Crate.Object.bBrokenFree)
		{
			continue;
		}
		const double Bottom = Crate.Position.Z - Crate.Half.Z;
		bool bSupported = Bottom - SupportTop(Crate.Position, Crate.Half) < GroundSnap;
		for (const FCrate& Other : CrateList)
		{
			const bool bUnder = &Other != &Crate && std::abs(Other.Position.X - Crate.Position.X) < Other.Half.X + Crate.Half.X &&
			                    std::abs(Other.Position.Y - Crate.Position.Y) < Other.Half.Y + Crate.Half.Y &&
			                    std::abs(Other.Position.Z + Other.Half.Z - Bottom) < GroundSnap;
			bSupported = bSupported || bUnder;
		}
		Crate.Object.bBrokenFree = !bSupported;
	}

	// Which crates hold this step: the thresholds see T, without the Scrapper bonus, so a trial
	// step on copies decides first (as the reference simulator does).
	std::vector<FVec3> Threshold(NB);
	std::vector<bool> Pulled(NB, false);
	for (const FWorldLine& L : LineList)
	{
		if (!L.bLive() || !IsActive(L.State.Mode))
		{
			continue;
		}
		FLineState Trial = L.State;
		const FLineStepResult R = StepLine(Trial, L.Params, L.TensionN, EndpointOf(L.A, false), EndpointOf(L.B, false), G, Dt, Now);
		const int A = BodyOf(L.A), B = BodyOf(L.B);
		if (A >= 0) Threshold[static_cast<size_t>(A)] = Threshold[static_cast<size_t>(A)] + R.BreakFreeOnA, Pulled[static_cast<size_t>(A)] = true;
		if (B >= 0) Threshold[static_cast<size_t>(B)] = Threshold[static_cast<size_t>(B)] + R.BreakFreeOnB, Pulled[static_cast<size_t>(B)] = true;
	}
	std::vector<bool> Held(CrateList.size(), false);
	for (size_t C = 0; C < CrateList.size(); ++C)
	{
		if (InvMassOfCrate(CrateList[C]) > 0.0)
		{
			const FObjectLineResponse R = ResolveObjectUnderLines(CrateList[C].Object, Threshold[NP + C], !Pulled[NP + C], Tables);
			Held[C] = R.InvMass <= 0.0;
			if (R.bBreaksFreeNow)
			{
				CrateList[C].Object.bBrokenFree = true;
			}
		}
	}

	// Haul and Jack forces.
	std::vector<FVec3> Pull(NB);
	std::vector<bool> Kick(NB, false);
	for (size_t I = 0; I < LineList.size();)
	{
		FWorldLine& L = LineList[I];
		if (!L.bLive() || !IsActive(L.State.Mode))
		{
			++I;
			continue;
		}
		const int A = BodyOf(L.A), B = BodyOf(L.B);
		const bool bHeldA = A >= static_cast<int>(NP) && Held[static_cast<size_t>(A) - NP];
		const bool bHeldB = B >= static_cast<int>(NP) && Held[static_cast<size_t>(B) - NP];
		const FLineStepResult R = StepLine(L.State, L.Params, L.TensionN, EndpointOf(L.A, bHeldA), EndpointOf(L.B, bHeldB), G, Dt, Now);
		const bool bKick = (R.Events & LineEvents::KickActive) != 0;
		if (A >= 0) Pull[static_cast<size_t>(A)] = Pull[static_cast<size_t>(A)] + R.ForceOnA, Kick[static_cast<size_t>(A)] = Kick[static_cast<size_t>(A)] || bKick;
		if (B >= 0) Pull[static_cast<size_t>(B)] = Pull[static_cast<size_t>(B)] + R.ForceOnB, Kick[static_cast<size_t>(B)] = Kick[static_cast<size_t>(B)] || bKick;
		if (R.Events & (LineEvents::DogsMet | LineEvents::AutoRelease))
		{
			EndLine(I, (R.Events & LineEvents::DogsMet) ? ELineEnded::DogsMet : ELineEnded::Released);
			continue;
		}
		++I;
	}

	// Velocities: players by footing, walking and air control; crates by their pull.
	for (size_t I = 0; I < NP; ++I)
	{
		FWorldPlayer& P = PlayerList[I];
		if (!P.bStanding)
		{
			continue;
		}
		const FHarnessTuning& H = Tables.Harness(P.Class);
		FPlayerInput In;
		if (I < Inputs.size() && P.bCanMove && Now >= P.StunnedUntil)
		{
			In = Inputs[I];
		}
		const FVec3 F{std::cos(P.Yaw), std::sin(P.Yaw), 0};
		const FVec3 R{std::sin(P.Yaw), -std::cos(P.Yaw), 0};
		FVec3 Wish = F * In.MoveForward + R * In.MoveRight;
		if (Wish.Length() > 1.0)
		{
			Wish = Wish.SafeNormal();
		}
		const FVec3 Desired = Wish * (In.bSprint ? H.SprintSpeed : H.RunSpeed);

		// Brace: crouch while still for 0.3 s (05 5.4).
		const bool bStill = Flat(P.Velocity).Length() < BraceStillSpeed && Wish.Length() < 0.1;
		if (H.bCanBrace && In.bBrace && P.bGrounded && bStill)
		{
			P.BraceHeld += Dt;
			P.bBraced = P.bBraced || P.BraceHeld >= BraceSeconds;
		}
		else
		{
			P.BraceHeld = 0.0;
			P.bBraced = false;
		}

		const FVec3 LineForce = Pull[I] + LastLineForce[I];
		const FPlayerLineResponse Resp =
		    ResolvePlayerUnderLine(PlayerBodyFor(H, P.bBraced, P.bGrounded), LineForce, Kick[I], G.Gravity);
		if (Resp.bBraceBroken)
		{
			P.bBraced = false;
		}
		const EFooting Was = P.Footing;
		P.Footing = Resp.Footing;
		// A Haul or Jack on a grounded player overrides their movement (05 5.1). A Hang line
		// only holds them at its length, which the solve does; they may still walk.
		// A pull with nothing along the ground (a Jack pinning them down) leaves them walking.
		const bool bLine = Flat(Pull[I]).Length() > 1e-6;
		switch (Resp.Footing)
		{
		case EFooting::Braced:
			P.Velocity = {0, 0, P.Velocity.Z};
			break;
		case EFooting::Skid:
			if (P.bGrounded)
			{
				FVec3 Flat2 = Flat(P.Velocity);
				if (bLine && !P.bUnderLine)
				{
					// The line takes over: the feet plant and only motion along the pull is kept.
					const FVec3 Dir = Flat(Pull[I]).SafeNormal();
					Flat2 = Dir * std::max(0.0, Flat2.Dot(Dir));
				}
				if (bLine)
				{
					Flat2 = Flat2 + Flat(Pull[I]) * (Dt / H.MassKg);  // The skid: the line's acceleration, nothing else
				}
				else
				{
					const FVec3 Gap = Desired - Flat2;
					const double MaxStep = WalkAccel * Dt;
					Flat2 = Flat2 + (Gap.Length() > MaxStep ? Gap.SafeNormal() * MaxStep : Gap);
				}
				P.Velocity = {Flat2.X, Flat2.Y, -G.Gravity * Dt};
				if (In.bJump && !bLine)
				{
					P.Velocity.Z = std::sqrt(2.0 * G.Gravity * H.JumpHeight);
					P.bGrounded = false;
				}
				break;
			}
			[[fallthrough]];
		case EFooting::Airborne:
		case EFooting::Hoisted:
			if ((Was == EFooting::Skid || Was == EFooting::Braced) && Resp.Footing != EFooting::Skid)
			{
				P.Impacts.NotifyHandoff();  // Grounded to flight body, once: the settle is not a hit
				P.bBraced = false;
			}
			P.bGrounded = Resp.Footing == EFooting::Skid ? P.bGrounded : false;
			P.Velocity = P.Velocity + Pull[I] * (Dt / H.MassKg);
			P.Velocity.Z -= G.Gravity * Dt;
			// Air control (2.3): only with input, only along it, up to the run speed, and not
			// while hoisted (5.2) or under a line.
			if (Resp.Footing == EFooting::Airborne && !bLine && Wish.Length() > 0.1)
			{
				const FVec3 Dir = Wish.SafeNormal();
				const double Along = Flat(P.Velocity).Dot(Dir);
				const double Room = Desired.Length() - Along;
				if (Room > 0.0)
				{
					P.Velocity = P.Velocity + Dir * std::min(Room, H.AirControl * AirAccel * Dt);
				}
			}
			break;
		}
		P.bUnderLine = bLine;
	}
	for (size_t C = 0; C < CrateList.size(); ++C)
	{
		FCrate& Crate = CrateList[C];
		if (Held[C])
		{
			Crate.Velocity = {};  // A placed crate under its threshold stays where it was put
			continue;
		}
		if (InvMassOfCrate(Crate) > 0.0)
		{
			Crate.Velocity = Crate.Velocity + Pull[NP + C] * (Dt / Crate.MassKg);
		}
		Crate.Velocity.Z -= G.Gravity * Dt;
	}

	// The Hang solve integrates every position (HangSolver.h), with the floor under each body.
	std::vector<FHangBody> Solve;
	Solve.reserve(NB + LineList.size() * 2);
	for (size_t I = 0; I < NP; ++I)
	{
		const FWorldPlayer& P = PlayerList[I];
		const double Top = SupportTop(P.Position, PlayerHalf);
		const bool bOnFloor = P.bGrounded && std::isfinite(Top);
		Solve.push_back({P.Position, P.Velocity,
		                 P.bStanding && !P.bBraced ? 1.0 / Tables.Harness(P.Class).MassKg : 0.0,
		                 bOnFloor ? std::optional<double>(Top + PlayerHalfHeight) : std::nullopt});
	}
	for (size_t C = 0; C < CrateList.size(); ++C)
	{
		const FCrate& Crate = CrateList[C];
		const double Top = SupportTop(Crate.Position, Crate.Half);
		const bool bOnFloor = std::isfinite(Top) && Crate.Position.Z - Crate.Half.Z - Top < GroundSnap;
		const double Inv = ResolveObjectUnderLines(Crate.Object, Threshold[NP + C], !Pulled[NP + C], Tables).InvMass;
		Solve.push_back({Crate.Position, Crate.Velocity, Held[C] ? 0.0 : Inv,
		                 bOnFloor ? std::optional<double>(Top + Crate.Half.Z) : std::nullopt});
	}
	std::vector<FHangLine> HangLines;
	std::vector<size_t> HangOwners;
	for (size_t I = 0; I < LineList.size(); ++I)
	{
		FWorldLine& L = LineList[I];
		if (!L.bLive() || !IsHang(L.State.Mode))
		{
			continue;
		}
		const auto Index = [&](const FLineEnd& E) {
			const int B = BodyOf(E);
			if (B >= 0)
			{
				return B;
			}
			Solve.push_back({E.Point, {}, 0.0, std::nullopt});  // A static Dog
			return static_cast<int>(Solve.size()) - 1;
		};
		const int A = Index(L.A);
		const int B = Index(L.B);
		HangLines.push_back({A, B, &L.State, L.Params, L.Params.HangLoadN, 0.0, {}, {}, 0});
		HangOwners.push_back(I);
	}
	SolveHangLines(Solve, HangLines, G, Dt);
	std::fill(LastLineForce.begin(), LastLineForce.end(), FVec3{});
	for (const FHangLine& H : HangLines)
	{
		if (static_cast<size_t>(H.BodyA) < NP) LastLineForce[static_cast<size_t>(H.BodyA)] = LastLineForce[static_cast<size_t>(H.BodyA)] + H.ForceOnA;
		if (static_cast<size_t>(H.BodyB) < NP) LastLineForce[static_cast<size_t>(H.BodyB)] = LastLineForce[static_cast<size_t>(H.BodyB)] + H.ForceOnB;
	}
	for (size_t I = 0; I < NP; ++I)
	{
		FWorldPlayer& P = PlayerList[I];
		if (P.bStanding)
		{
			P.Position = Solve[I].Position;
			P.Velocity = Solve[I].Velocity;
		}
	}
	for (size_t C = 0; C < CrateList.size(); ++C)
	{
		CrateList[C].Position = Solve[NP + C].Position;
		CrateList[C].Velocity = Solve[NP + C].Velocity;
	}
	// Lines the Hang solve let go of.
	for (size_t K = HangLines.size(); K-- > 0;)
	{
		if (HangLines[K].Events & LineEvents::RanOut)
		{
			EndLine(HangOwners[K], ELineEnded::RanOut);
		}
	}

	std::vector<std::vector<FContact>> Contacts(NP);
	ResolveCollisions(Contacts);
	HangLineContacts();
	for (size_t I = 0; I < NP; ++I)
	{
		FWorldPlayer& P = PlayerList[I];
		if (!P.bStanding)
		{
			continue;
		}
		const double Cap = G.HardVelocityCap;
		if (P.Velocity.Length() > Cap)
		{
			P.Velocity = P.Velocity * (Cap / P.Velocity.Length());
		}
		const EOutcome Outcome = P.Impacts.EndStep(Contacts[I], Now, Dt);
		if (Outcome == EOutcome::Stall)
		{
			EStallCause Cause = EStallCause::WorldImpact;
			for (const FContact& C : Contacts[I])
			{
				Cause = C.Kind == EContactKind::Object   ? EStallCause::ObjectImpact
				        : C.Kind == EContactKind::Player ? EStallCause::PlayerImpact
				                                         : Cause;
			}
			StallPlayer(static_cast<int>(I), Cause);
			continue;
		}
		if (Outcome == EOutcome::Stagger || Outcome == EOutcome::Flip)
		{
			P.StunnedUntil = Now + (Outcome == EOutcome::Flip ? FlipSeconds : StaggerSeconds);
			P.bBraced = false;
			FWorldEvent E;
			E.Kind = Outcome == EOutcome::Flip ? FWorldEvent::EKind::Flip : FWorldEvent::EKind::Stagger;
			E.Player = static_cast<int>(I);
			Events.push_back(E);
		}
		if (P.Position.Z < ArenaData.KillZ)
		{
			StallPlayer(static_cast<int>(I), EStallCause::RingOut);
		}
	}
}

void FWorld::ResolveCollisions(std::vector<std::vector<FContact>>& Contacts)
{
	const size_t NP = PlayerList.size();
	struct FRef
	{
		FVec3* Position;
		FVec3* Velocity;
		FVec3 Half;
		double InvMass;
		double MassKg;
		int Player;  // -1 for a crate
		int Crate;   // -1 for a player
		bool* bGrounded;
		bool bInFlight;  // A player on their flight body, or a crate moving: their contacts are judged
	};
	std::vector<FRef> Bodies;
	for (size_t I = 0; I < NP; ++I)
	{
		FWorldPlayer& P = PlayerList[I];
		if (!P.bStanding)
		{
			continue;
		}
		const double M = Tables.Harness(P.Class).MassKg;
		// Grounded, the character walks (CMC in the engine) and its bumps are not hits; only
		// the flight body is judged (player-physics section 4).
		const bool bFlight = !P.bGrounded || P.Footing == EFooting::Airborne || P.Footing == EFooting::Hoisted;
		Bodies.push_back({&P.Position, &P.Velocity, PlayerHalf, P.bBraced ? 0.0 : 1.0 / M, M, static_cast<int>(I), -1,
		                  &P.bGrounded, bFlight});
	}
	for (size_t C = 0; C < CrateList.size(); ++C)
	{
		FCrate& Crate = CrateList[C];
		const bool bMoving = Crate.Velocity.Length() > CrateHitSpeed;
		// Placed crates (a stack) stay put against bumps until they have broken free (6.2).
		const bool bFree = CanEverMoveUnderALine(Crate.Object) &&
		                   (Crate.Object.Placement == EPlacement::Loose || Crate.Object.bBrokenFree);
		Bodies.push_back({&Crate.Position, &Crate.Velocity, Crate.Half, bFree ? 1.0 / Crate.MassKg : 0.0, Crate.MassKg, -1,
		                  static_cast<int>(C), nullptr, bMoving});
	}

	// Against the solids.
	for (FRef& B : Bodies)
	{
		bool bGround = false;
		for (int Pass = 0; Pass < 3; ++Pass)
		{
			for (const FSolid& S : ArenaData.Solids)
			{
				const std::optional<FPush> Push = Overlap(*B.Position, B.Half, CenterOf(S), HalfOf(S));
				if (!Push)
				{
					continue;
				}
				*B.Position = *B.Position + Push->Normal * Push->Depth;
				const double Into = -B.Velocity->Dot(Push->Normal);
				if (Into > 0.0)
				{
					*B.Velocity = *B.Velocity + Push->Normal * Into;
					if (B.Player >= 0 && B.bInFlight)
					{
						FContact C;
						C.OtherId = -1000 - static_cast<int>(&S - ArenaData.Solids.data());
						C.Kind = EContactKind::Static;
						C.RelativeSpeed = Into;
						C.Surface = S.Surface;
						C.Normal = Push->Normal;
						Contacts[static_cast<size_t>(B.Player)].push_back(C);
					}
				}
				if (Push->Normal.Z > 0.7)
				{
					bGround = true;
				}
			}
		}
		// Standing on something within the snap distance counts as grounded.
		const double Top = SupportTop(*B.Position, B.Half);
		if (std::isfinite(Top) && B.Position->Z - B.Half.Z - Top < GroundSnap && B.Velocity->Z <= 0.1)
		{
			bGround = true;
		}
		if (B.bGrounded)
		{
			*B.bGrounded = bGround;
		}
		else if (bGround)
		{
			// Crates slide to a stop on the floor.
			const double K = std::max(0.0, 1.0 - CrateFriction * Dt);
			*B.Velocity = {B.Velocity->X * K, B.Velocity->Y * K, B.Velocity->Z};
		}
	}

	// Bodies against each other: pushed apart by inverse mass, the closing speed shared.
	for (size_t I = 0; I < Bodies.size(); ++I)
	{
		for (size_t J = I + 1; J < Bodies.size(); ++J)
		{
			FRef& A = Bodies[I];
			FRef& B = Bodies[J];
			const std::optional<FPush> Push = Overlap(*A.Position, A.Half, *B.Position, B.Half);
			const double WSum = A.InvMass + B.InvMass;
			if (!Push || WSum <= 0.0)
			{
				continue;
			}
			*A.Position = *A.Position + Push->Normal * (Push->Depth * A.InvMass / WSum);
			*B.Position = *B.Position - Push->Normal * (Push->Depth * B.InvMass / WSum);
			const double Closing = (*B.Velocity - *A.Velocity).Dot(Push->Normal);  // > 0: approaching
			if (Closing <= 0.0)
			{
				continue;
			}
			const double Impulse = Closing / WSum;  // Perfectly inelastic along the normal
			*A.Velocity = *A.Velocity + Push->Normal * (Impulse * A.InvMass);
			*B.Velocity = *B.Velocity - Push->Normal * (Impulse * B.InvMass);
			const bool bJudged = A.bInFlight || B.bInFlight;
			const auto Report = [&](const FRef& Me, const FRef& Other, const FVec3& Normal) {
				if (Me.Player < 0 || !bJudged)
				{
					return;
				}
				FContact C;
				C.OtherId = Other.Player >= 0 ? Other.Player : 10000 + Other.Crate;
				C.Kind = Other.Player >= 0 ? EContactKind::Player : EContactKind::Object;
				C.RelativeSpeed = Closing;
				C.ImpulseNs = Impulse;
				C.OtherMassKg = Other.MassKg;
				C.Normal = Normal;
				Contacts[static_cast<size_t>(Me.Player)].push_back(C);
			};
			Report(A, B, Push->Normal);
			Report(B, A, Push->Normal * -1.0);
		}
	}
}
// Hang and set lines are solid to players (05 3.5): walking into one stops you; over 3 m/s
// it staggers you (the line owner's class time: 1.0 s for a Rigger's line); over 6 m/s it
// flips you over it with your momentum. Haul and Jack lines are not solid. Crouching and
// sliding under are not in the prototype.
void FWorld::HangLineContacts()
{
	for (const FWorldLine& L : LineList)
	{
		if (!L.bLive() || !IsHang(L.State.Mode))
		{
			continue;
		}
		const FVec3 A = EndPosition(L.A);
		const FVec3 B = EndPosition(L.B);
		for (size_t I = 0; I < PlayerList.size(); ++I)
		{
			FWorldPlayer& P = PlayerList[I];
			const bool bOnIt = ((L.A.Seat == EDogSeat::Plate || L.A.Seat == EDogSeat::OwnPlate) && L.A.Player == static_cast<int>(I)) ||
			                   ((L.B.Seat == EDogSeat::Plate || L.B.Seat == EDogSeat::OwnPlate) && L.B.Player == static_cast<int>(I));
			if (!P.bStanding || bOnIt)
			{
				continue;
			}
			// A line lying on the floor (or within a step of it) is stepped over.
			const FVec3 Low = P.Position - PlayerHalf + FVec3{0, 0, LineStepOverM};
			if (SegmentBox(A, B, Low, P.Position + PlayerHalf).T > 1.0)
			{
				continue;
			}
			// The line's nearest point to the player, and the way out across it.
			const FVec3 AB = B - A;
			const double T = std::clamp((P.Position - A).Dot(AB) / std::max(AB.Dot(AB), 1e-9), 0.0, 1.0);
			const FVec3 Near = A + AB * T;
			FVec3 Out = Flat(P.Position - Near);
			const FVec3 Across = Flat(AB).SafeNormal();
			Out = (Out - Across * Out.Dot(Across)).SafeNormal();
			if (Out.Length() < 0.5)
			{
				continue;
			}
			const double Speed = -P.Velocity.Dot(Out);  // Into the line
			if (Speed <= 0.0)
			{
				continue;
			}
			// Already staggered or flipped: the line still blocks, but does not hit again.
			EOutcome Hit = Now < P.StunnedUntil ? EOutcome::None : Classify(EImpactTable::HangLineHit, Speed, ESurface::Hard, Tables);
			const FVec3 Over = P.Position - Out * (2.0 * PlayerHalfWidth + 0.1) + FVec3{0, 0, 0.4};
			bool bClear = true;
			for (const FSolid& S : ArenaData.Solids)
			{
				bClear = bClear && !Overlap(Over, PlayerHalf, CenterOf(S), HalfOf(S));
			}
			if (Hit == EOutcome::Flip && !bClear)
			{
				Hit = EOutcome::Stagger;  // No room on the far side: it stops them instead
			}
			if (Hit == EOutcome::Flip)
			{
				// Ragdolled over it, landing on the far side with momentum preserved.
				P.Position = Over;
				P.Velocity.Z = std::max(P.Velocity.Z, 3.0);
				P.bGrounded = false;
				P.StunnedUntil = Now + FlipSeconds;
			}
			else
			{
				P.Velocity = P.Velocity + Out * Speed;
				P.Position = Near + Out * (PlayerHalfWidth + 0.02) + FVec3{0, 0, P.Position.Z - Near.Z};
				if (Hit == EOutcome::Stagger)
				{
					P.StunnedUntil = Now + Tables.Harness(PlayerList[static_cast<size_t>(L.Owner)].Class).HangStaggerSeconds;
				}
			}
			if (Hit != EOutcome::None)
			{
				FWorldEvent E;
				E.Kind = Hit == EOutcome::Flip ? FWorldEvent::EKind::Flip : FWorldEvent::EKind::Stagger;
				E.Player = static_cast<int>(I);
				E.Line = L.Id;
				Events.push_back(E);
			}
		}
	}
}
} // namespace Kinetic::Web
