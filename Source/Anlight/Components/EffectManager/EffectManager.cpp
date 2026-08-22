// EffectManager.cpp
#include "Components/EffectManager/EffectManager.h"

UEffectManager::UEffectManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;
    SetIsReplicatedByDefault(true);
}

void UEffectManager::BeginPlay()
{
    Super::BeginPlay();
}

void UEffectManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!GetOwner()->HasAuthority())
        return;

    // Обновляем все эффекты
    ProcessTicks(DeltaTime);

    // Удаляем истекшие
    RemoveExpiredEffects();
}

// ============================================================
// ОСНОВНЫЕ МЕТОДЫ
// ============================================================

void UEffectManager::ApplyEffect(const FStatusEffect& Effect, EBodyPart TargetPart)
{
    if (!GetOwner()->HasAuthority())
        return;

    UE_LOG(LogTemp, Warning, TEXT("ApplyEffect called: %s, Part=%s"),
        *Effect.EffectID.ToString(),
        *UEnum::GetValueAsString(TargetPart));

    // Создаем копию с привязкой к части
    FStatusEffect NewEffect = Effect;
    NewEffect.TargetBodyPart = TargetPart;
    NewEffect.TimeRemaining = Effect.Duration;
    NewEffect.TimeUntilNextTick = Effect.TickInterval;

    // Проверяем, есть ли уже такой эффект
    int32 ExistingIndex = FindEffectIndex(NewEffect.EffectID, TargetPart);

    if (ExistingIndex != INDEX_NONE)
    {
        // Обновляем существующий
        ActiveEffects[ExistingIndex] = NewEffect;
        OnEffectApplied.Broadcast(NewEffect);
        bCacheDirty = true;
        return;
    }

    // Добавляем новый
    ActiveEffects.Add(NewEffect);
    OnEffectApplied.Broadcast(NewEffect);
    bCacheDirty = true;
}

void UEffectManager::RemoveEffect(FName EffectID, EBodyPart TargetPart)
{
    if (!GetOwner()->HasAuthority())
        return;

    int32 Index = FindEffectIndex(EffectID, TargetPart);
    if (Index != INDEX_NONE)
    {
        FStatusEffect Removed = ActiveEffects[Index];
        ActiveEffects.RemoveAt(Index);
        OnEffectRemoved.Broadcast(Removed);
        bCacheDirty = true;
    }
}

void UEffectManager::RemoveAllEffectsFromPart(EBodyPart Part)
{
    if (!GetOwner()->HasAuthority())
        return;

    for (int32 i = ActiveEffects.Num() - 1; i >= 0; --i)
    {
        if (ActiveEffects[i].TargetBodyPart == Part)
        {
            OnEffectRemoved.Broadcast(ActiveEffects[i]);
            ActiveEffects.RemoveAt(i);
        }
    }
    bCacheDirty = true;
}

void UEffectManager::RemoveAllEffects()
{
    if (!GetOwner()->HasAuthority())
        return;

    ActiveEffects.Empty();
    bCacheDirty = true;
}

bool UEffectManager::HasEffect(FName EffectID, EBodyPart TargetPart) const
{
    return FindEffectIndex(EffectID, TargetPart) != INDEX_NONE;
}

TArray<FStatusEffect> UEffectManager::GetEffectsForPart(EBodyPart Part) const
{
    TArray<FStatusEffect> Result;
    for (const FStatusEffect& Effect : ActiveEffects)
    {
        if (Effect.TargetBodyPart == Part || Effect.TargetBodyPart == EBodyPart::None)
        {
            Result.Add(Effect);
        }
    }
    return Result;
}

// ============================================================
// РАСЧЕТ МОДИФИКАТОРОВ
// ============================================================

float UEffectManager::GetTotalModifier(EAnlightStat Stat) const
{
    if (bCacheDirty)
        RecalculateCache();

    if (CachedGlobalModifiers.Contains(Stat))
        return CachedGlobalModifiers[Stat];

    return 0.f;
}

float UEffectManager::GetTotalModifierForPart(EAnlightStat Stat, EBodyPart Part) const
{
    if (bCacheDirty)
        RecalculateCache();

    if (CachedPartModifiers.Contains(Part))
    {
        const TMap<EAnlightStat, float>& PartMods = CachedPartModifiers[Part];
        if (PartMods.Contains(Stat))
            return PartMods[Stat];
    }

    return 0.f;
}

TMap<EAnlightStat, float> UEffectManager::GetAllModifiersForPart(EBodyPart Part) const
{
    if (bCacheDirty)
        RecalculateCache();

    if (CachedPartModifiers.Contains(Part))
        return CachedPartModifiers[Part];

    return TMap<EAnlightStat, float>();
}

// ============================================================
// ВСПОМОГАТЕЛЬНЫЕ МЕТОДЫ
// ============================================================

bool UEffectManager::HasBleedingOnPart(EBodyPart Part) const
{
    for (const FStatusEffect& Effect : ActiveEffects)
    {
        if (Effect.EffectID == FName("Bleeding") && Effect.TargetBodyPart == Part)
        {
            return true;
        }
    }
    return false;
}

// ============================================================
// ВНУТРЕННИЕ МЕТОДЫ
// ============================================================

void UEffectManager::ProcessTicks(float DeltaTime)
{
    for (FStatusEffect& Effect : ActiveEffects)
    {
        Effect.Tick(DeltaTime);

        if (Effect.ShouldTick() && Effect.TickValue != 0.f)
        {
            OnEffectTicked.Broadcast(Effect);
            Effect.ResetTickTimer();
        }
    }
}

void UEffectManager::RemoveExpiredEffects()
{
    for (int32 i = ActiveEffects.Num() - 1; i >= 0; --i)
    {
        if (ActiveEffects[i].IsExpired())
        {
            UE_LOG(LogTemp, Warning, TEXT("Effect expired: %s"), *ActiveEffects[i].EffectID.ToString());

            FStatusEffect ExpiredEffect = ActiveEffects[i];
            ActiveEffects.RemoveAt(i);
            OnEffectRemoved.Broadcast(ExpiredEffect);

            bCacheDirty = true;
        }
    }
}

void UEffectManager::RecalculateCache() const
{
    CachedGlobalModifiers.Empty();
    CachedPartModifiers.Empty();

    for (const FStatusEffect& Effect : ActiveEffects)
    {
        if (Effect.TargetBodyPart == EBodyPart::None)
        {
            // Глобальный модификатор
            if (CachedGlobalModifiers.Contains(Effect.AffectedStat))
                CachedGlobalModifiers[Effect.AffectedStat] += Effect.Value;
            else
                CachedGlobalModifiers.Add(Effect.AffectedStat, Effect.Value);
        }
        else
        {
            // Модификатор для части тела
            TMap<EAnlightStat, float>& PartMods = CachedPartModifiers.FindOrAdd(Effect.TargetBodyPart);

            if (PartMods.Contains(Effect.AffectedStat))
                PartMods[Effect.AffectedStat] += Effect.Value;
            else
                PartMods.Add(Effect.AffectedStat, Effect.Value);
        }
    }

    bCacheDirty = false;
}

int32 UEffectManager::FindEffectIndex(FName EffectID, EBodyPart TargetPart) const
{
    for (int32 i = 0; i < ActiveEffects.Num(); ++i)
    {
        if (ActiveEffects[i].EffectID == EffectID && ActiveEffects[i].TargetBodyPart == TargetPart)
        {
            return i;
        }
    }
    return INDEX_NONE;
}