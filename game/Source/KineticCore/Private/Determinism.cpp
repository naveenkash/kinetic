#include "Kinetic/Determinism.h"

#include "Kinetic/ReferenceSim.h"

#include <algorithm>
#include <cctype>
#include <cerrno>
#include <climits>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <sstream>

namespace Kinetic
{
namespace
{
constexpr uint64_t FnvOffset = 14695981039346656037ull;
constexpr uint64_t FnvPrime = 1099511628211ull;
constexpr const char* Header = "kinetic-determinism 2";
constexpr size_t MaxBodiesPerTick = 100000;

uint64_t Mix(uint64_t Hash, const void* Data, size_t Bytes)
{
	const auto* P = static_cast<const unsigned char*>(Data);
	for (size_t I = 0; I < Bytes; ++I)
	{
		Hash = (Hash ^ P[I]) * FnvPrime;
	}
	return Hash;
}

uint64_t BitsOf(double Value)
{
	uint64_t Bits;
	std::memcpy(&Bits, &Value, sizeof(Bits));
	return Bits;
}

double FromBits(uint64_t Bits)
{
	double Value;
	std::memcpy(&Value, &Bits, sizeof(Value));
	return Value;
}

std::string Hex(double Value)
{
	static constexpr char Digits[] = "0123456789abcdef";
	const uint64_t Bits = BitsOf(Value);
	std::string Out(16, '0');
	for (int I = 0; I < 16; ++I)
	{
		Out[15 - I] = Digits[(Bits >> (4 * I)) & 0xf];
	}
	return Out;
}

// Strict unsigned parse: digits only (no sign, no space), the whole token, in range.
bool ParseUnsigned(const std::string& Token, int Base, uint64_t Max, uint64_t& Out)
{
	if (Token.empty() || Token.size() > 20)
	{
		return false;
	}
	for (const char C : Token)
	{
		if (!(Base == 16 ? std::isxdigit(static_cast<unsigned char>(C)) : std::isdigit(static_cast<unsigned char>(C))))
		{
			return false;
		}
	}
	errno = 0;
	char* End = nullptr;
	const unsigned long long Value = std::strtoull(Token.c_str(), &End, Base);
	if (errno != 0 || End != Token.c_str() + Token.size() || Value > Max)
	{
		return false;
	}
	Out = Value;
	return true;
}

bool ParseSigned(const std::string& Token, int64_t Min, int64_t Max, int64_t& Out)
{
	const bool bNegative = !Token.empty() && Token[0] == '-';
	const uint64_t Limit = bNegative ? uint64_t(-(Min + 1)) + 1 : uint64_t(Max);
	uint64_t Magnitude = 0;
	if (!ParseUnsigned(bNegative ? Token.substr(1) : Token, 10, Limit, Magnitude))
	{
		return false;
	}
	Out = bNegative ? static_cast<int64_t>(0 - Magnitude) : static_cast<int64_t>(Magnitude);
	return true;
}

// Splits a line into exactly Count whitespace-separated tokens.
bool Tokens(const std::string& Line, size_t Count, std::vector<std::string>& Out)
{
	std::istringstream In(Line);
	Out.clear();
	std::string Token;
	while (In >> Token)
	{
		Out.push_back(Token);
	}
	return Out.size() == Count;
}

bool NextLine(std::istringstream& In, std::string& Line)
{
	if (!std::getline(In, Line))
	{
		return false;
	}
	if (!Line.empty() && Line.back() == '\r')
	{
		Line.pop_back();  // Written on Windows
	}
	return true;
}

bool SameBits(const FVec3& A, const FVec3& B)
{
	return BitsOf(A.X) == BitsOf(B.X) && BitsOf(A.Y) == BitsOf(B.Y) && BitsOf(A.Z) == BitsOf(B.Z);
}

const FBodySample* FindBody(const FTickSample& Sample, int BodyId)
{
	const auto Found = std::lower_bound(Sample.Bodies.begin(), Sample.Bodies.end(), BodyId,
	                                    [](const FBodySample& B, int Id) { return B.BodyId < Id; });
	return Found != Sample.Bodies.end() && Found->BodyId == BodyId ? &*Found : nullptr;
}

std::vector<int> IdsOf(const FTickSample& Sample)
{
	std::vector<int> Ids;
	for (const FBodySample& Body : Sample.Bodies)
	{
		Ids.push_back(Body.BodyId);
	}
	return Ids;
}
} // namespace

bool FDeterminismRecording::Record(int64_t Tick, std::vector<FBodySample> Bodies)
{
	std::sort(Bodies.begin(), Bodies.end(), [](const FBodySample& L, const FBodySample& R) { return L.BodyId < R.BodyId; });
	const bool bDuplicate = std::adjacent_find(Bodies.begin(), Bodies.end(), [](const FBodySample& L, const FBodySample& R) {
		                        return L.BodyId == R.BodyId;
	                        }) != Bodies.end();
	if (bDuplicate)
	{
		return false;
	}
	if (Samples.empty() || Samples.back().Tick < Tick)
	{
		Samples.push_back({Tick, std::move(Bodies)});  // The usual case: ticks arrive in order
		return true;
	}
	const auto At = std::lower_bound(Samples.begin(), Samples.end(), Tick,
	                                 [](const FTickSample& S, int64_t T) { return S.Tick < T; });
	if (At != Samples.end() && At->Tick == Tick)
	{
		At->Bodies = std::move(Bodies);  // Re-simulated: the later state replaces it
	}
	else
	{
		Samples.insert(At, {Tick, std::move(Bodies)});
	}
	return true;
}

void FDeterminismRecording::RecordSim(int64_t Tick, const FReferenceSim& Sim, int BodyCount)
{
	std::vector<FBodySample> Bodies;
	for (int I = 0; I < BodyCount; ++I)
	{
		Bodies.push_back({I, Sim.Body(I).Position, Sim.Body(I).Velocity});
	}
	Record(Tick, std::move(Bodies));
}

bool FDeterminismRecording::Has(int64_t Tick) const
{
	const auto At = std::lower_bound(Samples.begin(), Samples.end(), Tick,
	                                 [](const FTickSample& S, int64_t T) { return S.Tick < T; });
	return At != Samples.end() && At->Tick == Tick;
}

uint64_t FDeterminismRecording::TickHash(size_t Index) const
{
	uint64_t Hash = FnvOffset;
	const FTickSample& Sample = Samples.at(Index);
	Hash = Mix(Hash, &Sample.Tick, sizeof(Sample.Tick));
	for (const FBodySample& Body : Sample.Bodies)
	{
		const double Values[6] = {Body.Position.X, Body.Position.Y, Body.Position.Z,
		                          Body.Velocity.X, Body.Velocity.Y, Body.Velocity.Z};
		Hash = Mix(Hash, &Body.BodyId, sizeof(Body.BodyId));
		Hash = Mix(Hash, Values, sizeof(Values));
	}
	return Hash;
}

std::string FDeterminismRecording::Serialize() const
{
	std::string Out = std::string(Header) + "\nseed " + std::to_string(Seed) + "\n";
	for (const FTickSample& Sample : Samples)
	{
		Out += "tick " + std::to_string(Sample.Tick) + " " + std::to_string(Sample.Bodies.size()) + "\n";
		for (const FBodySample& B : Sample.Bodies)
		{
			Out += std::to_string(B.BodyId) + " " + Hex(B.Position.X) + " " + Hex(B.Position.Y) + " " +
			       Hex(B.Position.Z) + " " + Hex(B.Velocity.X) + " " + Hex(B.Velocity.Y) + " " + Hex(B.Velocity.Z) +
			       "\n";
		}
	}
	return Out;
}

std::optional<FDeterminismRecording> FDeterminismRecording::Parse(const std::string& Text)
{
	std::istringstream In(Text);
	std::string Line;
	std::vector<std::string> T;
	uint64_t ParsedSeed = 0;
	if (!NextLine(In, Line) || Line != Header || !NextLine(In, Line) || !Tokens(Line, 2, T) || T[0] != "seed" ||
	    !ParseUnsigned(T[1], 10, UINT64_MAX, ParsedSeed))
	{
		return std::nullopt;
	}
	FDeterminismRecording Out(ParsedSeed);
	while (NextLine(In, Line))
	{
		int64_t Tick = 0;
		uint64_t Count = 0;
		if (!Tokens(Line, 3, T) || T[0] != "tick" || !ParseSigned(T[1], INT64_MIN, INT64_MAX, Tick) ||
		    !ParseUnsigned(T[2], 10, MaxBodiesPerTick, Count))
		{
			return std::nullopt;
		}
		if (Out.Has(Tick))
		{
			return std::nullopt;  // A file holds each tick once
		}
		std::vector<FBodySample> Bodies(Count);
		for (FBodySample& Body : Bodies)
		{
			int64_t Id = 0;
			uint64_t Bits[6];
			if (!NextLine(In, Line) || !Tokens(Line, 7, T) || !ParseSigned(T[0], INT_MIN, INT_MAX, Id))
			{
				return std::nullopt;
			}
			for (int I = 0; I < 6; ++I)
			{
				if (T[I + 1].size() != 16 || !ParseUnsigned(T[I + 1], 16, UINT64_MAX, Bits[I]))
				{
					return std::nullopt;
				}
			}
			Body.BodyId = static_cast<int>(Id);
			Body.Position = {FromBits(Bits[0]), FromBits(Bits[1]), FromBits(Bits[2])};
			Body.Velocity = {FromBits(Bits[3]), FromBits(Bits[4]), FromBits(Bits[5])};
		}
		if (!Out.Record(Tick, std::move(Bodies)))
		{
			return std::nullopt;
		}
	}
	return Out;
}

FDivergence Compare(const FDeterminismRecording& A, const FDeterminismRecording& B, double ToleranceM,
                    const std::vector<int>& BodyIds)
{
	FDivergence Out;
	const auto Fail = [&Out](std::string Problem) {
		Out.bComparable = false;
		Out.Problem = std::move(Problem);
		return Out;
	};
	if (A.MatchSeed() != B.MatchSeed())
	{
		return Fail("different match seeds");
	}
	if (!(ToleranceM >= 0.0) || !std::isfinite(ToleranceM))
	{
		return Fail("the tolerance must be a finite number of meters, 0 or more");
	}
	const std::vector<FTickSample>& TicksA = A.Ticks();
	const std::vector<FTickSample>& TicksB = B.Ticks();
	if (TicksA.empty() && TicksB.empty())
	{
		return Fail("no ticks recorded");
	}
	for (size_t I = 0; I < std::max(TicksA.size(), TicksB.size()); ++I)
	{
		if (I >= TicksA.size() || I >= TicksB.size() || TicksA[I].Tick != TicksB[I].Tick)
		{
			const bool bOnlyB = I >= TicksA.size() || (I < TicksB.size() && TicksB[I].Tick < TicksA[I].Tick);
			const int64_t Tick = bOnlyB ? TicksB[I].Tick : TicksA[I].Tick;
			return Fail("tick " + std::to_string(Tick) + " is only in " + (bOnlyB ? "B" : "A"));
		}
		const FTickSample& SampleA = TicksA[I];
		const FTickSample& SampleB = TicksB[I];
		const std::vector<int> Ids = BodyIds.empty() ? IdsOf(SampleA) : BodyIds;
		if (BodyIds.empty() && Ids != IdsOf(SampleB))
		{
			return Fail("different bodies at tick " + std::to_string(SampleA.Tick));
		}
		++Out.ComparedTicks;
		for (const int Id : Ids)
		{
			const FBodySample* BodyA = FindBody(SampleA, Id);
			const FBodySample* BodyB = FindBody(SampleB, Id);
			if (!BodyA || !BodyB)
			{
				return Fail("body " + std::to_string(Id) + " is missing at tick " + std::to_string(SampleA.Tick));
			}
			const double Delta = (BodyA->Position - BodyB->Position).Length();
			const bool bSamePosition = SameBits(BodyA->Position, BodyB->Position);
			const bool bDiffers = ToleranceM == 0.0 ? !bSamePosition || !SameBits(BodyA->Velocity, BodyB->Velocity)
			                                        : !(Delta <= ToleranceM);
			// NaN sticks: a blown-up body is the worst case, unless both recordings hold the same bits.
			const bool bWorse = std::isnan(Delta) ? !bSamePosition && !std::isnan(Out.MaxPositionDelta)
			                                      : Delta > Out.MaxPositionDelta;
			if (bWorse)
			{
				Out.MaxPositionDelta = Delta;
				Out.MaxBodyId = Id;
				Out.MaxTick = SampleA.Tick;
			}
			if (bDiffers && !Out.bDiverged)
			{
				Out.bDiverged = true;
				Out.FirstTick = SampleA.Tick;
				Out.FirstBodyId = Id;
				Out.FirstPositionDelta = Delta;
			}
		}
	}
	return Out;
}
} // namespace Kinetic
