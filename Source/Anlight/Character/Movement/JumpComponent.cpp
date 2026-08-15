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
	// owner check
	OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Warning, TEXT("JumpComponent: Owner is not a Character!"));
		return;
	}
	// stamina component check
	StaminaComponent = OwnerCharacter->FindComponentByClass<UStaminaComponent>();
	if (!StaminaComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("JumpComponent: StaminaComponent not found on Owner!"));
	}
}
// can jump FLAG
bool UJumpComponent::CanJump() const
{
	if (!StaminaComponent || !OwnerCharacter)
	{
		return false;
	}
	// Проверка возможности прыгнуть (перс на земле, не в прыжке)
	if (!OwnerCharacter->CanJump())
	{
		return false;
	}

	// Проверка стамины учитывая расход
	float CurrentStamina = StaminaComponent->GetStamina();
	return CurrentStamina >= JumpCost;
}
// Функция выполнения прыжка
void UJumpComponent::StartJump()
{
	// проверка владельца и класса выносливости
	if (!OwnerCharacter || !StaminaComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("JumpComponent: Missing OwnerCharacter or StaminaComponent!"));
		return;
	}

	// Получение текущей стамины
	float CurrentStamina = StaminaComponent->GetStamina();

	// Проверка стамины с расход (задан .h)
	if (CurrentStamina < JumpCost)
	{
		UE_LOG(LogTemp, Warning, TEXT("Not enough stamina to jump! Have: %.1f, Need: %.1f"), CurrentStamina, JumpCost);
		return;
	}

	// Проверка физической возможности прыгнуть
	if (!OwnerCharacter->CanJump())
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot jump - not on ground or already jumping"));
		return;
	}

	// Рассход стамины
	StaminaComponent->UpdateStamina(-JumpCost);

	// Выполнение прыжка
	OwnerCharacter->Jump();

	
}
// функция остановки прыжка
void UJumpComponent::StopJump()
{
	if (!OwnerCharacter) return;
	OwnerCharacter->StopJumping();
}