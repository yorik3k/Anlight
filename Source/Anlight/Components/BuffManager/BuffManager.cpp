#include "Components/BuffManager/BuffManager.h"
#include "Components/BuffManager/StatusEffect.h"

UBuffManager::UBuffManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;
}

void UBuffManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    for (int32 i = ActiveEffects.Num() - 1; i >= 0; --i)
    {
        FActiveEffect& ActiveEffect = ActiveEffects[i];

        if (!ActiveEffect.Effect)
        {
            ActiveEffects.RemoveAt(i);
            continue;
        }

        if (ActiveEffect.Effect->Duration > 0.0f)
        {
            ActiveEffect.RemainingTime -= DeltaTime;
            if (ActiveEffect.RemainingTime <= 0.0f)
            {
                UStatusEffect* RemovedEffect = ActiveEffect.Effect;
                ActiveEffects.RemoveAt(i);
                OnEffectRemoved.Broadcast(RemovedEffect);
                continue;
            }
        }

        if (ActiveEffect.Effect->TickInterval > 0.0f && ActiveEffect.Effect->TickValue != 0.0f)
        {
            ActiveEffect.TimeUntilNextTick -= DeltaTime;
            if (ActiveEffect.TimeUntilNextTick <= 0.0f)
            {
                ActiveEffect.TimeUntilNextTick = ActiveEffect.Effect->TickInterval;
            }
        }
    }
}

void UBuffManager::AddEffect(UStatusEffect* Effect)
{
    if (!Effect)
    {
        return;
    }

    int32 ExistingIndex = FindEffectIndex(Effect->EffectID);

    if (ExistingIndex != INDEX_NONE)
    {
        FActiveEffect& Existing = ActiveEffects[ExistingIndex];

        if (Effect->bCanStack && Existing.CurrentStacks < Effect->MaxStacks)
        {
            Existing.CurrentStacks++;
        }

        if (Effect->Duration > 0.0f)
        {
            Existing.RemainingTime = Effect->Duration;
        }

        OnEffectAdded.Broadcast(Effect);
        return;
    }

    FActiveEffect NewEffect;
    NewEffect.Effect = Effect;
    NewEffect.RemainingTime = Effect->Duration;
    NewEffect.TimeUntilNextTick = Effect->TickInterval;
    NewEffect.CurrentStacks = 1;

    ActiveEffects.Add(NewEffect);
    OnEffectAdded.Broadcast(Effect);
}

void UBuffManager::RemoveEffect(FName EffectID)
{
    int32 Index = FindEffectIndex(EffectID);
    if (Index != INDEX_NONE)
    {
        UStatusEffect* RemovedEffect = ActiveEffects[Index].Effect;
        ActiveEffects.RemoveAt(Index);
        OnEffectRemoved.Broadcast(RemovedEffect);
    }
}

void UBuffManager::RemoveAllEffects()
{
    ActiveEffects.Empty();
}

bool UBuffManager::HasEffect(FName EffectID) const
{
    return FindEffectIndex(EffectID) != INDEX_NONE;
}

float UBuffManager::GetTotalModifier(EAnlightStat Stat) const
{
    float Total = 0.0f;
    float Multiplier = 1.0f;

    for (const FActiveEffect& ActiveEffect : ActiveEffects)
    {
        if (!ActiveEffect.Effect)
        {
            continue;
        }

        if (ActiveEffect.Effect->TargetStat != Stat)
        {
            continue;
        }

        float Value = ActiveEffect.Effect->ModifierValue * ActiveEffect.CurrentStacks;

        if (ActiveEffect.Effect->ModifierType == EModifierType::Add)
        {
            Total += Value;
        }
        else if (ActiveEffect.Effect->ModifierType == EModifierType::Multiply)
        {
            Multiplier += Value / 100.0f;
        }
    }

    return Total * Multiplier;
}

TArray<UStatusEffect*> UBuffManager::GetActiveEffects() const
{
    TArray<UStatusEffect*> Result;
    for (const FActiveEffect& ActiveEffect : ActiveEffects)
    {
        if (ActiveEffect.Effect)
        {
            Result.Add(ActiveEffect.Effect);
        }
    }
    return Result;
}

int32 UBuffManager::FindEffectIndex(FName EffectID) const
{
    for (int32 i = 0; i < ActiveEffects.Num(); ++i)
    {
        if (ActiveEffects[i].Effect && ActiveEffects[i].Effect->EffectID == EffectID)
        {
            return i;
        }
    }
    return INDEX_NONE;
}