// Replication encodings, priority and bandwidth, the rewind history, server cut validation and
// the grace window (docs/tech/networking.md sections 2, 3, 5, 6, 7 and 11), and section 7.3's
// validation matrix: the 05 4.3 drag at 30, 80 and 150 ms RTT with 20 ms jitter.
#include "Kinetic/NetCodec.h"
#include "Kinetic/NetPriority.h"
#include "Kinetic/ReferenceSim.h"
#include "Kinetic/Rewind.h"
#include "SimFixtures.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <optional>

using namespace Kinetic;
using namespace Kinetic::Test;

namespace
{
constexpr double Pi = 3.14159265358979323846;

// A small deterministic generator for the jitter and the fuzz (xorshift).
struct FRng
{
	uint64_t S = 0x9E3779B97F4A7C15ull;
	double Next()  // [0, 1)
	{
		S ^= S << 13;
		S ^= S >> 7;
		S ^= S << 17;
		return static_cast<double>(S >> 11) / static_cast<double>(uint64_t{1} << 53);
	}
};

double QuatAngle(const FQuat& A, const FQuat& B)
{
	return 2.0 * std::acos(std::min(1.0, std::abs(A.Dot(B))));
}
} // namespace

KTEST(BitsRoundTrip)
{
	FBitWriter W;
	W.Write(5, 3), W.Write(0xABCD, 16), W.Write(1, 1), W.Write(0x123456789ull, 37);
	KCHECK(W.BitCount() == 57 && W.Bytes().size() == 8);
	FBitReader R(W.Bytes());
	KCHECK(*R.Read(3) == 5 && *R.Read(16) == 0xABCD && *R.Read(1) == 1 && *R.Read(37) == 0x123456789ull);
	KCHECK(R.Read(7).has_value() && !R.Read(1).has_value());  // Padding, then the end
}

KTEST(PositionsQuantizeToACentimetre)
{
	const FArenaBounds Bounds;
	FRng Rng;
	double Worst = 0.0;
	for (int I = 0; I < 2000; ++I)
	{
		const FVec3 P{Bounds.Min.X + Rng.Next() * (Bounds.Max.X - Bounds.Min.X),
		              Bounds.Min.Y + Rng.Next() * (Bounds.Max.Y - Bounds.Min.Y),
		              Bounds.Min.Z + Rng.Next() * (Bounds.Max.Z - Bounds.Min.Z)};
		const FVec3 Back = DequantizePosition(QuantizePosition(P, Bounds), Bounds);
		Worst = std::max({Worst, std::abs(Back.X - P.X), std::abs(Back.Y - P.Y), std::abs(Back.Z - P.Z)});
	}
	KCHECK(Worst <= PositionStep / 2.0);  // Within half a centimetre on every axis
	const FVec3 Outside = DequantizePosition(QuantizePosition({1000, 0, 0}, Bounds), Bounds);
	KCHECK_NEAR(Outside.X, Bounds.Max.X, 1e-9);  // Clamped to the arena
	const FVec3 V = DequantizeVelocity(QuantizeVelocity({49.9, -12.3, 0}, LinearSpeedRange), LinearSpeedRange);
	KCHECK_NEAR(V.X, 49.9, 0.001);
	KCHECK_NEAR(V.Y, -12.3, 0.001);
	// At rest is exactly at rest: sleeping bodies do not drift when extrapolated.
	const FVec3 Rest = DequantizeVelocity(QuantizeVelocity({0, 0, 0}, AngularSpeedRange), AngularSpeedRange);
	KCHECK(Rest.X == 0.0 && Rest.Y == 0.0 && Rest.Z == 0.0);
	KCHECK(QuantizeUnit(std::nan(""), -1, 1) == 32767);
	KCHECK_NEAR(DequantizeUnit(65535, -1, 1), 1.0, 1e-12);  // The unused top code reads as the top
}

KTEST(SmallestThreeKeepsRotations)
{
	FRng Rng;
	double Worst = 0.0;
	for (int I = 0; I < 5000; ++I)
	{
		const FQuat Q = FQuat{Rng.Next() - 0.5, Rng.Next() - 0.5, Rng.Next() - 0.5, Rng.Next() - 0.5}.Normalized();
		Worst = std::max(Worst, QuatAngle(Q, UnpackQuat(PackQuat(Q))));
	}
	KCHECK(Worst < 0.01 * Pi / 180.0);  // Under a hundredth of a degree
	const FQuat Q = AxisAngle({0, 0, 1}, 1.0);
	const FQuat Neg{-Q.X, -Q.Y, -Q.Z, -Q.W};  // The same rotation
	KCHECK(QuatAngle(UnpackQuat(PackQuat(Neg)), Q) < 1e-4);
	KCHECK(PackQuat(Q) < (uint64_t{1} << 47));
}

