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
			// ← Убери пустую строку ""
		});

        PrivateDependencyModuleNames.AddRange(new string[] { });

        // ============================================================
        // ПУТИ ДЛЯ #include (чтобы компилятор видел все папки)
        // ============================================================
        PublicIncludePaths.AddRange(new string[] {
			// --- Корень модуля ---
			"Anlight",

			// --- Core (глобальные классы) ---
			"Anlight/Core",

			// --- Character ---
			"Anlight/Character",

			// --- Components (наши новые компоненты) ---
			"Anlight/Components",
            "Anlight/Components/Health",
            "Anlight/Components/Stamina",
            "Anlight/Components/Radiation",
            "Anlight/Components/BuffManager",
            "Anlight/Components/BuffManager/Effects",

			// --- Items ---
			"Anlight/Items",
            "Anlight/Items/Consumables",

			// --- World ---
			"Anlight/World",
            "Anlight/World/Zones",

			// --- UI ---
			"Anlight/UI",
            "Anlight/UI/Widgets",

			// --- Variants (оставляем как есть) ---
			"Anlight/Variant_Horror",
            "Anlight/Variant_Horror/UI",
            "Anlight/Variant_Shooter",
            "Anlight/Variant_Shooter/AI",
            "Anlight/Variant_Shooter/UI",
            "Anlight/Variant_Shooter/Weapons",
        });

        // Uncomment if you are using Slate UI
        // PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

        // Uncomment if you are using online features
        // PrivateDependencyModuleNames.Add("OnlineSubsystem");

        // To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
    }
}