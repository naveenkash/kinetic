// The determinism recorder (project-structure section 8; architecture-overview risk 4): records
// every body's state each tick on the server and on each client, with a shared match seed, and
// finds where two recordings first diverge. It backs the Phase 0 week 2 and week 5 tests
// ("Heavy-vs-Light pull resolves identically on both clients: record positions, diff"; the
// playtest-plan pass mark is under 10 cm at the line end).
//
// Recordings are text with every double written as its raw 64-bit pattern, so a write and a
// read give back the same bits (NaN payloads included), whatever the locale; a diff of zero
// means bit-identical. Ticks are kept in order; recording a tick again (a client re-simulating
// after a correction) replaces it.
#pragma once

#include "Kinetic/Vec3.h"

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace Kinetic
{
class FReferenceSim;

struct FBodySample
{
	int BodyId = 0;
	FVec3 Position;
	FVec3 Velocity;
};

struct FTickSample
{
	int64_t Tick = 0;
	std::vector<FBodySample> Bodies;  // In body id order
};

class FDeterminismRecording
{
public:
	explicit FDeterminismRecording(uint64_t InSeed = 0) : Seed(InSeed) {}

	// Returns false, recording nothing, if a body id appears twice.
	bool Record(int64_t Tick, std::vector<FBodySample> Bodies);
	void RecordSim(int64_t Tick, const FReferenceSim& Sim, int BodyCount);

	uint64_t MatchSeed() const { return Seed; }
	bool Has(int64_t Tick) const;
	const std::vector<FTickSample>& Ticks() const { return Samples; }
	uint64_t TickHash(size_t Index) const;  // FNV-1a over the tick's bits: compare ticks cheaply

	std::string Serialize() const;
	static std::optional<FDeterminismRecording> Parse(const std::string& Text);  // nullopt if malformed

private:
	uint64_t Seed = 0;
	std::vector<FTickSample> Samples;
};

struct FDivergence
{
	// Same seed, the same ticks, and in every tick the bodies that count. A tick only one
	// recording has, or a body only one has, means the runs cannot be compared.
	bool bComparable = true;
	std::string Problem;           // Why not, if not comparable
	bool bDiverged = false;        // Some position differs by more than the tolerance, or is NaN
	int64_t FirstTick = -1;
	int FirstBodyId = -1;
	double FirstPositionDelta = 0.0;
	double MaxPositionDelta = 0.0;  // Over every compared tick and body; NaN if any position is
	int MaxBodyId = -1;
	int64_t MaxTick = -1;
	int64_t ComparedTicks = 0;
};

// Compares two recordings tick by tick. ToleranceM 0 means bit-identical positions and
// velocities; above 0, positions within the tolerance count as equal. Only the listed bodies
// count when BodyIds is not empty; each must be in every tick of both.
FDivergence Compare(const FDeterminismRecording& A, const FDeterminismRecording& B, double ToleranceM,
                    const std::vector<int>& BodyIds = {});
} // namespace Kinetic
