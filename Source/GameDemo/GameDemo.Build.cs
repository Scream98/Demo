// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class GameDemo : ModuleRules
{
	public GameDemo(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "UnLua", "UMG", "GameplayAbilities", "GameplayTags", "GameplayTasks" });

		PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore", "Lua" });

		// 将子目录添加到包含路径（支持按功能组织源码）
		// Unlua/   - UnLua 集成相关（KDDBindingManager, KDDModuleLocator）
		// UMG/     - UI 基类（KDDView, KDDWidget）
		// Core/    - 应用核心（KDDGameInstance）
		PublicIncludePaths.Add(ModuleDirectory);
		PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Unlua"));
		PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "UMG"));
		PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Core"));
	}
}
