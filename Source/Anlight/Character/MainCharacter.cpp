// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/MainCharacter.h"
#include "Components/Health/HealthComponent.h"
#include "Components/Stamina/StaminaComponent.h"
#include "Character/Movement/SprintComponent.h"
#include "Character/Movement/JumpComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/InputComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"

AMainCharacter::AMainCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// ===== ДВИЖЕНИЕ =====
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->MaxWalkSpeed = 600.0f;
		MoveComp->JumpZVelocity = 600.0f;
		MoveComp->AirControl = 0.35f;
	}

	// ===== КАМЕРА =====
	FP_CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("FP_CameraBoom"));
	FP_CameraBoom->SetupAttachment(RootComponent);
	FP_CameraBoom->SetUsingAbsoluteRotation(true);
	FP_CameraBoom->bDoCollisionTest = false;
	FP_CameraBoom->TargetArmLength = 0.0f;
	FP_CameraBoom->SocketOffset = FVector(0, 0, 80);
	FP_CameraBoom->SetRelativeRotation(FRotator(0, 0, 0));

	FP_Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("FP_Camera"));
	FP_Camera->SetupAttachment(FP_CameraBoom, USpringArmComponent::SocketName);
	FP_Camera->SetRelativeLocation(FVector(0, 0, 0));
	FP_Camera->bUsePawnControlRotation = true;

	// ===== КОМПОНЕНТЫ =====
	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
	StaminaComponent = CreateDefaultSubobject<UStaminaComponent>(TEXT("StaminaComponent"));
	SprintComponent = CreateDefaultSubobject<USprintComponent>(TEXT("SprintComponent"));
	JumpComponent = CreateDefaultSubobject<UJumpComponent>(TEXT("JumpComponent"));
}

void AMainCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			if (DefaultMappingContext)
			{
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
			}
		}
	}

	// Привязываем событие смерти из HealthComponent
	if (HealthComponent)
	{
		HealthComponent->OnHealthDepleted.AddDynamic(this, &AMainCharacter::OnHealthDepletedHandler);
	}
}

void AMainCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Анимация смерти камеры
	if (bIsDying && FP_Camera)
	{
		DeathTimer += DeltaTime;

		float Alpha = FMath::Clamp(DeathTimer / DeathAnimDuration, 0.0f, 1.0f);
		float SmoothAlpha = FMath::InterpEaseInOut(0.0f, 1.0f, Alpha, 2.0f);

		FVector NewLocation = FMath::Lerp(InitialCameraLocation, TargetCameraLocation, SmoothAlpha);
		FP_Camera->SetRelativeLocation(NewLocation);

		FRotator NewRotation = FMath::Lerp(InitialCameraRotation, TargetCameraRotation, SmoothAlpha);
		FP_Camera->SetRelativeRotation(NewRotation);

		if (Alpha >= 1.0f)
		{
			bIsDying = false;
		}
	}
}

void AMainCharacter::Move(const FInputActionValue& Value)
{
	if (bIsDead) return;  // Мёртвый не двигается

	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		if (MovementVector.Y != 0.0f)
		{
			const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
			AddMovementInput(ForwardDirection, MovementVector.Y);
		}

		if (MovementVector.X != 0.0f)
		{
			const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
			AddMovementInput(RightDirection, MovementVector.X);
		}
	}
}

void AMainCharacter::Look(const FInputActionValue& Value)
{
	if (bIsDead) return;  // Мёртвый не поворачивается

	FVector2D LookVector = Value.Get<FVector2D>();

	if (Controller)
	{
		AddControllerYawInput(LookVector.X);
		AddControllerPitchInput(LookVector.Y);
	}
}

void AMainCharacter::StartJump()
{
	if (bIsDead) return;  // Мёртвый не прыгает

	if (JumpComponent)
	{
		JumpComponent->StartJump();
	}
}

void AMainCharacter::StopJump()
{
	if (bIsDead) return;

	if (JumpComponent)
	{
		JumpComponent->StopJump();
	}
}

void AMainCharacter::StartSprint()
{
	if (bIsDead) return;

	if (SprintComponent)
	{
		SprintComponent->StartSprint();
	}
}

void AMainCharacter::StopSprint()
{
	if (bIsDead) return;

	if (SprintComponent)
	{
		SprintComponent->StopSprint();
	}
}

void AMainCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (WalkAction)
		{
			EnhancedInputComponent->BindAction(WalkAction, ETriggerEvent::Triggered, this, &AMainCharacter::Move);
		}

		if (MouseLookAction)
		{
			EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &AMainCharacter::Look);
		}

		if (JumpAction)
		{
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AMainCharacter::StartJump);
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &AMainCharacter::StopJump);
		}

		if (SprintAction)
		{
			EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &AMainCharacter::StartSprint);
			EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &AMainCharacter::StopSprint);
		}
	}
}

// ===== СМЕРТЬ =====

void AMainCharacter::OnHealthDepletedHandler()
{
	Die();
}

void AMainCharacter::Die()
{
	if (bIsDead) return;

	bIsDead = true;
	bIsDying = true;
	DeathTimer = 0.0f;

	// Отключаем ввод
	if (APlayerController* PC = Cast<APlayerController>(Controller))
	{
		DisableInput(PC);
		PC->SetIgnoreLookInput(true);
		PC->SetIgnoreMoveInput(true);
	}

	// Отключаем движение
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->DisableMovement();
		MoveComp->StopMovementImmediately();
	}

	// Сохраняем позиции для анимации
	if (FP_Camera)
	{
		// Отключаем контроль поворота от контроллера
		FP_Camera->bUsePawnControlRotation = false;

		// Сохраняем текущий АБСОЛЮТНЫЙ поворот камеры
		InitialCameraRotation = FP_Camera->GetComponentRotation();
		InitialCameraLocation = FP_Camera->GetRelativeLocation();

		// Целевая позиция: опускаем вниз
		TargetCameraLocation = InitialCameraLocation - FVector(0, 0, CameraDropHeight);

		// Целевой поворот: текущий поворот + наклон вперёд на -90
		TargetCameraRotation = FRotator(
			InitialCameraRotation.Pitch + CameraPitchTarget,  // Добавляем -90 к текущему Pitch
			InitialCameraRotation.Yaw,                         // Yaw оставляем
			InitialCameraRotation.Roll                         // Roll оставляем
		);
	}

	// Вызываем событие смерти
	OnDeath.Broadcast();

	UE_LOG(LogTemp, Warning, TEXT("Player died!"));
}