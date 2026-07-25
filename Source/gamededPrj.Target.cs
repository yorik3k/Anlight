// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;
using System.Collections.Generic;

public class gamededPrjTarget : TargetRules
{
    public gamededPrjTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Game;
        DefaultBuildSettings = BuildSettingsVersion.V6;  // ← Меняем с V5 на V6
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_7;  // ← Меняем с Unreal5_4 на Unreal5_7
        ExtraModuleNames.Add("gamededPrj");
    }
}