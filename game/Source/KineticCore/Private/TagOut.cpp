#include "Kinetic/TagOut.h"

#include <algorithm>
#include <cmath>

namespace Kinetic
{
namespace
{
constexpr double Pi = 3.14159265358979323846;
constexpr double TimeEpsilon = 1e-6;  // Fixed steps of 1/60 s sum a hair under whole seconds

uint64_t SplitMix(uint64_t& State)
{
	uint64_t Z = (State += 0x9E3779B97F4A7C15ull);
	Z = (Z ^ (Z >> 30)) * 0xBF58476D1CE4E5B9ull;
	Z = (Z ^ (Z >> 27)) * 0x94D049BB133111EBull;
	return Z ^ (Z >> 31);
}

bool Reached(double Held, double Seconds) { return Held >= Seconds - TimeEpsilon; }
} // namespace

FVec3 PlateFacing(const FTagPose& Pose, ETagPlate Plate)
{
	switch (Plate)
	{
	case ETagPlate::LeftShoulder:
		return -Pose.Right;
	case ETagPlate::RightShoulder:
		return Pose.Right;
	case ETagPlate::Back:
	default:
		return -Pose.Forward;
	}
}

uint8_t ReadablePlates(const FVec3& ViewerEye, bool bLevelRaised, const FTagPose& Target,
                       const std::array<bool, PlateCount>& LineOfSight, const FTagOutRules& Rules, bool bCarrier)
{
	const FVec3 ToViewer = ViewerEye - Target.Chest;
	const double Distance = ToViewer.Length();
	const FVec3 Direction = ToViewer.SafeNormal();
	const double MinCos = std::cos(Rules.PlateConeDegrees * Pi / 180.0);
	uint8_t Plates = 0;
	for (int I = 0; I < PlateCount; ++I)
	{
		const ETagPlate Plate = static_cast<ETagPlate>(I);
		double Range = bLevelRaised ? Rules.LevelRangeM : Rules.NakedRangeM;
		if (bCarrier && Plate == ETagPlate::Back)
		{
			Range = std::max(Range, Rules.CarrierBackRangeM);
		}
		if (LineOfSight[I] && Distance <= Range && PlateFacing(Target, Plate).Dot(Direction) >= MinCos)
		{
			Plates |= static_cast<uint8_t>(1u << I);
		}
	}
	return Plates;
}

FVec3 TiltedPlateNormal(const FTagPose& Pose, ETagPlate Plate, const FTagOutRules& Rules)
{
	const double Tilt = Rules.PlateTiltDegrees * Pi / 180.0;
	return (PlateFacing(Pose, Plate) * std::cos(Tilt) + Up * std::sin(Tilt)).SafeNormal();
}

bool SegmentNearPoint(const FVec3& A, const FVec3& B, const FVec3& Centre, double Radius)
{
	const FVec3 AB = B - A;
	const double Length2 = AB.Dot(AB);
	const double T = Length2 > 1e-12 ? std::clamp((Centre - A).Dot(AB) / Length2, 0.0, 1.0) : 0.0;
	return (A + AB * T - Centre).Length() <= Radius;
}

int FTagRead::Known() const
{
	if (bStale)
	{
		return 0;
	}
	return static_cast<int>(std::count_if(Pips.begin(), Pips.end(), [](const std::optional<uint8_t>& Pip) { return Pip.has_value(); }));
}

FTagCode FTagOutMatch::RollCode(uint64_t Seed, int Slot, int Stamp)
{
	uint64_t State = Seed ^ (static_cast<uint64_t>(Slot) * 0xD1B54A32D192ED03ull) ^ (static_cast<uint64_t>(Stamp) * 0x8CB92BA72F3D8DD7ull);
	SplitMix(State);
	FTagCode Code;
	for (uint8_t& Pip : Code.Pips)
	{
		Pip = static_cast<uint8_t>(1 + (SplitMix(State) >> 62));  // The top two bits: 1 to 4
	}
	return Code;
}

FTagOutMatch::FTagOutMatch(const std::vector<FMatchPlayer>& Roster, const std::array<FVec3, 2>& Pads, const std::vector<FVec3>& Lockers,
                           uint64_t Seed, FTagOutRules InRules)
    : Config(InRules), HomePads(Pads), LockerAt(Lockers), MatchSeed(Seed)
{
	// Slots are 0 to N-1, in roster order.
	for (const FMatchPlayer& Player : Roster)
	{
		FTagRigger Rigger;
		Rigger.Slot = static_cast<int>(Seats.size());
		Rigger.Team = Player.Team;
		Rigger.Harness = Player.Harness;
		Seats.push_back(Rigger);
	}
	Stamps.assign(Seats.size(), 0);
	for (int Team = 0; Team < 2; ++Team)
	{
		Board[Team].assign(Seats.size(), FTagRead{});
		Beacons[Team].Team = Team;
		Beacons[Team].Stand = Pads[Team];
		Beacons[Team].At = Pads[Team];
	}
	for (FTagRigger& Rigger : Seats)
	{
		DropIn(Rigger.Slot);
	}
	Events.clear();  // The opening drop is the match's start, not a respawn
}

void FTagOutMatch::Emit(ETagEvent Kind, int Slot, int Other, int Team, ETagPlate Plate)
{
	Events.push_back({Kind, Clock, Slot, Other, Team, Plate});
}

std::vector<FTagEventRecord> FTagOutMatch::TakeEvents()
{
	std::vector<FTagEventRecord> Out;
	Out.swap(Events);
	return Out;
}

double FTagOutMatch::Remaining() const
{
	const double Live = Clock - Config.DropSeconds;
	switch (CurrentPhase)
	{
	case ETagPhase::Drop:
		return Config.RegulationSeconds;
	case ETagPhase::Live:
		return std::max(0.0, Config.RegulationSeconds - Live);
	case ETagPhase::Overtime:
		return std::max(0.0, Config.RegulationSeconds + Config.OvertimeSeconds - Live);
	case ETagPhase::Over:
	default:
		return 0.0;
	}
}

int FTagOutMatch::CalledBy(int Slot) const
{
	for (const FTagRigger& Rigger : Seats)
	{
		if (Rigger.Call == ECallState::Locking && Rigger.CallTarget == Slot)
		{
			return Rigger.Slot;
		}
	}
	return NoPlayer;
}

bool FTagOutMatch::LevelReady(int Slot) const
{
	const FTagRigger& Rigger = Seats[Slot];
	return Rigger.bLevelRaised && Rigger.LevelUpAt >= 0.0 && Clock >= Rigger.LevelUpAt - TimeEpsilon;
}

int FTagOutMatch::Carrying(int Slot) const
{
	for (const FTagBeacon& Beacon : Beacons)
	{
		if (Beacon.State == EBeaconState::Carried && Beacon.Carrier == Slot)
		{
			return Beacon.Team;
		}
	}
	return NoPlayer;
}

void FTagOutMatch::SetPose(int Slot, const FTagPose& Pose, double SpeedMs, bool bLevelRaised)
{
	FTagRigger& Rigger = Seats[Slot];
	Rigger.Pose = Pose;
	Rigger.SpeedMs = SpeedMs;
	if (bLevelRaised && !Rigger.bLevelRaised)
	{
		Rigger.LevelUpAt = Clock + Config.LevelRaiseSeconds;
	}
	else if (!bLevelRaised)
	{
		Rigger.LevelUpAt = -1.0;
	}
	Rigger.bLevelRaised = bLevelRaised;
}

void FTagOutMatch::Reveal(int Team, int Target, ETagPlate Plate)
{
	FTagRead& Read = Board[Team][Target];
	if (Read.bStale)
	{
		Read = FTagRead{};  // The first look at the new code replaces the old one
	}
	const int I = static_cast<int>(Plate);
	const uint8_t Pip = Seats[Target].Code.Pips[I];
	const bool bNew = Read.Pips[I] != Pip;
	Read.Pips[I] = Pip;
	Read.Seen[I] = Clock;
	if (bNew)
	{
		Emit(ETagEvent::Revealed, Target, NoPlayer, Team, Plate);
	}
}

bool FTagOutMatch::ThroughDust(const FVec3& From, const FVec3& To) const
{
	for (const FTagCloud& Cloud : Dust)
	{
		if (Cloud.Until > Clock && SegmentNearPoint(From, To, Cloud.At, Config.DustRadiusM))
		{
			return true;
		}
	}
	return false;
}

bool FTagOutMatch::Glares(int Viewer, int Target, ETagPlate Plate) const
{
	const FVec3 Normal = TiltedPlateNormal(Seats[Target].Pose, Plate, Config);
	if (Sun.Dot(Normal) <= 0.0)
	{
		return false;  // The sun is behind the plate
	}
	const FVec3 Light = -Sun;
	const FVec3 Reflected = Light - Normal * (2.0 * Light.Dot(Normal));
	const FVec3 ToViewer = (Seats[Viewer].Pose.Eye - Seats[Target].Pose.Chest).SafeNormal();
	return Reflected.Dot(ToViewer) >= std::cos(Config.GlareDegrees * Pi / 180.0);
}

bool FTagOutMatch::SunBehind(int Caller, int Target) const
{
	const FVec3 Look = (Seats[Target].Pose.Chest - Seats[Caller].Pose.Eye).SafeNormal();
	return Look.Dot(Sun) >= std::cos(Config.SunLockDegrees * Pi / 180.0);
}

uint8_t FTagOutMatch::Legible(int Viewer, int Target, uint8_t Plates) const
{
	const FVec3 Eye = Seats[Viewer].Pose.Eye;
	const FVec3 Chest = Seats[Target].Pose.Chest;
	if (ThroughDust(Eye, Chest))
	{
		return 0;
	}
	if ((Chest - Eye).Length() > Config.HazeRangeM)
	{
		for (const FTagHaze& Zone : Haze)
		{
			if (SegmentNearPoint(Eye, Chest, Zone.At, Zone.RadiusM))
			{
				return 0;
			}
		}
	}
	for (int I = 0; I < PlateCount; ++I)
	{
		const ETagPlate Plate = static_cast<ETagPlate>(I);
		if ((Plates & (1u << I)) && (Smeared(Target, Plate) || Glares(Viewer, Target, Plate)))
		{
			Plates = static_cast<uint8_t>(Plates & ~(1u << I));
		}
	}
	return Plates;
}

void FTagOutMatch::Show(int Viewer, int Target, uint8_t Plates)
{
	if (CurrentPhase == ETagPhase::Over || !Seats[Viewer].bStanding || !Seats[Target].bStanding ||
	    Seats[Viewer].Team == Seats[Target].Team)
	{
		return;
	}
	Plates = Legible(Viewer, Target, Plates);
	for (int I = 0; I < PlateCount; ++I)
	{
		if (Plates & (1u << I))
		{
			Reveal(Seats[Viewer].Team, Target, static_cast<ETagPlate>(I));
		}
	}
}

void FTagOutMatch::ClearBoardFor(int Slot)
{
	for (std::vector<FTagRead>& Crew : Board)
	{
		Crew[Slot].bStale = true;
	}
}

void FTagOutMatch::Recode(int Slot)
{
	// A new stamp never repeats the code it replaces: a re-stamp always buys a fresh code.
	const FTagCode Old = Seats[Slot].Code;
	do
	{
		Seats[Slot].Code = RollCode(MatchSeed, Slot, ++Stamps[Slot]);
	} while (Seats[Slot].Code == Old && Stamps[Slot] > 1);
	ClearBoardFor(Slot);
}

void FTagOutMatch::DropIn(int Slot)
{
	FTagRigger& Rigger = Seats[Slot];
	Recode(Slot);
	Rigger.bStanding = true;
	Rigger.Pose.Chest = HomePads[Rigger.Team];
	Rigger.Pose.Eye = HomePads[Rigger.Team];
	Rigger.ExposedUntil = 0.0;
	Rigger.GadgetsLeft = Config.GadgetsPerDrop;
	Rigger.SmearedUntil.fill(0.0);
	Rigger.SmearingUntil = -1.0;
	Emit(ETagEvent::Respawned, Slot, NoPlayer, Rigger.Team);
}

double FTagOutMatch::ReserveSecondsNow() const
{
	if (CurrentPhase == ETagPhase::Overtime)
	{
		return Config.ReserveSeconds[3];
	}
	const double Live = Clock - Config.DropSeconds;
	if (Live < Config.ReserveSteps[0])
	{
		return Config.ReserveSeconds[0];
	}
	return Live < Config.ReserveSteps[1] ? Config.ReserveSeconds[1] : Config.ReserveSeconds[2];
}

void FTagOutMatch::Down(int Slot)
{
	FTagRigger& Rigger = Seats[Slot];
	if (!Rigger.bStanding)
	{
		return;
	}
	Rigger.bStanding = false;
	Rigger.ReserveUntil = Clock + ReserveSecondsNow();
	Rigger.Call = ECallState::Idle;
	Rigger.CallTarget = NoPlayer;
	Rigger.Entered.clear();
	Rigger.UseHeld = 0.0;
	Rigger.UseOn = -1;
	Rigger.SmearingUntil = -1.0;
	Rigger.bLevelRaised = false;
	Rigger.LevelUpAt = -1.0;
	Emit(ETagEvent::Stalled, Slot, NoPlayer, Rigger.Team);
	const int Carried = Carrying(Slot);
	if (Carried != NoPlayer)
	{
		FTagBeacon& Beacon = Beacons[Carried];
		Beacon.State = EBeaconState::Dropped;
		Beacon.Carrier = NoPlayer;
		Beacon.At = Rigger.Pose.Chest;
		Beacon.DroppedAt = Clock;
		Emit(ETagEvent::BeaconDropped, Slot, NoPlayer, Carried);
	}
	ClearBoardFor(Slot);  // The code re-rolls when they drop back in: the board greys out now
}

void FTagOutMatch::Stall(int Slot)
{
	if (CurrentPhase != ETagPhase::Over)
	{
		Down(Slot);
	}
}

void FTagOutMatch::EnterPip(int Caller, int Pip)
{
	FTagRigger& Rigger = Seats[Caller];
	if (Pip < 1 || Pip > 4 || !Rigger.bStanding || Rigger.Call == ECallState::Locking)
	{
		return;
	}
	if (Rigger.Entered.size() >= PlateCount)
	{
		Rigger.Entered.clear();
	}
	Rigger.Entered.push_back(static_cast<uint8_t>(Pip));
}

void FTagOutMatch::ClearCall(int Caller)
{
	if (Seats[Caller].Call == ECallState::Idle)
	{
		Seats[Caller].Entered.clear();
	}
}

bool FTagOutMatch::SendCall(int Caller, int Target, bool bInSight)
{
	if (!CanCall() || Target < 0 || Target >= static_cast<int>(Seats.size()))
	{
		return false;
	}
	FTagRigger& Rigger = Seats[Caller];
	const FTagRigger& Called = Seats[Target];
	if (!Rigger.bStanding || !Called.bStanding || Called.Team == Rigger.Team || Rigger.Call != ECallState::Idle ||
	    Clock < Rigger.CallCooldownUntil - TimeEpsilon || Rigger.Entered.size() != PlateCount || !bInSight ||
	    (Called.Pose.Chest - Rigger.Pose.Chest).Length() > Config.CallRangeM || Rigger.SmearingUntil >= 0.0 ||
	    ThroughDust(Rigger.Pose.Eye, Called.Pose.Chest) || SunBehind(Caller, Target))
	{
		return false;
	}
	Rigger.Call = ECallState::Locking;
	Rigger.CallTarget = Target;
	Rigger.LockElapsed = 0.0;
	Emit(ETagEvent::CallSent, Caller, Target, Rigger.Team);
	return true;
}

void FTagOutMatch::ResolveCall(int Caller)
{
	FTagRigger& Rigger = Seats[Caller];
	const int Target = Rigger.CallTarget;
	const FTagRigger& Called = Seats[Target];
	FTagCode Entered;
	std::copy(Rigger.Entered.begin(), Rigger.Entered.end(), Entered.Pips.begin());
	Rigger.Call = ECallState::Idle;
	Rigger.CallTarget = NoPlayer;
	Rigger.Entered.clear();
	if (Entered == Called.Code)
	{
		Rigger.CallCooldownUntil = Clock + Config.CleanCooldown;
		Emit(ETagEvent::TaggedOut, Caller, Target, Rigger.Team);
		Down(Target);
		return;
	}
	// A wrong call leaks one of the caller's own pips to the crew it called, until the caller
	// re-stamps, and shows where the caller is.
	const int Them = Called.Team;
	const FTagRead& Theirs = Board[Them][Caller];
	int Leak = 0;
	for (int I = 0; I < PlateCount; ++I)
	{
		if (Theirs.bStale || !Theirs.Pips[I].has_value())
		{
			Leak = I;
			break;
		}
	}
	Rigger.CallCooldownUntil = Clock + Config.BadCooldown;
	Rigger.ExposedUntil = Clock + Config.BadCallRevealSeconds;
	Emit(ETagEvent::BadCall, Caller, Target, Rigger.Team, static_cast<ETagPlate>(Leak));
	Reveal(Them, Caller, static_cast<ETagPlate>(Leak));
}

bool FTagOutMatch::Touching(int Slot, const FVec3& At) const
{
	return (Seats[Slot].Pose.Chest - At).Length() <= Config.UseRangeM;
}

bool FTagOutMatch::OnPad(int Slot, int PadTeam) const
{
	const FVec3 Off = Seats[Slot].Pose.Chest - HomePads[PadTeam];
	return std::hypot(Off.X, Off.Y) <= Config.PadRadiusM && std::abs(Off.Z) <= 2.5;
}

void FTagOutMatch::HoldUse(int Slot, double Dt)
{
	FTagRigger& Rigger = Seats[Slot];
	if (!CanCall() || !Rigger.bStanding || Rigger.SmearingUntil >= 0.0)
	{
		return;  // Both hands on the rag
	}
	const int Mine = Rigger.Team;
	const int Theirs = 1 - Mine;
	const FTagBeacon& Enemy = Beacons[Theirs];
	const FTagBeacon& Own = Beacons[Mine];
	// What the hold works on, by priority, and how long it takes.
	int On = -1;
	double Seconds = 0.0;
	if (Carrying(Slot) == Theirs && OnPad(Slot, Mine))
	{
		On = UsePlant;
		Seconds = Config.PlantSeconds;
	}
	else if (Carrying(Slot) == NoPlayer && Enemy.State != EBeaconState::Carried && Touching(Slot, Enemy.At))
	{
		On = UseBeacon + Theirs;
		Seconds = Config.GrabSeconds;
	}
	else if (Carrying(Slot) == NoPlayer && Own.State == EBeaconState::Dropped && Touching(Slot, Own.At))
	{
		On = UseBeacon + Mine;
		Seconds = Config.ReturnSeconds;
	}
	else if (Carrying(Slot) == NoPlayer && LockersOpen() && Rigger.SpeedMs <= Config.StillSpeedMs)
	{
		for (int I = 0; I < static_cast<int>(LockerAt.size()); ++I)
		{
			if (Touching(Slot, LockerAt[I]))
			{
				On = UseLocker + I;
				Seconds = Config.RestampSeconds;
				break;
			}
		}
	}
	Rigger.bUsing = true;
	if (On != Rigger.UseOn)
	{
		Rigger.UseOn = On;
		Rigger.UseHeld = 0.0;
	}
	if (On < 0)
	{
		return;
	}
	Rigger.UseHeld += Dt;
	if (!Reached(Rigger.UseHeld, Seconds))
	{
		return;
	}
	Rigger.UseHeld = 0.0;
	Rigger.UseOn = -1;
	if (On == UsePlant)
	{
		Emit(ETagEvent::Planted, Slot, NoPlayer, Theirs);
		End(Mine);
	}
	else if (On == UseBeacon + Theirs)
	{
		FTagBeacon& Beacon = Beacons[Theirs];
		Beacon.State = EBeaconState::Carried;
		Beacon.Carrier = Slot;
		Beacon.At = Rigger.Pose.Chest;
		Emit(ETagEvent::BeaconGrabbed, Slot, NoPlayer, Theirs);
	}
	else if (On == UseBeacon + Mine)
	{
		SendHome(Mine, Slot);
	}
	else
	{
		Recode(Slot);
		Emit(ETagEvent::Restamped, Slot, On - UseLocker, Mine);
	}
}

void FTagOutMatch::SetLoadout(int Slot, ETagGadget Gadget) { Seats[Slot].Gadget = Gadget; }

bool FTagOutMatch::UseGadget(int Slot, const FVec3& At)
{
	FTagRigger& Rigger = Seats[Slot];
	if (!CanCall() || !Rigger.bStanding || Rigger.GadgetsLeft <= 0 || Carrying(Slot) != NoPlayer || Rigger.SmearingUntil >= 0.0 ||
	    Rigger.Call == ECallState::Locking)
	{
		return false;  // No gadgets while carrying (5.3)
	}
	if (Rigger.Gadget == ETagGadget::Dust)
	{
		--Rigger.GadgetsLeft;
		Dust.push_back({At, Clock + Config.DustSeconds, Slot});
		Emit(ETagEvent::DustCloud, Slot, NoPlayer, Rigger.Team);
		return true;
	}
	for (const ETagPlate Plate : {ETagPlate::Back, ETagPlate::LeftShoulder, ETagPlate::RightShoulder})
	{
		if (!Smeared(Slot, Plate))
		{
			Rigger.SmearPlate = Plate;
			Rigger.SmearingUntil = Clock + Config.GreaseApplySeconds;
			Emit(ETagEvent::Smearing, Slot, NoPlayer, Rigger.Team, Plate);
			return true;
		}
	}
	return false;  // All three already smeared
}

void FTagOutMatch::ReportRoute(int Team, double Metres)
{
	FTagBeacon& Beacon = Beacons[Team];
	if (Beacon.State == EBeaconState::Carried)
	{
		Beacon.bRouted = true;
		Beacon.ClosestToEnemyPadM = std::min(Beacon.ClosestToEnemyPadM, std::max(0.0, Metres));
	}
}

void FTagOutMatch::Throw(int Slot)
{
	const int Carried = Carrying(Slot);
	if (Carried == NoPlayer || !CanCall())
	{
		return;
	}
	FTagBeacon& Beacon = Beacons[Carried];
	Beacon.State = EBeaconState::Dropped;
	Beacon.Carrier = NoPlayer;
	Beacon.At = Seats[Slot].Pose.Chest;
	Beacon.DroppedAt = Clock;
	Emit(ETagEvent::BeaconDropped, Slot, NoPlayer, Carried);
}

void FTagOutMatch::SetBeaconAt(int Team, const FVec3& At)
{
	FTagBeacon& Beacon = Beacons[Team];
	if (Beacon.State != EBeaconState::Dropped)
	{
		return;
	}
	Beacon.At = At;
	if (At.Z < Config.KillZ)
	{
		SendHome(Team, NoPlayer);
	}
}

void FTagOutMatch::SendHome(int Team, int Returner)
{
	FTagBeacon& Beacon = Beacons[Team];
	Beacon.State = EBeaconState::OnStand;
	Beacon.Carrier = NoPlayer;
	Beacon.At = Beacon.Stand;
	Emit(ETagEvent::BeaconReturned, Returner, NoPlayer, Team);
}

void FTagOutMatch::End(int Team)
{
	CurrentPhase = ETagPhase::Over;
	WinningTeam = Team;
	for (FTagRigger& Rigger : Seats)
	{
		Rigger.Call = ECallState::Idle;
		Rigger.CallTarget = NoPlayer;
	}
	Emit(ETagEvent::MatchOver, NoPlayer, NoPlayer, Team);
}

void FTagOutMatch::Step(double Dt, const std::vector<bool>& CallTargetInView)
{
	if (CurrentPhase == ETagPhase::Over)
	{
		return;
	}
	Clock += Dt;
	const double Live = Clock - Config.DropSeconds;

	// The phase.
	if (CurrentPhase == ETagPhase::Drop && Live >= -TimeEpsilon)
	{
		CurrentPhase = ETagPhase::Live;
		Emit(ETagEvent::Live);
	}
	if (CurrentPhase == ETagPhase::Live && !bMidEventFired && Live >= Config.RegulationSeconds - Config.MidEventRemaining - TimeEpsilon)
	{
		bMidEventFired = true;
		Emit(ETagEvent::MidEvent);
	}
	if (CurrentPhase == ETagPhase::Live && Live >= Config.RegulationSeconds - TimeEpsilon)
	{
		CurrentPhase = ETagPhase::Overtime;
		for (FTagRigger& Rigger : Seats)
		{
			if (Rigger.UseOn >= UseLocker && Rigger.UseOn < UsePlant)
			{
				Rigger.UseHeld = 0.0;  // The lockers shut on whoever is at one
				Rigger.UseOn = -1;
			}
		}
		Emit(ETagEvent::Overtime);
	}
	if (CurrentPhase == ETagPhase::Overtime && Live >= Config.RegulationSeconds + Config.OvertimeSeconds - TimeEpsilon)
	{
		// Nobody planted: the crew whose carrier came closest to home wins (5.6).
		const double Zero = Beacons[1].ClosestToEnemyPadM;  // Team 0 captures team 1's beacon
		const double One = Beacons[0].ClosestToEnemyPadM;
		End(std::abs(Zero - One) < 1e-6 ? NoPlayer : (Zero < One ? 0 : 1));
		return;
	}

	// Grease going on, and dust settling.
	for (FTagRigger& Rigger : Seats)
	{
		if (Rigger.SmearingUntil >= 0.0 && Clock >= Rigger.SmearingUntil - TimeEpsilon)
		{
			Rigger.SmearingUntil = -1.0;
			Rigger.SmearedUntil[static_cast<int>(Rigger.SmearPlate)] = Clock + Config.GreaseSeconds;
			--Rigger.GadgetsLeft;
			Emit(ETagEvent::Smeared, Rigger.Slot, NoPlayer, Rigger.Team, Rigger.SmearPlate);
		}
	}
	Dust.erase(std::remove_if(Dust.begin(), Dust.end(), [this](const FTagCloud& Cloud) { return Cloud.Until <= Clock; }), Dust.end());

	// Holds that were not held this step let go.
	for (FTagRigger& Rigger : Seats)
	{
		if (!Rigger.bUsing)
		{
			Rigger.UseHeld = 0.0;
			Rigger.UseOn = -1;
		}
		Rigger.bUsing = false;
	}

	// The locks.
	for (FTagRigger& Rigger : Seats)
	{
		if (Rigger.Call != ECallState::Locking)
		{
			continue;
		}
		const FTagRigger& Called = Seats[Rigger.CallTarget];
		const bool bInView = Rigger.Slot < static_cast<int>(CallTargetInView.size()) && CallTargetInView[Rigger.Slot];
		if (!Rigger.bStanding || !Called.bStanding || !bInView ||
		    (Called.Pose.Chest - Rigger.Pose.Chest).Length() > Config.CallRangeM || ThroughDust(Rigger.Pose.Eye, Called.Pose.Chest) ||
		    SunBehind(Rigger.Slot, Rigger.CallTarget))
		{
			Emit(ETagEvent::LostLock, Rigger.Slot, Rigger.CallTarget, Rigger.Team);
			Rigger.Call = ECallState::Idle;
			Rigger.CallTarget = NoPlayer;
			Rigger.CallCooldownUntil = Clock + Config.LostCooldown;  // The pips stay entered
			continue;
		}
		Rigger.LockElapsed += Dt;
		if (Reached(Rigger.LockElapsed, Config.LockSeconds))
		{
			ResolveCall(Rigger.Slot);
		}
	}

	// The beacons: a carried one goes with its carrier, a dropped one goes home on its own.
	for (FTagBeacon& Beacon : Beacons)
	{
		if (Beacon.State == EBeaconState::Carried)
		{
			const FTagRigger& Carrier = Seats[Beacon.Carrier];
			Beacon.At = Carrier.Pose.Chest;
			if (!Beacon.bRouted)
			{
				const FVec3 Off = Beacon.At - HomePads[Carrier.Team];
				Beacon.ClosestToEnemyPadM = std::min(Beacon.ClosestToEnemyPadM, std::max(0.0, std::hypot(Off.X, Off.Y) - Config.PadRadiusM));
			}
			if (CurrentPhase == ETagPhase::Overtime)
			{
				for (int I = 0; I < PlateCount; ++I)
				{
					Reveal(Beacon.Team, Carrier.Slot, static_cast<ETagPlate>(I));  // The Lockout: the carrier's code goes up
				}
			}
		}
		else if (Beacon.State == EBeaconState::Dropped && Clock - Beacon.DroppedAt >= Config.AutoReturnSeconds - TimeEpsilon)
		{
			SendHome(Beacon.Team, NoPlayer);
		}
	}

	// The reserve.
	if (CanCall())
	{
		for (FTagRigger& Rigger : Seats)
		{
			if (!Rigger.bStanding && Clock >= Rigger.ReserveUntil - TimeEpsilon)
			{
				DropIn(Rigger.Slot);
			}
		}
	}
}
} // namespace Kinetic
