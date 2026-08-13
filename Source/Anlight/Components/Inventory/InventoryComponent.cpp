// логика инвентаря

#include "Components/Inventory/InventoryComponent.h"

#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Items/InventoryItemDefinition.h"
#include "World/Items/InventoryWorldItem.h"

UInventoryComponent::UInventoryComponent()
{
	// Инвентарь меняется только по вызову функций, поэтому Tick ему не нужен.
	PrimaryComponentTick.bCanEverTick = false;
}

int32 UInventoryComponent::AddItem(const FName ItemId, const int32 Quantity,
	const int32 MaxStackSize, const TSubclassOf<AInventoryWorldItem> WorldItemClass)
{
	return AddItemInternal(ItemId, Quantity, MaxStackSize, WorldItemClass, nullptr);
}

int32 UInventoryComponent::AddItemDefinition(UInventoryItemDefinition* ItemDefinition, const int32 Quantity)
{
	// Предмет без Data Asset или без уникального Item Id нельзя надёжно хранить.
	if (!ItemDefinition || ItemDefinition->ItemId.IsNone())
	{
		return 0;
	}

	return AddItemInternal(
		ItemDefinition->ItemId,
		Quantity,
		ItemDefinition->MaxStackSize,
		ItemDefinition->WorldItemClass,
		ItemDefinition);
}

int32 UInventoryComponent::AddItemInternal(const FName ItemId, const int32 Quantity,
	const int32 MaxStackSize, const TSubclassOf<AInventoryWorldItem> WorldItemClass,
	UInventoryItemDefinition* ItemDefinition)
{
	if (ItemId.IsNone() || Quantity <= 0)
	{
		return 0;
	}

	const int32 SafeMaxStackSize = FMath::Max(1, MaxStackSize);
	int32 Remaining = Quantity;

	// Сначала заполняем уже существующие неполные стаки этого предмета.
	for (FInventoryItemStack& Stack : ItemStacks)
	{
		if (Stack.ItemId != ItemId || Stack.Quantity >= Stack.MaxStackSize)
		{
			continue;
		}

		const int32 AddedToStack = FMath::Min(Remaining, Stack.MaxStackSize - Stack.Quantity);
		Stack.Quantity += AddedToStack;
		Remaining -= AddedToStack;

		// Старый стак мог быть создан без Data Asset. Если появились дополнительные
		// данные, сохраняем их, чтобы предмет правильно отображался и выбрасывался.
		if (!Stack.WorldItemClass && WorldItemClass)
		{
			Stack.WorldItemClass = WorldItemClass;
		}
		if (!Stack.Definition && ItemDefinition)
		{
			Stack.Definition = ItemDefinition;
		}

		if (Remaining == 0)
		{
			break;
		}
	}

	// Остаток раскладываем по новым стакам, пока есть свободные слоты.
	while (Remaining > 0 && (MaxSlots == 0 || ItemStacks.Num() < MaxSlots))
	{
		FInventoryItemStack& NewStack = ItemStacks.AddDefaulted_GetRef();
		NewStack.ItemId = ItemId;
		NewStack.MaxStackSize = SafeMaxStackSize;
		NewStack.Quantity = FMath::Min(Remaining, SafeMaxStackSize);
		NewStack.WorldItemClass = WorldItemClass;
		NewStack.Definition = ItemDefinition;
		Remaining -= NewStack.Quantity;
	}

	const int32 Added = Quantity - Remaining;
	if (Added > 0)
	{
		// Интерфейс подписан на это событие и сам обновит сетку.
		OnInventoryChanged.Broadcast();
	}

	return Added;
}

int32 UInventoryComponent::RemoveItem(const FName ItemId, const int32 Quantity)
{
	if (ItemId.IsNone() || Quantity <= 0)
	{
		return 0;
	}

	int32 Remaining = Quantity;
	// Идём с конца, чтобы удаление пустого стака не сбивало ещё не проверенные индексы.
	for (int32 Index = ItemStacks.Num() - 1; Index >= 0 && Remaining > 0; --Index)
	{
		FInventoryItemStack& Stack = ItemStacks[Index];
		if (Stack.ItemId != ItemId)
		{
			continue;
		}

		const int32 RemovedFromStack = FMath::Min(Remaining, Stack.Quantity);
		Stack.Quantity -= RemovedFromStack;
		Remaining -= RemovedFromStack;

		if (Stack.Quantity == 0)
		{
			ItemStacks.RemoveAt(Index);
		}
	}

	const int32 Removed = Quantity - Remaining;
	if (Removed > 0)
	{
		OnInventoryChanged.Broadcast();
	}

	return Removed;
}

int32 UInventoryComponent::GetItemCount(const FName ItemId) const
{
	int32 Total = 0;
	for (const FInventoryItemStack& Stack : ItemStacks)
	{
		if (Stack.ItemId == ItemId)
		{
			Total += Stack.Quantity;
		}
	}

	return Total;
}

bool UInventoryComponent::DropItem(const FName ItemId, const int32 Quantity, const float ForwardDistance)
{
	// Сначала убеждаемся, что предметы есть и что владелец находится в игровом мире.
	if (Quantity <= 0 || GetItemCount(ItemId) < Quantity || !GetOwner() || !GetWorld())
	{
		return false;
	}

	// Берём класс мира и Data Asset из первого подходящего стака.
	// Если класс не настроен, используется обычный AInventoryWorldItem.
	TSubclassOf<AInventoryWorldItem> DropClass = AInventoryWorldItem::StaticClass();
	UInventoryItemDefinition* Definition = nullptr;
	int32 MaxStackSize = Quantity;
	for (const FInventoryItemStack& Stack : ItemStacks)
	{
		if (Stack.ItemId == ItemId)
		{
			MaxStackSize = Stack.MaxStackSize;
			if (Stack.WorldItemClass)
			{
				DropClass = Stack.WorldItemClass;
			}
			Definition = Stack.Definition;
			break;
		}
	}

	const FVector SpawnLocation = GetOwner()->GetActorLocation()
		+ GetOwner()->GetActorForwardVector() * ForwardDistance
		+ FVector(0.0f, 0.0f, 50.0f);
	const FTransform SpawnTransform(GetOwner()->GetActorRotation(), SpawnLocation);

	// Deferred Spawn позволяет записать данные предмета до запуска Construction Script.
	AInventoryWorldItem* DroppedItem = GetWorld()->SpawnActorDeferred<AInventoryWorldItem>(
		DropClass,
		SpawnTransform,
		GetOwner(),
		Cast<APawn>(GetOwner()),
		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);

	if (!DroppedItem)
	{
		return false;
	}

	if (Definition)
	{
		DroppedItem->InitializeItemDefinition(Definition, Quantity);
	}
	else
	{
		DroppedItem->InitializeItem(ItemId, Quantity, MaxStackSize);
	}
	DroppedItem->FinishSpawning(SpawnTransform);
	DroppedItem->SetDroppedPhysicsEnabled(true);

	// Удаляем предмет из рюкзака только после успешного создания объекта.
	return RemoveItem(ItemId, Quantity) == Quantity;
}