KTEST(ABodyIsThirtyTwoBytes)
{
	const FArenaBounds Bounds;
	const FBodyState Body{{12.345, -67.89, 30.01}, AxisAngle({1, 2, 3}, 2.0), {14.0, -3.5, 0.25}, {1.0, -2.0, 6.0}, 123456, 0x5A};
	const std::vector<uint8_t> Bytes = EncodeBody(Body, Bounds);
	KCHECK(Bytes.size() == static_cast<size_t>(BodyPayloadBytes));
	const std::optional<FBodyState> Back = DecodeBody(Bytes, Bounds);
	KCHECK(Back.has_value());
	KCHECK((Back->Position - Body.Position).Length() < PositionStep);
	KCHECK(QuatAngle(Back->Rotation, Body.Rotation) < 1e-3);
	KCHECK((Back->LinearVelocity - Body.LinearVelocity).Length() < 0.002);
	KCHECK((Back->AngularVelocity - Body.AngularVelocity).Length() < 0.002);
	KCHECK(Back->ServerFrame == 123456 && Back->Flags == 0x5A);
	KCHECK(!DecodeBody(std::vector<uint8_t>(31, 0), Bounds));
}

KTEST(ALineIsFiveBytes)
{
	const FNetLine Line{4095, 4321.0, ENetLineMode::Hang, true, false, true, false, true, 23.45};
	const std::vector<uint8_t> Bytes = EncodeLine(Line).value();
	KCHECK(Bytes.size() == static_cast<size_t>(LineUpdateBytes));
	FNetLine TooMany = Line;
	TooMany.LineId = 4096;  // Would alias line 0
	KCHECK(!EncodeLine(TooMany));
	TooMany.LineId = 1, TooMany.ForceN = std::nan("");
	KCHECK(!EncodeLine(TooMany));
	const std::optional<FNetLine> Back = DecodeLine(Bytes);
	KCHECK(Back.has_value());
	KCHECK(Back->LineId == 4095 && Back->Mode == ENetLineMode::Hang);
	KCHECK_NEAR(Back->ForceN, 4321.0, LineForceRangeN / 255.0 / 2.0 + 1e-9);
	KCHECK(Back->bSlipping && !Back->bOverspin && Back->bSnapping && !Back->bReeling && Back->bSeatedInPlate);
	KCHECK_NEAR(Back->RestLengthM, 23.45, RestLengthRangeM / 4095.0 / 2.0 + 1e-9);
	std::vector<uint8_t> BadMode = Bytes;
	BadMode[2] = static_cast<uint8_t>(BadMode[2] | 0x70);  // Mode 7
	KCHECK(!DecodeLine(BadMode));
	KCHECK(!DecodeLine({1, 2, 3}));
}

KTEST(TheBandwidthTableAddsUp)
{
	// Section 3's worst case: about 102,000 B/s per client, over the 100,000 MaxClientRate,
	// which is why the scheduler sheds.
	const double Worst = DownstreamBytesPerSecond({});
	KCHECK_NEAR(Worst, 101964.0, 1e-6);
	KCHECK(Worst > MaxClientRateBytes);
}

KTEST(PriorityFollowsSectionFive)
{
	FNetActor Line{1, ENetActorKind::Line};
	KCHECK(NetPriority(Line) == 5 && UpdateHz(Line) == 30.0);
	Line.bOwnedByViewer = true;
	KCHECK(NetPriority(Line) == 8 && UpdateHz(Line) == 60.0);
	Line.bDogInViewerPlate = true;
	KCHECK(NetPriority(Line) == 10 && UpdateHz(Line) == 60.0);
	Line.bOwnedByViewer = false;
	KCHECK(NetPriority(Line) == 10 && UpdateHz(Line) == 60.0);
	KCHECK(NetPriority({2, ENetActorKind::OwnBody}) == 10);
	FNetActor Player{3, ENetActorKind::OtherPlayer};
	KCHECK(NetPriority(Player) == 6 && UpdateHz(Player) == 30.0);
	Player.bAirborne = true;
	KCHECK(NetPriority(Player) == 6 && UpdateHz(Player) == 60.0);
	FNetActor Body{4, ENetActorKind::Body};
	KCHECK(NetPriority(Body) == 1);
	Body.bInViewerFrustum = true;
	KCHECK(NetPriority(Body) == 3);
	Body.DistanceToViewer = 10.0;
	KCHECK(NetPriority(Body) == 4);
	Body.bOnLiveLine = true;
	KCHECK(NetPriority(Body) == 5);
	Body.bAwake = false;
	KCHECK(NetPriority(Body) == 0 && UpdateHz(Body) == 0.0);  // Dormant
	FNetActor Far{5, ENetActorKind::Body};
	Far.DistanceToAnyPlayer = 30.0;
	KCHECK(TierOf(Far) == EBodyTier::Tier2 && UpdateHz(Far) == 10.0);
	Far.DistanceToAnyPlayer = 90.0;
	KCHECK(TierOf(Far) == EBodyTier::Tier3 && UpdateHz(Far) == 3.0);
	Far.bOnLiveLine = true;
	KCHECK(TierOf(Far) == EBodyTier::Tier1 && UpdateHz(Far) == 30.0);
}

