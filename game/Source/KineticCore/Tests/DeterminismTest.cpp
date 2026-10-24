// The determinism recorder: record, write, read, and find the first divergent tick
// (project-structure section 8; architecture-overview week 2; playtest-plan Phase 0 exit).
#include "Kinetic/Determinism.h"
#include "SimFixtures.h"

#include <cmath>
#include <cstring>
#include <limits>

using namespace Kinetic;
using namespace Kinetic::Test;

namespace
{
constexpr uint64_t Seed = 0x5eed;

// Heavy-vs-Light: a braced Breaker Hauls a Climber across the floor (3.3).
FDeterminismRecording HeavyVsLight(double TensionN, int64_t ChangeAtTick = -1)
{
	FReferenceSim Sim(Tables(), StepSeconds);
	const int Breaker = Sim.AddBody(PlayerAt({0, 0, 0}, EHarnessClass::Breaker, true, true));
	const int Climber = Sim.AddBody(PlayerAt({12, 0, 0}, EHarnessClass::Climber, true));
	const int Line = Sim.AddLine(LineOf(EHarnessClass::Breaker, Breaker, ESeat::OwnPlate, Climber, ESeat::OtherPlate));
	Sim.Engage(Line, ELineMode::Haul);
	FDeterminismRecording Recording(Seed);
	for (int64_t Tick = 0; Tick < 120; ++Tick)
	{
		if (Tick == ChangeAtTick)
		{
			Sim.SetTension(Line, TensionN);
		}
		Sim.Step();
		Recording.RecordSim(Tick, Sim, Sim.BodyCount());
	}
	return Recording;
}
} // namespace

KTEST(IdenticalRunsAreBitIdentical)
{
	const FDeterminismRecording A = HeavyVsLight(1000.0);
	const FDeterminismRecording B = HeavyVsLight(1000.0);
	const FDivergence Same = Compare(A, B, 0.0);
	KCHECK(Same.bComparable && !Same.bDiverged && Same.ComparedTicks == 120);
	KCHECK_NEAR(Same.MaxPositionDelta, 0.0, 0.0);
	for (size_t I = 0; I < A.Ticks().size(); ++I)
	{
		KCHECK(A.TickHash(I) == B.TickHash(I));
	}
}

// Writing and reading gives back the same bits.
KTEST(RecordingsRoundTripExactly)
{
	const FDeterminismRecording A = HeavyVsLight(1000.0);
	const std::optional<FDeterminismRecording> Read = FDeterminismRecording::Parse(A.Serialize());
	KCHECK(Read.has_value());
	KCHECK(Read->MatchSeed() == Seed && Read->Ticks().size() == A.Ticks().size());
	KCHECK(Compare(A, *Read, 0.0).bComparable && !Compare(A, *Read, 0.0).bDiverged);
	KCHECK(Read->TickHash(119) == A.TickHash(119));
	KCHECK(Read->Serialize() == A.Serialize());

	KCHECK(!FDeterminismRecording::Parse("").has_value());
	const std::string Zero = "0000000000000000";
	const std::string Body = " " + Zero + " " + Zero + " " + Zero + " " + Zero + " " + Zero + " " + Zero;
	const std::string Head = "kinetic-determinism 2\nseed 1\n";
	KCHECK(FDeterminismRecording::Parse(Head + "tick 0 1\n0" + Body + "\n").has_value());
	KCHECK(FDeterminismRecording::Parse("kinetic-determinism 2\r\nseed 1\r\ntick 0 1\r\n0" + Body + "\r\n").has_value());
	for (const std::string& Bad : {std::string("kinetic-determinism 2\nseed x\n"), std::string("kinetic-determinism 2\nseed -1\n"),
	                              std::string("kinetic-determinism 2\nseed +5\n"), std::string("kinetic-determinism 2\nseed 99999999999999999999\n"),
	                              Head + "tick 0 1\n0 0 0 0 0 0 0\n",            // Not 16-digit bit patterns
	                              Head + "tick 0 2\n0" + Body + "\n",             // Fewer bodies than it says
	                              Head + "tick 0 1\nx" + Body + "\n",             // Not a body id
	                              Head + "tick 0 1\n99999999999" + Body + "\n",   // Out of range
	                              Head + "tick 0 1 7\n0" + Body + "\n",           // Trailing token
	                              Head + "tick 0 1\n0" + Body + " 1\n",
	                              Head + "tick 0 2\n0" + Body + "\n0" + Body + "\n",  // A body twice
	                              Head + "tick 0 1\n0" + Body + "\ntick 0 1\n0" + Body + "\n",  // A tick twice
	                              Head + "tick 0 100001\n"})
	{
		KCHECK(!FDeterminismRecording::Parse(Bad).has_value());
	}
}

