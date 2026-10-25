// KineticCore under Unreal: the same rules the headless suite tests (game/Tests/KineticCore),
// checked from inside the engine so a build-setting difference (exceptions, symbol visibility,
// the tables path) shows up here. Run: Automation RunTests Kinetic.
#include "Engine/Engine.h"
#include "KineticRulesSubsystem.h"
#include "Kinetic/MassRules.h"
#include "Kinetic/Tables.h"
#include "Misc/AutomationTest.h"

// Kinetic names are written out in full: Kinetic::FBox and the engine's FBox would collide.

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKineticTablesTest, "Kinetic.Core.Tables",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FKineticTablesTest::RunTest(const FString& Parameters)
{
	using Kinetic::EHarnessClass;
	using Kinetic::EMassTier;
	FString Error;
	const TUniquePtr<Kinetic::FKineticTables> Tables =
	    UKineticRulesSubsystem::LoadTables(UKineticRulesSubsystem::TablesDirectory(), Error);
	if (!TestNotNull(TEXT("tables load"), Tables.Get()))
	{
		AddError(Error);
		return false;
	}
	// bible/05 section 2.2.
	TestEqual(TEXT("Rigger mass"), Tables->Harness(EHarnessClass::Rigger).MassKg, 130.0);
	TestEqual(TEXT("Breaker tension"), Tables->Harness(EHarnessClass::Breaker).TensionN, 1000.0);
	TestTrue(TEXT("only the Rigger and the Breaker brace"),
	         Tables->Harness(EHarnessClass::Rigger).bCanBrace && Tables->Harness(EHarnessClass::Breaker).bCanBrace &&
	             !Tables->Harness(EHarnessClass::Climber).bCanBrace);
	// bible/05 section 6.1: a 47 kg propane cylinder is Medium, a 12 kg plank Light.
	TestTrue(TEXT("propane cylinder is Medium"), Kinetic::TierOf(47.0, false, Tables->MassTiers()) == EMassTier::Medium);
	TestTrue(TEXT("plank is Light"), Kinetic::TierOf(12.0, false, Tables->MassTiers()) == EMassTier::Light);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKineticRulesSubsystemTest, "Kinetic.Core.RulesSubsystem",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FKineticRulesSubsystemTest::RunTest(const FString& Parameters)
{
	// The engine subsystem loaded the tables at startup, as the game reads them.
	const UKineticRulesSubsystem* Rules = GEngine ? GEngine->GetEngineSubsystem<UKineticRulesSubsystem>() : nullptr;
	if (!TestNotNull(TEXT("the subsystem exists"), Rules))
	{
		return false;
	}
	if (!TestNotNull(TEXT("its tables loaded"), Rules->Tables()))
	{
		AddError(Rules->LoadError());
		return false;
	}
	TestEqual(TEXT("Climber line length"), Rules->Tables()->Harness(Kinetic::EHarnessClass::Climber).LineLength, 50.0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKineticTablesErrorTest, "Kinetic.Core.TablesError",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FKineticTablesErrorTest::RunTest(const FString& Parameters)
{
	// A missing directory is an FTableError caught at the engine boundary, not a crash.
	FString Error;
	const TUniquePtr<Kinetic::FKineticTables> Tables =
	    UKineticRulesSubsystem::LoadTables(FPaths::ProjectSavedDir() / TEXT("NoTablesHere"), Error);
	TestNull(TEXT("no tables"), Tables.Get());
	TestTrue(TEXT("the error names the file"), Error.Contains(TEXT("cannot open")));
	return true;
}

#endif
