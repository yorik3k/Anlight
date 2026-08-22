// окно инвентаря

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InventoryWidget.generated.h"

class UInventoryComponent;
class UButton;
class UImage;
class UTextBlock;
class UCanvasPanel;  
class UNutritionComponent;


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryWidgetClosed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemUsed, FName, ItemId);

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

	// ===== BindWidget — привязка к WBP =====
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Inventory UI")
	TObjectPtr<UCanvasPanel> SlotGrid;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Inventory UI")
	TObjectPtr<UTextBlock> Capacity;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Inventory UI")
	TObjectPtr<UTextBlock> Name;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Inventory UI")
	TObjectPtr<UTextBlock> ammount;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Inventory UI")
	TObjectPtr<UTextBlock> description;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Inventory UI")
	TObjectPtr<UImage> icon;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Inventory UI")
	TObjectPtr<UButton> Use;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Inventory UI")
	TObjectPtr<UButton> Drop_one;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Inventory UI")
	TObjectPtr<UButton> Drop_all;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Inventory UI")
	TObjectPtr<UButton> close;

	// weapon slots
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Inventory UI")
	TObjectPtr<UButton> WeapSlingButt;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Inventory UI")
	TObjectPtr<UButton> WeapSecondButt;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Inventory UI")
	TObjectPtr<UImage> WeapSlingImage;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Inventory UI")
	TObjectPtr<UImage> WeapSecondImage;

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnItemUsed OnItemUsed;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Inventory UI")
	TObjectPtr<UTextBlock> Hunger_capacity;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Inventory UI")
	TObjectPtr<UTextBlock> Thirst_capacity;

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeDestruct() override;

private:
	// Полностью обновляет слоты после любого изменения содержимого рюкзака.
	UFUNCTION()
	void RefreshInventory();

	UFUNCTION()
	void HandleSlotSelected(int32 SlotIndex);

	UFUNCTION()
	void HandleUseClicked();

	UFUNCTION()
	void HandleDropClicked();

	UFUNCTION()
	void HandleDropAllClicked();

	UFUNCTION()
	void HandleCloseClicked();

	void UpdateSelectionPanel();

	UFUNCTION()
	void HandleWeapSlingClicked();

	UFUNCTION()
	void HandleWeapSecondClicked();

	UPROPERTY()
	TObjectPtr<UInventoryComponent> Inventory;

	// Храним и номер стака, и Item Id: номер нужен для подсветки,
	// а Item Id — для подсчёта и выбрасывания всех одинаковых предметов.
	FName SelectedItemId = NAME_None;
	int32 SelectedStackIndex = INDEX_NONE;

	
	UPROPERTY()
	TObjectPtr<UNutritionComponent> NutritionComponent;

	UFUNCTION()
	void UpdateHungerText(float CurrentHunger, float MaxHunger);

	UFUNCTION()
	void UpdateThirstText(float CurrentThirst, float MaxThirst);
	// В private:
	FName EquippedWeaponSling = NAME_None;
	FName EquippedWeaponSecondary = NAME_None;
};