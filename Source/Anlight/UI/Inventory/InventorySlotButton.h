// кнопка слота инвентаря

#pragma once

#include "CoreMinimal.h"
#include "Components/Button.h"
#include "InventorySlotButton.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInventorySlotSelected, int32, SlotIndex);

// Обычная UMG-кнопка, которая дополнительно запоминает номер своего слота.
UCLASS()
class ANLIGHT_API UInventorySlotButton : public UButton
{
	GENERATED_BODY()

public:
	UInventorySlotButton();

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnInventorySlotSelected OnSlotSelected;

	void SetInventoryIndex(int32 InSlotIndex);

private:
	UFUNCTION()
	void HandleClicked();

	int32 SlotIndex = INDEX_NONE;
};
