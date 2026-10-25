// Lag compensation and the panic window (docs/tech/networking.md sections 6, 7 and 11):
// - FRewindHistory: 20 frames (333 ms) of every line segment, Catch Plate and player's eye, so
//   a cut or a Dog hit is judged at the client's frame, rewound at most 9 frames (150 ms).
// - FCutValidator: the server side of the Cutter (7.1 steps 1 to 4). It keeps each player's
//   camera from their input stream, turning it at most 3,000°/s between the frames it is
//   judged at, so no stamp can switch the clamp off, and remembers it per frame. Warm-up
//   starts at the judged draw frame. A swing is tested for reach and cone with the player's
//   eye, camera and the lines all as they were at the judged swing frame (a client that has
//   since turned cannot aim after the fact), and every swing, hit or miss, takes the Cutter's swing
//   and recovery (0.45 s, 27 frames) before the next: longer than the rewind, so a client
//   cannot try each frame of the window in turn.
// - ResweepDog: 7.2, a Dog's flight this frame tested against the plates at the client's frame.
// - FPendingStalls: the grace window G = min(150 ms, one-way latency + 33 ms + 15 ms) (7.1
//   steps 5 to 7), rounded up to whole frames in the victim's favour. A line-caused Stall is
//   pending for G; a valid cut judged at or before the Stall frame cancels it. A cut at the
//   Stall frame wins: a frame's inputs are applied before its physics step, which is where
//   the Stall is found. Going over an edge is never pending.
// - The latency policy (6) and misprediction statistics (11).
#pragma once

#include "Kinetic/Api.h"

#include "Kinetic/Cutter.h"
#include "Kinetic/Vec3.h"

#include <cstdint>
#include <optional>
#include <utility>
#include <vector>

namespace Kinetic
{
inline constexpr double NetTickHz = 60.0;
inline constexpr int HistoryFrames = 20;    // 333 ms
inline constexpr int MaxRewindFrames = 9;   // 150 ms

struct FLineSegment
{
	int LineId = 0;
	int OwnerPlayer = -1;
	int SeatedPlayer = -1;  // The player whose plate holds a Dog of this line, or -1
	FVec3 A, B;
};

struct FPlateState
{
	int Player = 0;
	int Plate = 0;  // EPlate
	FVec3 Position;
	FVec3 Normal;
};

struct FPlayerPose
{
	int Player = 0;
	FVec3 Eye;  // The blade's origin for the cut test
};

struct FFrameSnapshot
{
	int64_t Frame = 0;
	std::vector<FLineSegment> Lines;
	std::vector<FPlateState> Plates;
	std::vector<FPlayerPose> Players;
};

class KINETICCORE_API FRewindHistory
{
public:
	void Record(FFrameSnapshot Snapshot);  // Frames in ascending order
	int64_t Present() const { return Newest; }
	// The frame a request stamped ClientFrame is judged at: never later than now, never more
	// than MaxRewindFrames back, never older than the history holds.
	int64_t JudgeFrame(int64_t ClientFrame) const;
	const FFrameSnapshot* At(int64_t Frame) const;
	std::optional<FLineSegment> LineAt(int64_t Frame, int LineId) const;
	std::optional<FVec3> EyeAt(int64_t Frame, int Player) const;

private:
	std::vector<FFrameSnapshot> Ring;
	int64_t Newest = -1;
};

struct FCutDecision
{
	int LineId = 0;
	double At = 0.0;          // Along the segment, 0 at A
	int64_t JudgedFrame = 0;  // The cut's timestamp: the frame it is judged at
};

class KINETICCORE_API FCutValidator
{
public:
	explicit FCutValidator(const FRewindHistory& InHistory) : History(InHistory) {}

	// Every input packet's camera. The first sample for a player is taken as it is (the server
	// sets it at spawn); after that the camera turns toward each sample by at most the clamp
	// for the judged frames since the last, and never runs backward in frames. A sample that is
	// not a finite, non-zero direction is ignored.
	void Look(int Player, int64_t ClientFrame, const FVec3& Forward);
	std::optional<FVec3> Camera(int Player) const;
	// ServerCutterDraw: warm-up runs from the client's draw frame (rewound, capped). A new
	// draw restarts it.
	void Draw(int Player, int64_t ClientFrame, double WarmupSeconds);
	void Sheathe(int Player);
	// ServerCutSwing: which line it cuts, if any. The swing's camera is one more Look when it is
	// stamped at or after the latest camera sample; otherwise the camera recorded for the judged
	// frame is used and the swing's own is ignored.
	std::optional<FCutDecision> Swing(int Player, int64_t ClientFrame, const FVec3& Forward, double Reach,
	                                  double ConeDeg);

