// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;
using System.Collections.Generic;

public class gamededPrjEditorTarget : TargetRules
{
    public gamededPrjEditorTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Editor;
        DefaultBuildSettings = BuildSettingsVersion.V6;  // ← Меняем с V5 на V6
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_7;  // ← Меняем с Unreal5_4 на Unreal5_7
        ExtraModuleNames.Add("gamededPrj");
    }
}