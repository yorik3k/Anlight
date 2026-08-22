// внешний вид и работа окна инвентаря

#include "UI/Inventory/InventoryWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Components/Inventory/InventoryComponent.h"
#include "Components/Nutrition/NutritionComponent.h"
#include "Character/MainCharacter.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/TextBlock.h"
#include "GameFramework/PlayerController.h"
#include "Items/InventoryItemDefinition.h"
#include "UI/Inventory/InventorySlotButton.h"

namespace InventoryUI
{
	const FLinearColor Slot(0.07f, 0.08f, 0.07f, 1.0f);
	const FLinearColor SelectedSlot(0.24f, 0.28f, 0.15f, 1.0f);

	FText GetItemName(const FInventoryItemStack& Stack)
	{
		if (Stack.Definition && !Stack.Definition->DisplayName.IsEmpty())
		{
			return Stack.Definition->DisplayName;
		}
		if (Stack.ItemId == FName(TEXT("Bandage")))
		{
			return FText::FromString(TEXT("БИНТ"));
		}

		return FText::FromName(Stack.ItemId);
	}

	FText GetItemDescription(const FInventoryItemStack& Stack)
	{
		if (Stack.Definition && !Stack.Definition->Description.IsEmpty())
		{
			return Stack.Definition->Description;
		}
		if (Stack.ItemId == FName(TEXT("Bandage")))
		{
			return FText::FromString(TEXT("Обычный армейский бинт. Останавливает лёгкое кровотечение."));
		}

		return FText::FromString(TEXT("Предмет находится в рюкзаке."));
	}
}

void UInventoryWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	// Биндим кнопки из WBP
	if (Use)
	{
		Use->OnClicked.AddDynamic(this, &UInventoryWidget::HandleUseClicked);
	}
	if (Drop_one)
	{
		Drop_one->OnClicked.AddDynamic(this, &UInventoryWidget::HandleDropClicked);
	}
	if (Drop_all)
	{
		Drop_all->OnClicked.AddDynamic(this, &UInventoryWidget::HandleDropAllClicked);
	}
	if (close)
	{
		close->OnClicked.AddDynamic(this, &UInventoryWidget::HandleCloseClicked);
	}
	if (WeapSlingButt)
	{
		WeapSlingButt->OnClicked.AddDynamic(this, &UInventoryWidget::HandleWeapSlingClicked);

	}
	if (WeapSecondButt)
	{
		WeapSecondButt->OnClicked.AddDynamic(this, &UInventoryWidget::HandleWeapSecondClicked);

	}

	RefreshInventory();
}

void UInventoryWidget::NativeDestruct()
{
	if (Inventory)
	{
		Inventory->OnInventoryChanged.RemoveDynamic(this, &UInventoryWidget::RefreshInventory);
	}

	Super::NativeDestruct();
}

void UInventoryWidget::InitializeInventory(UInventoryComponent* InInventory)
{
	if (Inventory)
	{
		Inventory->OnInventoryChanged.RemoveDynamic(this, &UInventoryWidget::RefreshInventory);
	}

	Inventory = InInventory;
	if (Inventory)
	{
		Inventory->OnInventoryChanged.AddUniqueDynamic(this, &UInventoryWidget::RefreshInventory);
	}
	if (AMainCharacter* MainChar = Cast<AMainCharacter>(GetOwningPlayerPawn()))
	{
		NutritionComponent = MainChar->NutritionComponent;
		if (NutritionComponent)
		{
			NutritionComponent->OnHungerChanged.AddDynamic(this, &UInventoryWidget::UpdateHungerText);
			NutritionComponent->OnThirstChanged.AddDynamic(this, &UInventoryWidget::UpdateThirstText);
		}
	}

	RefreshInventory();
}

void UInventoryWidget::UpdateHungerText(float CurrentHunger, float MaxHunger)
{
	if (Hunger_capacity)
	{
		Hunger_capacity->SetText(FText::FromString(
			FString::Printf(TEXT("%.0f / %.0f"), CurrentHunger, MaxHunger)));
	}
}

void UInventoryWidget::UpdateThirstText(float CurrentThirst, float MaxThirst)
{
	if (Thirst_capacity)
	{
		Thirst_capacity->SetText(FText::FromString(
			FString::Printf(TEXT("%.0f / %.0f"), CurrentThirst, MaxThirst)));
	}
}