namespace
{
// Section 3's worst-case scene for one viewer.
std::vector<FNetActor> WorstCaseScene()
{
	std::vector<FNetActor> Scene;
	int Id = 0;
	Scene.push_back({Id++, ENetActorKind::OwnBody});
	for (int I = 0; I < 5; ++I)
	{
		FNetActor P{Id++, ENetActorKind::OtherPlayer};
		P.bAirborne = true;  // The worst case
		Scene.push_back(P);
	}
	for (int I = 0; I < 16; ++I)
	{
		FNetActor L{Id++, ENetActorKind::Line};
		L.bDogInViewerPlate = I == 0;
		Scene.push_back(L);
	}
	const auto Bodies = [&](int N, double Distance) {
		for (int I = 0; I < N; ++I)
		{
			FNetActor B{Id++, ENetActorKind::Body};
			B.DistanceToAnyPlayer = B.DistanceToViewer = Distance;
			Scene.push_back(B);
		}
	};
	Bodies(24, 10.0), Bodies(40, 30.0), Bodies(32, 90.0);
	for (int I = 0; I < 72; ++I)
	{
		FNetActor C{Id++, ENetActorKind::GeometryCluster};
		C.bNearestClusters = I < 36;
		Scene.push_back(C);
	}
	return Scene;
}

// Updates per second each actor got over Seconds.
std::vector<double> Rates(FNetScheduler& S, const std::vector<FNetActor>& Scene, double Seconds)
{
	std::vector<double> Count(Scene.size(), 0.0);
	for (int Tick = 0; Tick < static_cast<int>(Seconds * 60.0); ++Tick)
	{
		for (const int Id : S.Tick(Scene, Tick / 60.0))
		{
			Count[static_cast<size_t>(Id)] += 1.0;
		}
	}
	for (double& C : Count) C /= Seconds;
	return Count;
}
} // namespace

KTEST(TheSchedulerKeepsLinesAndPlayersWhenSaturated)
{
	const std::vector<FNetActor> Scene = WorstCaseScene();
	double NeverShedBytes = 0.0;
	for (const FNetActor& A : Scene)
	{
		NeverShedBytes += NeverShed(A) ? UpdateBytes(A) * UpdateHz(A) : 0.0;
	}
	for (const double Budget : {MaxClientRateBytes, 70000.0, 30000.0})
	{
		FNetScheduler S(60.0, Budget);
		const double Seconds = 4.0;
		const std::vector<double> Got = Rates(S, Scene, Seconds);
		std::array<double, 3> Share{0, 0, 0}, Hz{0, 0, 0}, Count{0, 0, 0};
		for (size_t I = 0; I < Scene.size(); ++I)
		{
			if (NeverShed(Scene[I]))
			{
				KCHECK_NEAR(Got[I], UpdateHz(Scene[I]), 0.5);  // Lines and players at their full rate
				continue;
			}
			// While the budget covers tier 1, nothing awake goes more than about a second unsent.
			if (Budget >= 70000.0)
			{
				KCHECK(Got[I] >= 1.0 / (FNetScheduler::StarvedSeconds + 1.0 / UpdateHz(Scene[I])) - 0.3);
			}
			if (Scene[I].Kind == ENetActorKind::Body)
			{
				const size_t Rank = static_cast<size_t>(ShedRank(Scene[I]));
				Share[Rank] += Got[I] / UpdateHz(Scene[I]);
				Hz[Rank] += Got[I];
				Count[Rank] += 1.0;
			}
		}
		for (size_t R = 0; R < 3; ++R) Share[R] /= Count[R], Hz[R] /= Count[R];
		// Tier 1 keeps the most of its rate; tier 2 is sent at least as often as tier 3.
		KCHECK(Share[0] >= Share[1] - 0.02 && Share[0] >= Share[2] - 0.02 && Hz[1] >= Hz[2] - 1e-9);
		if (Budget == 70000.0)
		{
			KCHECK(Share[0] > 0.99 && Share[1] < 0.5 && Share[2] < 0.5);  // Tiers 2 and 3 shed to the floor
		}
		if (Budget == 30000.0)
		{
			KCHECK(Share[0] > 0.15 && Share[1] == 0.0 && Share[2] == 0.0);  // All that is left goes to tier 1
		}
		// Over the budget only by what may never be shed.
		KCHECK(S.BytesSent() / Seconds <= std::max(Budget, NeverShedBytes) + NeverShedBytes * 0.05);
	}
}

