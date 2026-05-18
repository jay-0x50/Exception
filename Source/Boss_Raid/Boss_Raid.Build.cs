// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Boss_Raid : ModuleRules
{
	public Boss_Raid(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"Boss_Raid",
			"Boss_Raid/Variant_Platforming",
			"Boss_Raid/Variant_Platforming/Animation",
			"Boss_Raid/Variant_Combat",
			"Boss_Raid/Variant_Combat/AI",
			"Boss_Raid/Variant_Combat/Animation",
			"Boss_Raid/Variant_Combat/Gameplay",
			"Boss_Raid/Variant_Combat/Interfaces",
			"Boss_Raid/Variant_Combat/UI",
			"Boss_Raid/Variant_SideScrolling",
			"Boss_Raid/Variant_SideScrolling/AI",
			"Boss_Raid/Variant_SideScrolling/Gameplay",
			"Boss_Raid/Variant_SideScrolling/Interfaces",
			"Boss_Raid/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