	static constexpr double MaxTurnDegPerSecond = 3000.0;
	static constexpr int MaxTurnGapFrames = 3;  // A late or lost packet earns no more turn than this (PROPOSED)
	static constexpr double SwingCycleSeconds = 0.45;  // Swing and recovery, 05 4.1 (every variant)

private:
	struct FPlayer
	{
		int Player = 0;
		bool bCamera = false;
		int64_t CameraFrame = 0;
		FVec3 Camera{1, 0, 0};
		bool bDrawn = false;
		int64_t ReadyFrame = 0;
		int64_t NextSwingFrame = INT64_MIN;
		std::vector<std::pair<int64_t, FVec3>> Cameras;  // The camera as of each judged frame, the last HistoryFrames
	};
	FPlayer& PlayerFor(int Player);
	FVec3 Turn(FPlayer& P, int64_t Frame, const FVec3& Forward);
	static std::optional<FVec3> CameraAt(const FPlayer& P, int64_t Frame);

	const FRewindHistory& History;
	std::vector<FPlayer> Players;
};

// Turns From toward To by at most MaxDegrees (unit vectors in, unit vector out). Turning to
// the exact opposite goes by way of an arbitrary perpendicular.
KINETICCORE_API FVec3 ClampTurn(const FVec3& From, const FVec3& To, double MaxDegrees);

// 7.2: a Dog's sweep this frame, From to To, against the other players' plates at the client's
// frame. The sweep comes from the server's own simulation of the projectile, launched from the
// shooter's rewound position at the class Dog speed; never from the client. A Dog seats in the first player it reaches, in that player's plate nearest the point
// of impact (05 5.5). The plates are spheres of PlateHitRadiusM about the plate (PROPOSED).
struct FPlateHit
{
	int Player = 0;
	int Plate = 0;
	double At = 0.0;  // Along the sweep, 0 at From
	int64_t JudgedFrame = 0;
};
inline constexpr double DogRadiusM = 0.08;  // 05 3.1
inline constexpr double PlateHitRadiusM = 0.35;
KINETICCORE_API std::optional<FPlateHit> ResweepDog(const FRewindHistory& History, int64_t ClientFrame, int Shooter, const FVec3& From,
                                    const FVec3& To);

// The grace window for a victim at this one-way latency, in frames (7.1 step 5).
KINETICCORE_API int GraceFrames(double OneWayLatencySeconds);

struct FPendingStall
{
	int Victim = 0;
	int CausingLine = 0;
	int64_t StallFrame = 0;
	int64_t ConfirmFrame = 0;
};

class KINETICCORE_API FPendingStalls
{
public:
	// A line-caused Stall: pending for G (returns false), unless it was going over an edge,
	// which is final at once (returns true: confirmed now).
	bool Stall(int Victim, int CausingLine, int64_t StallFrame, double OneWayLatencySeconds, bool bOverAnEdge);
	// A valid cut arrived: cancels the pending Stalls caused by that line if the cut's judged
	// frame is at or before the Stall frame. Returns true if a Stall was cancelled.
	bool Cut(int LineId, int64_t JudgedFrame);
	// The Stalls whose grace ran out by Present: confirmed now. Run it after the frame's cuts.
	std::vector<FPendingStall> Confirm(int64_t Present);
	bool IsPending(int Victim) const;

private:
	std::vector<FPendingStall> Pending;
};

// One-way latency from round trips, smoothed (the grace window and the policy read it).
class KINETICCORE_API FLatencyEstimate
{
public:
	void Sample(double RoundTripSeconds);
	double OneWay() const { return Smoothed / 2.0; }
	double RoundTrip() const { return Smoothed; }
	static constexpr double Smoothing = 0.125;  // As TCP's SRTT

private:
	double Smoothed = 0.0;
	bool bAny = false;
};

// Section 6: over 300 ms a client's own flight body moves to Predictive Interpolation and the
// player is warned; over 400 ms they leave the region's matchmaking pools.
enum class ELatencyPolicy : uint8_t { Resimulation, PredictiveInterpolation, LeavePools };
KINETICCORE_API ELatencyPolicy PolicyFor(double RoundTripSeconds);

// Section 11: per-player correction statistics. Each time the server corrects a player's own
// prediction, record whether the prediction was better for the player than the server's state
// (a speed, an escape or a seat the server did not grant). Honest mispredictions fall both
// ways; a tampered client's fall its own way. A player with enough corrections, most of them
// in their own favour, is flagged (PROPOSED thresholds).
class FCorrectionStats
{
public:
	void Record(bool bPredictionFavouredPlayer) { ++Count; Favoured += bPredictionFavouredPlayer ? 1 : 0; }
	bool Flagged() const { return Count >= MinSamples && Favoured >= FlagShare * Count; }
	static constexpr int MinSamples = 200;
	static constexpr double FlagShare = 0.7;

private:
	int Count = 0;
	int Favoured = 0;
};
} // namespace Kinetic
