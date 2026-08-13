// логика тестового персонажа

#include "Character/InventoryTestCharacter.h"

#include "Anlight.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Components/Inventory/InventoryComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "InputCoreTypes.h"
#include "Items/InventoryItemDefinition.h"
#include "UI/Interaction/InteractionPromptWidget.h"
#include "UI/Inventory/InventoryWidget.h"
#include "World/Items/InventoryWorldItem.h"

const FName AInventoryTestCharacter::TestItemId(TEXT("Bandage"));

AInventoryTestCharacter::AInventoryTestCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	Inventory = CreateDefaultSubobject<UInventoryComponent>(TEXT("Inventory"));

	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCamera->SetupAttachment(GetCapsuleComponent());
	FirstPersonCamera->SetRelativeLocation(FVector(0.0f, 0.0f, 64.0f));
	FirstPersonCamera->bUsePawnControlRotation = true;
	bUseControllerRotationYaw = true;

	InventoryWidgetClass = UInventoryWidget::StaticClass();
	InteractionPromptWidgetClass = UInteractionPromptWidget::StaticClass();
	TestWorldItemClass = AInventoryWorldItem::StaticClass();
}

void AInventoryTestCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Семь предметов со стаком пять должны создать два стака: пять и два.
	if (TestItemDefinition)
	{
		Inventory->AddItemDefinition(TestItemDefinition, 7);
	}
	else
	{
		Inventory->AddItem(TestItemId, 7, 5, TestWorldItemClass);
	}

	// Два вида предметов появляются рядом, чтобы проверить подсказку и подбор.
	SpawnTestPickup(TestItemDefinition, 2, -60.0f);
	if (SecondaryTestItemDefinition)
	{
		SpawnTestPickup(SecondaryTestItemDefinition, 1, 60.0f);
	}

	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		InventoryWidget = CreateWidget<UInventoryWidget>(PlayerController, InventoryWidgetClass);
		if (InventoryWidget)
		{
			InventoryWidget->InitializeInventory(Inventory);
			InventoryWidget->OnInventoryClosed.AddUniqueDynamic(this, &AInventoryTestCharacter::HandleInventoryWidgetClosed);
			InventoryWidget->AddToViewport();
			SetInventoryUIVisible(false);
		}

		InteractionPromptWidget = CreateWidget<UInteractionPromptWidget>(
			PlayerController, InteractionPromptWidgetClass);
		if (InteractionPromptWidget)
		{
			InteractionPromptWidget->AddToViewport(20);
		}
	}

	UE_LOG(LogAnlight, Display, TEXT("Inventory test ready: %d Bandage in %d stacks"),
		Inventory->GetItemCount(TestItemId), Inventory->GetItems().Num());
}

void AInventoryTestCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Прямые клавиши здесь намеренно простые: это отдельный тестовый персонаж.
	PlayerInputComponent->BindKey(EKeys::One, IE_Pressed, this, &AInventoryTestCharacter::AddTestItem);
	PlayerInputComponent->BindKey(EKeys::Two, IE_Pressed, this, &AInventoryTestCharacter::DropTestItem);
	PlayerInputComponent->BindKey(EKeys::I, IE_Pressed, this, &AInventoryTestCharacter::ToggleInventoryUI);
	PlayerInputComponent->BindKey(EKeys::E, IE_Pressed, this, &AInventoryTestCharacter::PickUpFocusedItem);
}

void AInventoryTestCharacter::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	// Каждый кадр проверяем только небольшой участок перед камерой.
	UpdateInteractionFocus();
}

void AInventoryTestCharacter::AddTestItem()
{
	if (TestItemDefinition)
	{
		Inventory->AddItemDefinition(TestItemDefinition, 1);
	}
	else
	{
		Inventory->AddItem(TestItemId, 1, 5, TestWorldItemClass);
	}
	ShowInventoryMessage(FString::Printf(TEXT("Bandage: %d"), Inventory->GetItemCount(TestItemId)));
}

void AInventoryTestCharacter::DropTestItem()
{
	const bool bDropped = Inventory->DropItem(TestItemId, 1);
	ShowInventoryMessage(FString::Printf(TEXT("Drop: %s | Bandage: %d"),
		bDropped ? TEXT("OK") : TEXT("FAILED"), Inventory->GetItemCount(TestItemId)));
}

