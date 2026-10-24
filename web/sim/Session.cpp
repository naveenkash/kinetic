#include "Session.h"

#include "Kinetic/Json.h"

#include <string>

namespace Kinetic::Web
{
namespace
{
const char* PhaseName(EScrapPhase Phase)
{
	switch (Phase)
	{
	case EScrapPhase::Reset: return "reset";
	case EScrapPhase::Drop: return "drop";
	case EScrapPhase::Live: return "live";
	case EScrapPhase::SuddenDeath: return "sudden_death";
	case EScrapPhase::RoundEnd: return "round_end";
	case EScrapPhase::MatchOver: return "match_over";
	}
	return "reset";
}

const char* CallText(EMarshalCall Call)
{
	switch (Call)
	{
	case EMarshalCall::CrewsOnTheClaim: return "Crews on the claim.";
	case EMarshalCall::Live: return "Live.";
	case EMarshalCall::EventInFive: return "Event in five.";
	case EMarshalCall::TimeCount: return "Ten seconds.";
	case EMarshalCall::SuddenDeath: return "Sudden death.";
	case EMarshalCall::CrewForfeited: return "Crew's forfeited. Logged.";
	case EMarshalCall::OfficeNotesStill: return "Office notes the rigger standing still.";
	case EMarshalCall::OfficeCrewIn: return "Office crew in.";
	}
	return "";
}

const char* ModeName(const FWorldLine& L)
{
	if (L.bSet)
	{
		return "set";
	}
	switch (L.State.Mode)
	{
	case ELineMode::Haul: return "haul";
	case ELineMode::Jack: return "jack";
	case ELineMode::Hang: return "hang";
	case ELineMode::Set: return "set";
	case ELineMode::Slack: break;
	}
	return "slack";
}

const char* CauseText(EStallCause Cause)
{
	switch (Cause)
	{
	case EStallCause::RingOut: return "ring-out";
	case EStallCause::ObjectImpact: return "object impact";
	case EStallCause::WorldImpact: return "hit the world";
	case EStallCause::KinematicImpact: return "kinematic";
	case EStallCause::PlayerImpact: return "player impact";
	case EStallCause::Crush: return "crush";
	case EStallCause::Blast: return "blast";
	}
	return "stall";
}

void Vec(FJsonWriter& W, const char* Name, const FVec3& V)
{
	W.Key(Name).BeginArray().Number(V.X).Number(V.Y).Number(V.Z).EndArray();
}
} // namespace

FSession::FSession(const FKineticTables& InTables, uint64_t InSeed, EHarnessClass PlayerClass, bool bInHumanPlays)
    : Tables(InTables), Seed(InSeed), bHumanPlays(bInHumanPlays), TheWorld(InTables, MakeYard())
{
	// The crews (PROPOSED): a Breaker and a Climber with the player; a Breaker, a Rigger and a
	// Scrapper against.
	const EHarnessClass Classes[6] = {PlayerClass,          EHarnessClass::Breaker, EHarnessClass::Climber,
	                                  EHarnessClass::Breaker, EHarnessClass::Rigger, EHarnessClass::Scrapper};
	for (int Slot = 0; Slot < 6; ++Slot)
	{
		FMatchPlayer P;
		P.Slot = Slot;
		P.Team = Slot < 3 ? 0 : 1;
		P.bIsBot = Slot != 0 || !bHumanPlays;
		P.AccountId = P.bIsBot ? "" : "browser-player";
		P.Harness = Classes[Slot];
		Roster.push_back(P);
		TheWorld.AddPlayer(Slot, P.Team, P.Harness);
		Brains.push_back(P.bIsBot ? std::make_unique<FBotBrain>(Slot, Seed, Tables, P.Harness) : nullptr);
	}
	StartMatch();
}

void FSession::StartMatch()
{
	Log = std::make_unique<FEventLog>();
	TheMatch = std::make_unique<FScrapMatch>(Roster, FScrapRules{}, *Log);
	TheMatch->Start();
	MatchOverAt = -1.0;
}

void FSession::Say(std::string Text)
{
	Feed.push_back(std::move(Text));
	while (Feed.size() > 6)
	{
		Feed.pop_front();
	}
}

void FSession::Step(const FPlayerInput& Human)
{
	const double Dt = FWorld::Dt;
	TheMatch->Update(Dt);
	for (const FFlowEvent& E : TheMatch->TakeEvents())
	{
		switch (E.Kind)
		{
		case EFlow::ResetArena:
		{
			const int Sides[2] = {TheMatch->SpawnOf(0), TheMatch->SpawnOf(1)};
			TheWorld.Reset(Sides);
			for (auto& Brain : Brains)
			{
				if (Brain)
				{
					Brain->Reset();
				}
			}
			Pending.clear();
			break;
		}
		case EFlow::Call:
			Say(CallText(E.Call));
			break;
		case EFlow::RoundOver:
			Say(E.Team == NoPlayer ? "Round replayed." : E.Team == 0 ? "Your crew takes the round." : "Their crew takes the round.");
			break;
		case EFlow::MatchOver:
			Say(E.Team == 0 ? "Match: your crew." : E.Team == 1 ? "Match: their crew." : "No contest.");
			MatchOverAt = TheMatch->Time();
			++Finished;
			break;
		default:
			break;
		}
	}
	if (TheMatch->Phase() == EScrapPhase::MatchOver && MatchOverAt >= 0.0 && TheMatch->Time() - MatchOverAt >= RestartSeconds)
	{
		StartMatch();
		return;
	}

	std::vector<FPlayerInput> Inputs(TheWorld.Players().size());
	for (size_t I = 0; I < Inputs.size(); ++I)
	{
		FWorldPlayer& P = TheWorld.Player(static_cast<int>(I));
		P.bCanMove = TheMatch->CanMove(P.Slot);
		P.bCanUseLines = TheMatch->CanUseLines();
		Inputs[I] = Brains[I] ? Brains[I]->Think(TheWorld, Pending) : Human;
	}
	if (bHumanPlays)
	{
		// Any input counts against the idle rules (game-modes 8): moving, looking, any button.
		const bool bLooked = Human.Yaw != LastYaw || Human.Pitch != LastPitch;
		const bool bAny = Human.MoveForward != 0.0 || Human.MoveRight != 0.0 || Human.bFire || Human.bDraw || Human.bJump ||
		                  Human.bBrace || Human.bSelfDog || Human.bRelease || Human.bSwing || Human.bSprint || Human.Mode != 0 ||
		                  bLooked;
		LastYaw = Human.Yaw;
		LastPitch = Human.Pitch;
		if (bAny)
		{
			TheMatch->OnInput(0);
		}
	}
	TheWorld.Step(Inputs);
	Pending = TheWorld.TakeEvents();
	for (const FWorldEvent& E : Pending)
	{
		if (E.Kind == FWorldEvent::EKind::Stall)
		{
			const FWorldPlayer& P = TheWorld.Players()[static_cast<size_t>(E.Player)];
			if (TheMatch->OnStall(P.Slot))
			{
				++Stalls;
				const std::string Who = P.Slot == 0 && bHumanPlays ? "You" : std::string(ToString(P.Class)) + " " +
				                                                               (P.Team == 0 ? "(your crew)" : "(their crew)");
				Say(Who + " scrapped: " + CauseText(E.Cause) + ".");
			}
		}
	}
}

std::string FSession::ArenaJson() const
{
	FJsonWriter W;
	const FArena& A = TheWorld.Arena();
	W.BeginObject();
	W.Key("name").String(A.Name);
	W.Key("killZ").Number(A.KillZ);
	W.Key("solids").BeginArray();
	for (const FSolid& S : A.Solids)
	{
		W.BeginObject();
		Vec(W, "min", S.Min);
		Vec(W, "max", S.Max);
		W.Key("tag").String(S.Tag);
		W.EndObject();
	}
	W.EndArray();
	W.Key("crates").BeginArray();
	for (const FCrate& C : TheWorld.Crates())
	{
		W.BeginObject();
		Vec(W, "half", C.Half);
		W.Key("mass").Number(C.MassKg);
		W.EndObject();
	}
	W.EndArray();
	W.EndObject();
	return W.Text();
}

std::string FSession::SnapshotJson() const
{
	FJsonWriter W;
	W.BeginObject();
	W.Key("t").Number(TheWorld.Time());
	W.Key("phase").String(PhaseName(TheMatch->Phase()));
	W.Key("clock").Number(TheMatch->ClockRemaining(), 1);
	W.Key("round").Integer(TheMatch->Round());
	W.Key("wins").BeginArray().Integer(TheMatch->Wins(0)).Integer(TheMatch->Wins(1)).EndArray();
	W.Key("you").Integer(bHumanPlays ? 0 : -1);
	W.Key("resets").Integer(TheWorld.ResetCount());
	W.Key("players").BeginArray();
	for (size_t I = 0; I < TheWorld.Players().size(); ++I)
	{
		const FWorldPlayer& P = TheWorld.Players()[I];
		const double Warm = P.bDrawn ? std::min(1.0, (TheWorld.Time() - P.DrawnAt) / TheWorld.CutterWarmup(static_cast<int>(I))) : 0.0;
		W.BeginObject();
		W.Key("slot").Integer(P.Slot);
		W.Key("team").Integer(P.Team);
		W.Key("class").String(ToString(P.Class));
		Vec(W, "pos", P.Position);
		W.Key("yaw").Number(P.Yaw);
		W.Key("pitch").Number(P.Pitch);
		W.Key("standing").Bool(P.bStanding);
		W.Key("grounded").Bool(P.bGrounded);
		W.Key("braced").Bool(P.bBraced);
		W.Key("airborne").Bool(P.Footing == EFooting::Airborne || P.Footing == EFooting::Hoisted);
		W.Key("stunned").Bool(TheWorld.Time() < P.StunnedUntil);
		W.Key("drawn").Bool(P.bDrawn);
		W.Key("warm").Number(Warm, 2);
		W.Key("mode").String(P.Preselect == ELineMode::Haul ? "haul" : P.Preselect == ELineMode::Jack ? "jack" : "hang");
		W.EndObject();
	}
	W.EndArray();
	W.Key("crates").BeginArray();
	for (const FCrate& C : TheWorld.Crates())
	{
		W.BeginArray().Number(C.Position.X).Number(C.Position.Y).Number(C.Position.Z).EndArray();
	}
	W.EndArray();
	W.Key("lines").BeginArray();
	for (const FWorldLine& L : TheWorld.Lines())
	{
		W.BeginObject();
		W.Key("id").Integer(L.Id);
		W.Key("owner").Integer(L.Owner);
		W.Key("team").Integer(TheWorld.Players()[static_cast<size_t>(L.Owner)].Team);
		const bool bA = L.FlyingA || L.A.Seat != EDogSeat::None;
		const bool bB = L.FlyingB || L.B.Seat != EDogSeat::None;
		const FVec3 Hand = TheWorld.EyeOf(L.Owner) - FVec3{0, 0, 0.5};
		Vec(W, "a", bA ? TheWorld.DogOrEndPosition(L, true) : Hand);
		Vec(W, "b", bB ? TheWorld.DogOrEndPosition(L, false) : Hand);
		W.Key("live").Bool(L.bLive());
		W.Key("mode").String(ModeName(L));
		W.Key("force").Number(L.State.AppliedForce, 0);
		W.EndObject();
	}
	W.EndArray();
	// The edge indicator: where each line on the player's body leads.
	W.Key("onYou").BeginArray();
	if (bHumanPlays)
	{
		for (const int Id : TheWorld.LinesOnPlayer(0))
		{
			for (const FWorldLine& L : TheWorld.Lines())
			{
				if (L.Id == Id)
				{
					const bool bMineA = L.A.Seat == EDogSeat::Plate && L.A.Player == 0;
					W.BeginObject();
					Vec(W, "toward", TheWorld.DogOrEndPosition(L, !bMineA));
					W.Key("plate").String(ToString(bMineA ? L.A.Plate : L.B.Plate));
					W.EndObject();
				}
			}
		}
	}
	W.EndArray();
	W.Key("feed").BeginArray();
	for (const std::string& Line : Feed)
	{
		W.String(Line);
	}
	W.EndArray();
	W.EndObject();
	return W.Text();
}
} // namespace Kinetic::Web
