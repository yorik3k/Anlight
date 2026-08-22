// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Anlight : ModuleRules
{
    public Anlight(ReadOnlyTargetRules Target) : base(Target)
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
            "Slate",
            "SlateCore"
        });

        PrivateDependencyModuleNames.AddRange(new string[] { });

        // ============================================================
        // ПУТИ ДЛЯ #include
        // ============================================================
        PublicIncludePaths.AddRange(new string[] {
            // --- Корень модуля ---
            "Anlight",

            // --- Core ---
            "Anlight/Core",

            // --- Character ---
            "Anlight/Character",
            "Anlight/Character/Movement",

            // --- Components ---
            "Anlight/Components",
            "Anlight/Components/Health",
            "Anlight/Components/Stamina",
            "Anlight/Components/EffectManager",
            "Anlight/Components/Inventory",
            "Anlight/Components/Jump",
            "Anlight/Component/Nutrition",

            // --- Items ---
            "Anlight/Items",

            // --- World ---
            "Anlight/World",
            "Anlight/World/Items",

            // --- UI ---
            "Anlight/UI",
            "Anlight/UI/Interaction",
            "Anlight/UI/Inventory",

            // --- Variants ---
            "Anlight/Variant_Horror",
            "Anlight/Variant_Horror/UI",
            "Anlight/Variant_Shooter",
            "Anlight/Variant_Shooter/AI",
            "Anlight/Variant_Shooter/UI",
            "Anlight/Variant_Shooter/Weapons"
        });
    }
}