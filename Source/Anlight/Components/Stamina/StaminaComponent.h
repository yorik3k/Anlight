#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "StaminaComponent.generated.h"

class UBuffManager;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStaminaDepletedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStaminaChangedSignature, float, NewStamina, float, MaxStamina);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMaxStaminaChangedSignature, float, NewMaxStamina);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ANLIGHT_API UStaminaComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UStaminaComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ===== BASE =====
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stamina")
    float Stamina;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina")
    float BaseMaxStamina = 100.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stamina")
    float CurrentMaxStamina;

    // ===== REGEN =====
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina|Regen")
    float StaminaRegenRate = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina|Regen")
    float StaminaRegenDelay = 1.5f;

    float RegenDelayTimer = 0.0f;

    // ===== DRAIN =====
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina|Drain")
    float SprintDrainRate = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina|Settings")
    float MinStaminaForSprint = 5.0f;

    // ===== STATE =====
    bool bIsSprinting = false;

public:
    // ===== —Œ¡€“»ﬂ =====
    UPROPERTY(BlueprintAssignable, Category = "Stamina|Events")
    FOnStaminaChangedSignature OnStaminaChanged;

    UPROPERTY(BlueprintAssignable, Category = "Stamina|Events")
    FOnStaminaDepletedSignature OnStaminaDepleted;

    UPROPERTY(BlueprintAssignable, Category = "Stamina|Events")
    FOnMaxStaminaChangedSignature OnMaxStaminaChanged;

    // ===== √≈““≈–€ =====
    UFUNCTION(BlueprintCallable, Category = "Stamina")
    float GetStamina() const { return Stamina; }

    UFUNCTION(BlueprintCallable, Category = "Stamina")
    float GetMaxStamina() const;

    UFUNCTION(BlueprintCallable, Category = "Stamina")
    float GetStaminaPercent() const;

    UFUNCTION(BlueprintCallable, Category = "Stamina")
    bool IsStaminaFull() const;

    UFUNCTION(BlueprintCallable, Category = "Stamina")
    bool IsStaminaEmpty() const;

    // ===== œ–Œ¬≈– » =====
    UFUNCTION(BlueprintCallable, Category = "Stamina")
    bool CanSprint() const;

    // ===== ”œ–¿¬À≈Õ»≈ =====
    UFUNCTION(BlueprintCallable, Category = "Stamina")
    void UpdateStamina(float DeltaStamina);

    UFUNCTION(BlueprintCallable, Category = "Stamina")
    void SetIsSprinting(bool bInSprinting);

    // ===== —¬ﬂ«‹ — BUFF MANAGER =====
    UFUNCTION(BlueprintCallable, Category = "Stamina")
    void SetBuffManager(UBuffManager* InBuffManager);

private:
    UPROPERTY()
    UBuffManager* BuffManager = nullptr;

    void UpdateStaminaUI();
    void RecalculateMaxStamina();
};