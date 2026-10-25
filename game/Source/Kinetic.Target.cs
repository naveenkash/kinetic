// The game target. The dedicated server target needs a source build of the engine
// (docs/tech/standalone-server-option.md); the Launcher build ships editor and game only.
using UnrealBuildTool;

public class KineticTarget : TargetRules
{
	public KineticTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		// Pinned, so an engine upgrade changes them only when this file does.
		DefaultBuildSettings = BuildSettingsVersion.V7;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_8;
		ExtraModuleNames.AddRange(new[] { "KineticCore", "KineticGameplay" });
	}
}
