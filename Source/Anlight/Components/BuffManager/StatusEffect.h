#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "StatType.h"
#include "StatusEffect.generated.h"

UENUM(BlueprintType)
enum class EEffectType : uint8
{
    Buff        UMETA(DisplayName = "Buff"),
    Debuff      UMETA(DisplayName = "Debuff"),
    Neutral     UMETA(DisplayName = "Neutral")
};

UENUM(BlueprintType)
enum class EModifierType : uint8
{
    Add         UMETA(DisplayName = "Add (flat)"),
    Multiply    UMETA(DisplayName = "Multiply (percent)")
};

UCLASS(BlueprintType)
class ANLIGHT_API UStatusEffect : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect|Identity")
    FName EffectID;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect|Identity")
    FText EffectName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect|Identity")
    EEffectType EffectType = EEffectType::Buff;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect|Modifier")
    EAnlightStat TargetStat = EAnlightStat::Health;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect|Modifier")
    EModifierType ModifierType = EModifierType::Add;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect|Modifier")
    float ModifierValue = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect|Duration")
    float Duration = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect|Tick")
    float TickInterval = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect|Tick")
    float TickValue = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect|Stacking")
    bool bCanStack = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect|Stacking")
    int32 MaxStacks = 1;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect|UI")
    TSoftObjectPtr<UTexture2D> Icon;
};