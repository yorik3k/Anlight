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

	// Создаём дефолтный контейнер — рюкзак 6x5
	if (Containers.IsEmpty())
	{
		FContainerData Backpack;
		Backpack.ContainerId = FName(TEXT("Backpack"));
		Backpack.DisplayName = FText::FromString(TEXT("Рюкзак"));
		Backpack.Width = 6;
		Backpack.Height = 5;
		Containers.Add(Backpack);
	}
}

void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
}

int32 UInventoryComponent::AddItem(const FName ItemId, const int32 Quantity,
	const int32 MaxStackSize, const TSubclassOf<AInventoryWorldItem> WorldItemClass,
	const int32 ItemWidth, const int32 ItemHeight)
{
	return AddItemInternal(ItemId, Quantity, MaxStackSize, WorldItemClass, nullptr, ItemWidth, ItemHeight);
}

int32 UInventoryComponent::AddItemDefinition(UInventoryItemDefinition* ItemDefinition, const int32 Quantity)
{
	if (!ItemDefinition || ItemDefinition->ItemId.IsNone())
	{
		return 0;
	}

	return AddItemInternal(
		ItemDefinition->ItemId,
		Quantity,
		ItemDefinition->MaxStackSize,
		ItemDefinition->WorldItemClass,
		ItemDefinition,
		ItemDefinition->Width,
		ItemDefinition->Height);
}

int32 UInventoryComponent::AddItemInternal(const FName ItemId, const int32 Quantity,
	const int32 MaxStackSize, const TSubclassOf<AInventoryWorldItem> WorldItemClass,
	UInventoryItemDefinition* ItemDefinition, const int32 ItemWidth, const int32 ItemHeight)
{
	if (ItemId.IsNone() || Quantity <= 0)
	{
		return 0;
	}

	const int32 SafeMaxStackSize = FMath::Max(1, MaxStackSize);
	const int32 SafeWidth = FMath::Max(1, ItemWidth);
	const int32 SafeHeight = FMath::Max(1, ItemHeight);
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

		// Дополняем данные, если старый стак был создан без них
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

	// Остаток раскладываем по новым стакам
	while (Remaining > 0)
	{
		FInventoryItemStack NewStack;
		NewStack.ItemId = ItemId;
		NewStack.MaxStackSize = SafeMaxStackSize;
		NewStack.Quantity = FMath::Min(Remaining, SafeMaxStackSize);
		NewStack.WorldItemClass = WorldItemClass;
		NewStack.Definition = ItemDefinition;
		NewStack.Width = SafeWidth;
		NewStack.Height = SafeHeight;

		// Ищем свободное место в сетке
		int32 ContainerIdx, PosX, PosY;
		if (!FindFreeSpace(NewStack, ContainerIdx, PosX, PosY))
		{
			// Нет места — прекращаем добавление
			break;
		}

		NewStack.ContainerIndex = ContainerIdx;
		NewStack.PositionX = PosX;
		NewStack.PositionY = PosY;

		ItemStacks.Add(NewStack);
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
	// Идём с конца, чтобы удаление пустого стака не сбивало индексы.
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

bool UInventoryComponent::CanPlaceItem(const FInventoryItemStack& Item,
	const int32 ContainerIndex, const int32 PosX, const int32 PosY) const
{
	// Проверяем, что контейнер существует
	if (!Containers.IsValidIndex(ContainerIndex))
	{
		return false;
	}

	const FContainerData& Container = Containers[ContainerIndex];

	// Проверяем границы: позиция не отрицательная и предмет не выходит за пределы
	if (PosX < 0 || PosY < 0 ||
		PosX + Item.Width > Container.Width ||
		PosY + Item.Height > Container.Height)
	{
		return false;
	}

	// Проверяем пересечение с другими предметами
	for (const FInventoryItemStack& Other : ItemStacks)
	{
		// Пропускаем предметы из других контейнеров или неразмещённые
		if (Other.ContainerIndex != ContainerIndex ||
			Other.PositionX < 0 || Other.PositionY < 0)
		{
			continue;
		}

		// Проверяем пересечение прямоугольников
		// Если хотя бы одно условие истинно — прямоугольники НЕ пересекаются
		const bool bNoOverlap =
			PosX + Item.Width <= Other.PositionX ||      // Новый левее существующего
			Other.PositionX + Other.Width <= PosX ||      // Новый правее существующего
			PosY + Item.Height <= Other.PositionY ||      // Новый выше существующего
			Other.PositionY + Other.Height <= PosY;       // Новый ниже существующего

		if (!bNoOverlap)
		{
			// Прямоугольники пересекаются — место занято
			return false;
		}
	}

	return true;
}

bool UInventoryComponent::FindFreeSpace(const FInventoryItemStack& Item,
	int32& OutContainerIndex, int32& OutPosX, int32& OutPosY) const
{
	// Перебираем все контейнеры
	for (int32 ContainerIdx = 0; ContainerIdx < Containers.Num(); ++ContainerIdx)
	{
		const FContainerData& Container = Containers[ContainerIdx];

		// Перебираем все возможные позиции
		// От 0 до (Container.Height - Item.Height) — чтобы предмет не выходил за границы
		for (int32 Y = 0; Y <= Container.Height - Item.Height; ++Y)
		{
			for (int32 X = 0; X <= Container.Width - Item.Width; ++X)
			{
				if (CanPlaceItem(Item, ContainerIdx, X, Y))
				{
					OutContainerIndex = ContainerIdx;
					OutPosX = X;
					OutPosY = Y;
					return true;
				}
			}
		}
	}

	return false;
}

bool UInventoryComponent::MoveItem(const int32 StackIndex,
	const int32 NewContainerIndex, const int32 NewPosX, const int32 NewPosY)
{
	if (!ItemStacks.IsValidIndex(StackIndex))
	{
		return false;
	}

	FInventoryItemStack& Item = ItemStacks[StackIndex];

	// Проверяем, можно ли разместить в новой позиции
	if (!CanPlaceItem(Item, NewContainerIndex, NewPosX, NewPosY))
	{
		return false;
	}

	// Перемещаем
	Item.ContainerIndex = NewContainerIndex;
	Item.PositionX = NewPosX;
	Item.PositionY = NewPosY;

	OnInventoryChanged.Broadcast();
	return true;
}

bool UInventoryComponent::DropItem(const FName ItemId, const int32 Quantity, const float ForwardDistance)
{
	// Сначала убеждаемся, что предметы есть и что владелец находится в игровом мире.
	if (Quantity <= 0 || GetItemCount(ItemId) < Quantity || !GetOwner() || !GetWorld())
	{
		return false;
	}

	// Берём класс мира и Data Asset из первого подходящего стака.
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