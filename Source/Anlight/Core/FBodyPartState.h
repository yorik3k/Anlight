// FBodyPartState.h
#pragma once

#include "CoreMinimal.h"
#include "EBodyPart.h"
#include "FBodyPartState.generated.h"

USTRUCT(BlueprintType)
struct ANLIGHT_API FBodyPartState
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EBodyPart Part = EBodyPart::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CurrentHealth = 100.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxHealth = 100.f;

    UPROPERTY(BlueprintReadOnly)
    bool bIsDestroyed = false;

    UPROPERTY(BlueprintReadWrite)
    bool bIsBleeding = false;

    UPROPERTY(BlueprintReadWrite)
    float BleedingIntensity = 0.f;

    void Initialize(EBodyPart InPart)
    {
        Part = InPart;

        switch (InPart)
        {
        case EBodyPart::Head:
            MaxHealth = 70.f;
            break;
        case EBodyPart::Chest:
            MaxHealth = 150.f;
            break;
        case EBodyPart::LeftArm:
        case EBodyPart::RightArm:
            MaxHealth = 80.f;
            break;
        case EBodyPart::LeftLeg:
        case EBodyPart::RightLeg:
            MaxHealth = 95.f;
            break;
        default:
            MaxHealth = 100.f;
            break;
        }

        CurrentHealth = MaxHealth;
        bIsDestroyed = false;
        bIsBleeding = false;
        BleedingIntensity = 0.f;
    }

    float ApplyDamage(float InDamage)
    {
        if (bIsDestroyed || InDamage <= 0.f)
            return InDamage;

        const float OldHealth = CurrentHealth;
        CurrentHealth = FMath::Max(0.f, CurrentHealth - InDamage);

        if (CurrentHealth <= 0.f)
        {
            bIsDestroyed = true;
            CurrentHealth = 0.f;
            return InDamage - OldHealth;
        }

        return 0.f;
    }

    void Heal(float InHealAmount)
    {
        if (bIsDestroyed)
            return;

        CurrentHealth = FMath::Min(CurrentHealth + InHealAmount, MaxHealth);
    }

    void PerformSurgery(float RestoredHealth)
    {
        if (!bIsDestroyed)
            return;

        bIsDestroyed = false;
        CurrentHealth = FMath::Clamp(RestoredHealth, 1.f, MaxHealth);
    }

    void StartBleeding(float Intensity)
    {
        if (bIsDestroyed)
            return;

        bIsBleeding = true;
        BleedingIntensity = FMath::Clamp(Intensity, 0.f, 1.f);
    }

    void StopBleeding()
    {
        bIsBleeding = false;
        BleedingIntensity = 0.f;
    }

    bool IsAlive() const
    {
        return !bIsDestroyed && CurrentHealth > 0.f;
    }

    float GetBleedingRate() const
    {
        return BleedingIntensity * 5.f;
    }
};