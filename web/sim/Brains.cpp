#include "Brains.h"

#include <algorithm>
#include <cmath>

namespace Kinetic::Web
{
namespace
{
const FWorldLine* FindLine(const FWorld& World, int Id)
{
	for (const FWorldLine& L : World.Lines())
	{
		if (L.Id == Id)
		{
			return &L;
		}
	}
	return nullptr;
}

const FWorldLine* ActiveLine(const FWorld& World, int Player)
{
	for (const FWorldLine& L : World.Lines())
	{
		if (L.Owner == Player && !L.bSet)
		{
			return &L;
		}
	}
	return nullptr;
}

FVec3 Flat2(const FVec3& V) { return {V.X, V.Y, 0.0}; }

int ModeButton(ELineMode Mode)
{
	switch (Mode)
	{
	case ELineMode::Haul: return 1;
	case ELineMode::Jack: return 2;
	case ELineMode::Hang:
	case ELineMode::Set: return 3;
	case ELineMode::Slack: break;
	}
	return 0;
}
} // namespace

FBotBrain::FBotBrain(int InPlayer, uint64_t MatchSeed, const FKineticTables& InTables, EHarnessClass Class)
    : Player(InPlayer), Tables(InTables), Random(MatchSeed, InPlayer),
      Defense(EBotProfile::Backfill, Class, ECutterVariant::Standard, InTables, FBotRandom(MatchSeed ^ 0x5DEECE66Dull, InPlayer))
{
}

void FBotBrain::Reset()
{
	for (const int Id : Defending)
	{
		Defense.Ended(Id);
	}
	Defending.clear();
	Stage = EStage::Idle;
	StageUntil = 0.0;
	LineId = -1;
}

bool FBotBrain::OverAnEdge(const FWorld& World, const FVec3& From, const FVec3& To) const
{
	for (int I = 1; I <= 12; ++I)
	{
		const FVec3 P = From + (To - From) * (static_cast<double>(I) / 13.0);
		if (!World.HasFloorAt({P.X, P.Y, 0.0}))
		{
			return true;
		}
	}
	return false;
}

FBotView FBotBrain::ViewOf(const FWorld& World) const
{
	const FWorldPlayer& Me = World.Players()[static_cast<size_t>(Player)];
	FBotView V;
	V.Slot = Me.Slot;
	V.Team = Me.Team;
	V.Class = Me.Class;
	V.Position = Me.Position;
	V.bLineLive = ActiveLine(World, Player) != nullptr;
	V.bLineOnSelf = !World.LinesOnPlayer(Player).empty();
	const FWorldPlayer* NearestEnemy = nullptr;
	for (size_t I = 0; I < World.Players().size(); ++I)
	{
		const FWorldPlayer& O = World.Players()[I];
		if (static_cast<int>(I) == Player)
		{
			continue;
		}
		V.Others.push_back({O.Slot, O.Team, O.Position, O.Velocity, O.bStanding});
		if (O.Team != Me.Team && O.bStanding &&
		    (!NearestEnemy || (O.Position - Me.Position).Length() < (NearestEnemy->Position - Me.Position).Length()))
		{
			NearestEnemy = &O;
		}
	}
	for (const FAnchorSpec& A : World.Arena().Anchors)
	{
		const bool bEdge = NearestEnemy && OverAnEdge(World, NearestEnemy->Position, A.Point);
		V.Anchors.push_back({A.Point, bEdge});
	}
	for (const FDoorwaySpec& D : World.Arena().Doorways)
	{
		V.Doorways.push_back({D.Left, D.Right});
	}
	return V;
}

FVec3 FBotBrain::AimPoint(const FWorld& World, const FVec3& Target, const FVec3& TargetVelocity) const
{
	const FWorldPlayer& Me = World.Players()[static_cast<size_t>(Player)];
	const double Speed = Tables.Harness(Me.Class).DogSpeed;
	const double Time = (Target - World.EyeOf(Player)).Length() / Speed;
	const double G = Tables.Globals().Gravity;
	return Target + TargetVelocity * Time + FVec3{0, 0, 0.5 * G * Time * Time};
}

void FBotBrain::FaceToward(const FWorld& World, const FVec3& Point, FPlayerInput& In) const
{
	const FVec3 D = Point - World.EyeOf(Player);
	In.Yaw = std::atan2(D.Y, D.X);
	In.Pitch = std::atan2(D.Z, std::sqrt(D.X * D.X + D.Y * D.Y));
}

void FBotBrain::Walk(const FWorld& World, FPlayerInput& In) const
{
	const FWorldPlayer& Me = World.Players()[static_cast<size_t>(Player)];
	const FWorldPlayer* Target = nullptr;
	for (const FWorldPlayer& O : World.Players())
	{
		if (O.Team != Me.Team && O.bStanding &&
		    (!Target || (O.Position - Me.Position).Length() < (Target->Position - Me.Position).Length()))
		{
			Target = &O;
		}
	}
	if (!Target)
	{
		return;
	}
	const double Range = Tables.Harness(Me.Class).LineLength * ApproachShare;
	FVec3 Goal = Target->Position;
	// Across the trench: by the nearer bridge.
	if (Me.Position.X * Target->Position.X < 0.0 && std::abs(Me.Position.X) > 2.6)
	{
		const double BridgeY = std::abs(Me.Position.Y - 13.0) < std::abs(Me.Position.Y + 13.0) ? 13.0 : -13.0;
		Goal = {Me.Position.X > 0.0 ? 3.0 : -3.0, BridgeY, Me.Position.Z};
		if ((FVec3{Goal.X, Goal.Y, 0} - FVec3{Me.Position.X, Me.Position.Y, 0}).Length() < 1.0)
		{
			Goal = {-Goal.X, BridgeY, Me.Position.Z};
		}
	}
	else if ((Target->Position - Me.Position).Length() <= Range)
	{
		FaceToward(World, Target->Position, In);
		return;  // Close enough to rig
	}
	const FVec3 To{Goal.X - Me.Position.X, Goal.Y - Me.Position.Y, 0};
	if (To.Length() < 0.5)
	{
		return;
	}
	const FVec3 Dir = To.SafeNormal();
	const FVec3 Feet = Me.Position - FVec3{0, 0, PlayerHalfHeight};
	if (!World.HasFloorAt(Feet + Dir * 1.2))
	{
		return;  // Never walk off a drop
	}
	In.Yaw = std::atan2(Dir.Y, Dir.X);
	In.Pitch = 0.0;
	In.MoveForward = 1.0;
}

FPlayerInput FBotBrain::Think(const FWorld& World, const std::vector<FWorldEvent>& Events)
{
	const FWorldPlayer& Me = World.Players()[static_cast<size_t>(Player)];
	FPlayerInput In;
	In.Yaw = Me.Yaw;
	In.Pitch = Me.Pitch;
	const double Now = World.Time();
	if (!Me.bStanding)
	{
		return In;
	}
	for (const FWorldEvent& E : Events)
	{
		if (E.Kind == FWorldEvent::EKind::Seat && E.Target == Player && E.Player != Player)
		{
			Defense.Seated(E.Line, Now);
			Defending.push_back(E.Line);
		}
		if (E.Kind == FWorldEvent::EKind::LineEnded)
		{
			if (std::find(Defending.begin(), Defending.end(), E.Line) != Defending.end())
			{
				Defense.Ended(E.Line);
				std::erase(Defending, E.Line);
			}
			if (E.Line == LineId && Stage != EStage::Cooldown)
			{
				Stage = EStage::Cooldown;
				StageUntil = Now + Random.Uniform(1.0, 2.0);
			}
		}
	}

	// Defense first: the cut reflex on its own plates.
	const FBotDefense::FActions Act = Defense.Tick(Now);
	In.bDraw = !Defense.Plans().empty();
	if (Act.Swing)
	{
		if (const FWorldLine* L = FindLine(World, *Act.Swing))
		{
			const bool bMineA = L->A.Seat == EDogSeat::Plate && L->A.Player == Player;
			const FVec3 Plate = World.EndPosition(bMineA ? L->A : L->B);
			const FVec3 Far = World.DogOrEndPosition(*L, !bMineA);
			FaceToward(World, Plate + (Far - Plate).SafeNormal() * 1.0, In);
			In.bSwing = true;
		}
	}
	if (!Defending.empty())
	{
		return In;  // Cutting free comes before anything else
	}

	const FWorldLine* Mine = ActiveLine(World, Player);
	switch (Stage)
	{
	case EStage::Idle:
	{
		if (Now < StageUntil)
		{
			Walk(World, In);
			break;
		}
		Intent = ChooseRig(ViewOf(World), Tables);
		if (Intent.Rig == EBotRig::Slingshot)
		{
			// The tree's own check: never launch over a drop. The Kick's V carries about V²/g.
			const double V = Tables.Harness(Me.Class).KickLaunchSpeedV;
			const FVec3 Dir = Flat2(Me.Position - Intent.DogB).SafeNormal();
			const FVec3 Feet = Me.Position - FVec3{0, 0, PlayerHalfHeight};
			if (OverAnEdge(World, Feet, Feet + Dir * (V * V / Tables.Globals().Gravity)))
			{
				Intent.Rig = EBotRig::None;
			}
		}
		if (Intent.Rig == EBotRig::None)
		{
			Walk(World, In);
			break;
		}
		In.Mode = ModeButton(Intent.Mode);
		Stage = EStage::AimA;
		StageUntil = Now + AimSeconds;
		break;
	}
	case EStage::AimA:
	{
		FVec3 TargetV;
		for (const FWorldPlayer& O : World.Players())
		{
			if (O.Slot == Intent.TargetSlot)
			{
				TargetV = O.Velocity;
				if (!Intent.bSelfDogA && Intent.Rig != EBotRig::Clothesline)
				{
					Intent.DogA = O.Position;  // Track the target while aiming
				}
			}
		}
		if (!Intent.bSelfDogA)
		{
			const FVec3 Err{Random.Uniform(-AimErrorM, AimErrorM), Random.Uniform(-AimErrorM, AimErrorM),
			                Random.Uniform(-AimErrorM, AimErrorM)};
			const bool bMoving = Intent.Rig != EBotRig::Clothesline;
			FaceToward(World, AimPoint(World, Intent.DogA, bMoving ? TargetV : FVec3{}) + Err, In);
		}
		if (Now >= StageUntil)
		{
			(Intent.bSelfDogA ? In.bSelfDog : In.bFire) = true;
			Stage = EStage::WaitA;
			StageUntil = Now + SeatTimeoutSeconds;
		}
		break;
	}
	case EStage::WaitA:
		if (Mine)
		{
			LineId = Mine->Id;
		}
		if (Mine && Mine->A.Seat != EDogSeat::None)
		{
			Stage = EStage::AimB;
			StageUntil = Now + AimSeconds;
		}
		else if (Now >= StageUntil || !Mine)
		{
			In.bRelease = true;
			Stage = EStage::Cooldown;
			StageUntil = Now + Random.Uniform(1.0, 2.0);
		}
		break;
	case EStage::AimB:
		FaceToward(World, AimPoint(World, Intent.DogB, {}), In);
		if (Now >= StageUntil)
		{
			In.bFire = true;
			Stage = EStage::WaitB;
			StageUntil = Now + SeatTimeoutSeconds;
		}
		break;
	case EStage::WaitB:
		if (Mine && Mine->bLive())
		{
			Stage = EStage::Holding;
			StageUntil = Now + HoldSeconds;
			if (Intent.bReleaseAfterSet && Mine->State.Mode == ELineMode::Hang)
			{
				In.bRelease = true;  // The Clothesline: Hang, then a set line
				Stage = EStage::Cooldown;
				StageUntil = Now + Random.Uniform(1.0, 2.0);
			}
		}
		else if (Now >= StageUntil || !Mine)
		{
			In.bRelease = true;
			Stage = EStage::Cooldown;
			StageUntil = Now + Random.Uniform(1.0, 2.0);
		}
		break;
	case EStage::Holding:
	{
		bool bTargetDown = false;
		for (const FWorldPlayer& O : World.Players())
		{
			bTargetDown = bTargetDown || (O.Slot == Intent.TargetSlot && !O.bStanding);
		}
		if (!Mine || Now >= StageUntil || bTargetDown)
		{
			In.bRelease = Mine != nullptr;
			Stage = EStage::Cooldown;
			StageUntil = Now + Random.Uniform(1.0, 2.0);
		}
		break;
	}
	case EStage::Cooldown:
		Walk(World, In);
		if (Now >= StageUntil)
		{
			Stage = EStage::Idle;
			StageUntil = Now;
		}
		break;
	}
	return In;
}
} // namespace Kinetic::Web
