// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "MainCharacter.generated.h"

class UInputMappingContext;
class UInputAction;
class UCameraComponent;
class USpringArmComponent;
class UHealthComponent;
class UStaminaComponent;
class USprintComponent;
class UJumpComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeathSignature);  // ← Добавляем

UCLASS()
class ANLIGHT_API AMainCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AMainCharacter();

protected:
	virtual void BeginPlay() override;

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void StartJump();
	void StopJump();

public:
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// ===== КАМЕРА =====
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	USpringArmComponent* FP_CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* FP_Camera;

	// ===== ЗДОРОВЬЕ =====
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character")
	UHealthComponent* HealthComponent;

	// ===== СТАМИНА =====
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character")
	UStaminaComponent* StaminaComponent;

	// ===== БЕГ =====
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	USprintComponent* SprintComponent;

	// jump
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	UJumpComponent* JumpComponent;

	// ===== ВВОД =====
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* WalkAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* MouseLookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* SprintAction;

	void StartSprint();
	void StopSprint();

	// ===== СМЕРТЬ =====

	// Событие смерти (можно использовать в UI)
	UPROPERTY(BlueprintAssignable, Category = "Health|Events")
	FOnDeathSignature OnDeath;

	// Флаг смерти
	UPROPERTY(BlueprintReadOnly, Category = "Health")
	bool bIsDead = false;

	// Вызов смерти
	UFUNCTION(BlueprintCallable, Category = "Health")
	void Die();

protected:
	// Параметры анимации смерти
	UPROPERTY(EditAnywhere, Category = "Death")
	float DeathAnimDuration = 2.0f;

	UPROPERTY(EditAnywhere, Category = "Death")
	float CameraDropHeight = 80.0f;

	UPROPERTY(EditAnywhere, Category = "Death")
	float CameraPitchTarget = -90.0f;

private:
	// Для плавной анимации
	bool bIsDying = false;
	float DeathTimer = 0.0f;

	FVector InitialCameraLocation;
	FRotator InitialCameraRotation;
	FVector TargetCameraLocation;
	FRotator TargetCameraRotation;

	// Привязка к событию смерти из HealthComponent
	UFUNCTION()
	void OnHealthDepletedHandler();
};