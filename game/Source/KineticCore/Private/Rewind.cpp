#include "Kinetic/Rewind.h"

#include <algorithm>
#include <cmath>

namespace Kinetic
{
namespace
{
constexpr double Pi = 3.14159265358979323846;
constexpr double GraceCapSeconds = 0.150;
constexpr double GraceFrameSeconds = 0.033;  // Two frames of processing
constexpr double GraceSlackSeconds = 0.015;
constexpr double PredictiveAboveSeconds = 0.300;
constexpr double LeavePoolsAboveSeconds = 0.400;

int64_t FramesFor(double Seconds) { return static_cast<int64_t>(std::ceil(Seconds * NetTickHz - 1e-9)); }
} // namespace

void FRewindHistory::Record(FFrameSnapshot Snapshot)
{
	Newest = Snapshot.Frame;
	Ring.push_back(std::move(Snapshot));
	if (Ring.size() > static_cast<size_t>(HistoryFrames))
	{
		Ring.erase(Ring.begin());
	}
}

int64_t FRewindHistory::JudgeFrame(int64_t ClientFrame) const
{
	const int64_t Oldest = Ring.empty() ? Newest : Ring.front().Frame;
	return std::clamp(ClientFrame, std::max(Newest - MaxRewindFrames, Oldest), Newest);
}

const FFrameSnapshot* FRewindHistory::At(int64_t Frame) const
{
	for (const FFrameSnapshot& S : Ring)
	{
		if (S.Frame == Frame)
		{
			return &S;
		}
	}
	return nullptr;
}

std::optional<FLineSegment> FRewindHistory::LineAt(int64_t Frame, int LineId) const
{
	const FFrameSnapshot* S = At(Frame);
	if (!S)
	{
		return std::nullopt;
	}
	for (const FLineSegment& L : S->Lines)
	{
		if (L.LineId == LineId)
		{
			return L;
		}
	}
	return std::nullopt;
}

std::optional<FVec3> FRewindHistory::EyeAt(int64_t Frame, int Player) const
{
	const FFrameSnapshot* S = At(Frame);
	if (!S)
	{
		return std::nullopt;
	}
	for (const FPlayerPose& P : S->Players)
	{
		if (P.Player == Player)
		{
			return P.Eye;
		}
	}
	return std::nullopt;
}

FCutValidator::FPlayer& FCutValidator::PlayerFor(int Player)
{
	for (FPlayer& P : Players)
	{
		if (P.Player == Player)
		{
			return P;
		}
	}
	Players.push_back({});
	Players.back().Player = Player;
	return Players.back();
}

FVec3 FCutValidator::Turn(FPlayer& P, int64_t Frame, const FVec3& Forward)
{
	const double Length = Forward.Length();
	if (!std::isfinite(Length) || Length < 1e-6)
	{
		return P.Camera;  // Not a direction: the camera stays where it was
	}
	if (!P.bCamera)
	{
		P.bCamera = true;
		P.CameraFrame = Frame;
		P.Camera = Forward.SafeNormal();
	}
	else
	{
		const int64_t Gap = std::clamp<int64_t>(Frame - P.CameraFrame, 0, MaxTurnGapFrames);
		P.Camera = ClampTurn(P.Camera, Forward, MaxTurnDegPerSecond * static_cast<double>(Gap) / NetTickHz);
		P.CameraFrame = std::max(P.CameraFrame, Frame);
	}
	if (!P.Cameras.empty() && P.Cameras.back().first == P.CameraFrame)
	{
		P.Cameras.back().second = P.Camera;
	}
	else
	{
		P.Cameras.push_back({P.CameraFrame, P.Camera});
		if (P.Cameras.size() > static_cast<size_t>(HistoryFrames))
		{
			P.Cameras.erase(P.Cameras.begin());
		}
	}
	return P.Camera;
}

std::optional<FVec3> FCutValidator::CameraAt(const FPlayer& P, int64_t Frame)
{
	std::optional<FVec3> At;
	for (const auto& [F, Camera] : P.Cameras)
	{
		if (F <= Frame)
		{
			At = Camera;
		}
	}
	return At;
}

void FCutValidator::Look(int Player, int64_t ClientFrame, const FVec3& Forward)
{
	Turn(PlayerFor(Player), History.JudgeFrame(ClientFrame), Forward);
}

std::optional<FVec3> FCutValidator::Camera(int Player) const
{
	for (const FPlayer& P : Players)
	{
		if (P.Player == Player && P.bCamera)
		{
			return P.Camera;
		}
	}
	return std::nullopt;
}

void FCutValidator::Draw(int Player, int64_t ClientFrame, double WarmupSeconds)
{
	FPlayer& P = PlayerFor(Player);
	P.bDrawn = true;
	P.ReadyFrame = History.JudgeFrame(ClientFrame) + FramesFor(WarmupSeconds);
}

void FCutValidator::Sheathe(int Player) { PlayerFor(Player).bDrawn = false; }

FVec3 ClampTurn(const FVec3& From, const FVec3& To, double MaxDegrees)
{
	const FVec3 F = From.SafeNormal();
	const FVec3 T = To.SafeNormal();
	const double Angle = std::acos(std::clamp(F.Dot(T), -1.0, 1.0));
	const double Max = MaxDegrees * Pi / 180.0;
	if (Angle <= Max || Angle < 1e-12)
	{
		return T;
	}
	// Slerp from F toward T by Max. Exactly opposite, any perpendicular will do.
	FVec3 Ortho = T - F * F.Dot(T);
	if (Ortho.Length() < 1e-9)
	{
		const FVec3 Axis = std::abs(F.Z) < 0.9 ? FVec3{0, 0, 1} : FVec3{1, 0, 0};
		Ortho = Axis - F * F.Dot(Axis);
	}
	Ortho = Ortho.SafeNormal();
	return (F * std::cos(Max) + Ortho * std::sin(Max)).SafeNormal();
}

std::optional<FCutDecision> FCutValidator::Swing(int Player, int64_t ClientFrame, const FVec3& Forward, double Reach,
                                                 double ConeDeg)
{
	FPlayer& P = PlayerFor(Player);
	const int64_t Frame = History.JudgeFrame(ClientFrame);
	if (!P.bDrawn || !P.bCamera || Frame < P.ReadyFrame || Frame < P.NextSwingFrame)
	{
		return std::nullopt;  // Not drawn, still warming up, or still in the last swing's cycle
	}
	// The camera as it was at the swing's frame: a later sample already moved it on.
	const std::optional<FVec3> Look = Frame >= P.CameraFrame ? std::optional<FVec3>(Turn(P, Frame, Forward)) : CameraAt(P, Frame);
	P.NextSwingFrame = Frame + FramesFor(SwingCycleSeconds);  // Hit or miss, the swing is spent
	const FFrameSnapshot* Snapshot = History.At(Frame);
	const std::optional<FVec3> Eye = History.EyeAt(Frame, Player);
	if (!Snapshot || !Eye || !Look)
	{
		return std::nullopt;
	}
	std::optional<FCutDecision> Best;
	double BestDistance = 1e300;
	for (const FLineSegment& L : Snapshot->Lines)
	{
		const std::optional<double> T = FindCutPoint({*Eye, *Look, Reach, ConeDeg}, L.A, L.B);
		if (!T)
		{
			continue;
		}
		const double D = ((L.A + (L.B - L.A) * *T) - *Eye).Length();
		if (D < BestDistance)
		{
			BestDistance = D;
			Best = FCutDecision{L.LineId, *T, Frame};
		}
	}
	return Best;
}

std::optional<FPlateHit> ResweepDog(const FRewindHistory& History, int64_t ClientFrame, int Shooter, const FVec3& From,
                                    const FVec3& To)
{
	const int64_t Frame = History.JudgeFrame(ClientFrame);
	const FFrameSnapshot* S = History.At(Frame);
	if (!S)
	{
		return std::nullopt;
	}
	const FVec3 D = To - From;
	const double L2 = D.Dot(D);
	const double Hit = DogRadiusM + PlateHitRadiusM;
	// The first player the sweep reaches: the earliest entry into any of their plate spheres.
	std::optional<FPlateHit> First;
	for (const FPlateState& Plate : S->Plates)
	{
		if (Plate.Player == Shooter)
		{
			continue;  // Self-dog is its own input
		}
		const FVec3 M = From - Plate.Position;
		const double B = M.Dot(D);
		const double C = M.Dot(M) - Hit * Hit;
		double T = 0.0;
		if (C > 0.0)
		{
			const double Disc = B * B - L2 * C;
			if (L2 <= 0.0 || B > 0.0 || Disc < 0.0)
			{
				continue;
			}
			T = (-B - std::sqrt(Disc)) / L2;
			if (T > 1.0)
			{
				continue;
			}
		}
		if (!First || T < First->At)
		{
			First = FPlateHit{Plate.Player, Plate.Plate, T, Frame};
		}
	}
	if (!First)
	{
		return std::nullopt;
	}
	// That player's plate nearest the point of impact.
	const FVec3 Impact = From + D * First->At;
	double Nearest = 1e300;
	for (const FPlateState& Plate : S->Plates)
	{
		const double Distance = (Plate.Position - Impact).Length();
		if (Plate.Player == First->Player && Distance < Nearest)
		{
			Nearest = Distance;
			First->Plate = Plate.Plate;
		}
	}
	return First;
}

int GraceFrames(double OneWayLatencySeconds)
{
	const double G = std::min(GraceCapSeconds, OneWayLatencySeconds + GraceFrameSeconds + GraceSlackSeconds);
	return static_cast<int>(FramesFor(G));
}

bool FPendingStalls::Stall(int Victim, int CausingLine, int64_t StallFrame, double OneWayLatencySeconds, bool bOverAnEdge)
{
	if (bOverAnEdge)
	{
		return true;  // The point of no return: no cut can take it back
	}
	Pending.push_back({Victim, CausingLine, StallFrame, StallFrame + GraceFrames(OneWayLatencySeconds)});
	return false;
}

bool FPendingStalls::Cut(int LineId, int64_t JudgedFrame)
{
	const size_t Before = Pending.size();
	std::erase_if(Pending, [&](const FPendingStall& P) { return P.CausingLine == LineId && JudgedFrame <= P.StallFrame; });
	return Pending.size() != Before;
}

std::vector<FPendingStall> FPendingStalls::Confirm(int64_t Present)
{
	std::vector<FPendingStall> Done;
	std::erase_if(Pending, [&](const FPendingStall& P) {
		if (P.ConfirmFrame <= Present)
		{
			Done.push_back(P);
			return true;
		}
		return false;
	});
	return Done;
}

bool FPendingStalls::IsPending(int Victim) const
{
	return std::any_of(Pending.begin(), Pending.end(), [Victim](const FPendingStall& P) { return P.Victim == Victim; });
}

void FLatencyEstimate::Sample(double RoundTripSeconds)
{
	if (!bAny)
	{
		Smoothed = RoundTripSeconds;
		bAny = true;
		return;
	}
	Smoothed += Smoothing * (RoundTripSeconds - Smoothed);
}

ELatencyPolicy PolicyFor(double RoundTripSeconds)
{
	if (RoundTripSeconds > LeavePoolsAboveSeconds)
	{
		return ELatencyPolicy::LeavePools;
	}
	return RoundTripSeconds > PredictiveAboveSeconds ? ELatencyPolicy::PredictiveInterpolation
	                                                 : ELatencyPolicy::Resimulation;
}
} // namespace Kinetic