KTEST(ATierOneBodyBehindTheViewerIsNotStarved)
{
	// Near an enemy (tier 1) but behind the viewer and out of view (priority 1), competing with
	// tier-3 bodies in the viewer's frustum (priority 3): tier wins.
	std::vector<FNetActor> Scene;
	FNetActor Near{0, ENetActorKind::Body};
	Near.DistanceToAnyPlayer = 5.0, Near.DistanceToViewer = 30.0;
	Scene.push_back(Near);
	for (int I = 1; I <= 40; ++I)
	{
		FNetActor Far{I, ENetActorKind::Body};
		Far.DistanceToAnyPlayer = Far.DistanceToViewer = 90.0;
		Far.bInViewerFrustum = true;
		Scene.push_back(Far);
	}
	KCHECK(NetPriority(Scene[0]) < NetPriority(Scene[1]));
	FNetScheduler S(60.0, 24000.0);
	const std::vector<double> Got = Rates(S, Scene, 10.0);
	KCHECK_NEAR(Got[0], 30.0, 0.5);
	S.Forget(0);
}

KTEST(TheHistoryRewindsAtMostNineFrames)
{
	FRewindHistory H;
	for (int64_t F = 100; F < 130; ++F)
	{
		H.Record({F, {{7, 0, 1, {static_cast<double>(F), 0, 0}, {0, 0, 0}}}, {}, {}});
	}
	KCHECK(H.Present() == 129);
	KCHECK(H.JudgeFrame(125) == 125);
	KCHECK(H.JudgeFrame(100) == 120);  // Capped at 150 ms
	KCHECK(H.JudgeFrame(140) == 129);  // Never in the future
	KCHECK(!H.At(109) && H.At(110));   // 20 frames kept
	KCHECK_NEAR(H.LineAt(124, 7)->A.X, 124.0, 1e-12);
	KCHECK(!H.LineAt(124, 8));
}

namespace
{
FVec3 Heading(double Degrees)
{
	return {std::cos(Degrees * Pi / 180.0), std::sin(Degrees * Pi / 180.0), 0.0};
}

double DegreesBetween(const FVec3& A, const FVec3& B)
{
	return std::acos(std::clamp(A.SafeNormal().Dot(B.SafeNormal()), -1.0, 1.0)) * 180.0 / Pi;
}
} // namespace

