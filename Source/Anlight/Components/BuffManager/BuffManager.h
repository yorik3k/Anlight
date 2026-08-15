#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "StatType.h"
#include "StatusEffect.h"
#include "BuffManager.generated.h"

USTRUCT()
struct FActiveEffect
{
    GENERATED_BODY()

    UPROPERTY()
    UStatusEffect* Effect = nullptr;

    UPROPERTY()
    float RemainingTime = 0.0f;

    UPROPERTY()
    float TimeUntilNextTick = 0.0f;

    UPROPERTY()
    int32 CurrentStacks = 1;
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ANLIGHT_API UBuffManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UBuffManager();

    UFUNCTION(BlueprintCallable, Category = "Buff Manager")
    void AddEffect(UStatusEffect* Effect);

    UFUNCTION(BlueprintCallable, Category = "Buff Manager")
    void RemoveEffect(FName EffectID);

    UFUNCTION(BlueprintCallable, Category = "Buff Manager")
    void RemoveAllEffects();

    UFUNCTION(BlueprintCallable, Category = "Buff Manager")
    bool HasEffect(FName EffectID) const;

    UFUNCTION(BlueprintCallable, Category = "Buff Manager")
    float GetTotalModifier(EAnlightStat Stat) const;

    UFUNCTION(BlueprintCallable, Category = "Buff Manager")
    TArray<UStatusEffect*> GetActiveEffects() const;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEffectAddedSignature, UStatusEffect*, Effect);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEffectRemovedSignature, UStatusEffect*, Effect);

    UPROPERTY(BlueprintAssignable, Category = "Buff Manager|Events")
    FOnEffectAddedSignature OnEffectAdded;

    UPROPERTY(BlueprintAssignable, Category = "Buff Manager|Events")
    FOnEffectRemovedSignature OnEffectRemoved;

protected:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    UPROPERTY()
    TArray<FActiveEffect> ActiveEffects;

    int32 FindEffectIndex(FName EffectID) const;
};