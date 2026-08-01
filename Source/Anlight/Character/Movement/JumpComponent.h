// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "JumpComponent.generated.h"

class ACharacter;
class UStaminaComponent;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ANLIGHT_API UJumpComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UJumpComponent();

protected:
	virtual void BeginPlay() override;

public:
	// ===== НАСТРОЙКИ =====

	// Стоимость прыжка (тратится из стамины)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jump|Settings")
	float JumpCost = 15.0f;

	// ===== МЕТОДЫ =====

	// Выполнить прыжок (с проверкой стамины)
	UFUNCTION(BlueprintCallable, Category = "Jump")
	void StartJump();

	// Остановить прыжок (вызывается при отпускании)
	UFUNCTION(BlueprintCallable, Category = "Jump")
	void StopJump();

	// Можно ли прыгнуть сейчас
	UFUNCTION(BlueprintCallable, Category = "Jump")
	bool CanJump() const;

protected:
	// ===== ССЫЛКИ =====

	UPROPERTY()
	ACharacter* OwnerCharacter;

	UPROPERTY()
	UStaminaComponent* StaminaComponent;
};