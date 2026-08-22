// EffectManager.h
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "StatType.h"
#include "StatusEffect.h"
#include "EffectManager.generated.h"

// ===== ДЕЛЕГАТЫ =====
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEffectApplied, const FStatusEffect&, Effect);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEffectRemoved, const FStatusEffect&, Effect);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEffectTicked, const FStatusEffect&, Effect);

/**
 * Управляет всеми временными эффектами персонажа
 * - Кровотечения
 * - Переломы (штрафы)
 * - Баффы от лекарств
 * - Дебаффы от интоксикации
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ANLIGHT_API UEffectManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UEffectManager();

    // ===== СОБЫТИЯ =====
    UPROPERTY(BlueprintAssignable, Category = "EffectManager|Events")
    FOnEffectApplied OnEffectApplied;

    UPROPERTY(BlueprintAssignable, Category = "EffectManager|Events")
    FOnEffectRemoved OnEffectRemoved;

    UPROPERTY(BlueprintAssignable, Category = "EffectManager|Events")
    FOnEffectTicked OnEffectTicked;

    // ===== ОСНОВНЫЕ МЕТОДЫ =====

    /** Добавить эффект */
    UFUNCTION(BlueprintCallable, Category = "EffectManager")
    void ApplyEffect(const FStatusEffect& Effect, EBodyPart TargetPart = EBodyPart::None);

    /** Удалить эффект по ID */
    UFUNCTION(BlueprintCallable, Category = "EffectManager")
    void RemoveEffect(FName EffectID, EBodyPart TargetPart = EBodyPart::None);

    /** Удалить все эффекты с части тела */
    UFUNCTION(BlueprintCallable, Category = "EffectManager")
    void RemoveAllEffectsFromPart(EBodyPart Part);

    /** Удалить все эффекты */
    UFUNCTION(BlueprintCallable, Category = "EffectManager")
    void RemoveAllEffects();

    /** Проверить наличие эффекта */
    UFUNCTION(BlueprintCallable, Category = "EffectManager")
    bool HasEffect(FName EffectID, EBodyPart TargetPart = EBodyPart::None) const;

    /** Получить все активные эффекты */
    UFUNCTION(BlueprintCallable, Category = "EffectManager")
    TArray<FStatusEffect> GetActiveEffects() const { return ActiveEffects; }

    /** Получить эффекты для конкретной части */
    UFUNCTION(BlueprintCallable, Category = "EffectManager")
    TArray<FStatusEffect> GetEffectsForPart(EBodyPart Part) const;

    // ===== РАСЧЕТ МОДИФИКАТОРОВ =====

    /** Получить суммарный модификатор (глобально) */
    UFUNCTION(BlueprintCallable, Category = "EffectManager")
    float GetTotalModifier(EAnlightStat Stat) const;

    /** Получить суммарный модификатор для части тела */
    UFUNCTION(BlueprintCallable, Category = "EffectManager")
    float GetTotalModifierForPart(EAnlightStat Stat, EBodyPart Part) const;

    /** Получить все модификаторы для части */
    UFUNCTION(BlueprintCallable, Category = "EffectManager")
    TMap<EAnlightStat, float> GetAllModifiersForPart(EBodyPart Part) const;

    // ===== ВСПОМОГАТЕЛЬНЫЕ МЕТОДЫ =====

    /** Есть ли активное кровотечение на части? */
    UFUNCTION(BlueprintCallable, Category = "EffectManager")
    bool HasBleedingOnPart(EBodyPart Part) const;

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    // ===== ДАННЫЕ =====

    // Все активные эффекты
    UPROPERTY()
    TArray<FStatusEffect> ActiveEffects;

    // Кэш модификаторов (для оптимизации)
    mutable bool bCacheDirty = true;
    mutable TMap<EAnlightStat, float> CachedGlobalModifiers;
    mutable TMap<EBodyPart, TMap<EAnlightStat, float>> CachedPartModifiers;

    // ===== ВНУТРЕННИЕ МЕТОДЫ =====

    void AddEffectInternal(const FStatusEffect& Effect);
    void RemoveEffectInternal(const FStatusEffect& Effect);
    void RemoveExpiredEffects();
    void ProcessTicks(float DeltaTime);
    void RecalculateCache() const;
    int32 FindEffectIndex(FName EffectID, EBodyPart TargetPart) const;
};