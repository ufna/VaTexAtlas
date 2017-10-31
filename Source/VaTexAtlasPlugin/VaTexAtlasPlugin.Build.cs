// Copyright 2015-2017 Mail.Ru Group. All Rights Reserved.

using System.IO;

namespace UnrealBuildTool.Rules
{
	public class VaTexAtlasPlugin : ModuleRules
	{
		public VaTexAtlasPlugin(ReadOnlyTargetRules Target) : base(Target)
		{
            PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

            PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			});

			PrivateIncludePaths.AddRange(
				new string[] {
					"VaTexAtlasPlugin/Private"
					// ... add other private include paths required here ...
				});

			PublicDependencyModuleNames.AddRange(
				new string[]
				{
					"Core",
					"CoreUObject",
					"Engine",
                    "SlateCore"
					// ... add other public dependencies that you statically link with here ...
				});
		}
	}
}
