// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class HoloPlayProEditor : ModuleRules
{
	public HoloPlayProEditor(ReadOnlyTargetRules Target) : base(Target)
	{
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
            }
            );
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
                "InputCore",
                "Projects",
                "UnrealEd",
                "LevelEditor",
                "Settings",
                "HoloPlayEditor",
                "HoloPlayProRuntime"
            }
		);
	}
}
