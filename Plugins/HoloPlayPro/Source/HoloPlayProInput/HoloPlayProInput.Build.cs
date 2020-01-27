// Copyright (c) Microsoft Corporation. All rights reserved.

using System;
using System.IO;
using UnrealBuildTool;
using Microsoft.Win32;

namespace UnrealBuildTool.Rules
{
	public class HoloPlayProInput : ModuleRules
	{
        public HoloPlayProInput(ReadOnlyTargetRules Target) : base(Target)
        {	
            PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
            bFasterWithoutUnity = true;
            bEnableExceptions = true;

            PrivateIncludePathModuleNames.AddRange(
                new string[]
				{
					"TargetPlatform",
                    "InputDevice",
				});

			PrivateDependencyModuleNames.AddRange(
				new string[]
				{
                    "ApplicationCore",
                    "Core",
                    "CoreUObject",
                    "Engine",
                    "InputCore",
                    "Slate",
                    "SlateCore",
                    "HoloPlayRuntime"
                });

            if (Target.bBuildEditor == true)
            {
                PrivateDependencyModuleNames.Add("UnrealEd");
            }
        }
    }
}