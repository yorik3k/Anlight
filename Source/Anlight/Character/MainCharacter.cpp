// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/MainCharacter.h"
#include "Anlight.h"
#include "Components/Health/HealthComponent.h"
#include "Components/Inventory/InventoryComponent.h"
#include "Components/Stamina/StaminaComponent.h"
#include "Components/EffectManager/EffectManager.h"
#include "Character/Movement/SprintComponent.h"
#include "Character/Movement/JumpComponent.h"
#include "Nutrition/NutritionComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/InputComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PlayerController.h"
#include "InputCoreTypes.h"
#include "Items/InventoryItemDefinition.h"
#include "UI/Interaction/InteractionPromptWidget.h"
#include "UI/Inventory/InventoryWidget.h"
#include "World/Items/InventoryWorldItem.h"

AMainCharacter::AMainCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        MoveComp->MaxWalkSpeed = BaseWalkSpeed;
        MoveComp->JumpZVelocity = 600.0f;
        MoveComp->AirControl = 0.35f;
    }

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

    HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
    StaminaComponent = CreateDefaultSubobject<UStaminaComponent>(TEXT("StaminaComponent"));
    SprintComponent = CreateDefaultSubobject<USprintComponent>(TEXT("SprintComponent"));
    JumpComponent = CreateDefaultSubobject<UJumpComponent>(TEXT("JumpComponent"));
    InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));
    EffectManager = CreateDefaultSubobject<UEffectManager>(TEXT("EffectManager"));
    NutritionComponent = CreateDefaultSubobject<UNutritionComponent>(TEXT("NutritionComponent"));

    
    InteractionPromptWidgetClass = UInteractionPromptWidget::StaticClass();
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

    if (HealthComponent)
    {
        HealthComponent->OnHealthDepleted.AddDynamic(this, &AMainCharacter::OnHealthDepletedHandler);
    }

    if (StaminaComponent)
    {
        StaminaComponent->SetEffectManager(EffectManager);
    }

    if (EffectManager)
    {
        if (HealthComponent)
        {
            HealthComponent->SetEffectManager(EffectManager);
        }
    }
    if (JumpComponent)
    {
        JumpComponent->SetEffectManager(EffectManager);
    }
    if (NutritionComponent)
    {
        NutritionComponent->SetHealthComponent(HealthComponent);
        NutritionComponent->SetEffectManager(EffectManager);
    }

    InitializeInventoryInterface();
    if (HealthWidgetClass)
    {
        APlayerController* PC = Cast<APlayerController>(Controller);
        if (PC)
        {
            HealthWidget = CreateWidget<UUserWidget>(PC, HealthWidgetClass);
            if (HealthWidget)
            {
                HealthWidget->AddToViewport(0);
            }
        }
    }
    SpawnInventoryTestItem();
}

// ===== TICK =====
void AMainCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Применяем модификатор скорости из EffectManager
    if (EffectManager && GetCharacterMovement())
    {
        float SpeedMod = EffectManager->GetTotalModifier(EAnlightStat::MovementSpeedMultiplier);

        if (SpeedMod < 1.0f && SpeedMod > 0.0f)
        {
            GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed * SpeedMod;
        }
        else
        {
            GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
        }
    }

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

    UpdateInventoryFocus();
}



// ===== INPUT =====
void AMainCharacter::Move(const FInputActionValue& Value)
{
    if (bIsDead) return;

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
    if (bIsDead) return;

    FVector2D LookVector = Value.Get<FVector2D>();

    if (Controller)
    {
        AddControllerYawInput(LookVector.X);
        AddControllerPitchInput(LookVector.Y);
    }
}

void AMainCharacter::StartJump()
{
    if (bIsDead) return;

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

        if (InventoryAction)
        {
            EnhancedInputComponent->BindAction(InventoryAction, ETriggerEvent::Started, this, &AMainCharacter::ToggleInventory);
        }

        if (InteractAction)
        {
            EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &AMainCharacter::PickUpFocusedItem);
        }
    }

    if (!InventoryAction)
    {
        PlayerInputComponent->BindKey(EKeys::I, IE_Pressed, this, &AMainCharacter::ToggleInventory);
    }
    if (!InteractAction)
    {
        PlayerInputComponent->BindKey(EKeys::E, IE_Pressed, this, &AMainCharacter::PickUpFocusedItem);
    }
}

