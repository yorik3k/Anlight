// внешний вид и работа подсказки подбора

#include "UI/Interaction/InteractionPromptWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/Inventory/InventoryTypes.h"
#include "Items/InventoryItemDefinition.h"

namespace InteractionPrompt
{
	// Небольшие вспомогательные функции используются только этим виджетом.
	void SetFontSize(UTextBlock* Text, const int32 Size)
	{
		FSlateFontInfo Font = Text->GetFont();
		Font.Size = Size;
		Text->SetFont(Font);
	}

	FText GetDisplayName(const FInventoryItemStack& Stack)
	{
		if (Stack.Definition && !Stack.Definition->DisplayName.IsEmpty())
		{
			return Stack.Definition->DisplayName;
		}

		return Stack.ItemId == FName(TEXT("Bandage"))
			? FText::FromString(TEXT("БИНТ"))
			: FText::FromName(Stack.ItemId);
	}
}

void UInteractionPromptWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (!WidgetTree->RootWidget)
	{
		// Пустой Blueprint всё равно получит полностью рабочую подсказку.
		BuildWidgetTree();
	}

	HidePrompt();
}

void UInteractionPromptWidget::ShowItem(const FInventoryItemStack& ItemStack)
{
	if (!ItemNameText || !ActionText)
	{
		return;
	}

	// Название берётся из Data Asset, а рядом показывается количество в мире.
	ItemNameText->SetText(FText::Format(
		FText::FromString(TEXT("{0}   x{1}")),
		InteractionPrompt::GetDisplayName(ItemStack),
		FText::AsNumber(ItemStack.Quantity)));
	ActionText->SetText(PickupText);
	SetVisibility(ESlateVisibility::HitTestInvisible);
}

void UInteractionPromptWidget::HidePrompt()
{
	SetVisibility(ESlateVisibility::Collapsed);
}

void UInteractionPromptWidget::BuildWidgetTree()
{
	// Подсказка строится кодом, а её тексты и цвет доступны в Class Defaults.
	UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("Root"));
	WidgetTree->RootWidget = Root;

	UBorder* Panel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("PromptPanel"));
	Panel->SetBrushColor(FLinearColor(0.015f, 0.02f, 0.017f, 0.94f));
	Panel->SetPadding(FMargin(14.0f, 9.0f));
	Root->AddChild(Panel);
	if (UCanvasPanelSlot* PanelSlot = Cast<UCanvasPanelSlot>(Panel->Slot))
	{
		PanelSlot->SetAnchors(FAnchors(0.5f, 0.68f));
		PanelSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		PanelSlot->SetSize(FVector2D(340.0f, 68.0f));
	}

	UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("PromptRow"));
	Panel->SetContent(Row);

	UBorder* KeyBadge = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("KeyBadge"));
	KeyBadge->SetBrushColor(AccentColor);
	KeyBadge->SetPadding(FMargin(16.0f, 8.0f));
	Row->AddChildToHorizontalBox(KeyBadge);
	if (UHorizontalBoxSlot* KeySlot = Cast<UHorizontalBoxSlot>(KeyBadge->Slot))
	{
		KeySlot->SetVerticalAlignment(VAlign_Center);
		KeySlot->SetPadding(FMargin(0.0f, 0.0f, 14.0f, 0.0f));
	}

	UTextBlock* KeyText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("KeyText"));
	KeyText->SetText(InteractionKeyText);
	KeyText->SetColorAndOpacity(FSlateColor(FLinearColor(0.03f, 0.04f, 0.025f, 1.0f)));
	InteractionPrompt::SetFontSize(KeyText, 22);
	KeyBadge->SetContent(KeyText);

	UVerticalBox* Copy = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("PromptCopy"));
	Row->AddChildToHorizontalBox(Copy);
	if (UHorizontalBoxSlot* CopySlot = Cast<UHorizontalBoxSlot>(Copy->Slot))
	{
		CopySlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		CopySlot->SetVerticalAlignment(VAlign_Center);
	}

	ItemNameText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("ItemName"));
	ItemNameText->SetColorAndOpacity(FSlateColor(FLinearColor(0.9f, 0.92f, 0.84f, 1.0f)));
	InteractionPrompt::SetFontSize(ItemNameText, 17);
	Copy->AddChildToVerticalBox(ItemNameText);

	ActionText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("Action"));
	ActionText->SetColorAndOpacity(FSlateColor(AccentColor));
	InteractionPrompt::SetFontSize(ActionText, 12);
	Copy->AddChildToVerticalBox(ActionText);
}
