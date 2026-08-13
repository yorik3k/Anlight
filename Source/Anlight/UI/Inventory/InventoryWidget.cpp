// внешний вид и работа окна инвентаря

#include "UI/Inventory/InventoryWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"
#include "Components/Inventory/InventoryComponent.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/ScaleBox.h"
#include "Components/SizeBox.h"
#include "Components/Spacer.h"
#include "Components/TextBlock.h"
#include "Components/UniformGridPanel.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "GameFramework/PlayerController.h"
#include "Items/InventoryItemDefinition.h"
#include "UI/Inventory/InventorySlotButton.h"

namespace InventoryUI
{
	// Общие размеры и цвета стандартного интерфейса.
	constexpr int32 Columns = 6;
	constexpr int32 DefaultSlots = 24;

	const FLinearColor Backdrop(0.005f, 0.007f, 0.006f, 0.78f);
	const FLinearColor Panel(0.025f, 0.032f, 0.029f, 0.99f);
	const FLinearColor SubPanel(0.045f, 0.054f, 0.047f, 1.0f);
	const FLinearColor Slot(0.07f, 0.08f, 0.07f, 1.0f);
	const FLinearColor SelectedSlot(0.24f, 0.28f, 0.15f, 1.0f);
	const FLinearColor Accent(0.68f, 0.76f, 0.35f, 1.0f);
	const FLinearColor Muted(0.48f, 0.52f, 0.47f, 1.0f);
	const FLinearColor PrimaryText(0.88f, 0.9f, 0.82f, 1.0f);

	void SetFontSize(UTextBlock* Text, const int32 Size)
	{
		FSlateFontInfo Font = Text->GetFont();
		Font.Size = Size;
		Text->SetFont(Font);
	}

	void SetVerticalSlot(UWidget* Widget, const FMargin& Padding, const ESlateSizeRule::Type Rule = ESlateSizeRule::Automatic)
	{
		if (UVerticalBoxSlot* SlotWidget = Cast<UVerticalBoxSlot>(Widget->Slot))
		{
			SlotWidget->SetPadding(Padding);
			SlotWidget->SetSize(FSlateChildSize(Rule));
		}
	}

	void SetHorizontalFill(UWidget* Widget, const float Value, const FMargin& Padding = FMargin())
	{
		if (UHorizontalBoxSlot* SlotWidget = Cast<UHorizontalBoxSlot>(Widget->Slot))
		{
			FSlateChildSize Size(ESlateSizeRule::Fill);
			Size.Value = Value;
			SlotWidget->SetSize(Size);
			SlotWidget->SetPadding(Padding);
		}
	}

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
			return FText::FromString(TEXT("Обычный армейский бинт. Останавливает лёгкое кровотечение. СЕЙЧАС ЛЕЧИТ МАКСИМУМ ПСИХИЧЕСКОЕ РАССТРОЙСТВО."));
		}

		return FText::FromString(TEXT("Предмет находится в рюкзаке."));
	}

	UTextBlock* MakeText(UWidgetTree* Tree, const FString& Value, const int32 Size,
		const FLinearColor& Color = PrimaryText)
	{
		UTextBlock* Text = Tree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		Text->SetText(FText::FromString(Value));
		Text->SetColorAndOpacity(FSlateColor(Color));
		SetFontSize(Text, Size);
		return Text;
	}
}

void UInventoryWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (!WidgetTree->RootWidget)
	{
		// Blueprint может оставить дерево пустым: тогда рабочий интерфейс строится C++.
		BuildWidgetTree();
	}

	RefreshInventory();
}

void UInventoryWidget::NativeDestruct()
{
	// Виджет может быть уничтожен раньше персонажа, поэтому снимаем подписку.
	if (Inventory)
	{
		Inventory->OnInventoryChanged.RemoveDynamic(this, &UInventoryWidget::RefreshInventory);
	}

	Super::NativeDestruct();
}

