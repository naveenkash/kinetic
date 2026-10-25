// KineticCore: the rules of the game in standard C++20 with no engine types (game/README.md).
// The same Public/Private sources build headless with game/Makefile and for the browser with
// web/Makefile; only Private/Unreal/ is engine code, and only Unreal builds it.
using UnrealBuildTool;

public class KineticCore : ModuleRules
{
	public KineticCore(ReadOnlyTargetRules Target) : base(Target)
	{
		// The sources include no engine headers, so no shared PCH; and some files share
		// file-local names, so no unity build.
		PCHUsage = PCHUsageMode.NoPCHs;
		bUseUnity = false;
		// FKineticTables reports a bad table by throwing FTableError.
		bEnableExceptions = true;
		// What other modules link is marked KINETICCORE_API (Kinetic/Api.h), which UBT defines as
		// the export or import for this module.
		PublicDependencyModuleNames.Add("Core");
	}
}