void UInventoryWidget::RefreshInventory()
{
	if (!SlotGrid || !Capacity)
	{
		return;
	}

	SlotGrid->ClearChildren();

	// ===== РИСУЕМ СЕТКУ =====
	const float CellSize = 64.0f; // размер клетки в пикселях

	if (Inventory && !Inventory->Containers.IsEmpty())
	{
		const FContainerData& Container = Inventory->Containers[0]; // Пока только рюкзак

		// Рисуем пустые клетки
		for (int32 Y = 0; Y < Container.Height; ++Y)
		{
			for (int32 X = 0; X < Container.Width; ++X)
			{
				UBorder* EmptyCell = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
				EmptyCell->SetBrushColor(InventoryUI::Slot);
				EmptyCell->SetPadding(FMargin(2.0f));

				if (UCanvasPanelSlot* CellSlot = SlotGrid->AddChildToCanvas(EmptyCell))
				{
					CellSlot->SetPosition(FVector2D(X * CellSize, Y * CellSize));
					CellSlot->SetSize(FVector2D(CellSize, CellSize));
					CellSlot->SetAutoSize(false);
				}
			}
		}

		// Рисуем предметы
		for (int32 Index = 0; Index < Inventory->GetItems().Num(); ++Index)
		{
			const FInventoryItemStack& Stack = Inventory->GetItems()[Index];

			// Пропускаем неразмещённые предметы
			if (Stack.PositionX < 0 || Stack.PositionY < 0)
			{
				continue;
			}

			UInventorySlotButton* ItemButton = WidgetTree->ConstructWidget<UInventorySlotButton>(UInventorySlotButton::StaticClass());
			ItemButton->SetInventoryIndex(Index);
			ItemButton->OnSlotSelected.AddDynamic(this, &UInventoryWidget::HandleSlotSelected);
			ItemButton->SetBackgroundColor(Index == SelectedStackIndex ? InventoryUI::SelectedSlot : InventoryUI::Slot);

			if (UCanvasPanelSlot* ItemSlot = SlotGrid->AddChildToCanvas(ItemButton))
			{
				ItemSlot->SetPosition(FVector2D(Stack.PositionX * CellSize, Stack.PositionY * CellSize));
				ItemSlot->SetSize(FVector2D(Stack.Width * CellSize, Stack.Height * CellSize));
				ItemSlot->SetAutoSize(false);
			}

			// Содержимое кнопки
			UOverlay* ItemOverlay = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass());
			ItemButton->SetContent(ItemOverlay);

			// Иконка или цветной квадрат
			if (Stack.Definition && Stack.Definition->InventoryIcon)
			{
				UImage* IconImage = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());
				IconImage->SetBrushFromTexture(Stack.Definition->InventoryIcon, true);
				if (UOverlaySlot* IconSlot = ItemOverlay->AddChildToOverlay(IconImage))
				{
					IconSlot->SetHorizontalAlignment(HAlign_Fill);
					IconSlot->SetVerticalAlignment(VAlign_Fill);
					IconSlot->SetPadding(FMargin(4.0f));
				}
			}
			else if (Stack.Definition)
			{
				UBorder* Cube = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
				Cube->SetBrushColor(Stack.Definition->InventoryColor);
				if (UOverlaySlot* CubeSlot = ItemOverlay->AddChildToOverlay(Cube))
				{
					CubeSlot->SetHorizontalAlignment(HAlign_Fill);
					CubeSlot->SetVerticalAlignment(VAlign_Fill);
					CubeSlot->SetPadding(FMargin(8.0f));
				}
			}

			// Количество
			if (Stack.Quantity > 1)
			{
				UTextBlock* QuantityText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
				QuantityText->SetText(FText::FromString(FString::Printf(TEXT("x%d"), Stack.Quantity)));
				QuantityText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
				FSlateFontInfo Font = QuantityText->GetFont();
				Font.Size = 14;
				QuantityText->SetFont(Font);
				if (UOverlaySlot* QuantitySlot = ItemOverlay->AddChildToOverlay(QuantityText))
				{
					QuantitySlot->SetHorizontalAlignment(HAlign_Right);
					QuantitySlot->SetVerticalAlignment(VAlign_Bottom);
					QuantitySlot->SetPadding(FMargin(4.0f));
				}
			}
		}
	}

	// Обновляем Capacity
	const int32 UsedSlots = Inventory ? Inventory->GetItems().Num() : 0;
	int32 TotalItems = 0;
	if (Inventory)
	{
		for (const FInventoryItemStack& Stack : Inventory->GetItems())
		{
			TotalItems += Stack.Quantity;
		}
	}
	Capacity->SetText(FText::FromString(FString::Printf(TEXT("%d ячеек • %d предметов"), UsedSlots, TotalItems)));

	// Обновляем выбор
	if (Inventory && !Inventory->GetItems().IsEmpty())
	{
		if (!Inventory->GetItems().IsValidIndex(SelectedStackIndex))
		{
			SelectedStackIndex = 0;
		}
		SelectedItemId = Inventory->GetItems()[SelectedStackIndex].ItemId;
	}
	else
	{
		SelectedItemId = NAME_None;
		SelectedStackIndex = INDEX_NONE;
	}

	UpdateSelectionPanel();
}

