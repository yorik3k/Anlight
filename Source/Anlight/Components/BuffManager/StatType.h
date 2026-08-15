#pragma once

#include "CoreMinimal.h"
#include "StatType.generated.h"

UENUM(BlueprintType)
enum class EAnlightStat : uint8
{
    Health          UMETA(DisplayName = "Health"),
    Stamina         UMETA(DisplayName = "Stamina"),
    MoveSpeed       UMETA(DisplayName = "Move Speed"),
    HealthRegen     UMETA(DisplayName = "Health Regen"),
    StaminaRegen    UMETA(DisplayName = "Stamina Regen"),
    DamageResist    UMETA(DisplayName = "Damage Resistance"),
    CarryWeight     UMETA(DisplayName = "Carry Weight")
};