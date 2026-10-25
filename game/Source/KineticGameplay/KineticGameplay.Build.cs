// KineticGameplay: the engine side of the game (docs/tech/project-structure.md section 1), built
// on KineticCore's rules. Today it loads the rules tables and runs KineticCore under Unreal's
// automation tests; characters, the Spool and the arenas come next.
using System.IO;
using UnrealBuildTool;

public class KineticGameplay : ModuleRules
{
	public KineticGameplay(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		// KineticCore reports a bad table by throwing; the subsystem catches it.
		bEnableExceptions = true;
		PublicDependencyModuleNames.AddRange(new[] { "Core", "CoreUObject", "Engine", "KineticCore" });
	}
}
