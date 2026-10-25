#include "KineticRulesSubsystem.h"

#include "Kinetic/Tables.h"
#include "Misc/Paths.h"

DEFINE_LOG_CATEGORY_STATIC(LogKineticRules, Log, All);

FString UKineticRulesSubsystem::TablesDirectory()
{
	return FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir() / TEXT("Core/Tables"));
}

TUniquePtr<Kinetic::FKineticTables> UKineticRulesSubsystem::LoadTables(const FString& Directory, FString& OutError)
{
	// Nothing may throw past here: the engine is built without exceptions.
	try
	{
		// From UTF-8 explicitly: a plain char string would be read in the system code page on
		// Windows.
		const FTCHARToUTF8 Utf8(*Directory);
		const std::u8string Bytes(reinterpret_cast<const char8_t*>(Utf8.Get()), Utf8.Length());
		return MakeUnique<Kinetic::FKineticTables>(Kinetic::FKineticTables::LoadFromDirectory(std::filesystem::path(Bytes)));
	}
	catch (const Kinetic::FTableError& Failure)
	{
		OutError = UTF8_TO_TCHAR(Failure.what());
	}
	catch (const std::exception& Failure)
	{
		OutError = FString::Printf(TEXT("unexpected error: %s"), UTF8_TO_TCHAR(Failure.what()));
	}
	return nullptr;
}

void UKineticRulesSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	const FString Directory = TablesDirectory();
	Loaded = LoadTables(Directory, Error);
	if (Loaded)
	{
		UE_LOG(LogKineticRules, Log, TEXT("Loaded the rules tables from %s"), *Directory);
	}
	else
	{
		UE_LOG(LogKineticRules, Error, TEXT("The rules tables in %s did not load: %s"), *Directory, *Error);
	}
}

void UKineticRulesSubsystem::Deinitialize()
{
	Loaded.Reset();
	Super::Deinitialize();
}
