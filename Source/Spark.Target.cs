// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class SparkTarget : TargetRules
{
    public SparkTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Game;
        DefaultBuildSettings = BuildSettingsVersion.V5;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_5;
        ExtraModuleNames.Add("Spark");

        // VS2026(14.51) 툴체인은 UE 5.5.4가 지원하지 않아 initializer_list 헤더를 못 찾고 빌드가 깨진다.
        // 검증된 VS2022 14.44 빌드 툴즈 버전으로 고정한다.
        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            WindowsPlatform.CompilerVersion = "14.44.35207";
        }
    }
}
