// The rules tables (game/Content/Core/Tables), loaded once per engine from the same CSV files the
// headless tests and the bible check read.
#pragma once

#include "CoreMinimal.h"
#include "Kinetic/Tables.h"  // Whole, not forward-declared: UHT's generated code destroys the member
#include "Subsystems/EngineSubsystem.h"
#include "Templates/UniquePtr.h"

#include "KineticRulesSubsystem.generated.h"

UCLASS()
class KINETICGAMEPLAY_API UKineticRulesSubsystem : public UEngineSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Null if the tables failed to load; the error is in the log and in LoadError().
	const Kinetic::FKineticTables* Tables() const { return Loaded.Get(); }
	const FString& LoadError() const { return Error; }

	// Where the tables are read from: Content/Core/Tables in the editor and staged as loose files
	// in a packaged build (Config/DefaultGame.ini).
	static FString TablesDirectory();

	// Loads the tables from a directory; on failure returns null and sets OutError.
	static TUniquePtr<Kinetic::FKineticTables> LoadTables(const FString& Directory, FString& OutError);

private:
	TUniquePtr<Kinetic::FKineticTables> Loaded;
	FString Error;
};
