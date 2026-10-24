// The panic-window bench (project-structure section 8): for every attacker and victim class,
// a Haul from 15 m by a braced or anchored attacker on a grounded victim. Prints when the
// victim's feet leave the floor and when they have been dragged 15 m, against the design
// target of 1.0 to 2.0 s from Dog seat to the point of no return (bible/05 section 4.3).
#include "Kinetic/ReferenceSim.h"

#include <cstdio>
#include <exception>
#include <string>

namespace
{
constexpr double DistanceM = 15.0;
constexpr double StepSeconds = 1.0 / 60.0;
constexpr double MaxSeconds = 10.0;

std::string Seconds(const std::optional<double>& Value)
{
	if (!Value)
	{
		return "never";
	}
	char Buffer[16];
	std::snprintf(Buffer, sizeof(Buffer), "%.2f s", *Value);
	return Buffer;
}
} // namespace

int main(int Argc, char** Argv)
{
	using namespace Kinetic;
	const char* TablesDir = Argc > 1 ? Argv[1] : KINETIC_TABLES_DIR;
	try
	{
		const FKineticTables Tables = FKineticTables::LoadFromDirectory(TablesDir);
		std::printf("Haul from %.0f m, attacker braced or anchored, victim grounded\n\n", DistanceM);
		std::printf("%-9s %-9s %10s %12s %10s\n", "Attacker", "Victim", "Feet off", "At 15 m", "Speed");
		for (const EHarnessClass Attacker : AllHarnessClasses)
		{
			for (const EHarnessClass Victim : AllHarnessClasses)
			{
				const FPanicRun Run = RunHaulPanic(Tables, Attacker, Victim, DistanceM, StepSeconds, MaxSeconds);
				std::printf("%-9s %-9s %10s %12s %7.1f m/s\n", std::string(ToString(Attacker)).c_str(),
				            std::string(ToString(Victim)).c_str(), Seconds(Run.FeetOffAt).c_str(),
				            Seconds(Run.DistanceAt).c_str(), Run.SpeedAtDistance);
			}
		}
		return 0;
	}
	catch (const std::exception& Error)
	{
		std::fprintf(stderr, "panic-bench: %s\n", Error.what());
		return 1;
	}
}