KTEST(TheServerJudgesTheSwingAtItsFrame)
{
	FRewindHistory H;
	// A line beside the victim until frame 115, then whipped away out of reach. The victim's
	// eye stands at the origin until 115, then walks off.
	const auto RecordTo = [&H](int64_t Last) {
		for (int64_t F = H.Present() + 1; F <= Last; ++F)
		{
			const double Y = F < 115 ? 0.8 : 5.0;
			const FVec3 Eye = F < 115 ? FVec3{0, 0, 1.5} : FVec3{0, -3, 1.5};
			H.Record({F, {{3, 1, 0, {-5, Y, 1.5}, {5, Y, 1.5}}}, {}, {{0, Eye}, {1, {9, 9, 1.5}}}});
		}
	};
	RecordTo(102);
	FCutValidator V(H);
	V.Look(0, 100, {0, 1, 0});
	V.Look(1, 100, {0, 1, 0});
	V.Draw(0, 100, 0.2);  // Arrives at 102: 12 frames of warm-up from the client's frame 100
	RecordTo(119);        // The swing arrives at 119, stamped 112, after the line was whipped away
	KCHECK(!V.Swing(1, 112, {0, 1, 0}, 1.5, 70));  // Another player never drew
	KCHECK(!V.Swing(0, 111, {0, 1, 0}, 1.5, 70));  // Still warming up
	const std::optional<FCutDecision> Cut = V.Swing(0, 112, {0, 1, 0}, 1.5, 70);
	KCHECK(Cut && Cut->LineId == 3 && Cut->JudgedFrame == 112);  // Cut where it, and they, were
	// Swing and recovery: no second swing for 27 frames, whatever its stamp.
	for (int64_t F = 110; F <= 125; ++F)
	{
		KCHECK(!V.Swing(0, F, {0, 1, 0}, 1.5, 70));
	}
	// A new draw restarts the warm-up, with the line in reach throughout.
	FRewindHistory R;
	const auto Reach = [&R](int64_t Last) {
		for (int64_t F = R.Present() + 1; F <= Last; ++F)
		{
			R.Record({F, {{3, 1, 0, {-5, 0.8, 1.5}, {5, 0.8, 1.5}}}, {}, {{0, {0, 0, 1.5}}}});
		}
	};
	Reach(30);
	FCutValidator W(R);
	W.Look(0, 25, {0, 1, 0});
	W.Draw(0, 25, 0.0);   // Ready at once
	W.Draw(0, 28, 0.2);   // Drawn again: ready at 40
	Reach(45);
	KCHECK(!W.Swing(0, 39, {0, 1, 0}, 1.5, 70));
	const std::optional<FCutDecision> Redrawn = W.Swing(0, 40, {0, 1, 0}, 1.5, 70);
	KCHECK(Redrawn && Redrawn->JudgedFrame == 40);
	W.Sheathe(0);
	Reach(80);
	KCHECK(!W.Swing(0, 80, {0, 1, 0}, 1.5, 70));  // Sheathed
	// No eye recorded at the frame: nothing to judge from.
	FRewindHistory Bare;
	for (int64_t F = 0; F < 20; ++F) Bare.Record({F, {{3, 1, 0, {-5, 0.8, 1.5}, {5, 0.8, 1.5}}}, {}, {}});
	FCutValidator B(Bare);
	B.Look(0, 0, {0, 1, 0});
	B.Draw(0, 0, 0.0);
	KCHECK(!B.Swing(0, 19, {0, 1, 0}, 1.5, 70));
}

KTEST(TheCameraTurnsAtMostThreeThousandDegreesASecond)
{
	FRewindHistory H;
	for (int64_t F = 0; F <= 200; ++F) H.Record({F, {}, {}, {}});
	FCutValidator V(H);
	V.Look(0, 192, Heading(0));
	V.Look(0, 193, Heading(45));
	KCHECK_NEAR(DegreesBetween(*V.Camera(0), Heading(45)), 0.0, 1e-9);  // 45° in a frame: allowed
	V.Look(0, 194, Heading(100));
	KCHECK_NEAR(DegreesBetween(*V.Camera(0), Heading(95)), 0.0, 1e-9);  // 55°: clamped to 50°
	// A stamp in the future is judged at present, and a gap earns at most 3 frames of turn.
	V.Look(0, 100000, Heading(-100));
	KCHECK_NEAR(DegreesBetween(*V.Camera(0), Heading(95)), 150.0, 1e-9);
	// A stamp older than the last sample earns nothing; nor do repeats of a frame.
	const FVec3 Before = *V.Camera(0);
	V.Look(0, 195, Heading(0));
	V.Look(0, 200, Heading(0));
	KCHECK_NEAR(DegreesBetween(*V.Camera(0), Before), 0.0, 1e-9);
	// Exactly opposite still turns, by the clamp.
	const FVec3 Turned = ClampTurn({1, 0, 0}, {-1, 0, 0}, 30.0);
	KCHECK_NEAR(DegreesBetween(Turned, {1, 0, 0}), 30.0, 1e-9);
	// Something that is not a direction is ignored, so it cannot reset the clamp.
	FCutValidator Z(H);
	Z.Look(0, 192, {1, 0, 0});
	Z.Look(0, 194, {0, 0, 0});
	Z.Look(0, 195, {std::nan(""), 0, 0});
	Z.Look(0, 196, {-1, 0, 0});
	KCHECK_NEAR(DegreesBetween(*Z.Camera(0), {1, 0, 0}), 150.0, 1e-9);  // 3 frames of turn from 192, no more
	FCutValidator First(H);
	First.Look(0, 192, {0, 0, 0});
	KCHECK(!First.Camera(0));  // Nor taken as the first sample
	// A flick from behind the line to it in one frame does not cut.
	FRewindHistory L;
	for (int64_t F = 0; F < 20; ++F) L.Record({F, {{3, 1, 0, {-5, 0.8, 1.5}, {5, 0.8, 1.5}}}, {}, {{0, {0, 0, 1.5}}}});
	FCutValidator C(L);
	C.Look(0, 17, {0, -1, 0});
	C.Draw(0, 0, 0.0);
	KCHECK(!C.Swing(0, 18, {0, 1, 0}, 1.5, 70));
}