void UInventoryWidget::UpdateSelectionPanel()
{
	if (!Name || !ammount || !description || !Drop_one || !Drop_all)
	{
		return;
	}

	const int32 Count = Inventory && !SelectedItemId.IsNone() ? Inventory->GetItemCount(SelectedItemId) : 0;
	const bool bHasSelection = Count > 0;
	const FInventoryItemStack* SelectedStack = Inventory && Inventory->GetItems().IsValidIndex(SelectedStackIndex)
		? &Inventory->GetItems()[SelectedStackIndex]
		: nullptr;

	Name->SetText(SelectedStack ? InventoryUI::GetItemName(*SelectedStack)
		: FText::FromString(TEXT("НЕТ ПРЕДМЕТА")));

	ammount->SetText(SelectedStack
		? FText::FromString(FString::Printf(TEXT("СТАК: %d / %d   •   ВСЕГО: %d"),
			SelectedStack->Quantity, SelectedStack->MaxStackSize, Count))
		: FText::FromString(TEXT("КОЛИЧЕСТВО: 0")));

	description->SetText(SelectedStack ? InventoryUI::GetItemDescription(*SelectedStack)
		: FText::FromString(TEXT("Выберите предмет в рюкзаке.")));

	if (icon)
	{
		const bool bHasIcon = SelectedStack && SelectedStack->Definition && SelectedStack->Definition->InventoryIcon;
		icon->SetVisibility(bHasIcon ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
		if (bHasIcon)
		{
			icon->SetBrushFromTexture(SelectedStack->Definition->InventoryIcon, true);
		}
	}

	Drop_one->SetIsEnabled(bHasSelection);
	Drop_all->SetIsEnabled(bHasSelection);
	if (Use)
	{
		Use->SetIsEnabled(bHasSelection);
	}
}

void UInventoryWidget::HandleSlotSelected(const int32 SlotIndex)
{
	SelectedStackIndex = SlotIndex;
	if (Inventory && Inventory->GetItems().IsValidIndex(SelectedStackIndex))
	{
		SelectedItemId = Inventory->GetItems()[SelectedStackIndex].ItemId;
	}
	RefreshInventory();
}

void UInventoryWidget::HandleUseClicked()
{
	if (Inventory && !SelectedItemId.IsNone())
	{
		OnItemUsed.Broadcast(SelectedItemId);
	}
}

void UInventoryWidget::HandleDropClicked()
{
	if (Inventory && !SelectedItemId.IsNone())
	{
		Inventory->DropItem(SelectedItemId, 1);
	}
}

void UInventoryWidget::HandleDropAllClicked()
{
	if (Inventory && !SelectedItemId.IsNone())
	{
		Inventory->DropItem(SelectedItemId, Inventory->GetItemCount(SelectedItemId));
	}
}

void UInventoryWidget::HandleCloseClicked()
{
	SetVisibility(ESlateVisibility::Collapsed);
	if (APlayerController* PlayerController = GetOwningPlayer())
	{
		PlayerController->SetShowMouseCursor(false);
		PlayerController->SetInputMode(FInputModeGameOnly());
	}

	OnInventoryClosed.Broadcast();
}

void UInventoryWidget::HandleWeapSlingClicked()
{
	if (SelectedItemId.IsNone() || !Inventory)
		return;

	// Найти выбранный предмет
	const TArray<FInventoryItemStack>& Items = Inventory->GetItems();
	for (const FInventoryItemStack& Stack : Items)
	{
		if (Stack.ItemId == SelectedItemId && Stack.Definition)
		{
			// Проверить, что это оружие (потом)
			EquippedWeaponSling = SelectedItemId;

			// Показать иконку в слоте
			if (WeapSlingImage && Stack.Definition->InventoryIcon)
			{
				WeapSlingImage->SetBrushFromTexture(Stack.Definition->InventoryIcon);
			}

			UE_LOG(LogTemp, Warning, TEXT("Equipped: %s"), *SelectedItemId.ToString());
			break;
		}
	}
}
void UInventoryWidget::HandleWeapSecondClicked()
{
	UE_LOG(LogTemp, Warning, TEXT("WeapOnSling clicked"));
}
