// A minimal test harness so the library builds and tests with nothing but a C++20 compiler.
// In the engine these become Unreal Automation Spec tests (project-structure section 7).
#pragma once

#include "Kinetic/Tables.h"

#include <cmath>
#include <cstdio>
#include <functional>
#include <string>
#include <vector>

namespace Kinetic::Test
{
struct FCase
{
	const char* Name;
	std::function<void()> Body;
};

std::vector<FCase>& Registry();
int& Failures();

struct FRegistrar
{
	FRegistrar(const char* Name, std::function<void()> Body) { Registry().push_back({Name, std::move(Body)}); }
};

inline void Check(bool bOk, const std::string& What, const char* File, int Line)
{
	if (!bOk)
	{
		++Failures();
		std::printf("  FAIL %s:%d: %s\n", File, Line, What.c_str());
	}
}

inline void CheckNear(double Actual, double Expected, double Tolerance, const char* Expr, const char* File, int Line)
{
	const bool bOk = std::abs(Actual - Expected) <= Tolerance;
	Check(bOk, std::string(Expr) + " = " + std::to_string(Actual) + ", expected " + std::to_string(Expected) + " ± " +
	               std::to_string(Tolerance),
	      File, Line);
}

// The project's data tables, loaded once from Content/Core/Tables.
const FKineticTables& Tables();
} // namespace Kinetic::Test

#define KTEST(Name)                                                                                  \
	static void Name();                                                                              \
	static const ::Kinetic::Test::FRegistrar Name##Registrar(#Name, Name);                           \
	static void Name()

#define KCHECK(Condition) ::Kinetic::Test::Check((Condition), #Condition, __FILE__, __LINE__)
#define KCHECK_NEAR(Actual, Expected, Tolerance) \
	::Kinetic::Test::CheckNear((Actual), (Expected), (Tolerance), #Actual, __FILE__, __LINE__)