// ===== INVENTORY =====
void AMainCharacter::InitializeInventoryInterface()
{
    APlayerController* PlayerController = Cast<APlayerController>(Controller);
    if (!PlayerController || !PlayerController->IsLocalController())
    {
        return;
    }

    if (InventoryWidgetClass)
    {
        InventoryWidget = CreateWidget<UInventoryWidget>(PlayerController, InventoryWidgetClass);
        if (InventoryWidget)
        {
            InventoryWidget->InitializeInventory(InventoryComponent);
            InventoryWidget->OnInventoryClosed.AddUniqueDynamic(this, &AMainCharacter::HandleInventoryWidgetClosed);
            InventoryWidget->OnItemUsed.AddUniqueDynamic(this, &AMainCharacter::UseItem);
            InventoryWidget->AddToViewport(10);
        }
    }

    if (InteractionPromptWidgetClass)
    {
        InteractionPromptWidget = CreateWidget<UInteractionPromptWidget>(PlayerController, InteractionPromptWidgetClass);
        if (InteractionPromptWidget)
        {
            InteractionPromptWidget->AddToViewport(20);
        }
    }

    SetInventoryVisible(false);
}

void AMainCharacter::ToggleInventory()
{
    if (!bIsDead && InventoryWidget)
    {
        SetInventoryVisible(!bInventoryVisible);
    }
}

void AMainCharacter::SetInventoryVisible(const bool bVisible)
{
    const bool bVisibilityChanged = bInventoryVisible != bVisible;
    bInventoryVisible = bVisible;

    if (InventoryWidget)
    {
        InventoryWidget->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    }

    if (bVisible)
    {
        ClearInventoryFocus();
    }

    if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
    {
        PlayerController->SetShowMouseCursor(bVisible);
        if (bVisible)
        {
            FInputModeGameAndUI InputMode;
            InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
            PlayerController->SetInputMode(InputMode);
        }
        else
        {
            PlayerController->SetInputMode(FInputModeGameOnly());
        }

        if (bVisibilityChanged)
        {
            PlayerController->SetIgnoreMoveInput(bVisible);
            PlayerController->SetIgnoreLookInput(bVisible);
        }
    }
}

void AMainCharacter::UpdateInventoryFocus()
{
    if (!FP_Camera || bIsDead || bInventoryVisible)
    {
        ClearInventoryFocus();
        return;
    }

    const FVector Start = FP_Camera->GetComponentLocation();
    const FVector End = Start + FP_Camera->GetForwardVector() * InventoryInteractionDistance;
    FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(MainCharacterInventoryInteraction), false, this);
    FHitResult Hit;
    GetWorld()->SweepSingleByChannel(
        Hit,
        Start,
        End,
        FQuat::Identity,
        ECC_Visibility,
        FCollisionShape::MakeSphere(18.0f),
        QueryParams);

    AInventoryWorldItem* HitItem = Cast<AInventoryWorldItem>(Hit.GetActor());
    if (!IsValid(HitItem) || !HitItem->ItemStack.IsValid())
    {
        ClearInventoryFocus();
        return;
    }

    FocusedInventoryItem = HitItem;
    if (InteractionPromptWidget)
    {
        InteractionPromptWidget->ShowItem(HitItem->ItemStack);
    }
}

