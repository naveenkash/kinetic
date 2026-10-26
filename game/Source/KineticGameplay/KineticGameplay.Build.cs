// KineticGameplay: the engine side of the game (docs/tech/project-structure.md section 1), built
// on KineticCore's rules: the rules tables, the harness character and its movement, ladders,
// the Scrap match and its bots, the game mode and HUD, and the automation tests that run KineticCore inside the engine.
using System.IO;
using UnrealBuildTool;

public class KineticGameplay : ModuleRules
{
	public KineticGameplay(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		// Each source keeps its own unit: their small file-local helpers (ToCore, Dt) share names.
		bUseUnity = false;
		// KineticCore reports a bad table by throwing; the subsystem catches it.
		bEnableExceptions = true;
		PublicDependencyModuleNames.AddRange(new[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "AIModule", "NavigationSystem", "KineticCore", "HairStrandsCore" });
		if (Target.bBuildEditor)
		{
			// The automation tests that play a level in the editor.
			PrivateDependencyModuleNames.Add("UnrealEd");
		}
	}
}