KTEST(TheSwingUsesTheCameraAsItWasAtTheSwing)
{
	// The line is in reach until 115, then whipped away. The player faces away from it until
	// 115, then turns to where it was over 116 to 119. A swing arriving at 119 stamped 112, when
	// they faced away, does not cut: the camera is judged at 112 too.
	FRewindHistory H;
	FCutValidator V(H);
	for (int64_t F = 100; F <= 119; ++F)
	{
		const double Y = F < 115 ? 0.8 : 5.0;
		H.Record({F, {{3, 1, 0, {-5, Y, 1.5}, {5, Y, 1.5}}}, {}, {{0, {0, 0, 1.5}}}});
		V.Look(0, F, F <= 115 ? FVec3{0, -1, 0} : Heading(-90.0 + 45.0 * static_cast<double>(F - 115)));
	}
	V.Draw(0, 100, 0.0);
	KCHECK(!V.Swing(0, 112, {0, 1, 0}, 1.5, 70));
	// Facing it at the time, the same late swing cuts.
	FRewindHistory G;
	FCutValidator U(G);
	for (int64_t F = 100; F <= 119; ++F)
	{
		const double Y = F < 115 ? 0.8 : 5.0;
		G.Record({F, {{3, 1, 0, {-5, Y, 1.5}, {5, Y, 1.5}}}, {}, {{0, {0, 0, 1.5}}}});
		U.Look(0, F, {0, 1, 0});
	}
	U.Draw(0, 100, 0.0);
	const std::optional<FCutDecision> Cut = U.Swing(0, 112, {0, -1, 0}, 1.5, 70);  // Its own sample is ignored
	KCHECK(Cut && Cut->JudgedFrame == 112);
}

KTEST(ADogIsJudgedAgainstThePlatesAtItsFrame)
{
	FRewindHistory H;
	// Player 2 strafes along y at 6 m/s; their back plate at y, their chest plate 0.4 m ahead.
	for (int64_t F = 100; F < 120; ++F)
	{
		const double Y = 0.1 * static_cast<double>(F - 100);
		H.Record({F, {}, {{2, 2, {10, Y, 1.3}, {}}, {2, 1, {10.4, Y, 1.3}, {}}, {1, 1, {0, 0, 1.3}, {}}}, {}});
	}
	// A Dog fired by player 1 at frame 110, straight down x at y = 1.0, reaches player 2's back
	// plate first, where they were then. At present they are 0.9 m further on.
	const std::optional<FPlateHit> Hit = ResweepDog(H, 110, 1, {0, 1.0, 1.3}, {20, 1.0, 1.3});
	KCHECK(Hit && Hit->Player == 2 && Hit->Plate == 2 && Hit->JudgedFrame == 110);
	KCHECK_NEAR(Hit->At * 20.0, 10.0 - std::sqrt(0.43 * 0.43), 1e-9);
	KCHECK(!ResweepDog(H, 119, 1, {0, 1.0, 1.3}, {20, 1.0, 1.3}));  // Judged now: a miss
	KCHECK(!ResweepDog(H, 110, 1, {0, 1.0, 1.3}, {5, 1.0, 1.3}));   // Short of them
	KCHECK(!ResweepDog(H, 110, 2, {0, 1.0, 1.3}, {20, 1.0, 1.3}));  // Their own Dog
}

KTEST(GraceCoversTheVictimsLatency)
{
	KCHECK(GraceFrames(0.015) == 4);  // 63 ms
	KCHECK(GraceFrames(0.040) == 6);  // 88 ms
	KCHECK(GraceFrames(0.075) == 8);  // 123 ms
	KCHECK(GraceFrames(0.200) == 9);  // Capped at 150 ms
	FPendingStalls P;
	KCHECK(!P.Stall(2, 9, 500, 0.075, false));
	KCHECK(P.IsPending(2));
	KCHECK(!P.Cut(9, 501));        // A cut judged after the Stall frame is too late
	KCHECK(P.Cut(9, 500));         // One judged at it, applied before its physics, saves them
	KCHECK(!P.IsPending(2));
	KCHECK(P.Stall(3, 9, 600, 0.075, true));  // Over an edge: final at once
	KCHECK(!P.IsPending(3));
	P.Stall(4, 11, 700, 0.075, false);
	KCHECK(P.Confirm(707).empty());
	const std::vector<FPendingStall> Done = P.Confirm(708);
	KCHECK(Done.size() == 1 && Done[0].Victim == 4);
}

