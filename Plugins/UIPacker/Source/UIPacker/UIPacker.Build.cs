// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class UIPacker : ModuleRules
{
	public UIPacker(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
			}
			);
				
		PrivateIncludePaths.AddRange(
			new string[] {
			}
			);
			
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
			}
			);
			
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
                "Core",
                "Projects",
				"InputCore",
				"UnrealEd",
				"LevelEditor",
				"CoreUObject",
                "Engine",
			    "PythonScriptPlugin",
                "Slate",
				"SlateCore",
			    "UMG",
			    "UMGEditor",
			    "AssetTools",
                "ContentBrowser",
				"ToolMenus",
				"Paper2D"
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
			}
			);
	}
}
