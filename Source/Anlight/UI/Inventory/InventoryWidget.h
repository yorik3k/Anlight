// окно инвентаря

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InventoryWidget.generated.h"

class UInventoryComponent;
class UButton;
class UImage;
class UTextBlock;
class UUniformGridPanel;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryWidgetClosed);

// Общее окно инвентаря для основного и тестового персонажей.
// Оно только показывает данные и передаёт команды настоящему InventoryComponent.
UCLASS(Blueprintable)
class ANLIGHT_API UInventoryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void InitializeInventory(UInventoryComponent* InInventory);

	// сообщает персонажу что окно закрыли кнопкой X
	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnInventoryWidgetClosed OnInventoryClosed;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory UI|Text")
	FText HeaderTitle = FText::FromString(TEXT("ИНВЕНТАРЬ пока что красивый"));

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory UI|Text")
	FText HeaderSubtitle = FText::FromString(TEXT("ТЕСТОВЫЙ ПЕРСОНАЖ  •  РЮКЗАК"));

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory UI|Text")
	FText BackpackTitle = FText::FromString(TEXT("РЮКЗАК"));

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory UI|Text")
	FText GridHint = FText::FromString(TEXT("ЛКМ — выбрать предмет    •    I — закрыть инвентарь"));

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory UI|Text")
	FText DropOneLabel = FText::FromString(TEXT("ВЫБРОСИТЬ 1"));

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory UI|Text")
	FText DropAllLabel = FText::FromString(TEXT("ВЫБРОСИТЬ ВСЁ"));

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory UI|Style")
	FLinearColor AccentColor = FLinearColor(0.68f, 0.76f, 0.35f, 1.0f);

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeDestruct() override;

private:
	// Создаёт стандартный внешний вид, если Blueprint не построил свой Widget Tree.
	void BuildWidgetTree();

	// Полностью обновляет слоты после любого изменения содержимого рюкзака.
	UFUNCTION()
	void RefreshInventory();

	UFUNCTION()
	void HandleSlotSelected(int32 SlotIndex);

	UFUNCTION()
	void HandleDropClicked();

	UFUNCTION()
	void HandleDropAllClicked();

	UFUNCTION()
	void HandleCloseClicked();

	void UpdateSelectionPanel();

	UPROPERTY()
	TObjectPtr<UInventoryComponent> Inventory;

	UPROPERTY()
	TObjectPtr<UUniformGridPanel> SlotGrid;

	UPROPERTY()
	TObjectPtr<UTextBlock> ItemNameText;

	UPROPERTY()
	TObjectPtr<UTextBlock> ItemQuantityText;

	UPROPERTY()
	TObjectPtr<UTextBlock> ItemDescriptionText;

	UPROPERTY()
	TObjectPtr<UTextBlock> CapacityText;

	UPROPERTY()
	TObjectPtr<UImage> PreviewImage;

	UPROPERTY()
	TObjectPtr<UTextBlock> PreviewFallback;

	UPROPERTY()
	TObjectPtr<UButton> DropOneButton;

	UPROPERTY()
	TObjectPtr<UButton> DropAllButton;

	// Храним и номер стака, и Item Id: номер нужен для подсветки,
	// а Item Id — для подсчёта и выбрасывания всех одинаковых предметов.
	FName SelectedItemId = NAME_None;
	int32 SelectedStackIndex = INDEX_NONE;
};
