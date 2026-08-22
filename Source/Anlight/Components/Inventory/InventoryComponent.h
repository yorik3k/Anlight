// компонент инвентаря

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/Inventory/InventoryTypes.h"
#include "InventoryComponent.generated.h"

class AInventoryWorldItem;
class UInventoryItemDefinition;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryChanged);

// Компонент инвентаря. Хранит предметы в сетке (EFT-стиль).
UCLASS(ClassGroup = (Inventory), meta = (BlueprintSpawnableComponent))
class ANLIGHT_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInventoryComponent();

	// Контейнеры инвентаря (рюкзак, карманы и т.д.)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
	TArray<FContainerData> Containers;

	// Вызывается при любом изменении инвентаря
	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnInventoryChanged OnInventoryChanged;

	// Добавляет предмет по отдельным параметрам
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 AddItem(FName ItemId, int32 Quantity = 1, int32 MaxStackSize = 99,
		TSubclassOf<AInventoryWorldItem> WorldItemClass = nullptr,
		int32 ItemWidth = 1, int32 ItemHeight = 1);

	// Основной способ добавления: все настройки из Data Asset
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 AddItemDefinition(UInventoryItemDefinition* ItemDefinition, int32 Quantity = 1);

	// Удаляет предметы из стаков
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 RemoveItem(FName ItemId, int32 Quantity = 1);

	// Сколько всего предметов данного типа
	UFUNCTION(BlueprintPure, Category = "Inventory")
	int32 GetItemCount(FName ItemId) const;

	// Проверка: можно ли разместить предмет в указанной позиции
	UFUNCTION(BlueprintPure, Category = "Inventory")
	bool CanPlaceItem(const FInventoryItemStack& Item, int32 ContainerIndex, int32 PosX, int32 PosY) const;

	// Ищет первое свободное место для предмета
	UFUNCTION(BlueprintPure, Category = "Inventory")
	bool FindFreeSpace(const FInventoryItemStack& Item, int32& OutContainerIndex, int32& OutPosX, int32& OutPosY) const;

	// Перемещает предмет на новую позицию
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool MoveItem(int32 StackIndex, int32 NewContainerIndex, int32 NewPosX, int32 NewPosY);

	// Копия всех стаков для Blueprint
	UFUNCTION(BlueprintPure, Category = "Inventory")
	TArray<FInventoryItemStack> GetItemStacks() const
	{
		return ItemStacks;
	}

	// Создаёт предмет перед персонажем
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool DropItem(FName ItemId, int32 Quantity = 1, float ForwardDistance = 150.0f);

	// Константная ссылка на стаки (для C++)
	const TArray<FInventoryItemStack>& GetItems() const
	{
		return ItemStacks;
	}

protected:
	virtual void BeginPlay() override;

private:
	// Общая внутренняя функция для добавления
	int32 AddItemInternal(FName ItemId, int32 Quantity, int32 MaxStackSize,
		TSubclassOf<AInventoryWorldItem> WorldItemClass, UInventoryItemDefinition* ItemDefinition,
		int32 ItemWidth = 1, int32 ItemHeight = 1);

	// Все предметы в инвентаре
	UPROPERTY(VisibleAnywhere, Category = "Inventory", meta = (AllowPrivateAccess = "true"))
	TArray<FInventoryItemStack> ItemStacks;
};