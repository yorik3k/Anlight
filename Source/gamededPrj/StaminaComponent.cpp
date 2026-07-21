#include "StaminaComponent.h"

UStaminaComponent::UStaminaComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UStaminaComponent::BeginPlay()
{
    Super::BeginPlay();
    stamina = maxStamina;
    currentRegenMultiplier = 1.0f;
}

float UStaminaComponent::getStamina() const
{
    return stamina;
}

float UStaminaComponent::getMaxStamina() const
{
    return maxStamina;
}

void UStaminaComponent::updateStamina(float DeltaStamina)
{
    stamina += DeltaStamina;
    stamina = FMath::Clamp(stamina, 0.0f, maxStamina);

    OnStaminaChanged.Broadcast(stamina, maxStamina);

    if (stamina == 0.0f) OnStaminaDepleted.Broadcast();

    if (DeltaStamina < 0.0f)
    {
        StopRegen();
        StartRegen();
    }
}

void UStaminaComponent::StartRegen()
{
    if (bIsRegenerating) return;
    if (stamina >= maxStamina) return;

    bIsRegenerating = true;
    GetWorld()->GetTimerManager().SetTimer(RegenTimerHandle, this, &UStaminaComponent::RegenTick, 0.1f, true, staminaRegenDelay);
}

void UStaminaComponent::StopRegen()
{
    bIsRegenerating = false;
    GetWorld()->GetTimerManager().ClearTimer(RegenTimerHandle);
}

void UStaminaComponent::RegenTick()
{
    // ↓↓↓ УМНОЖАЕМ НА МНОЖИТЕЛЬ ↓↓↓
    float regenAmount = staminaRegen * 0.1f * currentRegenMultiplier;
    stamina = FMath::Clamp(stamina + regenAmount, 0.0f, maxStamina);
    OnStaminaChanged.Broadcast(stamina, maxStamina);

    if (stamina >= maxStamina)
    {
        StopRegen();
    }
}

void UStaminaComponent::SetMovementState(EMovementState NewState)
{
    switch (NewState)
    {
    case EMovementState::Idle:
        currentRegenMultiplier = regenMultiplierIdle;
        break;
    case EMovementState::Walking:
        currentRegenMultiplier = regenMultiplierWalking;
        break;
    case EMovementState::Sprinting:
        currentRegenMultiplier = regenMultiplierSprinting;
        break;
    default:
        currentRegenMultiplier = 1.0f;
        break;
    }

    if (currentRegenMultiplier <= 0.0f)
    {
        StopRegen();
    }
    else if (stamina < maxStamina && !bIsRegenerating)
    {
        StartRegen();
    }
}