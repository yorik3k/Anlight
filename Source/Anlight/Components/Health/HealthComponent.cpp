#include "Components/Health/HealthComponent.h"
#include "Components/BuffManager/BuffManager.h"

UHealthComponent::UHealthComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Тикаем 10 раз в секунду
}

void UHealthComponent::BeginPlay()
{
    Super::BeginPlay();
    CurrentHealth = BaseMaxHealth;
    TimeSinceLastDamage = RegenDelayAfterDamage;
}

void UHealthComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // ===== КРОВОТЕЧЕНИЕ =====
    if (bBleeding && BleedingRate > 0.0f)
    {
        // Кровотечение наносит урон
        CurrentHealth = FMath::Max(0.0f, CurrentHealth - BleedingRate * DeltaTime);

        // Кровотечение сбрасывает таймер регенерации
        TimeSinceLastDamage = 0.0f;

        UpdateHealthUI();

        if (CurrentHealth <= 0.0f)
        {
            OnHealthDepleted.Broadcast();
            return;
        }
    }

    // ===== РЕГЕНЕРАЦИЯ =====
    
    if (!bBleeding && CurrentHealth > 0.0f)
    {
        TimeSinceLastDamage += DeltaTime;

        if (TimeSinceLastDamage >= RegenDelayAfterDamage && HealthRegenRate > 0.0f)
        {
            float MaxHealth = GetMaxHealth();
            if (CurrentHealth < MaxHealth)
            {
                CurrentHealth = FMath::Min(MaxHealth, CurrentHealth + HealthRegenRate * DeltaTime);
                UpdateHealthUI();
            }
        }
    }
}

void UHealthComponent::TakeDamage(float DamageAmount)
{
    if (DamageAmount <= 0.0f || CurrentHealth <= 0.0f)
    {
        return;
    }

    CurrentHealth = FMath::Max(0.0f, CurrentHealth - DamageAmount);
    TimeSinceLastDamage = 0.0f; // Сбрасываем таймер регенерации

    UpdateHealthUI();

    if (CurrentHealth <= 0.0f)
    {
        OnHealthDepleted.Broadcast();
    }
}

void UHealthComponent::Heal(float HealAmount)
{
    if (HealAmount <= 0.0f || CurrentHealth <= 0.0f)
    {
        return;
    }

    float MaxHealth = GetMaxHealth();
    CurrentHealth = FMath::Min(MaxHealth, CurrentHealth + HealAmount);
    UpdateHealthUI();
}

void UHealthComponent::StartBleeding(float BleedRate)
{
    if (BleedRate <= 0.0f)
    {
        return;
    }

    // Если уже кровоточим — берём большую скорость
    BleedingRate = FMath::Max(BleedingRate, BleedRate);

    if (!bBleeding)
    {
        bBleeding = true;
        OnBleedingChanged.Broadcast(true);
    }
}

void UHealthComponent::StopBleeding()
{
    if (!bBleeding)
    {
        return;
    }

    bBleeding = false;
    BleedingRate = 0.0f;
    OnBleedingChanged.Broadcast(false);
}

void UHealthComponent::SetBuffManager(UBuffManager* InBuffManager)
{
    BuffManager = InBuffManager;
}

float UHealthComponent::GetMaxHealth() const
{
    float Modifier = 0.0f;
    if (BuffManager)
    {
        Modifier = BuffManager->GetTotalModifier(EAnlightStat::Health);
    }
    return FMath::Max(1.0f, BaseMaxHealth + Modifier);
}

void UHealthComponent::UpdateHealthUI()
{
    OnHealthChanged.Broadcast(CurrentHealth, GetMaxHealth());
}