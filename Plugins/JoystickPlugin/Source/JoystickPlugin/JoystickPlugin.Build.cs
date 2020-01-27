//

namespace UnrealBuildTool.Rules
{
	using System;
	using System.IO;
	using System.Collections.Generic;

	public class JoystickPlugin : ModuleRules
	{
        public JoystickPlugin(ReadOnlyTargetRules Target) : base(Target)
		{
            PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
 
            PublicDependencyModuleNames.AddRange(
				new string[]
				{
					"Core",
					"CoreUObject",
					"Engine",
					"InputCore",
					"Slate",
					"SlateCore",
                });

            PrivateDependencyModuleNames.AddRange(
				new string[]
                {
                    "SDL"
                });

			PrivateIncludePathModuleNames.AddRange(
				new string[]
				{
					"InputDevice",
                });

			if (Target.Type == TargetRules.TargetType.Editor)
			{
				PrivateIncludePathModuleNames.AddRange(
					new string[]
					{
						"PropertyEditor",
						"ActorPickerMode",
						"DetailCustomizations",
					});

				PrivateDependencyModuleNames.AddRange(
					new string[]
					{
						"PropertyEditor",
						"DetailCustomizations",
						// ... add private dependencies that you statically link with here ...
					});
			}
        }
	}

}
