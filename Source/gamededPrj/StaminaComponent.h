// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "StaminaComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStaminaDepletedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStaminaChangedSignature, float, NewStamina, float, maxStamina);

// ========== 1. ENUM ƒÀﬂ —Œ—“ŒﬂÕ»… ==========
UENUM(BlueprintType)
enum class EMovementState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walking     UMETA(DisplayName = "Walking"),
    Sprinting   UMETA(DisplayName = "Sprinting")
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class GAMEDEDPRJ_API UStaminaComponent : public UActorComponent
{
    GENERATED_BODY()

    UPROPERTY()
    float stamina;

    UPROPERTY(EditDefaultsOnly, Category = "Movement")
    float maxStamina = 100.0f;

public:
    UStaminaComponent();

    // ========== —Œ¡€“»ﬂ ==========
    UPROPERTY(BlueprintAssignable, Category = "Movement")
    FOnStaminaChangedSignature OnStaminaChanged;

    UPROPERTY(BlueprintAssignable, Category = "Movement")
    FOnStaminaDepletedSignature OnStaminaDepleted;

    // ========== Õ¿—“–Œ… » –≈√≈Õ≈–¿÷»» ==========
    UPROPERTY(EditDefaultsOnly, Category = "Movement")
    float staminaRegen = 10.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Movement")
    float staminaRegenDelay = 1.0f;

    // ========== 2. ÃŒƒ»‘» ¿“Œ–€ –≈√≈Õ≈–¿÷»» ==========
    UPROPERTY(EditDefaultsOnly, Category = "Stamina|Modifiers")
    float regenMultiplierIdle = 1.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Stamina|Modifiers")
    float regenMultiplierWalking = 0.5f;

    UPROPERTY(EditDefaultsOnly, Category = "Stamina|Modifiers")
    float regenMultiplierSprinting = 0.0f;

    // ========== Ã≈“Œƒ€ ==========
    void StartRegen();
    void StopRegen();
    void RegenTick();

    // ========== 3. Ã≈“Œƒ ƒÀﬂ —Ã≈Õ€ —Œ—“ŒﬂÕ»ﬂ ==========
    UFUNCTION(BlueprintCallable)
    void SetMovementState(EMovementState NewState);

protected:
    virtual void BeginPlay() override;

    FTimerHandle RegenTimerHandle;
    bool bIsRegenerating = false;

public:
    UFUNCTION(BlueprintCallable)
    float getStamina() const;

    UFUNCTION(BlueprintCallable)
    float getMaxStamina() const;

    UFUNCTION(BlueprintCallable)
    void updateStamina(float deltastamina);

private:
    // ========== 4. “≈ ”Ÿ»… ÃÕŒ∆»“≈À‹ ==========
    float currentRegenMultiplier = 1.0f;
};