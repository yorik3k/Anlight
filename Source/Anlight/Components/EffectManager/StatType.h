// StatType.h
#pragma once

#include "CoreMinimal.h"
#include "StatType.generated.h"

/**
 * Òèïû ñòàòîâ, íà êîòîğûå ìîãóò âëèÿòü ıôôåêòû
 */
UENUM(BlueprintType)
enum class EAnlightStat : uint8
{
    // ===== ÁÀÇÎÂÛÅ ÑÒÀÒÛ =====
    Health                 UMETA(DisplayName = "Health"),
    HealthRegenRate        UMETA(DisplayName = "Health Regen Rate"),
    MovementSpeed          UMETA(DisplayName = "Movement Speed"),

    // ===== ØÒĞÀÔÛ ÎÒ ÒÎĞÑÀ =====
    StaminaMultiplier      UMETA(DisplayName = "Stamina Multiplier"),
    StaminaRegenMultiplier UMETA(DisplayName = "Stamina Regen Multiplier"),

    // ===== ØÒĞÀÔÛ ÎÒ ĞÓÊ =====
    WeaponSpreadMultiplier  UMETA(DisplayName = "Weapon Spread Multiplier"),
    AnimationSpeedMultiplier UMETA(DisplayName = "Animation Speed Multiplier"),

    // ===== ØÒĞÀÔÛ ÎÒ ÍÎÃ =====
    MovementSpeedMultiplier UMETA(DisplayName = "Movement Speed Multiplier"),
    bCanJump               UMETA(DisplayName = "Can Jump"),

    // ===== ÄËß İÔÔÅÊÒÎÂ =====
    IncomingDamageModifier UMETA(DisplayName = "Incoming Damage Modifier"),
    HealingEfficiency      UMETA(DisplayName = "Healing Efficiency"),
    BleedingResistance     UMETA(DisplayName = "Bleeding Resistance"),
    ToxinResistance        UMETA(DisplayName = "Toxin Resistance"),
    ToxinReductionRate     UMETA(DisplayName = "Toxin Reduction Rate")
};