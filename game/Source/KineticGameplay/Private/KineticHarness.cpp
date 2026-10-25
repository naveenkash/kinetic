#include "KineticHarness.h"

static_assert(static_cast<int>(EKineticHarness::Breaker) == static_cast<int>(Kinetic::EHarnessClass::Breaker),
              "EKineticHarness must follow Kinetic::EHarnessClass");

bool KineticHarness::Parse(const FString& Name, EKineticHarness& Out)
{
	for (const Kinetic::EHarnessClass Class : Kinetic::AllHarnessClasses)
	{
		const std::string_view Core = Kinetic::ToString(Class);
		if (Name.Equals(FString(static_cast<int32>(Core.size()), Core.data()), ESearchCase::IgnoreCase))
		{
			Out = static_cast<EKineticHarness>(Class);
			return true;
		}
	}
	return false;
}

FString KineticHarness::Name(EKineticHarness Harness)
{
	const std::string_view Core = Kinetic::ToString(ToCore(Harness));
	return FString(static_cast<int32>(Core.size()), Core.data());
}
