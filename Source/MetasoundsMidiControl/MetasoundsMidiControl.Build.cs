// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class MetasoundsMidiControl : ModuleRules
{
	public MetasoundsMidiControl(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

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
				"UnrealEd"
			}
		);
		
		PublicDependencyModuleNames.AddRange
			(
				new string[]
				{
					"Core",
					"Serialization",
					"SignalProcessing",
					"AudioExtensions"
				}
			);

		PrivateDependencyModuleNames.AddRange
		(
			new string[]
			{
				"MetasoundEngine",
				"MIDIDevice"
			}
		);
	}
}
