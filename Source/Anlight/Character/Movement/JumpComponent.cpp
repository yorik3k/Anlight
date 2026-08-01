// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/Movement/JumpComponent.h"
#include "Components/Stamina/StaminaComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UJumpComponent::UJumpComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UJumpComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Warning, TEXT("JumpComponent: Owner is not a Character!"));
		return;
	}

	StaminaComponent = OwnerCharacter->FindComponentByClass<UStaminaComponent>();
	if (!StaminaComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("JumpComponent: StaminaComponent not found on Owner!"));
	}
}

bool UJumpComponent::CanJump() const
{
	if (!StaminaComponent || !OwnerCharacter)
	{
		return false;
	}

	// Проверяем, может ли персонаж физически прыгнуть (на земле, не в прыжке)
	if (!OwnerCharacter->CanJump())
	{
		return false;
	}

	// Проверяем стамину, используя НАШУ цену прыжка
	float CurrentStamina = StaminaComponent->GetStamina();
	return CurrentStamina >= JumpCost;
}

void UJumpComponent::StartJump()
{
	if (!OwnerCharacter || !StaminaComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("JumpComponent: Missing OwnerCharacter or StaminaComponent!"));
		return;
	}

	// Получаем текущую стамину
	float CurrentStamina = StaminaComponent->GetStamina();

	// Проверяем стамину с НАШЕЙ ценой
	if (CurrentStamina < JumpCost)
	{
		UE_LOG(LogTemp, Warning, TEXT("Not enough stamina to jump! Have: %.1f, Need: %.1f"), CurrentStamina, JumpCost);
		return;
	}

	// Проверяем физическую возможность прыжка
	if (!OwnerCharacter->CanJump())
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot jump - not on ground or already jumping"));
		return;
	}

	// Тратим стамину
	StaminaComponent->UpdateStamina(-JumpCost);

	// Выполняем прыжок
	OwnerCharacter->Jump();

	UE_LOG(LogTemp, Log, TEXT("Jump executed! Stamina cost: %.1f, Remaining: %.1f"), JumpCost, StaminaComponent->GetStamina());
}

void UJumpComponent::StopJump()
{
	if (!OwnerCharacter) return;
	OwnerCharacter->StopJumping();
}