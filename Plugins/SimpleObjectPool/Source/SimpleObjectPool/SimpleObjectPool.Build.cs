// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class SimpleObjectPool : ModuleRules
{
	public SimpleObjectPool(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"DeveloperSettings",
				"Engine"
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"AIModule",
				"Slate",
				"SlateCore"
			}
		);
	}
}