// Every bit pattern comes back, NaN payloads and signs included.
KTEST(EveryDoubleRoundTrips)
{
	const uint64_t Patterns[] = {0x0000000000000000ull, 0x8000000000000000ull, 0x0000000000000001ull, 0x7ff0000000000000ull,
	                             0xfff0000000000000ull, 0x7ff8000000000000ull, 0xfff8000000000000ull, 0x7ff8000000000001ull,
	                             0x7fefffffffffffffull, 0x3fb999999999999aull};
	FDeterminismRecording Recording(9);
	int Id = 0;
	std::vector<FBodySample> Bodies;
	for (const uint64_t Bits : Patterns)
	{
		double Value;
		std::memcpy(&Value, &Bits, sizeof(Value));
		Bodies.push_back({Id++, {Value, -Value, 0.5}, {Value, 1.0, Value}});
	}
	KCHECK(Recording.Record(0, Bodies));
	const std::optional<FDeterminismRecording> Read = FDeterminismRecording::Parse(Recording.Serialize());
	KCHECK(Read.has_value() && Read->TickHash(0) == Recording.TickHash(0));
	KCHECK(Read->Serialize() == Recording.Serialize());
}

// Ticks stay in order; a re-simulated tick replaces the earlier one; a body twice is refused.
// Whatever Record accepts, Serialize writes and Parse reads back: negative ticks and ids too.
KTEST(EveryRecordingParsesBack)
{
	FDeterminismRecording Recording(UINT64_MAX);
	Recording.Record(INT64_MIN, {{-5, {1, 2, 3}, {}}, {INT32_MAX, {}, {}}});
	Recording.Record(INT64_MAX, {{INT32_MIN, {}, {}}});
	const std::optional<FDeterminismRecording> Read = FDeterminismRecording::Parse(Recording.Serialize());
	KCHECK(Read.has_value() && Read->Serialize() == Recording.Serialize());
	KCHECK(Read->Ticks().front().Tick == INT64_MIN && Read->Ticks().front().Bodies.front().BodyId == -5);
}

KTEST(RecordingKeepsOneStatePerTick)
{
	FDeterminismRecording Recording(1);
	KCHECK(Recording.Record(10, {{0, {1, 0, 0}, {}}}));
	KCHECK(Recording.Record(5, {{0, {2, 0, 0}, {}}}));
	KCHECK(Recording.Record(10, {{0, {3, 0, 0}, {}}}));
	KCHECK(Recording.Ticks().size() == 2 && Recording.Ticks()[0].Tick == 5 && Recording.Ticks()[1].Tick == 10);
	KCHECK(Recording.Ticks()[1].Bodies[0].Position.X == 3.0);
	KCHECK(!Recording.Record(11, {{0, {}, {}}, {0, {}, {}}}));
	KCHECK(Recording.Ticks().size() == 2);
}

// A change from tick 60 is reported at tick 60, on the body it moved.
KTEST(TheFirstDivergentTickIsFound)
{
	const FDeterminismRecording A = HeavyVsLight(1000.0);
	const FDeterminismRecording B = HeavyVsLight(1001.0, 60);
	const FDivergence Exact = Compare(A, B, 0.0);
	KCHECK(Exact.bComparable && Exact.bDiverged && Exact.FirstTick == 60);
	KCHECK(Exact.FirstBodyId == 1);  // The braced Breaker does not move; the Climber does
	KCHECK(Exact.MaxTick == 119 && Exact.MaxBodyId == 1 && Exact.MaxPositionDelta > 0.0);

	// The Phase 0 pass mark: under 10 cm.
	const FDivergence Loose = Compare(A, B, 0.10);
	KCHECK(!Loose.bDiverged && Loose.MaxPositionDelta < 0.10);
	const FDivergence Far = Compare(A, HeavyVsLight(1500.0, 30), 0.10);
	KCHECK(Far.bDiverged && Far.FirstTick > 30 && Far.FirstPositionDelta > 0.10);

	// Only the listed bodies count.
	KCHECK(!Compare(A, B, 0.0, {0}).bDiverged);
}

