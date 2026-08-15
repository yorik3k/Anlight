#include "Components/Stamina/StaminaComponent.h"
#include "Components/BuffManager/BuffManager.h"

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

    // ===== РЕГЕНЕРАЦИЯ =====
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
        float NewStamina = Stamina + StaminaRegenRate * DeltaTime;
        Stamina = FMath::Min(NewStamina, MaxStamina);
        UpdateStaminaUI();
    }
}
float UStaminaComponent::GetMaxStamina() const
{
    float Modifier = 0.0f;
    if (BuffManager)
    {
        Modifier = BuffManager->GetTotalModifier(EAnlightStat::Stamina);
    }
    return FMath::Max(1.0f, BaseMaxStamina + Modifier);
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

void UStaminaComponent::SetBuffManager(UBuffManager* InBuffManager)
{
    BuffManager = InBuffManager;
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