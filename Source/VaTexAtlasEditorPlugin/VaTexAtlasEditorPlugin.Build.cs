// Copyright 2016 Vladimir Alyamkin. All Rights Reserved.

using System.IO;

namespace UnrealBuildTool.Rules
{
	public class VaTexAtlasEditorPlugin : ModuleRules
	{
		public VaTexAtlasEditorPlugin(TargetInfo Target)
		{
			PublicIncludePaths.AddRange(
			new string[] {
				"VaTexAtlasPlugin",
				"VaTexAtlasPlugin/Public"
				// ... add public include paths required here ...
			});

			PrivateIncludePaths.AddRange(
				new string[] {
					"VaTexAtlasEditorPlugin/Private",
					// ... add other private include paths required here ...
				});

			PublicDependencyModuleNames.AddRange(
				new string[]
				{
					"Core",
					"CoreUObject",
					"Engine",
					"VaTexAtlasPlugin"
					// ... add other public dependencies that you statically link with here ...
				});

			PrivateDependencyModuleNames.AddRange(
				new string[]
				{
					// ...
				});
		}
	}
}