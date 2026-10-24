#include "Kinetic/MatchEvents.h"

#include "Kinetic/Json.h"
#include "Kinetic/Sha256.h"

namespace Kinetic
{
const char* ToString(ESeatTarget Value)
{
	switch (Value)
	{
	case ESeatTarget::Static: return "static";
	case ESeatTarget::Kinematic: return "kinematic";
	case ESeatTarget::Object: return "object";
	case ESeatTarget::Player: return "player";
	}
	return "unknown";
}

const char* ToString(EPlate Value)
{
	switch (Value)
	{
	case EPlate::None: return "none";
	case EPlate::Chest: return "chest";
	case EPlate::Back: return "back";
	case EPlate::LeftShoulder: return "left_shoulder";
	case EPlate::RightShoulder: return "right_shoulder";
	}
	return "unknown";
}

const char* ToString(ELineOutcome Value)
{
	switch (Value)
	{
	case ELineOutcome::Released: return "released";
	case ELineOutcome::Cut: return "cut";
	case ELineOutcome::Snapped: return "snapped";
	case ELineOutcome::Completed: return "completed";
	case ELineOutcome::RanOut: return "ran_out";
	case ELineOutcome::BodyStalled: return "body_stalled";
	}
	return "unknown";
}

const char* ToString(EStallCause Value)
{
	switch (Value)
	{
	case EStallCause::ObjectImpact: return "object_impact";
	case EStallCause::WorldImpact: return "world_impact";
	case EStallCause::KinematicImpact: return "kinematic_impact";
	case EStallCause::PlayerImpact: return "player_impact";
	case EStallCause::Crush: return "crush";
	case EStallCause::Blast: return "blast";
	case EStallCause::RingOut: return "ring_out";
	}
	return "unknown";
}

const char* ToString(ERig Value)
{
	switch (Value)
	{
	case ERig::None: return "none";
	case ERig::Gallows: return "Gallows";
	case ERig::Guillotine: return "Guillotine";
	case ERig::Clothesline: return "Clothesline";
	case ERig::Slingshot: return "Slingshot";
	case ERig::Pendulum: return "Pendulum";
	case ERig::Kiss: return "Kiss";
	case ERig::Scatter: return "Scatter";
	case ERig::Caboose: return "Caboose";
	case ERig::Avalanche: return "Avalanche";
	case ERig::Overboard: return "Overboard";
	case ERig::AnchorDrop: return "Anchor Drop";
	}
	return "unknown";
}

const char* ToString(ERoundEnd Value)
{
	switch (Value)
	{
	case ERoundEnd::CrewDown: return "crew_down";
	case ERoundEnd::TimeOut: return "time_out";
	case ERoundEnd::SuddenDeath: return "sudden_death";
	case ERoundEnd::Replayed: return "replayed";
	case ERoundEnd::Forfeit: return "forfeit";
	}
	return "unknown";
}

const char* ToString(EConnection Value)
{
	switch (Value)
	{
	case EConnection::Disconnected: return "disconnected";
	case EConnection::Returned: return "returned";
	case EConnection::IdleWarning: return "idle_warning";
	case EConnection::Withdrawn: return "withdrawn";
	case EConnection::Backfilled: return "backfilled";
	case EConnection::Abandoned: return "abandoned";
	}
	return "unknown";
}

const char* ToString(ELineMode Value)
{
	switch (Value)
	{
	case ELineMode::Slack: return "slack";
	case ELineMode::Haul: return "haul";
	case ELineMode::Jack: return "jack";
	case ELineMode::Hang: return "hang";
	case ELineMode::Set: return "set";
	}
	return "unknown";
}

namespace
{
const char* ToString(EOutcome Value)
{
	switch (Value)
	{
	case EOutcome::None: return "none";
	case EOutcome::Stagger: return "stagger";
	case EOutcome::Flip: return "flip";
	case EOutcome::Stall: return "stall";
	}
	return "unknown";
}

void Head(FJsonWriter& J, const char* Type, double Time, int Round)
{
	J.Key("type").String(Type).Key("v").Integer(EventSchemaVersion).Key("t").Number(Time).Key("round").Integer(Round);
}

void Write(FJsonWriter& J, const FDogSeatEvent& E)
{
	Head(J, "dog_seat", E.Time, E.Round);
	J.Key("shooter").Integer(E.Shooter).Key("line").Integer(E.LineId).Key("target").String(ToString(E.Target));
	J.Key("target_player").Integer(E.TargetPlayer).Key("plate").String(ToString(E.Plate));
	J.Key("shooter_class");
	E.ShooterClass ? J.String(ToString(*E.ShooterClass)) : J.Null();
	J.Key("target_class");
	E.TargetClass ? J.String(ToString(*E.TargetClass)) : J.Null();
}

void Write(FJsonWriter& J, const FModeEvent& E)
{
	Head(J, "mode", E.Time, E.Round);
	J.Key("owner").Integer(E.Owner).Key("line").Integer(E.LineId).Key("mode").String(ToString(E.Mode));
	J.Key("tension_n").Number(E.TensionN).Key("seconds").Number(E.Seconds);
	J.Key("mass_a_kg").Number(E.MassAKg).Key("mass_b_kg").Number(E.MassBKg).Key("outcome").String(ToString(E.Outcome));
}

void Write(FJsonWriter& J, const FCutEvent& E)
{
	Head(J, "cut", E.Time, E.Round);
	J.Key("cutter").Integer(E.Cutter).Key("line_owner").Integer(E.LineOwner).Key("line").Integer(E.LineId);
	J.Key("seat_to_cut_s").Number(E.SeatToCutSeconds).Key("plate").String(ToString(E.Plate));
	J.Key("turn_deg").Number(E.TurnDegrees, 1);
}

void Write(FJsonWriter& J, const FStallEvent& E)
{
	Head(J, "stall", E.Time, E.Round);
	J.Key("victim").Integer(E.Victim).Key("cause").String(ToString(E.Cause)).Key("object").Integer(E.ObjectId);
	J.Key("object_mass_kg").Number(E.ObjectMassKg).Key("value").Number(E.Value);
	J.Key("killer").Integer(E.Killer).Key("spotter").Integer(E.Spotter).Key("assists").BeginArray();
	for (const int Assist : E.Assists)
	{
		J.Integer(Assist);
	}
	J.EndArray().Key("rig").String(ToString(E.Rig)).Key("ringout_height_m").Number(E.RingOutHeightM);
}

void Write(FJsonWriter& J, const FHitEvent& E)
{
	Head(J, "hit", E.Time, E.Round);
	J.Key("player").Integer(E.Player).Key("outcome").String(ToString(E.Outcome)).Key("cause").String(ToString(E.Cause));
	J.Key("value").Number(E.Value);
}

void Write(FJsonWriter& J, const FSnapEvent& E)
{
	Head(J, "snap", E.Time, E.Round);
	J.Key("owner").Integer(E.Owner).Key("line").Integer(E.LineId).Key("force_n").Number(E.ForceN);
}

void Write(FJsonWriter& J, const FRatedCommitEvent& E)
{
	Head(J, "rated_commit", E.Time, E.Round);
	J.Key("owner").Integer(E.Owner).Key("mass_kg").Number(E.MassKg);
}

void Write(FJsonWriter& J, const FPlayerRoundEvent& E)
{
	J.Key("type").String("player_round").Key("v").Integer(EventSchemaVersion).Key("round").Integer(E.Round);
	J.Key("player").Integer(E.Player).Key("floor_s").Number(E.FloorSeconds).Key("airborne_s").Number(E.AirborneSeconds);
	J.Key("hoisted_s").Number(E.HoistedSeconds).Key("mass_distance_kg_m").Number(E.MassDistanceKgM);
	J.Key("bot").Bool(E.bBotControlled);
}

void Write(FJsonWriter& J, const FRoundEndEvent& E)
{
	Head(J, "round_end", E.Time, E.Round);
	J.Key("winner_team").Integer(E.WinnerTeam).Key("seconds").Number(E.Seconds).Key("stalls").Integer(E.Stalls);
	J.Key("reason").String(ToString(E.Reason));
}

void Write(FJsonWriter& J, const FMatchEndEvent& E)
{
	J.Key("type").String("match_end").Key("v").Integer(EventSchemaVersion).Key("t").Number(E.Time);
	J.Key("winner_team").Integer(E.WinnerTeam).Key("forfeit_team").Integer(E.ForfeitTeam);
	J.Key("forfeit_penalty_free").Bool(E.bForfeitPenaltyFree).Key("no_contest").Bool(E.bNoContest);
}

void Write(FJsonWriter& J, const FConnectionEvent& E)
{
	J.Key("type").String("connection").Key("v").Integer(EventSchemaVersion).Key("t").Number(E.Time);
	J.Key("player").Integer(E.Player).Key("change").String(ToString(E.Change));
}

void Write(FJsonWriter& J, const FServerSecondEvent& E)
{
	J.Key("type").String("server_second").Key("v").Integer(EventSchemaVersion).Key("t").Number(E.Time);
	J.Key("frame_ms").Number(E.FrameMs).Key("physics_ms").Number(E.PhysicsMs);
	J.Key("dynamic_bodies").Integer(E.DynamicBodies).Key("active_lines").Integer(E.ActiveLines);
}
} // namespace

std::string ToJson(const FMatchEvent& Event)
{
	FJsonWriter J;
	J.BeginObject();
	std::visit([&J](const auto& E) { Write(J, E); }, Event);
	J.EndObject();
	return J.Text();
}

std::string FEventLog::JsonLines() const
{
	std::string Out;
	for (const FMatchEvent& Event : Events)
	{
		Out += ToJson(Event);
		Out += '\n';
	}
	return Out;
}

std::string FEventLog::Sha256() const { return FSha256::HexOf(JsonLines()); }
} // namespace Kinetic
