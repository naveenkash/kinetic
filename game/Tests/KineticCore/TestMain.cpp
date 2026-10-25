#include "TestHarness.h"

#include <cstdio>
#include <exception>

#ifndef KINETIC_TABLES_DIR
#error "Build with -DKINETIC_TABLES_DIR pointing at Content/Core/Tables"
#endif

namespace Kinetic::Test
{
std::vector<FCase>& Registry()
{
	static std::vector<FCase> Cases;
	return Cases;
}

int& Failures()
{
	static int Count = 0;
	return Count;
}

const FKineticTables& Tables()
{
	static const FKineticTables Loaded = FKineticTables::LoadFromDirectory(KINETIC_TABLES_DIR);
	return Loaded;
}
} // namespace Kinetic::Test

int main()
{
	using namespace Kinetic::Test;
	int FailedCases = 0;
	for (const FCase& Case : Registry())
	{
		const int Before = Failures();
		try
		{
			Case.Body();
		}
		catch (const std::exception& Error)
		{
			++Failures();
			std::printf("  FAIL exception: %s\n", Error.what());
		}
		const bool bPassed = Failures() == Before;
		FailedCases += bPassed ? 0 : 1;
		std::printf("%s %s\n", bPassed ? "ok  " : "FAIL", Case.Name);
	}
	std::printf("\n%zu tests, %d failed, %d failed checks\n", Registry().size(), FailedCases, Failures());
	return FailedCases == 0 ? 0 : 1;
}
