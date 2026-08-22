// HealthComponent.h
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/FBodyPartState.h"
#include "Core/FPainThresholds.h"
#include "Core/FAnlightDamageEvent.h"
#include "HealthComponent.generated.h"

class UEffectManager;

// ===== ДЕЛЕГАТЫ =====
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHealthDepletedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChangedSignature, float, NewHealth, float, NewMaxHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBleedingChangedSignature, bool, bIsBleeding);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBodyPartHealthChanged, EBodyPart, Part, float, NewHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBodyPartDestroyed, EBodyPart, Part);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPainThresholdChanged, float, CurrentValue, float, MaxValue);

// Делегат для обновления цвета шкалы выносливости
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnToxicityVisualChanged, FLinearColor, BarColor);

// Делегат для показа иконки критической интоксикации
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCriticalToxicity, bool, bIsCritical);

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

    UPROPERTY(BlueprintAssignable, Category = "Health|Events")
    FOnBodyPartHealthChanged OnBodyPartHealthChanged;

    UPROPERTY(BlueprintAssignable, Category = "Health|Events")
    FOnBodyPartDestroyed OnBodyPartDestroyed;

    UPROPERTY(BlueprintAssignable, Category = "Health|Events")
    FOnPainThresholdChanged OnBloodLossChanged;

    UPROPERTY(BlueprintAssignable, Category = "Health|Events")
    FOnPainThresholdChanged OnToxicityChanged;

    //
    UPROPERTY(BlueprintAssignable, Category = "Health|Toxicity")
    FOnToxicityVisualChanged OnToxicityVisualChanged;

    UPROPERTY(BlueprintAssignable, Category = "Health|Toxicity")
    FOnCriticalToxicity OnCriticalToxicity;

    // ===== БАЗОВЫЕ ПАРАМЕТРЫ =====
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Health|Regen")
    float HealthRegenRate = 0.0033f; // 0.0033 HP/сек (как в дизайне)

    // ===== МЕТОДЫ УПРАВЛЕНИЯ =====
    UFUNCTION(BlueprintCallable, Category = "Health")
    void InitializeHealth();

    UFUNCTION(BlueprintCallable, Category = "Health")
    float ApplyDamageToPart(EBodyPart Part, float Damage, const FAnlightDamageEvent& DamageEvent);

    UFUNCTION(BlueprintCallable, Category = "Health")
    void HealPart(EBodyPart Part, float Amount);

    UFUNCTION(BlueprintCallable, Category = "Health")
    void PerformSurgery(EBodyPart Part, float RestoredHealth);

    // ===== КРОВОТЕЧЕНИЕ =====
    UFUNCTION(BlueprintCallable, Category = "Health|Bleeding")
    void StartBleeding(EBodyPart Part, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Health|Bleeding")
    void StopBleeding(EBodyPart Part);

    UFUNCTION(BlueprintCallable, Category = "Health|Bleeding")
    void StopAllBleeding();

    UFUNCTION(BlueprintCallable, Category = "Health|Bleeding")
    bool HasBleedingOnPart(EBodyPart Part) const;

    // ===== КРОВОПОТЕРЯ =====
    UFUNCTION(BlueprintCallable, Category = "Health|BloodLoss")
    void AddBloodLoss(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Health|BloodLoss")
    float GetBloodLoss() const { return PainData.CurrentBloodLoss; }

    UFUNCTION(BlueprintCallable, Category = "Health|BloodLoss")
    float GetMaxBloodLoss() const { return PainData.MaxBloodLoss; }

    UFUNCTION(BlueprintCallable, Category = "Health|BloodLoss")
    bool IsDeadFromBloodLoss() const { return PainData.IsDeadFromBloodLoss(); }

    // ===== ИНТОКСИКАЦИЯ =====
    UFUNCTION(BlueprintCallable, Category = "Health|Toxicity")
    void AddToxicity(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Health|Toxicity")
    void ReduceToxicity(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Health|Toxicity")
    float GetToxicity() const { return PainData.CurrentToxicity; }

    UFUNCTION(BlueprintCallable, Category = "Health|Toxicity")
    float GetMaxToxicity() const { return PainData.MaxToxicity; }

    UFUNCTION(BlueprintCallable, Category = "Health|Toxicity")
    bool IsDeadFromToxicity() const { return PainData.IsDeadFromToxicity(); }

    // ===== ГЕТТЕРЫ =====
    UFUNCTION(BlueprintCallable, Category = "Health")
    FBodyPartState GetBodyPartState(EBodyPart Part) const;

    UFUNCTION(BlueprintCallable, Category = "Health")
    float GetTotalHealth() const;

    UFUNCTION(BlueprintCallable, Category = "Health")
    float GetTotalMaxHealth() const;

    UFUNCTION(BlueprintCallable, Category = "Health")
    bool IsDead() const;

    UFUNCTION(BlueprintCallable, Category = "Health")
    bool IsPartDestroyed(EBodyPart Part) const;

    // ===== СВЯЗЬ С EFFECT MANAGER =====
    UFUNCTION(BlueprintCallable, Category = "Health")
    void SetEffectManager(UEffectManager* InEffectManager);
    // В public:
    UFUNCTION(BlueprintCallable, Category = "Health")
    void ApplyPainkiller(float Duration);

    UFUNCTION(BlueprintCallable, Category = "Health")
    void RemovePainkiller();

    // ==== = МЕДИЦИНА ==== =
        UFUNCTION(BlueprintCallable, Category = "Health|Medical")
    void UseMedkitEDC(float HealAmount = 35.0f);
    UFUNCTION(BlueprintCallable, Category = "Health|Medicine")
    void UseEsmarch(float BleedReduction = 50.0f);

    UFUNCTION(BlueprintCallable, Category = "Health|Medical")
    void UseTourniquet(float BleedReduction = 0.0f);

    // Универсальное обезболивающее (с возможным отравлением)
    UFUNCTION(BlueprintCallable, Category = "Health|Medicine")
    void UsePainkiller(float Duration, float ToxicityAmount = 0.0f);

    // Универсальный антидот
    UFUNCTION(BlueprintCallable, Category = "Health|Medicine")
    void UseAntidote(float ToxicityReduction, float Duration);



protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    // ===== ДАННЫЕ =====
    UPROPERTY(ReplicatedUsing = OnRep_BodyStates)
    TArray<FBodyPartState> BodyStates;

    UPROPERTY(Replicated)
    FPainThresholds PainData;

    UPROPERTY()
    UEffectManager* EffectManager = nullptr;

    // ===== ВНУТРЕННИЕ МЕТОДЫ =====
    void UpdateHealthUI();
    void UpdatePartPenalties(EBodyPart Part, bool bIsDestroyed);
    void UpdateAllPartPenalties();
    void SpreadRemainingDamage(EBodyPart SourcePart, float RemainingDamage, const FAnlightDamageEvent& DamageEvent);
    void ApplyBleedingDamage(float DeltaTime);
    void ApplyRegeneration(float DeltaTime);
    void ApplyToxinReduction(float DeltaTime);
    void CheckDeathConditions();
    void UpdateToxicityVisual();
    UFUNCTION()
    void OnPainkillerExpired(const FStatusEffect& Effect);

    int32 GetPartIndex(EBodyPart Part) const;
    bool IsVitalPart(EBodyPart Part) const;

    UFUNCTION()
    void OnRep_BodyStates();

public:
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};