void UInventoryWidget::InitializeInventory(UInventoryComponent* InInventory)
{
	// При повторной инициализации сначала отписываемся от старого компонента.
	if (Inventory)
	{
		// Теперь любое добавление, удаление или выбрасывание обновит окно автоматически.
		Inventory->OnInventoryChanged.RemoveDynamic(this, &UInventoryWidget::RefreshInventory);
	}

	Inventory = InInventory;
	if (Inventory)
	{
		Inventory->OnInventoryChanged.AddUniqueDynamic(this, &UInventoryWidget::RefreshInventory);
	}

	RefreshInventory();
}

void UInventoryWidget::BuildWidgetTree()
{
	// Интерфейс создаётся из обычных UMG-компонентов. Blueprint-наследник может
	// менять тексты и цвета через Class Defaults, не трогая эту рабочую логику.
	UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("Root"));
	WidgetTree->RootWidget = Root;

	UBorder* Dim = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("Backdrop"));
	Dim->SetBrushColor(InventoryUI::Backdrop);
	Root->AddChild(Dim);
	if (UCanvasPanelSlot* DimSlot = Cast<UCanvasPanelSlot>(Dim->Slot))
	{
		DimSlot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
		DimSlot->SetOffsets(FMargin(0.0f));
	}

	UScaleBox* ScreenScale = WidgetTree->ConstructWidget<UScaleBox>(UScaleBox::StaticClass(), TEXT("ScreenScale"));
	ScreenScale->SetStretch(EStretch::ScaleToFit);
	ScreenScale->SetStretchDirection(EStretchDirection::Both);
	Root->AddChild(ScreenScale);
	if (UCanvasPanelSlot* ScaleSlot = Cast<UCanvasPanelSlot>(ScreenScale->Slot))
	{
		ScaleSlot->SetAnchors(FAnchors(0.03f, 0.04f, 0.97f, 0.96f));
		ScaleSlot->SetOffsets(FMargin(0.0f));
	}

	USizeBox* WindowSize = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("WindowSize"));
	WindowSize->SetWidthOverride(1120.0f);
	WindowSize->SetHeightOverride(680.0f);
	ScreenScale->SetContent(WindowSize);

	UBorder* Window = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("Window"));
	Window->SetBrushColor(InventoryUI::Panel);
	Window->SetPadding(FMargin(24.0f));
	WindowSize->SetContent(Window);

	UVerticalBox* MainLayout = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("MainLayout"));
	Window->SetContent(MainLayout);

	UHorizontalBox* Header = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("Header"));
	MainLayout->AddChildToVerticalBox(Header);
	InventoryUI::SetVerticalSlot(Header, FMargin(0.0f, 0.0f, 0.0f, 14.0f));

	UVerticalBox* HeaderCopy = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("HeaderCopy"));
	Header->AddChildToHorizontalBox(HeaderCopy);
	InventoryUI::SetHorizontalFill(HeaderCopy, 1.0f);

	UTextBlock* Title = InventoryUI::MakeText(WidgetTree, HeaderTitle.ToString(), 30, AccentColor);
	HeaderCopy->AddChildToVerticalBox(Title);

	UTextBlock* Subtitle = InventoryUI::MakeText(WidgetTree, HeaderSubtitle.ToString(), 14, InventoryUI::Muted);
	HeaderCopy->AddChildToVerticalBox(Subtitle);
	InventoryUI::SetVerticalSlot(Subtitle, FMargin(1.0f, 3.0f, 0.0f, 0.0f));

	CapacityText = InventoryUI::MakeText(WidgetTree, TEXT("0 / 24 ЯЧЕЕК"), 16, InventoryUI::PrimaryText);
	CapacityText->SetJustification(ETextJustify::Right);
	Header->AddChildToHorizontalBox(CapacityText);
	if (UHorizontalBoxSlot* CapacitySlot = Cast<UHorizontalBoxSlot>(CapacityText->Slot))
	{
		CapacitySlot->SetVerticalAlignment(VAlign_Center);
		CapacitySlot->SetPadding(FMargin(20.0f, 0.0f));
	}

	UButton* CloseButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("CloseButton"));
	CloseButton->SetBackgroundColor(FLinearColor(0.14f, 0.16f, 0.14f, 1.0f));
	CloseButton->OnClicked.AddDynamic(this, &UInventoryWidget::HandleCloseClicked);
	Header->AddChildToHorizontalBox(CloseButton);
	UTextBlock* CloseLabel = InventoryUI::MakeText(WidgetTree, TEXT("  X  "), 18);
	CloseButton->SetContent(CloseLabel);

	UBorder* Divider = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("Divider"));
	Divider->SetBrushColor(FLinearColor(0.24f, 0.28f, 0.19f, 1.0f));
	Divider->SetDesiredSizeScale(FVector2D(1.0f, 0.07f));
	MainLayout->AddChildToVerticalBox(Divider);
	InventoryUI::SetVerticalSlot(Divider, FMargin(0.0f, 0.0f, 0.0f, 18.0f));

	UHorizontalBox* Body = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("Body"));
	MainLayout->AddChildToVerticalBox(Body);
	InventoryUI::SetVerticalSlot(Body, FMargin(), ESlateSizeRule::Fill);

	UBorder* BackpackPanel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("BackpackPanel"));
	BackpackPanel->SetBrushColor(InventoryUI::SubPanel);
	BackpackPanel->SetPadding(FMargin(18.0f));
	Body->AddChildToHorizontalBox(BackpackPanel);
	InventoryUI::SetHorizontalFill(BackpackPanel, 2.1f, FMargin(0.0f, 0.0f, 16.0f, 0.0f));

	UVerticalBox* BackpackLayout = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("BackpackLayout"));
	BackpackPanel->SetContent(BackpackLayout);

	UTextBlock* BackpackTitleText = InventoryUI::MakeText(WidgetTree, BackpackTitle.ToString(), 19);
	BackpackLayout->AddChildToVerticalBox(BackpackTitleText);
	InventoryUI::SetVerticalSlot(BackpackTitleText, FMargin(2.0f, 0.0f, 0.0f, 12.0f));

	SlotGrid = WidgetTree->ConstructWidget<UUniformGridPanel>(UUniformGridPanel::StaticClass(), TEXT("SlotGrid"));
	SlotGrid->SetSlotPadding(FMargin(5.0f));
	SlotGrid->SetMinDesiredSlotWidth(98.0f);
	SlotGrid->SetMinDesiredSlotHeight(98.0f);
	BackpackLayout->AddChildToVerticalBox(SlotGrid);
	InventoryUI::SetVerticalSlot(SlotGrid, FMargin(), ESlateSizeRule::Fill);

	UTextBlock* GridHintText = InventoryUI::MakeText(WidgetTree, GridHint.ToString(), 13, InventoryUI::Muted);
	BackpackLayout->AddChildToVerticalBox(GridHintText);
	InventoryUI::SetVerticalSlot(GridHintText, FMargin(3.0f, 12.0f, 0.0f, 0.0f));

	UBorder* DetailsPanel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("DetailsPanel"));
	DetailsPanel->SetBrushColor(FLinearColor(0.035f, 0.041f, 0.037f, 1.0f));
	DetailsPanel->SetPadding(FMargin(24.0f));
	Body->AddChildToHorizontalBox(DetailsPanel);
	InventoryUI::SetHorizontalFill(DetailsPanel, 1.0f);

	UVerticalBox* DetailsLayout = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("DetailsLayout"));
	DetailsPanel->SetContent(DetailsLayout);

	UTextBlock* DetailsTitle = InventoryUI::MakeText(WidgetTree, TEXT("ПРЕДМЕТ"), 13, InventoryUI::Muted);
	DetailsLayout->AddChildToVerticalBox(DetailsTitle);

	USizeBox* PreviewSize = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("PreviewSize"));
	PreviewSize->SetHeightOverride(190.0f);
	DetailsLayout->AddChildToVerticalBox(PreviewSize);
	InventoryUI::SetVerticalSlot(PreviewSize, FMargin(0.0f, 14.0f, 0.0f, 18.0f));

	UBorder* Preview = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("Preview"));
	Preview->SetBrushColor(FLinearColor(0.08f, 0.095f, 0.07f, 1.0f));
	PreviewSize->SetContent(Preview);
	UOverlay* PreviewOverlay = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("PreviewOverlay"));
	Preview->SetContent(PreviewOverlay);
	PreviewImage = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("PreviewImage"));
	if (UOverlaySlot* PreviewImageSlot = PreviewOverlay->AddChildToOverlay(PreviewImage))
	{
		PreviewImageSlot->SetHorizontalAlignment(HAlign_Fill);
		PreviewImageSlot->SetVerticalAlignment(VAlign_Fill);
		PreviewImageSlot->SetPadding(FMargin(18.0f));
	}
	PreviewFallback = InventoryUI::MakeText(WidgetTree, TEXT("+"), 86, AccentColor);
	PreviewFallback->SetJustification(ETextJustify::Center);
	if (UOverlaySlot* PreviewFallbackSlot = PreviewOverlay->AddChildToOverlay(PreviewFallback))
	{
		PreviewFallbackSlot->SetHorizontalAlignment(HAlign_Fill);
		PreviewFallbackSlot->SetVerticalAlignment(VAlign_Center);
	}

	ItemNameText = InventoryUI::MakeText(WidgetTree, TEXT("НЕТ ПРЕДМЕТА"), 24, InventoryUI::PrimaryText);
	DetailsLayout->AddChildToVerticalBox(ItemNameText);

	ItemQuantityText = InventoryUI::MakeText(WidgetTree, TEXT("КОЛИЧЕСТВО: 0"), 15, InventoryUI::Accent);
	DetailsLayout->AddChildToVerticalBox(ItemQuantityText);
	InventoryUI::SetVerticalSlot(ItemQuantityText, FMargin(0.0f, 6.0f, 0.0f, 16.0f));

	USizeBox* DescriptionSize = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("DescriptionSize"));
	DescriptionSize->SetHeightOverride(98.0f);
	DetailsLayout->AddChildToVerticalBox(DescriptionSize);

	ItemDescriptionText = InventoryUI::MakeText(WidgetTree, TEXT("Выберите предмет в рюкзаке."), 14, InventoryUI::Muted);
	ItemDescriptionText->SetAutoWrapText(true);
	DescriptionSize->SetContent(ItemDescriptionText);

	DropOneButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("DropOneButton"));
	DropOneButton->SetBackgroundColor(FLinearColor(0.42f, 0.22f, 0.08f, 1.0f));
	DropOneButton->OnClicked.AddDynamic(this, &UInventoryWidget::HandleDropClicked);
	DetailsLayout->AddChildToVerticalBox(DropOneButton);
	InventoryUI::SetVerticalSlot(DropOneButton, FMargin(0.0f, 8.0f, 0.0f, 8.0f));
	UTextBlock* DropOneLabelText = InventoryUI::MakeText(WidgetTree, DropOneLabel.ToString(), 15, FLinearColor::White);
	DropOneLabelText->SetJustification(ETextJustify::Center);
	DropOneLabelText->SetMargin(FMargin(0.0f, 10.0f));
	DropOneButton->SetContent(DropOneLabelText);

	DropAllButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("DropAllButton"));
	DropAllButton->SetBackgroundColor(FLinearColor(0.17f, 0.18f, 0.15f, 1.0f));
	DropAllButton->OnClicked.AddDynamic(this, &UInventoryWidget::HandleDropAllClicked);
	DetailsLayout->AddChildToVerticalBox(DropAllButton);
	UTextBlock* DropAllLabelText = InventoryUI::MakeText(WidgetTree, DropAllLabel.ToString(), 14, InventoryUI::PrimaryText);
	DropAllLabelText->SetJustification(ETextJustify::Center);
	DropAllLabelText->SetMargin(FMargin(0.0f, 9.0f));
	DropAllButton->SetContent(DropAllLabelText);
}