void AInventoryTestCharacter::ToggleInventoryUI()
{
	if (InventoryWidget)
	{
		SetInventoryUIVisible(!bInventoryUIVisible);
	}
}

void AInventoryTestCharacter::PickUpFocusedItem()
{
	// Во время работы с открытым рюкзаком случайно подбирать предметы нельзя.
	if (!FocusedWorldItem || (InventoryWidget && InventoryWidget->IsVisible()))
	{
		return;
	}

	const FName ItemId = FocusedWorldItem->ItemStack.ItemId;
	const int32 Added = FocusedWorldItem->PickUp(Inventory);
	if (Added > 0)
	{
		ShowInventoryMessage(FString::Printf(TEXT("Picked up %d x %s"), Added, *ItemId.ToString()));
		UE_LOG(LogAnlight, Display, TEXT("Picked up %d x %s. Inventory now contains %d"),
			Added, *ItemId.ToString(), Inventory->GetItemCount(ItemId));
	}

	ClearInteractionFocus();
}

void AInventoryTestCharacter::SetInventoryUIVisible(const bool bVisible)
{
	if (!InventoryWidget)
	{
		return;
	}

	bInventoryUIVisible = bVisible;
	// Виджет создаётся один раз, а затем только показывается или скрывается.
	InventoryWidget->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	if (bVisible)
	{
		ClearInteractionFocus();
	}

	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
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
	}
}

void AInventoryTestCharacter::HandleInventoryWidgetClosed()
{
	SetInventoryUIVisible(false);
}

void AInventoryTestCharacter::SpawnTestPickup(
	UInventoryItemDefinition* Definition,
	const int32 Quantity,
	const float RightOffset)
{
	// Data Asset может указать свой Blueprint предмета мира.
	TSubclassOf<AInventoryWorldItem> PickupClass = TestWorldItemClass;
	if (Definition && Definition->WorldItemClass)
	{
		PickupClass = Definition->WorldItemClass;
	}

	const FVector SpawnLocation = FirstPersonCamera->GetComponentLocation()
		+ FirstPersonCamera->GetForwardVector() * 220.0f
		+ FirstPersonCamera->GetRightVector() * RightOffset;
	const FTransform SpawnTransform(FirstPersonCamera->GetComponentRotation(), SpawnLocation);
	// Сначала создаём объект без Construction Script, записываем его данные
	// и только потом завершаем создание.
	if (AInventoryWorldItem* TestPickup = GetWorld()->SpawnActorDeferred<AInventoryWorldItem>(
		PickupClass,
		SpawnTransform,
		nullptr,
		this,
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn))
	{
		if (Definition)
		{
			TestPickup->InitializeItemDefinition(Definition, Quantity);
		}
		else
		{
			TestPickup->InitializeItem(TestItemId, Quantity, 5);
		}
		TestPickup->FinishSpawning(SpawnTransform);
	}
}

void AInventoryTestCharacter::UpdateInteractionFocus()
{
	if (!FirstPersonCamera || !InteractionPromptWidget || (InventoryWidget && InventoryWidget->IsVisible()))
	{
		ClearInteractionFocus();
		return;
	}

	// Сферическая проверка немного шире обычного луча, поэтому на маленький предмет
	// проще навестись центром экрана.
	const FVector Start = FirstPersonCamera->GetComponentLocation();
	const FVector End = Start + FirstPersonCamera->GetForwardVector() * InteractionDistance;
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(InventoryInteraction), false, this);
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
	if (!HitItem || !HitItem->ItemStack.IsValid())
	{
		ClearInteractionFocus();
		return;
	}

	FocusedWorldItem = HitItem;
	InteractionPromptWidget->ShowItem(HitItem->ItemStack);
}

void AInventoryTestCharacter::ClearInteractionFocus()
{
	FocusedWorldItem = nullptr;
	if (InteractionPromptWidget)
	{
		InteractionPromptWidget->HidePrompt();
	}
}

void AInventoryTestCharacter::ShowInventoryMessage(const FString& Message) const
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Green, Message);
	}
}
