// EBodyPart.h
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "EBodyPart.generated.h"

UENUM(BlueprintType)
enum class EBodyPart : uint8
{
    None UMETA(DisplayName = "None"),
    Head UMETA(DisplayName = "Head"),
    Chest UMETA(DisplayName = "Chest"),
    LeftArm UMETA(DisplayName = "Left Arm"),
    RightArm UMETA(DisplayName = "Right Arm"),
    LeftLeg UMETA(DisplayName = "Left Leg"),
    RightLeg UMETA(DisplayName = "Right Leg"),
    MAX UMETA(Hidden)
};