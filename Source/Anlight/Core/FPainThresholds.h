// FPainThresholds.h
#pragma once

#include "CoreMinimal.h"
#include "FPainThresholds.generated.h"

USTRUCT(BlueprintType)
struct ANLIGHT_API FPainThresholds
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BloodLoss")
    float MaxBloodLoss = 150.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BloodLoss")
    float CurrentBloodLoss = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Toxicity")
    float MaxToxicity = 800.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Toxicity")
    float CurrentToxicity = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Toxicity")
    float NaturalToxinReductionRate = 1.f / 60.f;

    void AddBloodLoss(float Amount)
    {
        CurrentBloodLoss = FMath::Min(CurrentBloodLoss + Amount, MaxBloodLoss);
    }

    void AddToxicity(float Amount)
    {
        CurrentToxicity = FMath::Min(CurrentToxicity + Amount, MaxToxicity);
    }

    void ReduceToxicity(float Amount)
    {
        CurrentToxicity = FMath::Max(0.f, CurrentToxicity - Amount);
    }

    bool IsDeadFromBloodLoss() const
    {
        return CurrentBloodLoss >= MaxBloodLoss;
    }

    bool IsDeadFromToxicity() const
    {
        return CurrentToxicity >= MaxToxicity;
    }
};