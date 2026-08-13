// логика кнопки слота

#include "UI/Inventory/InventorySlotButton.h"

UInventorySlotButton::UInventorySlotButton()
{
	// Переводим обычное событие кнопки в событие с номером слота.
	OnClicked.AddDynamic(this, &UInventorySlotButton::HandleClicked);
}

void UInventorySlotButton::SetInventoryIndex(const int32 InSlotIndex)
{
	SlotIndex = InSlotIndex;
}

void UInventorySlotButton::HandleClicked()
{
	// Главный виджет узнает, какую именно ячейку выбрал игрок.
	OnSlotSelected.Broadcast(SlotIndex);
}
