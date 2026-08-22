#include "Character/Movement/JumpComponent.h"
#include "Components/Stamina/StaminaComponent.h"
#include "Components/EffectManager/EffectManager.h"
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

void UJumpComponent::SetEffectManager(UEffectManager* InEffectManager)
{
	EffectManager = InEffectManager;
}

bool UJumpComponent::CanJump() const
{
	if (!StaminaComponent || !OwnerCharacter)
	{
		return false;
	}

	// Проверка: не заблокирован ли прыжок через EffectManager
	if (EffectManager && EffectManager->HasEffect(FName("Legs_Jump_Penalty")))
	{
		return false;
	}

	if (!OwnerCharacter->CanJump())
	{
		return false;
	}

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

	// Проверка: не заблокирован ли прыжок
	if (EffectManager && EffectManager->HasEffect(FName("Legs_Jump_Penalty")))
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot jump - legs are destroyed!"));
		return;
	}

	float CurrentStamina = StaminaComponent->GetStamina();

	if (CurrentStamina < JumpCost)
	{
		UE_LOG(LogTemp, Warning, TEXT("Not enough stamina to jump! Have: %.1f, Need: %.1f"), CurrentStamina, JumpCost);
		return;
	}

	if (!OwnerCharacter->CanJump())
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot jump - not on ground or already jumping"));
		return;
	}

	StaminaComponent->UpdateStamina(-JumpCost);
	OwnerCharacter->Jump();
}

void UJumpComponent::StopJump()
{
	if (!OwnerCharacter) return;
	OwnerCharacter->StopJumping();
}