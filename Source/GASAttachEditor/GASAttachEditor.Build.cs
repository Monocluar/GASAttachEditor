// <Copyright (C) Monocluar. 2021. All Rights Reserved.

using UnrealBuildTool;

public class GASAttachEditor : ModuleRules
{
	public GASAttachEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
			}
			);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Projects",
				"InputCore",
                "CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"GameplayAbilities",
				"GameplayTags",
				"AssetRegistry",
				"ApplicationCore",
			}
			);

        if (Target.Type == TargetType.Editor)
        {
            PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "UnrealEd",
                "EditorStyle",
				"GameplayTagsEditor",
				"WorkspaceMenuStructure",
				"ToolMenus",
			}
            );
        }
    }
}
