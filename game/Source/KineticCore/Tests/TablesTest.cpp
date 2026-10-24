// The tables load, and the values the solver depends on are the bible's.
#include "Kinetic/Tables.h"
#include "Kinetic/Units.h"
#include "TestHarness.h"

#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>

using namespace Kinetic;

KTEST(TablesLoadEveryClass)
{
	const FKineticTables& T = Test::Tables();
	KCHECK_NEAR(T.Harness(EHarnessClass::Climber).MassKg, 90, 0);
	KCHECK_NEAR(T.Harness(EHarnessClass::Breaker).TensionN, 1000, 0);
	KCHECK_NEAR(T.Harness(EHarnessClass::Rigger).HangLoadN, 12000, 0);
	KCHECK_NEAR(T.Harness(EHarnessClass::Doubler).OverspinTensionN, 1200, 0);
	KCHECK_NEAR(T.Harness(EHarnessClass::Breaker).BracedFootHold, 20, 0);
	KCHECK(T.Harness(EHarnessClass::Rigger).bCanBrace);
	KCHECK(!T.Harness(EHarnessClass::Doubler).bCanBrace);
	KCHECK(T.Harness(EHarnessClass::Climber).bCanPerch);
	KCHECK(T.Harness(EHarnessClass::Rigger).SetLines == 2);
	KCHECK(T.Harness(EHarnessClass::Doubler).ActiveLines == 2);
}

KTEST(ScrapperBonusIsOnePointSixNineTimesForce)
{
	KCHECK_NEAR(Test::Tables().Harness(EHarnessClass::Scrapper).SmallPropForceMul(), 1.69, 1e-12);
	KCHECK_NEAR(Test::Tables().Harness(EHarnessClass::Rigger).SmallPropForceMul(), 1.0, 1e-12);
}

KTEST(MissingThresholdsStayMissing)
{
	const FThresholds& HangLine = Test::Tables().Impact(EImpactTable::HangLineHit);
	KCHECK(HangLine.Stagger.has_value() && *HangLine.Stagger == 3);
	KCHECK(!HangLine.Stall.has_value());
	KCHECK_NEAR(Test::Tables().Crush().ForceN, 8000, 0);
}

KTEST(MissingTableDirectoryFailsWithItsPath)
{
	bool bThrew = false;
	try
	{
		FKineticTables::LoadFromDirectory("/nonexistent/kinetic-tables");
	}
	catch (const FTableError& Error)
	{
		bThrew = std::string(Error.what()).find("/nonexistent/kinetic-tables") != std::string::npos;
	}
	KCHECK(bThrew);
}

KTEST(UnitConversionToEngine)
{
	KCHECK_NEAR(Units::MetersToCm(1.5), 150, 1e-12);
	KCHECK_NEAR(Units::NewtonsToEngineForce(1000), 100000, 1e-9);
	KCHECK_NEAR(Units::CmToMeters(Units::MetersToCm(FVec3{1, 2, 3})).Z, 3, 1e-12);
}

namespace
{
// A copy of the tables with one cell replaced; returns the loader's error, or "" if it loaded.
std::string LoadWith(const std::string& Table, const std::string& Old, const std::string& New)
{
	const std::filesystem::path Copy = std::filesystem::temp_directory_path() / "kinetic-tables-test";
	std::filesystem::remove_all(Copy);
	std::filesystem::copy(KINETIC_TABLES_DIR, Copy);
	const std::filesystem::path File = Copy / (Table + ".csv");
	std::ifstream In(File);
	std::string Text((std::istreambuf_iterator<char>(In)), std::istreambuf_iterator<char>());
	In.close();
	const size_t At = Text.find(Old);
	if (At == std::string::npos)
	{
		return "fixture text not found: " + Old;
	}
	Text.replace(At, Old.size(), New);
	std::ofstream(File) << Text;
	try
	{
		FKineticTables::LoadFromDirectory(Copy);
	}
	catch (const FTableError& Error)
	{
		std::filesystem::remove_all(Copy);
		return Error.what();
	}
	std::filesystem::remove_all(Copy);
	return "";
}
} // namespace

// Non-finite numbers and out-of-range whole numbers fail the load, naming the cell.
KTEST(BadNumbersFailTheLoad)
{
	KCHECK(LoadWith("DT_SurfaceModifiers", "StallGel,4.0", "StallGel,nan").find("StallGel].Multiplier") !=
	       std::string::npos);
	KCHECK(LoadWith("DT_SurfaceModifiers", "StallGel,4.0", "StallGel,inf") != "");
	KCHECK(LoadWith("DT_SpoolGlobals", "KinematicSeverSubsteps,4", "KinematicSeverSubsteps,1e300") != "");
	KCHECK(LoadWith("DT_SpoolGlobals", "KinematicSeverSubsteps,4", "KinematicSeverSubsteps,-2") != "");
	KCHECK(LoadWith("DT_SpoolGlobals", "KinematicSeverSubsteps,4", "KinematicSeverSubsteps,4") == "");
}