void AMainCharacter::PickUpFocusedItem()
{
    if (bIsDead || bInventoryVisible || !IsValid(FocusedInventoryItem) || !InventoryComponent)
    {
        return;
    }

    FocusedInventoryItem->PickUp(InventoryComponent);
    ClearInventoryFocus();
}
void AMainCharacter::UseItem(FName ItemId)
{
    if (!InventoryComponent || !HealthComponent)
        return;

    const TArray<FInventoryItemStack>& Items = InventoryComponent->GetItems();
    for (const FInventoryItemStack& Stack : Items)
    {
        if (Stack.ItemId != ItemId)
            continue;

        if (!Stack.Definition)
            continue;

        switch (Stack.Definition->UsageType)
        {
        case EItemUsageType::PrimitiveMedkit:
            HealthComponent->UseMedkitEDC(Stack.Definition->MedicinePower);
            InventoryComponent->RemoveItem(ItemId, 1);
            break;

        case EItemUsageType::TourniquetType:
            HealthComponent->UseTourniquet(Stack.Definition->MedicinePower);
            InventoryComponent->RemoveItem(ItemId, 1);
            break;

        case EItemUsageType::PainkillerItem:
            HealthComponent->UsePainkiller(
                Stack.Definition->Duration,
                Stack.Definition->ToxicityAmount);
            InventoryComponent->RemoveItem(ItemId, 1);
            break;

        case EItemUsageType::Antidote:
            HealthComponent->UseAntidote(Stack.Definition->MedicinePower, Stack.Definition->Duration);
            InventoryComponent->RemoveItem(ItemId, 1);
            break;

        case EItemUsageType::Food:
            if (NutritionComponent)
            {
                NutritionComponent->Eat(Stack.Definition->NutritionValue);
                InventoryComponent->RemoveItem(ItemId, 1);
            }
            break;

        case EItemUsageType::Water:
            if (NutritionComponent)
            {
                NutritionComponent->Drink(Stack.Definition->HydrationValue);
                InventoryComponent->RemoveItem(ItemId, 1);
            }
            break;

        default:
            break;
        }

        return;
    }
}

void AMainCharacter::ClearInventoryFocus()
{
    FocusedInventoryItem = nullptr;
    if (InteractionPromptWidget)
    {
        InteractionPromptWidget->HidePrompt();
    }
}

void AMainCharacter::HandleInventoryWidgetClosed()
{
    SetInventoryVisible(false);
}

void AMainCharacter::SpawnInventoryTestItem()
{
    if (!bSpawnInventoryTestItem || !InventoryTestItemDefinition || !FP_Camera || !GetWorld())
    {
        return;
    }

    TSubclassOf<AInventoryWorldItem> PickupClass = InventoryTestItemDefinition->WorldItemClass;
    if (!PickupClass)
    {
        PickupClass = AInventoryWorldItem::StaticClass();
    }

    const FVector SpawnLocation = FP_Camera->GetComponentLocation()
        + FP_Camera->GetForwardVector() * 220.0f
        + FP_Camera->GetRightVector() * 70.0f;
    const FTransform SpawnTransform(FP_Camera->GetComponentRotation(), SpawnLocation);
    if (AInventoryWorldItem* TestItem = GetWorld()->SpawnActorDeferred<AInventoryWorldItem>(
        PickupClass,
        SpawnTransform,
        nullptr,
        this,
        ESpawnActorCollisionHandlingMethod::AlwaysSpawn))
    {
        TestItem->InitializeItemDefinition(InventoryTestItemDefinition, 2);
        TestItem->FinishSpawning(SpawnTransform);
        UE_LOG(LogAnlight, Display, TEXT("Spawned inventory test item: %s"),
            *InventoryTestItemDefinition->ItemId.ToString());
    }
}

// ===== DEATH =====
void AMainCharacter::OnHealthDepletedHandler()
{
    UE_LOG(LogTemp, Error, TEXT("OnHealthDepletedHandler called!"));
    Die();
}

void AMainCharacter::Die()
{
    if (bIsDead) return;

    if (bInventoryVisible)
    {
        SetInventoryVisible(false);
    }
    ClearInventoryFocus();

    bIsDead = true;
    bIsDying = true;
    DeathTimer = 0.0f;

    if (APlayerController* PC = Cast<APlayerController>(Controller))
    {
        DisableInput(PC);
        PC->SetIgnoreLookInput(true);
        PC->SetIgnoreMoveInput(true);
    }

    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        MoveComp->DisableMovement();
        MoveComp->StopMovementImmediately();
    }

    if (FP_Camera)
    {
        FP_Camera->bUsePawnControlRotation = false;
        InitialCameraRotation = FP_Camera->GetComponentRotation();
        InitialCameraLocation = FP_Camera->GetRelativeLocation();

        TargetCameraLocation = InitialCameraLocation - FVector(0, 0, CameraDropHeight);
        TargetCameraRotation = FRotator(
            InitialCameraRotation.Pitch + CameraPitchTarget,
            InitialCameraRotation.Yaw,
            InitialCameraRotation.Roll
        );
    }

    OnDeath.Broadcast();
    UE_LOG(LogTemp, Warning, TEXT("Player died!"));
}