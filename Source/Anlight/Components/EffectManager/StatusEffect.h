// StatusEffect.h
#pragma once

#include "CoreMinimal.h"
#include "StatType.h"
#include "EBodyPart.h"
#include "StatusEffect.generated.h"

/**
 * Один эффект (бафф/дебафф)
 * Может быть привязан к конкретной части тела
 */
USTRUCT(BlueprintType)
struct ANLIGHT_API FStatusEffect
{
    GENERATED_BODY()

public:
    // ===== ИДЕНТИФИКАЦИЯ =====

    // Уникальный ID эффекта (например, "Bleeding_LeftArm")
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName EffectID;

    // На какой стат влияет
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EAnlightStat AffectedStat = EAnlightStat::Health;

    // ===== ЗНАЧЕНИЯ =====

    // Модификатор (+10, -0.5, 0.125)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Value = 0.f;

    // Длительность (0 = бесконечный)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Duration = 0.f;

    // ===== ПРИВЯЗКА К ЧАСТИ ТЕЛА =====

    // None = глобальный эффект
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EBodyPart TargetBodyPart = EBodyPart::None;

    // ===== ТИК-ЭФФЕКТЫ (для кровотечений) =====

    // Интервал тика (0 = нет тика)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TickInterval = 0.f;

    // Урон/лечение за тик
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TickValue = 0.f;

    // ===== ВНУТРЕННЕЕ СОСТОЯНИЕ =====

    // Оставшееся время
    float TimeRemaining = 0.f;

    // Время до следующего тика
    float TimeUntilNextTick = 0.f;

    // ===== МЕТОДЫ =====

    // Проверка, истек ли эффект
    bool IsExpired() const
    {
        return Duration > 0.f && TimeRemaining <= 0.f;
    }

    // Обновить время
    void Tick(float DeltaTime)
    {
        if (Duration > 0.f)
        {
            TimeRemaining -= DeltaTime;
        }

        if (TickInterval > 0.f)
        {
            TimeUntilNextTick -= DeltaTime;
        }
    }

    // Проверка, нужно ли сделать тик
    bool ShouldTick() const
    {
        return TickInterval > 0.f && TimeUntilNextTick <= 0.f;
    }

    // Сбросить таймер тика
    void ResetTickTimer()
    {
        TimeUntilNextTick = TickInterval;
    }
};