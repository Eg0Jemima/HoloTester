// Copyright (c) Microsoft Corporation. All rights reserved.

using System;
using System.IO;
using UnrealBuildTool;
using System.Collections.Generic;


namespace UnrealBuildTool.Rules
{
    public class HoloPlayProRuntime : ModuleRules
    {
        public HoloPlayProRuntime(ReadOnlyTargetRules Target) : base(Target)
        {
            PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
            bEnableExceptions = true;

            PrivateIncludePaths.AddRange(
            new string[]
            {
                "HoloPlayProRuntime/Private",
            });

            PublicDependencyModuleNames.AddRange(
            	new string[]
            	{
            		"HoloPlayRuntime",
            	}
            );

            PrivateDependencyModuleNames.AddRange(
            	new string[]
            	{
            		"Core",
            		"CoreUObject",
            		"Engine",
            		"InputCore",
            		"RHI",
            		"RenderCore",
            		"Renderer",
            		"Slate",
            		"SlateCore",
            		"Projects",
                    "UMG"
            	}
            );

            if (Target.bBuildEditor == true)
            {
            	PrivateDependencyModuleNames.Add("UnrealEd");
            }
        }
    }
}
