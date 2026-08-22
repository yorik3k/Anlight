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
class UInventoryComponent;
class UInventoryWidget;
class UInteractionPromptWidget;
class UInventoryItemDefinition;
class AInventoryWorldItem;
class UEffectManager;
class UNutritionComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeathSignature);

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

    // ===== ДВИЖЕНИЕ =====
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float BaseWalkSpeed = 600.0f;

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

    // ===== ИНВЕНТАРЬ =====
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
    UInventoryComponent* InventoryComponent;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory|Interface")
    TSubclassOf<UInventoryWidget> InventoryWidgetClass;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory|Interface")
    TSubclassOf<UInteractionPromptWidget> InteractionPromptWidgetClass;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory|Interface")
    TObjectPtr<UInventoryWidget> InventoryWidget;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory|Interface")
    TObjectPtr<UInteractionPromptWidget> InteractionPromptWidget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Interaction", meta = (ClampMin = "50.0"))
    float InventoryInteractionDistance = 350.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory|Test")
    bool bSpawnInventoryTestItem = false;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory|Test")
    TObjectPtr<UInventoryItemDefinition> InventoryTestItemDefinition;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
    TSubclassOf<UUserWidget> HealthWidgetClass;

    UPROPERTY()
    UUserWidget* HealthWidget;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character")
    UNutritionComponent* NutritionComponent;

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

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* InventoryAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* InteractAction;

    void StartSprint();
    void StopSprint();

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void ToggleInventory();

    UFUNCTION(BlueprintCallable, Category = "Inventory|Interaction")
    void PickUpFocusedItem();

    // ===== СМЕРТЬ =====
    UPROPERTY(BlueprintAssignable, Category = "Health|Events")
    FOnDeathSignature OnDeath;

    UPROPERTY(BlueprintReadOnly, Category = "Health")
    bool bIsDead = false;

    UFUNCTION(BlueprintCallable, Category = "Health")
    void Die();

    // Effect Manager
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character")
    UEffectManager* EffectManager;

    // use item
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void UseItem(FName ItemId);

protected:
    UPROPERTY(EditAnywhere, Category = "Death")
    float DeathAnimDuration = 2.0f;

    UPROPERTY(EditAnywhere, Category = "Death")
    float CameraDropHeight = 80.0f;

    UPROPERTY(EditAnywhere, Category = "Death")
    float CameraPitchTarget = -90.0f;

private:
    bool bIsDying = false;
    float DeathTimer = 0.0f;

    FVector InitialCameraLocation;
    FRotator InitialCameraRotation;
    FVector TargetCameraLocation;
    FRotator TargetCameraRotation;

    UFUNCTION()
    void OnHealthDepletedHandler();

    void InitializeInventoryInterface();
    void SetInventoryVisible(bool bVisible);
    void UpdateInventoryFocus();
    void ClearInventoryFocus();
    void SpawnInventoryTestItem();

    UFUNCTION()
    void HandleInventoryWidgetClosed();

    UPROPERTY()
    TObjectPtr<AInventoryWorldItem> FocusedInventoryItem;

    bool bInventoryVisible = false;
};