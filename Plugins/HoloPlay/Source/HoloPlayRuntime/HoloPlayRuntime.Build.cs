// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

namespace UnrealBuildTool.Rules
{
    using System;
    using System.IO;
    using System.Collections.Generic;

    public class HoloPlayRuntime : ModuleRules
    {
        private string ModulePath
        {
            get { return ModuleDirectory; }
        }

        private string ThirdPartyPath
        {
            get { return Path.GetFullPath(Path.Combine(ModulePath, "../../ThirdParty/")); }
        }

        private string BinariesPath
        {
            get { return Path.GetFullPath(Path.Combine(ModulePath, "../../Binaries/")); }
        }

        public string GetUProjectPath()
        {
            return Path.Combine(ModuleDirectory, "../../../..");
        }

        private void CopyToProjectBinaries(string Filepath, ReadOnlyTargetRules Target)
        {
            System.Console.WriteLine("UE4 is: " + Path.GetFullPath(GetUProjectPath()));

            string BinariesDir = Path.Combine(GetUProjectPath(), "Binaries", Target.Platform.ToString());
            string Filename = Path.GetFileName(Filepath);

            //convert relative path 
            string FullBinariesDir = Path.GetFullPath(BinariesDir);

            if (!Directory.Exists(FullBinariesDir))
            {
                Directory.CreateDirectory(FullBinariesDir);
            }

            string FullBinaryFilepath = Path.Combine(FullBinariesDir, Filename);
            if (!File.Exists(FullBinaryFilepath))
            {
                System.Console.WriteLine("HoloPlay: Copied from " + Filepath + ", to " + FullBinaryFilepath);
                File.Copy(Filepath, FullBinaryFilepath, true);
            }
        }


        public HoloPlayRuntime(ReadOnlyTargetRules Target) : base(Target)
        {
            PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
            bEnableExceptions = true;


            PublicIncludePaths.AddRange(
                new string[] {
                    // ... add public include paths required here ...
                }
                );


            PrivateIncludePaths.AddRange(
                new string[] {
                    // ... add other private include paths required here ...
                }
                );


            PublicDependencyModuleNames.AddRange(
                new string[]
                {
                    "Core",
                    "InputCore"
                }
                );


            PrivateDependencyModuleNames.AddRange(
                new string[]
                {
                    "ApplicationCore",
                    "Renderer",
                    "RenderCore",
                    "RHI",
                    "UtilityShaders",
                    "CoreUObject",
                    "Engine",
                    "Slate",
                    "SlateCore",
                    "HeadMountedDisplay",
                    "ImageWrapper",
                    "EngineSettings",
                    "MovieSceneCapture"
                }
                );


            if (Target.bBuildEditor == true)
            {
                PrivateDependencyModuleNames.Add("UnrealEd");
            }

            string LibraryPlatformFolder = string.Empty;
            switch (Target.Platform)
            {
                case UnrealTargetPlatform.Win64:
                    LibraryPlatformFolder = "Win64";
                    break;
                case UnrealTargetPlatform.Mac:
                    LibraryPlatformFolder = "osx";
                    break;
                case UnrealTargetPlatform.Linux:
                    LibraryPlatformFolder = "linux64";
                    break;
            }

            string IncludePath = Path.Combine(ThirdPartyPath, "HoloPlay", "Include");
            PublicIncludePaths.Add(IncludePath);

            string LibraryPath = Path.Combine(ThirdPartyPath, "HoloPlay", "Lib", LibraryPlatformFolder);
            string DLLPath = Path.Combine(ThirdPartyPath, "HoloPlay", "Bin", LibraryPlatformFolder);


            if (Target.Platform == UnrealTargetPlatform.Win64)
            {
                //Lib
                PublicLibraryPaths.Add(LibraryPath);
                PublicAdditionalLibraries.Add("HoloPlay.lib");

                //DLL
                string PluginDLLPath = Path.Combine(DLLPath, "HoloPlay.dll");

                System.Console.WriteLine("Project plugin detected, using dll at " + PluginDLLPath);

                // This copy DLL from Plugin\ThirdParty\HoloPlay\Bin\ to UE4Project\Binaries\Win64\
                CopyToProjectBinaries(PluginDLLPath, Target);

                string ProjectDLLFile = Path.GetFullPath(Path.Combine(GetUProjectPath(), "Binaries", LibraryPlatformFolder, "HoloPlay.dll"));
                // Request copy DLL from UE4Project\Binaries\Win64\ to UE4Build\Binaries\Win64\
                RuntimeDependencies.Add(ProjectDLLFile);

                // Delay load DLLs - typically used for External(third party) modules
                // We need to delay the Lib in order to Build, otherwise it will lock HoloPlay.dll from the UE4Project\Binaries\Win64\
                PublicDelayLoadDLLs.Add("HoloPlay.dll");
            }
        }
    }
}
