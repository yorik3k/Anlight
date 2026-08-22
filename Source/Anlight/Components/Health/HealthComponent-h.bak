#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"

class UBuffManager;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHealthDepletedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChangedSignature, float, NewHealth, float, NewMaxHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBleedingChangedSignature, bool, bIsBleeding);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ANLIGHT_API UHealthComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UHealthComponent();

    // ===== СОБЫТИЯ =====
    UPROPERTY(BlueprintAssignable, Category = "Health|Events")
    FOnHealthChangedSignature OnHealthChanged;

    UPROPERTY(BlueprintAssignable, Category = "Health|Events")
    FOnHealthDepletedSignature OnHealthDepleted;

    UPROPERTY(BlueprintAssignable, Category = "Health|Events")
    FOnBleedingChangedSignature OnBleedingChanged;

    // ===== БАЗОВЫЕ ЗНАЧЕНИЯ =====
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Health|Base", meta = (ClampMin = "1.0"))
    float BaseMaxHealth = 100.0f;

    // Скорость регенерации (HP в секунду)
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Health|Regen", meta = (ClampMin = "0.0"))
    float HealthRegenRate = 0.5f;

    // Задержка перед началом регенерации после получения урона
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Health|Regen", meta = (ClampMin = "0.0"))
    float RegenDelayAfterDamage = 3.0f;

    // ===== ГЕТТЕРЫ =====
    UFUNCTION(BlueprintCallable, Category = "Health")
    float GetCurrentHealth() const { return CurrentHealth; }

    UFUNCTION(BlueprintCallable, Category = "Health")
    float GetMaxHealth() const;

    UFUNCTION(BlueprintCallable, Category = "Health")
    bool IsBleeding() const { return bBleeding; }

    UFUNCTION(BlueprintCallable, Category = "Health")
    float GetBleedingRate() const { return BleedingRate; }

    // ===== СВЯЗЬ С BUFF MANAGER =====
    UFUNCTION(BlueprintCallable, Category = "Health")
    void SetBuffManager(UBuffManager* InBuffManager);

    // ===== ОСНОВНЫЕ МЕТОДЫ =====
    UFUNCTION(BlueprintCallable, Category = "Health")
    void TakeDamage(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Health")
    void Heal(float HealAmount);

    // ===== КРОВОТЕЧЕНИЕ =====
    UFUNCTION(BlueprintCallable, Category = "Health|Bleeding")
    void StartBleeding(float BleedRate);

    UFUNCTION(BlueprintCallable, Category = "Health|Bleeding")
    void StopBleeding();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    UPROPERTY()
    float CurrentHealth;

    UPROPERTY()
    bool bBleeding = false;

    UPROPERTY()
    float BleedingRate = 0.0f;

    // Таймер до начала регенерации после урона
    float TimeSinceLastDamage = 0.0f;

    UPROPERTY()
    UBuffManager* BuffManager = nullptr;

    void UpdateHealthUI();
};