void UInventoryWidget::RefreshInventory()
{
	if (!SlotGrid || !CapacityText)
	{
		return;
	}

	// Проще и надёжнее пересобрать небольшую сетку из 24 слотов целиком.
	SlotGrid->ClearChildren();
	const int32 SlotCount = Inventory && Inventory->MaxSlots > 0 ? Inventory->MaxSlots : InventoryUI::DefaultSlots;
	const int32 UsedSlots = Inventory ? Inventory->GetItems().Num() : 0;
	int32 TotalItems = 0;

	if (Inventory)
	{
		for (const FInventoryItemStack& Stack : Inventory->GetItems())
		{
			TotalItems += Stack.Quantity;
		}
	}

	if (!Inventory || Inventory->GetItems().IsEmpty())
	{
		SelectedItemId = NAME_None;
		SelectedStackIndex = INDEX_NONE;
	}
	else
	{
		// Если выбранный стак исчез после удаления, выбираем первый оставшийся.
		if (!Inventory->GetItems().IsValidIndex(SelectedStackIndex))
		{
			SelectedStackIndex = 0;
		}
		SelectedItemId = Inventory->GetItems()[SelectedStackIndex].ItemId;
	}

	for (int32 Index = 0; Index < SlotCount; ++Index)
	{
		// Для каждого номера создаём либо пустую ячейку, либо кнопку со стаком.
		USizeBox* CellSize = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
		CellSize->SetWidthOverride(98.0f);
		CellSize->SetHeightOverride(98.0f);
		SlotGrid->AddChildToUniformGrid(CellSize, Index / InventoryUI::Columns, Index % InventoryUI::Columns);

		if (!Inventory || !Inventory->GetItems().IsValidIndex(Index))
		{
			UBorder* EmptyCell = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
			EmptyCell->SetBrushColor(InventoryUI::Slot);
			EmptyCell->SetPadding(FMargin(8.0f));
			CellSize->SetContent(EmptyCell);
			UTextBlock* IndexText = InventoryUI::MakeText(WidgetTree, FString::FromInt(Index + 1), 11,
				FLinearColor(0.22f, 0.25f, 0.22f, 1.0f));
			EmptyCell->SetContent(IndexText);
			continue;
		}

		const FInventoryItemStack& Stack = Inventory->GetItems()[Index];
		UInventorySlotButton* ItemButton = WidgetTree->ConstructWidget<UInventorySlotButton>(UInventorySlotButton::StaticClass());
		ItemButton->SetInventoryIndex(Index);
		ItemButton->SetBackgroundColor(Index == SelectedStackIndex ? InventoryUI::SelectedSlot : InventoryUI::Slot);
		ItemButton->OnSlotSelected.AddDynamic(this, &UInventoryWidget::HandleSlotSelected);
		CellSize->SetContent(ItemButton);

		UOverlay* ItemOverlay = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass());
		ItemButton->SetContent(ItemOverlay);

		UTextBlock* ItemLabel = InventoryUI::MakeText(WidgetTree, InventoryUI::GetItemName(Stack).ToString(), 11,
			InventoryUI::PrimaryText);
		if (UOverlaySlot* LabelSlot = ItemOverlay->AddChildToOverlay(ItemLabel))
		{
			LabelSlot->SetHorizontalAlignment(HAlign_Left);
			LabelSlot->SetVerticalAlignment(VAlign_Top);
			LabelSlot->SetPadding(FMargin(8.0f, 6.0f));
		}

		USizeBox* IconSize = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
		IconSize->SetWidthOverride(48.0f);
		IconSize->SetHeightOverride(48.0f);
		if (UOverlaySlot* IconSlot = ItemOverlay->AddChildToOverlay(IconSize))
		{
			IconSlot->SetHorizontalAlignment(HAlign_Center);
			IconSlot->SetVerticalAlignment(VAlign_Center);
		}

		if (Stack.Definition && Stack.Definition->InventoryIcon)
		{
			UImage* IconImage = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());
			IconImage->SetBrushFromTexture(Stack.Definition->InventoryIcon, true);
			IconSize->SetContent(IconImage);
		}
		else
		{
			UBorder* Cube = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
			Cube->SetBrushColor(Stack.Definition ? Stack.Definition->InventoryColor
				: FLinearColor(0.32f, 0.39f, 0.17f, 1.0f));
			IconSize->SetContent(Cube);
			UTextBlock* CubeMark = InventoryUI::MakeText(WidgetTree, TEXT("+"), 30, FLinearColor(0.92f, 0.94f, 0.82f, 1.0f));
			CubeMark->SetJustification(ETextJustify::Center);
			Cube->SetContent(CubeMark);
		}

		UBorder* QuantityBadge = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
		QuantityBadge->SetBrushColor(FLinearColor(0.015f, 0.018f, 0.015f, 0.95f));
		QuantityBadge->SetPadding(FMargin(7.0f, 3.0f));
		if (UOverlaySlot* BadgeSlot = ItemOverlay->AddChildToOverlay(QuantityBadge))
		{
			BadgeSlot->SetHorizontalAlignment(HAlign_Right);
			BadgeSlot->SetVerticalAlignment(VAlign_Bottom);
			BadgeSlot->SetPadding(FMargin(5.0f));
		}
		UTextBlock* Quantity = InventoryUI::MakeText(WidgetTree,
			FString::Printf(TEXT("x%d"), Stack.Quantity), 13, FLinearColor::White);
		QuantityBadge->SetContent(Quantity);
	}

	CapacityText->SetText(FText::FromString(FString::Printf(TEXT("%d / %d ЯЧЕЕК   •   %d ПРЕДМЕТОВ"),
		UsedSlots, SlotCount, TotalItems)));
	UpdateSelectionPanel();
}