KTEST(LatencyPolicyAndCorrections)
{
	FLatencyEstimate L;
	L.Sample(0.100);
	KCHECK_NEAR(L.OneWay(), 0.050, 1e-12);
	for (int I = 0; I < 60; ++I) L.Sample(0.200);
	KCHECK_NEAR(L.RoundTrip(), 0.200, 0.001);
	KCHECK(PolicyFor(0.150) == ELatencyPolicy::Resimulation);
	KCHECK(PolicyFor(0.310) == ELatencyPolicy::PredictiveInterpolation);
	KCHECK(PolicyFor(0.410) == ELatencyPolicy::LeavePools);
	FCorrectionStats Honest, Suspect;
	for (int I = 0; I < 300; ++I)
	{
		Honest.Record(I % 2 == 0);
		Suspect.Record(I % 10 != 0);
	}
	KCHECK(!Honest.Flagged() && Suspect.Flagged());
	FCorrectionStats Few;
	for (int I = 0; I < 50; ++I) Few.Record(true);
	KCHECK(!Few.Flagged());  // Not enough to judge
}

namespace
{
constexpr double EdgeDistanceM = 15.0;

struct FTrialResult
{
	bool bSurvived = false;
	bool bCancelledPending = false;  // A cut took back a pending Stall
	int64_t JudgedAt = -1;           // The cut's judged frame, if it cut
	int64_t ConfirmedAt = -1;        // Frame the Stall was confirmed, if it was
};

// One run of the 05 4.3 drag: a Breaker's Haul from 15 m takes a Rigger over an edge at about
// 2.2 s. The Rigger's back is to the line; they draw 0.6 s and start turning to the line
// ReactSeconds - 0.2 s after the seat reaches them, at 900°/s, and swing at ReactSeconds. The
// client stamps each request with its estimate of server time and sends its camera every
// frame; everything takes one-way latency plus jitter to arrive. The edge is final; a
// line-caused Stall at StallFrame (if set) is pending for the grace window.
FTrialResult RunDrag(double RoundTrip, double Jitter, double ReactSeconds, std::optional<int64_t> StallFrame = {})
{
	FReferenceSim Sim(Tables(), StepSeconds);
	const int Breaker = Sim.AddBody(PlayerAt({0, 0, 0}, EHarnessClass::Breaker, true, true));
	const int Rigger = Sim.AddBody(PlayerAt({15, 0, 0}, EHarnessClass::Rigger, true));
	Sim.Engage(Sim.AddLine(LineOf(EHarnessClass::Breaker, Breaker, ESeat::OwnPlate, Rigger, ESeat::OtherPlate)),
	           ELineMode::Haul);
	const double OneWay = RoundTrip / 2.0;
	const auto Frames = [](double Seconds) { return static_cast<int64_t>(std::llround(Seconds * NetTickHz)); };
	const int64_t DrawStamp = Frames(OneWay + 0.6);
	const int64_t SwingStamp = Frames(OneWay + ReactSeconds);
	const int64_t TurnStamp = SwingStamp - Frames(0.2);
	const int64_t Delay = Frames(OneWay + Jitter);  // Every request, this trial
	const FVec3 Away = FVec3{1, 0.3, 0}.SafeNormal();  // The back plate faces the line
	FRewindHistory History;
	FCutValidator Validator(History);
	FPendingStalls Pending;
	FTrialResult Result;
	std::vector<FVec3> ClientCamera;  // What the client looked at, by its stamp
	bool bCut = false;
	bool bFinal = false;
	for (int64_t F = 0; F < Frames(5.0); ++F)
	{
		Sim.Step();
		const FVec3 A = Sim.Body(Breaker).Position;
		const FVec3 B = Sim.Body(Rigger).Position;
		const FVec3 Eye = B + FVec3{0, 0, 0.4};  // The eyes 0.4 m above the chest plate
		History.Record({F, {{1, 0, 1, A, B}}, {}, {{Rigger, Eye}}});
		// The client's camera at this stamp: away until it turns, then toward the line.
		const double Turned = 900.0 * static_cast<double>(std::max<int64_t>(0, F - TurnStamp)) / NetTickHz;
		ClientCamera.push_back(ClampTurn(Away, (A - Eye).SafeNormal(), Turned));
		if (!bCut && !bFinal && (B - FVec3{15, 0, 0}).Length() >= EdgeDistanceM)
		{
			// Over the edge (the panic bench's 15 m): final, no cut can undo it.
			bFinal = Pending.Stall(Rigger, 1, F, OneWay, true);
			Result.ConfirmedAt = F;
		}
		if (!bCut && !bFinal && StallFrame && F == *StallFrame)
		{
			Pending.Stall(Rigger, 1, F, OneWay, false);  // Pending for the grace window
		}
		const int64_t Stamp = F - Delay;  // The requests arriving now
		if (Stamp >= 0)
		{
			Validator.Look(Rigger, Stamp, ClientCamera[static_cast<size_t>(Stamp)]);
		}
		if (Stamp == DrawStamp)
		{
			Validator.Draw(Rigger, DrawStamp, CutterWarmupSeconds(ECutterVariant::Standard, EHarnessClass::Rigger, Tables()));
		}
		if (Stamp == SwingStamp && !bFinal && !bCut)
		{
			const std::optional<FCutDecision> Cut =
			    Validator.Swing(Rigger, SwingStamp, ClientCamera[static_cast<size_t>(SwingStamp)], 1.5, 70.0);
			const bool bWasPending = Pending.IsPending(Rigger);
			// A cut judged after a pending Stall's frame does not undo it.
			if (Cut && (!bWasPending || Pending.Cut(1, Cut->JudgedFrame)))
			{
				bCut = true;
				Result.bCancelledPending = bWasPending;
				Result.JudgedAt = Cut->JudgedFrame;
			}
		}
		if (!Pending.Confirm(F).empty())
		{
			bFinal = true;
			Result.ConfirmedAt = F;
		}
		if (Result.ConfirmedAt >= 0)
		{
			break;
		}
	}
	Result.bSurvived = bCut && Result.ConfirmedAt < 0;
	return Result;
}

int64_t SwingStampFor(double RoundTrip, double React)
{
	return static_cast<int64_t>(std::llround((RoundTrip / 2.0 + React) * NetTickHz));
}
} // namespace

