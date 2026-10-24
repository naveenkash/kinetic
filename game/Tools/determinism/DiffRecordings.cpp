// Compares two determinism recordings and prints where they first diverge.
//
//   diff-recordings A.txt B.txt [tolerance_m] [body_id ...]
//
// Tolerance (meters, 0 by default) of 0 means bit-identical. Body ids limit the comparison to
// those bodies. Exit status: 0 the same within the tolerance, 1 diverged, 2 bad arguments,
// unreadable, or not comparable (different seeds, ticks or bodies).
#include "Kinetic/Determinism.h"

#include <cerrno>
#include <climits>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

using namespace Kinetic;

namespace
{
std::optional<FDeterminismRecording> Load(const char* Path)
{
	std::ifstream File(Path);
	if (!File)
	{
		std::fprintf(stderr, "diff-recordings: cannot open %s\n", Path);
		return std::nullopt;
	}
	const std::string Text((std::istreambuf_iterator<char>(File)), std::istreambuf_iterator<char>());
	std::optional<FDeterminismRecording> Recording = FDeterminismRecording::Parse(Text);
	if (!Recording)
	{
		std::fprintf(stderr, "diff-recordings: %s is not a recording\n", Path);
	}
	return Recording;
}

bool ParseTolerance(const char* Text, double& Out)
{
	errno = 0;
	char* End = nullptr;
	Out = std::strtod(Text, &End);
	return *Text != '\0' && *End == '\0' && errno == 0 && std::isfinite(Out) && Out >= 0.0;
}

bool ParseBodyId(const char* Text, int& Out)
{
	errno = 0;
	char* End = nullptr;
	const long Value = std::strtol(Text, &End, 10);
	if (*Text == '\0' || *End != '\0' || errno != 0 || Value < 0 || Value > INT_MAX)
	{
		return false;
	}
	Out = static_cast<int>(Value);
	return true;
}
} // namespace

int main(int Argc, char** Argv)
{
	if (Argc < 3)
	{
		std::fprintf(stderr, "usage: diff-recordings A B [tolerance_m] [body_id ...]\n");
		return 2;
	}
	const std::optional<FDeterminismRecording> A = Load(Argv[1]);
	const std::optional<FDeterminismRecording> B = Load(Argv[2]);
	if (!A || !B)
	{
		return 2;
	}
	double Tolerance = 0.0;
	if (Argc > 3 && !ParseTolerance(Argv[3], Tolerance))
	{
		std::fprintf(stderr, "diff-recordings: the tolerance must be a number of meters, 0 or more: %s\n", Argv[3]);
		return 2;
	}
	std::vector<int> Bodies;
	for (int I = 4; I < Argc; ++I)
	{
		int Id = 0;
		if (!ParseBodyId(Argv[I], Id))
		{
			std::fprintf(stderr, "diff-recordings: not a body id: %s\n", Argv[I]);
			return 2;
		}
		Bodies.push_back(Id);
	}
	const FDivergence D = Compare(*A, *B, Tolerance, Bodies);
	if (!D.bComparable)
	{
		std::fprintf(stderr, "diff-recordings: not comparable: %s\n", D.Problem.c_str());
		return 2;
	}
	std::printf("compared %lld ticks; largest position difference %.6g m (body %d, tick %lld)\n",
	            static_cast<long long>(D.ComparedTicks), D.MaxPositionDelta, D.MaxBodyId,
	            static_cast<long long>(D.MaxTick));
	if (!D.bDiverged)
	{
		std::printf("same%s\n", Tolerance > 0.0 ? " within the tolerance" : ", bit for bit");
		return 0;
	}
	std::printf("first divergence at tick %lld, body %d, %.6g m apart\n", static_cast<long long>(D.FirstTick),
	            D.FirstBodyId, D.FirstPositionDelta);
	return 1;
}
