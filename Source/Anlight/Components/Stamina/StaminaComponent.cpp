// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/Stamina/StaminaComponent.h"

UStaminaComponent::UStaminaComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
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

	// Обновляем модификаторы (убираем истёкшие)
	UpdateModifiers(DeltaTime);

	// ===== РЕГЕНЕРАЦИЯ =====

	// Если бежим — регенерация недоступна
	if (bIsSprinting)
	{
		RegenDelayTimer = 0.0f;
		return;
	}

	// Если стамина уже полная — не регенерируем
	if (Stamina >= CurrentMaxStamina)
	{
		RegenDelayTimer = 0.0f;
		return;
	}

	// Считаем задержку
	RegenDelayTimer += DeltaTime;

	// Если задержка прошла — регенерируем
	if (RegenDelayTimer >= StaminaRegenDelay)
	{
		float NewStamina = Stamina + StaminaRegenRate * DeltaTime;
		Stamina = FMath::Min(NewStamina, CurrentMaxStamina);
		OnStaminaChanged.Broadcast(Stamina, CurrentMaxStamina);
	}
}

// ===== ГЕТТЕРЫ =====

float UStaminaComponent::GetStaminaPercent() const
{
	if (CurrentMaxStamina <= 0.0f) return 0.0f;
	return Stamina / CurrentMaxStamina;
}

bool UStaminaComponent::IsStaminaFull() const
{
	return Stamina >= CurrentMaxStamina;
}

bool UStaminaComponent::IsStaminaEmpty() const
{
	return Stamina <= 0.0f;
}

// ===== ПРОВЕРКИ =====

bool UStaminaComponent::CanSprint() const
{
	return Stamina > MinStaminaForSprint;
}

// CanJump удалён - проверка теперь в JumpComponent

// ===== УПРАВЛЕНИЕ =====

void UStaminaComponent::UpdateStamina(float DeltaStamina)
{
	Stamina = FMath::Clamp(Stamina + DeltaStamina, 0.0f, CurrentMaxStamina);
	OnStaminaChanged.Broadcast(Stamina, CurrentMaxStamina);

	if (Stamina <= 0.0f)
	{
		OnStaminaDepleted.Broadcast();
	}
}

void UStaminaComponent::SetIsSprinting(bool bInSprinting)
{
	bIsSprinting = bInSprinting;

	// Если перестал бежать — сбрасываем таймер задержки
	if (!bIsSprinting)
	{
		RegenDelayTimer = 0.0f;
	}
}

// ===== МОДИФИКАТОРЫ =====

void UStaminaComponent::AddMaxStaminaModifier(float ModifierAmount, float Duration)
{
	FStaminaModifier NewModifier;
	NewModifier.Amount = ModifierAmount;
	NewModifier.Duration = Duration;
	NewModifier.TimeRemaining = Duration;

	ActiveModifiers.Add(NewModifier);
	RecalculateMaxStamina();
}

void UStaminaComponent::RemoveMaxStaminaModifier(float ModifierAmount)
{
	for (int32 i = ActiveModifiers.Num() - 1; i >= 0; i--)
	{
		if (FMath::IsNearlyEqual(ActiveModifiers[i].Amount, ModifierAmount))
		{
			ActiveModifiers.RemoveAt(i);
			RecalculateMaxStamina();
			return;
		}
	}
}

void UStaminaComponent::RecalculateMaxStamina()
{
	float TotalModifier = 0.0f;

	for (const FStaminaModifier& Mod : ActiveModifiers)
	{
		TotalModifier += Mod.Amount;
	}

	CurrentMaxStamina = FMath::Max(BaseMaxStamina + TotalModifier, 1.0f);

	// Если стамина стала больше максимума — обрезаем
	if (Stamina > CurrentMaxStamina)
	{
		Stamina = CurrentMaxStamina;
		OnStaminaChanged.Broadcast(Stamina, CurrentMaxStamina);
	}

	OnMaxStaminaChanged.Broadcast(CurrentMaxStamina);
}

void UStaminaComponent::UpdateModifiers(float DeltaTime)
{
	for (int32 i = ActiveModifiers.Num() - 1; i >= 0; i--)
	{
		ActiveModifiers[i].TimeRemaining -= DeltaTime;

		if (ActiveModifiers[i].TimeRemaining <= 0.0f)
		{
			ActiveModifiers.RemoveAt(i);
			RecalculateMaxStamina();
		}
	}
}