// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class SparkEditor : ModuleRules
{
    public SparkEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
            { "Core", "CoreUObject", "Engine", "AnimationModifiers", "AnimationModifierLibrary", "AnimationBlueprintLibrary" });
    }
}
