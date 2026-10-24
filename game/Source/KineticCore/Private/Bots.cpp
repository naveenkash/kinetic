#include "Kinetic/Bots.h"

#include "Kinetic/Cutter.h"

#include <algorithm>
#include <cmath>
#include <cstdio>

namespace Kinetic
{
namespace
{
constexpr double Pi = 3.14159265358979323846;

uint64_t SplitMix(uint64_t& State)
{
	uint64_t Z = (State += 0x9E3779B97F4A7C15ull);
	Z = (Z ^ (Z >> 30)) * 0xBF58476D1CE4E5B9ull;
	Z = (Z ^ (Z >> 27)) * 0x94D049BB133111EBull;
	return Z ^ (Z >> 31);
}

FVec3 NearestOnSegment(const FVec3& P, const FVec3& A, const FVec3& B)
{
	const FVec3 AB = B - A;
	const double L2 = AB.Dot(AB);
	const double T = L2 > 0.0 ? std::clamp((P - A).Dot(AB) / L2, 0.0, 1.0) : 0.0;
	return A + AB * T;
}

double DistanceToSegment(const FVec3& P, const FVec3& A, const FVec3& B)
{
	return (P - NearestOnSegment(P, A, B)).Length();
}

double Horizontal(const FVec3& A, const FVec3& B)
{
	const double X = A.X - B.X;
	const double Y = A.Y - B.Y;
	return std::sqrt(X * X + Y * Y);
}
} // namespace

double FBotHauler::ArrivesAt(const FVec3& P) const
{
	const double Along = (NearestOnSegment(P, TrackA, TrackB) - TrackA).Length();
	return ArrivesInSeconds + Along / std::max(SpeedMps, 1e-6);
}

FBotRandom::FBotRandom(uint64_t MatchSeed, int Slot) : State(MatchSeed)
{
	State ^= SplitMix(State) + static_cast<uint64_t>(Slot) * 0xD1B54A32D192ED03ull;
	SplitMix(State);
}

double FBotRandom::Uniform()
{
	return static_cast<double>(SplitMix(State) >> 11) * (1.0 / 9007199254740992.0);
}

std::string BotCallsign(EBotProfile Profile, int Number)
{
	char Buffer[16];
	std::snprintf(Buffer, sizeof(Buffer), "%c-%02d", Profile == EBotProfile::Probationer ? 'P' : 'O', Number);
	return Buffer;
}

FBotDefense::FBotDefense(EBotProfile InProfile, EHarnessClass Class, ECutterVariant Variant, const FKineticTables& Tables,
                         FBotRandom InRandom)
    : Profile(InProfile), WarmupSeconds(CutterWarmupSeconds(Variant, Class, Tables)),
      CycleSeconds(Tables.Cutter(Variant).Swing + Tables.Cutter(Variant).Recovery), Random(InRandom)
{
}

void FBotDefense::Seated(int LineId, double Now)
{
	Ended(LineId);
	const double Delay = Profile == EBotProfile::Probationer
	                         ? ProbationerCutSeconds
	                         : Random.Uniform(BackfillCutMinSeconds, BackfillCutMaxSeconds);
	const double SwingAt = Now + Delay;
	Pending.push_back({LineId, Now, std::max(Now, SwingAt - WarmupSeconds), SwingAt});
}

void FBotDefense::Ended(int LineId)
{
	std::erase_if(Pending, [LineId](const FBotCutPlan& P) { return P.LineId == LineId; });
	if (Pending.empty())
	{
		DrawnAt = -1.0;  // Sheathed: nothing left on the bot
	}
}

FBotDefense::FActions FBotDefense::Tick(double Now)
{
	FActions Out;
	if (Pending.empty())
	{
		return Out;
	}
	const auto Next = std::min_element(Pending.begin(), Pending.end(),
	                                   [](const FBotCutPlan& L, const FBotCutPlan& R) { return L.SwingAt < R.SwingAt; });
	if (DrawnAt < 0.0 && Now >= Next->DrawAt)
	{
		DrawnAt = Now;
		Out.bDraw = true;
	}
	// The swing needs the warm-up done, the previous swing recovered, and its own time come.
	constexpr double Slack = 1e-9;
	if (DrawnAt >= 0.0 && Now >= Next->SwingAt - Slack && Now >= DrawnAt + WarmupSeconds - Slack &&
	    Now >= NextSwingFree - Slack)
	{
		Out.Swing = Next->LineId;
		NextSwingFree = Now + CycleSeconds;
		Next->SwingAt = NextSwingFree;  // Again after the recovery, unless the line ends first
	}
	return Out;
}

bool BotMayUse(const FBotUse& Use)
{
	if (Use.bMapCable)
	{
		return false;  // Bots do not use map cables in Hangtown
	}
	if (Use.bTargetIsTeammate && Use.Mode == ELineMode::Haul)
	{
		return false;  // Never Haul a teammate
	}
	if (Use.Profile == EBotProfile::Probationer && (Use.Mode == ELineMode::Jack || Use.bRatedLoad))
	{
		return false;  // The tutorial crew never Jacks and never moves rated loads
	}
	return true;
}

FBotIntent ChooseRig(const FBotView& V, const FKineticTables& Tables, const FBotRigRules& Rules)
{
	FBotIntent None;
	if (V.bLineLive || V.bLineOnSelf)
	{
		return None;  // Working a line already, or cutting one off
	}
	const double Range = Tables.Harness(V.Class).LineLength;
	const auto InRange = [&](const FVec3& P) { return (P - V.Position).Length() <= Range; };
	std::vector<const FBotOther*> Enemies;
	for (const FBotOther& O : V.Others)
	{
		if (O.Team != V.Team && O.bStanding)
		{
			Enemies.push_back(&O);
		}
	}
	if (Enemies.empty())
	{
		return None;
	}

	// Caboose: an enemy near a Hauler's track as the train comes.
	for (const FBotHauler& H : V.Haulers)
	{
		for (const FBotOther* E : Enemies)
		{
			const FVec3 Beside = NearestOnSegment(E->Position, H.TrackA, H.TrackB);
			const double Arrives = H.ArrivesAt(E->Position);
			if (Arrives < 0.0 || Arrives > Rules.CabooseLeadSeconds)
			{
				continue;  // The train is past them, or too far off for Dog B to wait
			}
			if (InRange(E->Position) && InRange(Beside) && Horizontal(E->Position, Beside) <= Rules.CabooseTrenchM)
			{
				return {EBotRig::Caboose, E->Slot, false, E->Position, Beside, ELineMode::Haul, false};
			}
		}
	}

	// Haul to an anchor: the best pairing of an enemy in range with an anchor in range, over an
	// edge first, then the longest drag.
	std::optional<FBotIntent> Best;
	double BestScore = -1.0;
	for (const FBotOther* E : Enemies)
	{
		if (!InRange(E->Position))
		{
			continue;
		}
		for (const FBotAnchor& A : V.Anchors)
		{
			const double Drag = (A.Point - E->Position).Length();
			if (!InRange(A.Point) || Drag < Rules.HaulMinDragM ||
			    DistanceToSegment(V.Position, E->Position, A.Point) < Rules.HaulClearOfPathM)
			{
				continue;
			}
			const double Score = (A.bOverAnEdge ? 1000.0 : 0.0) + Drag;
			if (Score > BestScore)
			{
				BestScore = Score;
				Best = FBotIntent{EBotRig::HaulToAnchor, E->Slot, false, E->Position, A.Point, ELineMode::Haul, false};
			}
		}
	}
	if (Best && BestScore >= 1000.0)
	{
		return *Best;
	}

	// Clothesline: an enemy heading for a doorway the bot can string.
	for (const FBotDoorway& D : V.Doorways)
	{
		if (!InRange(D.Left) || !InRange(D.Right))
		{
			continue;
		}
		for (const FBotOther* E : Enemies)
		{
			const FVec3 ToDoor = D.Centre() - E->Position;
			if (Horizontal(D.Centre(), E->Position) <= Rules.ClotheslineNearM && E->Velocity.Dot(ToDoor) > 0.0)
			{
				return {EBotRig::Clothesline, E->Slot, false, D.Left, D.Right, ELineMode::Hang, true};
			}
		}
	}
	if (Best)
	{
		return *Best;
	}

	// Out of range of every enemy: Slingshot toward the nearest off a wall behind the bot.
	const FBotOther* Nearest = *std::min_element(Enemies.begin(), Enemies.end(), [&](const FBotOther* L, const FBotOther* R) {
		return (L->Position - V.Position).Length() < (R->Position - V.Position).Length();
	});
	if (InRange(Nearest->Position))
	{
		return None;  // Nothing to rig on them from here; the tree repositions on foot
	}
	const FVec3 Toward = (Nearest->Position - V.Position).SafeNormal();
	const double MinCos = std::cos(Rules.SlingshotMinAngleDeg * Pi / 180.0);
	const FBotAnchor* Wall = nullptr;
	double WallCos = 2.0;
	for (const FBotAnchor& A : V.Anchors)
	{
		const double C = (A.Point - V.Position).SafeNormal().Dot(Toward);
		if (InRange(A.Point) && C <= MinCos && C < WallCos)
		{
			WallCos = C;
			Wall = &A;
		}
	}
	if (Wall)
	{
		return {EBotRig::Slingshot, Nearest->Slot, true, V.Position, Wall->Point, ELineMode::Jack, false};
	}
	return None;
}

void FProbationerCrew::Seated(int Bot, double Now)
{
	FBot& B = Bots.at(static_cast<size_t>(Bot));
	if (B.Stage == EStage::InFlight)
	{
		B.Stage = EStage::Brace;
		B.Until = Now + BraceSeconds;
	}
}

void FProbationerCrew::Missed(int Bot, double Now)
{
	FBot& B = Bots.at(static_cast<size_t>(Bot));
	B.Stage = EStage::Recall;
	B.Until = Now + RecallSeconds;
}

void FProbationerCrew::Cut(int Bot, double Now)
{
	FBot& B = Bots.at(static_cast<size_t>(Bot));
	B.Stage = EStage::Recall;
	B.Until = Now + ConfusedSeconds + RecallSeconds;
	B.NoFireUntil = Now + RefireAfterCutSeconds;
	if (Bot == LeadBot)
	{
		LeadBot = (LeadBot + 1) % 3;  // P-02 steps forward instead
	}
}

void FProbationerCrew::Ended(int Bot, double Now)
{
	FBot& B = Bots.at(static_cast<size_t>(Bot));
	if (B.Stage != EStage::Idle)
	{
		B.Stage = EStage::Recall;
		B.Until = Now + RecallSeconds;
	}
}

void FProbationerCrew::PassLead(const std::array<bool, 3>& bAlive)
{
	for (int Step = 0; Step < 3 && !bAlive[static_cast<size_t>(LeadBot)]; ++Step)
	{
		LeadBot = (LeadBot + 1) % 3;
	}
}

std::array<FProbationerOrder, 3> FProbationerCrew::Tick(const FProbationerView& V)
{
	std::array<FProbationerOrder, 3> Orders{};
	const bool bCapped = V.PlayerScrapped >= ScrapCap;
	PassLead(V.bAlive);
	if (bCapped && AvalancheAt >= 0.0 && !bAvalancheDone)
	{
		AvalancheAt = -1.0;  // Called off: it could be a third scrap
	}
	for (size_t I = 0; I < 3; ++I)
	{
		FBot& B = Bots[I];
		FProbationerOrder& O = Orders[I];
		if (!V.bAlive[I])
		{
			B.Stage = EStage::Idle;
			if (I == 1)
			{
				AvalancheAt = -1.0;
			}
			O.Task = EProbationerTask::Down;
			continue;
		}
		if (B.Stage == EStage::Recall && V.Now >= B.Until)
		{
			B.Stage = EStage::Idle;
		}
		if (B.Stage == EStage::Brace && V.Now >= B.Until)
		{
			B.Stage = EStage::Haul;
		}
		if (B.Stage == EStage::WindUp && V.Now >= B.Until)
		{
			B.Stage = EStage::InFlight;
			O = {EProbationerTask::FireDog, bCapped ? MissM : 0.0};
			continue;
		}
		switch (B.Stage)
		{
		case EStage::WindUp: O.Task = EProbationerTask::WindUp; continue;
		case EStage::InFlight: O.Task = EProbationerTask::AwaitSeat; continue;
		case EStage::Brace: O.Task = EProbationerTask::Brace; continue;
		case EStage::Haul: O.Task = EProbationerTask::Haul; continue;
		case EStage::Recall: O.Task = EProbationerTask::Recall; continue;
		case EStage::Idle: break;
		}

		// P-02's one Avalanche: a 2 s wind-up once the player stands within 6 m of a stack.
		if (I == 1 && !bAvalancheDone)
		{
			if (AvalancheAt < 0.0 && !bCapped && V.PlayerToStackM <= AvalancheRangeM)
			{
				AvalancheAt = V.Now;
			}
			if (AvalancheAt >= 0.0)
			{
				if (V.Now >= AvalancheAt + AvalancheWindUpSeconds)
				{
					bAvalancheDone = true;
					B.Stage = EStage::Haul;  // On the stack's line until it ends
					O.Task = EProbationerTask::Avalanche;
				}
				else
				{
					O.Task = EProbationerTask::AvalancheWindUp;
				}
				continue;
			}
		}
		// P-03's one Caboose: the klaxon sounds and the player is within 10 m of the trench.
		if (I == 2 && !bCabooseDone && !bCapped && V.bKlaxon && V.PlayerToTrenchM <= CabooseRangeM)
		{
			bCabooseDone = true;
			B.Stage = EStage::Haul;  // On the player and the Hauler until it ends
			O.Task = EProbationerTask::Caboose;
			continue;
		}
		// The lead fires at 15 m after a 1 s wind-up.
		if (static_cast<int>(I) == LeadBot && V.Now >= B.NoFireUntil && V.DistanceToPlayer[I] <= FireRangeM)
		{
			B.Stage = EStage::WindUp;
			B.Until = V.Now + WindUpSeconds;
			O.Task = EProbationerTask::WindUp;
			continue;
		}
		O.Task = V.bOnMarker[I] ? EProbationerTask::HoldMarker : EProbationerTask::WalkRoute;
	}
	return Orders;
}
} // namespace Kinetic