KTEST(TheValidationMatrixGivesEveryLatencyTheSameOutcome)
{
	// 7.3: 30, 80 and 150 ms RTT, 20 ms jitter, 20 trials each, the victim cutting 1.1 s after
	// the seat: everyone survives the drag, every cut judged at the frame it was swung.
	FRng Rng;
	for (const double Rtt : {0.030, 0.080, 0.150})
	{
		for (int Trial = 0; Trial < 20; ++Trial)
		{
			const double Jitter = Rng.Next() * 0.020;
			const FTrialResult R = RunDrag(Rtt, Jitter, 1.1);
			KCHECK(R.bSurvived && R.JudgedAt == SwingStampFor(Rtt, 1.1));
		}
	}
	// With no reaction the edge takes them, in every bucket alike.
	for (const double Rtt : {0.030, 0.150})
	{
		KCHECK(!RunDrag(Rtt, 0.0, 10.0).bSurvived);
	}
}

KTEST(TheGraceWindowSavesALateArrivingCut)
{
	// A line-caused Stall (an object into the victim) lands the frame after the victim's
	// swing, before the cut arrives (20 ms of jitter keeps the cut 2 frames or more behind at
	// 30 ms). The cut, judged at the swing, takes the pending Stall back. At every bucket.
	for (const double Rtt : {0.030, 0.080, 0.150})
	{
		const double React = 1.1;
		const double Jitter = 0.020;
		const int64_t Stall = SwingStampFor(Rtt, React) + 1;
		const FTrialResult Saved = RunDrag(Rtt, Jitter, React, Stall);
		KCHECK(Saved.bSurvived && Saved.bCancelledPending);
		// A swing stamped after the Stall does not take it back.
		KCHECK(!RunDrag(Rtt, Jitter, React + 2.0 / NetTickHz, Stall).bSurvived);
		// Without a cut the Stall is confirmed G after it: 63, 88 and 123 ms, in whole frames.
		const FTrialResult Late = RunDrag(Rtt, Jitter, 10.0, Stall);
		const int64_t G = Rtt < 0.05 ? 4 : Rtt < 0.1 ? 6 : 8;
		KCHECK(!Late.bSurvived && Late.ConfirmedAt == Stall + G);
	}
}
