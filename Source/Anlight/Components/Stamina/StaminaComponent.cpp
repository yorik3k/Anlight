#include "Components/Stamina/StaminaComponent.h"
#include "Components/EffectManager/EffectManager.h"  // <-- ИЗМЕНЕНО

UStaminaComponent::UStaminaComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;
}

void UStaminaComponent::BeginPlay()
{
    Super::BeginPlay();
    CurrentMaxStamina = BaseMaxStamina;
    Stamina = CurrentMaxStamina;
}

void UStaminaComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    float MaxStamina = GetMaxStamina();

    if (bIsSprinting)
    {
        RegenDelayTimer = 0.0f;
        return;
    }

    if (Stamina >= MaxStamina)
    {
        RegenDelayTimer = 0.0f;
        return;
    }

    RegenDelayTimer += DeltaTime;

    if (RegenDelayTimer >= StaminaRegenDelay)
    {
        float RegenMultiplier = 1.0f;
        if (EffectManager)
        {
            RegenMultiplier += EffectManager->GetTotalModifier(EAnlightStat::StaminaRegenMultiplier);
        }
        RegenMultiplier = FMath::Max(0.0f, RegenMultiplier);

        float NewStamina = Stamina + (StaminaRegenRate * RegenMultiplier) * DeltaTime;
        Stamina = FMath::Min(NewStamina, MaxStamina);
        UpdateStaminaUI();
    }
}

float UStaminaComponent::GetMaxStamina() const
{
    float Multiplier = 1.0f;
    if (EffectManager)
    {
        Multiplier += EffectManager->GetTotalModifier(EAnlightStat::StaminaMultiplier);
    }
    return FMath::Max(1.0f, BaseMaxStamina * Multiplier);
}

float UStaminaComponent::GetStaminaPercent() const
{
    float Max = GetMaxStamina();
    if (Max <= 0.0f) return 0.0f;
    return Stamina / Max;
}

bool UStaminaComponent::IsStaminaFull() const
{
    return Stamina >= GetMaxStamina();
}

bool UStaminaComponent::IsStaminaEmpty() const
{
    return Stamina <= 0.0f;
}

bool UStaminaComponent::CanSprint() const
{
    return Stamina > MinStaminaForSprint;
}

void UStaminaComponent::UpdateStamina(float DeltaStamina)
{
    float Max = GetMaxStamina();
    Stamina = FMath::Clamp(Stamina + DeltaStamina, 0.0f, Max);
    UpdateStaminaUI();

    if (Stamina <= 0.0f)
    {
        OnStaminaDepleted.Broadcast();
    }
}

void UStaminaComponent::SetIsSprinting(bool bInSprinting)
{
    bIsSprinting = bInSprinting;

    if (!bIsSprinting)
    {
        RegenDelayTimer = 0.0f;
    }
}

void UStaminaComponent::SetEffectManager(UEffectManager* InEffectManager)  // <-- ИЗМЕНЕНО
{
    EffectManager = InEffectManager;  // <-- ИЗМЕНЕНО
    RecalculateMaxStamina();
}

void UStaminaComponent::RecalculateMaxStamina()
{
    CurrentMaxStamina = GetMaxStamina();

    if (Stamina > CurrentMaxStamina)
    {
        Stamina = CurrentMaxStamina;
        UpdateStaminaUI();
    }

    OnMaxStaminaChanged.Broadcast(CurrentMaxStamina);
}

void UStaminaComponent::UpdateStaminaUI()
{
    OnStaminaChanged.Broadcast(Stamina, GetMaxStamina());
}