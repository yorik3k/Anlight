// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/Movement/SprintComponent.h"
#include "Components/Stamina/StaminaComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

USprintComponent::USprintComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void USprintComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Warning, TEXT("SprintComponent: Owner is not a Character!"));
		return;
	}

	if (UCharacterMovementComponent* MoveComp = OwnerCharacter->GetCharacterMovement())
	{
		OriginalWalkSpeed = MoveComp->MaxWalkSpeed;
	}

	StaminaComponent = OwnerCharacter->FindComponentByClass<UStaminaComponent>();
	if (!StaminaComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("SprintComponent: StaminaComponent not found on Owner!"));
	}
}

void USprintComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bIsSprinting) return;

	if (!StaminaComponent || !StaminaComponent->CanSprint())
	{
		StopSprint();
		return;
	}

	StaminaComponent->UpdateStamina(-SprintDrainRate * DeltaTime);

	if (!StaminaComponent->CanSprint())
	{
		StopSprint();
	}
}

void USprintComponent::StartSprint()
{
	if (bIsSprinting) return;

	if (!StaminaComponent || !StaminaComponent->CanSprint())
	{
		return;
	}

	bIsSprinting = true;
	StaminaComponent->SetIsSprinting(true);
	UpdateSpeed();
}

void USprintComponent::StopSprint()
{
	if (!bIsSprinting) return;

	bIsSprinting = false;

	if (StaminaComponent)
	{
		StaminaComponent->SetIsSprinting(false);
	}

	UpdateSpeed();
}

void USprintComponent::UpdateSpeed()
{
	if (!OwnerCharacter) return;

	UCharacterMovementComponent* MoveComp = OwnerCharacter->GetCharacterMovement();
	if (!MoveComp) return;

	if (bIsSprinting)
	{
		MoveComp->MaxWalkSpeed = OriginalWalkSpeed * SprintSpeedMultiplier;
	}
	else
	{
		MoveComp->MaxWalkSpeed = OriginalWalkSpeed;
	}
}