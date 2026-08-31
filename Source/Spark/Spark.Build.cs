// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Spark : ModuleRules
{
    public Spark(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        // Allows includes like "Framework/SparkPlayerController.h" from module root,
        // matching the C++ Folder Structure defined in Docs/07_Coding_Convention.md.
        PublicIncludePaths.Add(ModuleDirectory);

        PublicDependencyModuleNames.AddRange(new string[]
            { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "Niagara", "PhysicsCore", "UMG", "CableComponent" });

        PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

        // Uncomment if you are using online features
        // PrivateDependencyModuleNames.Add("OnlineSubsystem");

        // To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
    }
}
