// Copyright Deskillz Games. All Rights Reserved.

using UnrealBuildTool;

public class DeskillzEditor : ModuleRules
{
	public DeskillzEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"Deskillz"
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Slate",
				"SlateCore",
				"UnrealEd",
				"EditorStyle",
				"PropertyEditor",
				"Settings",
				"ToolMenus",
				"Projects",
				"InputCore",
				"DesktopPlatform"
			}
		);
	}
}
