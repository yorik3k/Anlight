// Fill out your copyright notice in the Description page of Project Settings.

#include "SprintComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "StaminaComponent.h"

USprintComponent::USprintComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void USprintComponent::BeginPlay()
{
    Super::BeginPlay();

    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter)
    {
        originalWalkSpeed = OwnerCharacter->GetCharacterMovement()->MaxWalkSpeed;
        StaminaComponent = OwnerCharacter->FindComponentByClass<UStaminaComponent>();
    }
}

void USprintComponent::StartSprint()
{
    if (!OwnerCharacter || bIsSprinting) return;
    if (StaminaComponent && StaminaComponent->getStamina() <= 0.0f) return;

    bIsSprinting = true;
}

void USprintComponent::StopSprint()
{
    if (!bIsSprinting) return;

    bIsSprinting = false;

    if (OwnerCharacter)
    {
        OwnerCharacter->GetCharacterMovement()->MaxWalkSpeed = originalWalkSpeed;
    }

    // «апускаем регенерацию после остановки бега
    if (StaminaComponent && StaminaComponent->getStamina() < StaminaComponent->getMaxStamina())
    {
        StaminaComponent->StartRegen();
    }
}

void USprintComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    UpdateSpeed(DeltaTime);
}

void USprintComponent::UpdateSpeed(float DeltaTime)
{
    if (!OwnerCharacter) return;

    if (bIsSprinting)
    {
        if (StaminaComponent && StaminaComponent->getStamina() > 0.0f)
        {
            StaminaComponent->updateStamina(-DeltaTime * staminaCost);
            OwnerCharacter->GetCharacterMovement()->MaxWalkSpeed = originalWalkSpeed * sprintSpeedMultiplier;
        }
        else
        {
            StopSprint();
        }
    }
}