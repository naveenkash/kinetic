using UnrealBuildTool;

public class KineticEditorTarget : TargetRules
{
	public KineticEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		// Pinned, so an engine upgrade changes them only when this file does.
		DefaultBuildSettings = BuildSettingsVersion.V7;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_8;
		ExtraModuleNames.AddRange(new[] { "KineticCore", "KineticGameplay" });
	}
}