void UInventoryWidget::UpdateSelectionPanel()
{
	if (!ItemNameText || !ItemQuantityText || !ItemDescriptionText || !DropOneButton || !DropAllButton)
	{
		return;
	}

	// Count считает этот вид предмета во всех стаках, а SelectedStack показывает
	// содержимое только выбранной ячейки.
	const int32 Count = Inventory && !SelectedItemId.IsNone() ? Inventory->GetItemCount(SelectedItemId) : 0;
	const bool bHasSelection = Count > 0;
	const FInventoryItemStack* SelectedStack = Inventory && Inventory->GetItems().IsValidIndex(SelectedStackIndex)
		? &Inventory->GetItems()[SelectedStackIndex]
		: nullptr;

	ItemNameText->SetText(SelectedStack ? InventoryUI::GetItemName(*SelectedStack)
		: FText::FromString(TEXT("НЕТ ПРЕДМЕТА")));
	ItemQuantityText->SetText(SelectedStack
		? FText::FromString(FString::Printf(TEXT("СТАК: %d / %d   •   ВСЕГО: %d"),
			SelectedStack->Quantity, SelectedStack->MaxStackSize, Count))
		: FText::FromString(TEXT("КОЛИЧЕСТВО: 0")));
	ItemDescriptionText->SetText(SelectedStack ? InventoryUI::GetItemDescription(*SelectedStack)
		: FText::FromString(TEXT("Выберите предмет в рюкзаке.")));
	if (PreviewImage && PreviewFallback)
	{
		const bool bHasIcon = SelectedStack && SelectedStack->Definition && SelectedStack->Definition->InventoryIcon;
		PreviewImage->SetVisibility(bHasIcon ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
		PreviewFallback->SetVisibility(bHasIcon ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
		if (bHasIcon)
		{
			PreviewImage->SetBrushFromTexture(SelectedStack->Definition->InventoryIcon, true);
		}
	}
	DropOneButton->SetIsEnabled(bHasSelection);
	DropAllButton->SetIsEnabled(bHasSelection);
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

void UInventoryWidget::HandleDropClicked()
{
	// Вся надёжная проверка и создание объекта находятся в InventoryComponent.
	if (Inventory && !SelectedItemId.IsNone())
	{
		Inventory->DropItem(SelectedItemId, 1);
	}
}

void UInventoryWidget::HandleDropAllClicked()
{
	// GetItemCount позволяет выбросить сразу все стаки выбранного вида.
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

	// Персонаж получает событие и возвращает управление камерой и движением.
	OnInventoryClosed.Broadcast();
}