KTEST(RecordingsMustMatchToCompare)
{
	const FDeterminismRecording A = HeavyVsLight(1000.0);
	FDeterminismRecording OtherSeed(Seed + 1);
	OtherSeed.Record(0, {});
	const FDivergence Seeds = Compare(A, OtherSeed, 0.0);
	KCHECK(!Seeds.bComparable && Seeds.Problem == "different match seeds");

	// Ticks only one recording has: a shorter run, a shifted one, an empty one.
	FDeterminismRecording Shorter(Seed);
	for (size_t I = 0; I < 60; ++I)
	{
		Shorter.Record(A.Ticks()[I].Tick, A.Ticks()[I].Bodies);
	}
	const FDivergence Short = Compare(A, Shorter, 0.0);
	KCHECK(!Short.bComparable && Short.Problem == "tick 60 is only in A");
	FDeterminismRecording Shifted(Seed);
	for (const FTickSample& Sample : A.Ticks())
	{
		Shifted.Record(Sample.Tick + 1, Sample.Bodies);
	}
	KCHECK(!Compare(A, Shifted, 0.0).bComparable);
	KCHECK(!Compare(A, FDeterminismRecording(Seed), 0.0).bComparable);
	KCHECK(!Compare(FDeterminismRecording(Seed), FDeterminismRecording(Seed), 0.0).bComparable);

	// Different bodies: not comparable, unless only bodies outside the filter differ.
	FDeterminismRecording Extra(Seed);
	for (const FTickSample& Sample : A.Ticks())
	{
		std::vector<FBodySample> Bodies = Sample.Bodies;
		Bodies.push_back({7, {}, {}});
		Extra.Record(Sample.Tick, Bodies);
	}
	KCHECK(!Compare(A, Extra, 0.0).bComparable);
	const FDivergence Filtered = Compare(A, Extra, 0.0, {0, 1});
	KCHECK(Filtered.bComparable && !Filtered.bDiverged);
	KCHECK(!Compare(A, Extra, 0.0, {7}).bComparable);  // Body 7 is missing from A
	const FDivergence MissingFromB = Compare(Extra, A, 0.0, {7});
	KCHECK(!MissingFromB.bComparable && MissingFromB.Problem == "body 7 is missing at tick 0");
	const FDivergence OnlyB = Compare(Shorter, A, 0.0);
	KCHECK(!OnlyB.bComparable && OnlyB.Problem == "tick 60 is only in B");

	KCHECK(!Compare(A, A, -0.1).bComparable);
	KCHECK(!Compare(A, A, std::nan("")).bComparable);
}

// A blown-up body (NaN) is always a divergence, at any tolerance.
KTEST(NaNIsADivergence)
{
	FDeterminismRecording A(1);
	FDeterminismRecording B(1);
	A.Record(0, {{0, {0, 0, 0}, {}}});
	B.Record(0, {{0, {std::nan(""), 0, 0}, {}}});
	for (const double Tolerance : {0.0, 0.1, 1e9})
	{
		const FDivergence D = Compare(A, B, Tolerance);
		KCHECK(D.bComparable && D.bDiverged && D.FirstTick == 0 && std::isnan(D.MaxPositionDelta));
		KCHECK(D.MaxTick == 0 && D.MaxBodyId == 0);
	}
	// The same NaN in both, bit for bit, is no difference.
	const FDivergence Same = Compare(B, B, 0.0);
	KCHECK(!Same.bDiverged && Same.MaxPositionDelta == 0.0);
}

// Tolerance 0 is bit-exact on velocity as well as position; above 0, a delta exactly at the
// tolerance passes; the tick hash covers the tick and the body ids.
KTEST(CompareAndHashSeeEverything)
{
	FDeterminismRecording A(1);
	FDeterminismRecording B(1);
	A.Record(3, {{0, {0, 0, 0}, {1, 0, 0}}});
	B.Record(3, {{0, {0, 0, 0}, {1, 0, std::numeric_limits<double>::denorm_min()}}});
	KCHECK(Compare(A, B, 0.0).bDiverged && Compare(A, B, 0.0).FirstTick == 3);
	KCHECK(A.TickHash(0) != B.TickHash(0));

	FDeterminismRecording Near(1);
	Near.Record(3, {{0, {0.25, 0, 0}, {1, 0, 0}}});
	KCHECK(!Compare(A, Near, 0.25).bDiverged && Compare(A, Near, 0.2499).bDiverged);
	KCHECK(Compare(A, Near, 0.2499).MaxTick == 3 && Compare(A, Near, 0.2499).MaxBodyId == 0);

	FDeterminismRecording OtherTick(1);
	OtherTick.Record(4, {{0, {0, 0, 0}, {1, 0, 0}}});
	FDeterminismRecording OtherId(1);
	OtherId.Record(3, {{1, {0, 0, 0}, {1, 0, 0}}});
	KCHECK(A.TickHash(0) != OtherTick.TickHash(0) && A.TickHash(0) != OtherId.TickHash(0));
}
