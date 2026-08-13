// компонент инвентаря

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/Inventory/InventoryTypes.h"
#include "InventoryComponent.generated.h"

class AInventoryWorldItem;
class UInventoryItemDefinition;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryChanged);

// Этот компонент можно добавить любому персонажу.
// Он хранит содержимое рюкзака, объединяет одинаковые предметы в стаки
// и отвечает за безопасное добавление, удаление и выбрасывание.
UCLASS(ClassGroup = (Inventory), meta = (BlueprintSpawnableComponent))
class ANLIGHT_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInventoryComponent();

	// Максимальное число занятых слотов. 0 означает, что ограничения нет.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (ClampMin = "0"))
	int32 MaxSlots = 24;

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnInventoryChanged OnInventoryChanged;

	// Добавляет предмет по отдельным параметрам и возвращает реальное добавленное количество.
	// Этот вариант полезен для тестов и предметов, у которых ещё нет Data Asset.
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 AddItem(FName ItemId, int32 Quantity = 1, int32 MaxStackSize = 99,
		TSubclassOf<AInventoryWorldItem> WorldItemClass = nullptr);

	// Основной способ добавления: все настройки берутся из Data Asset предмета.
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 AddItemDefinition(UInventoryItemDefinition* ItemDefinition, int32 Quantity = 1);

	// Удаляет предметы из стаков и возвращает реальное удалённое количество.
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 RemoveItem(FName ItemId, int32 Quantity = 1);

	UFUNCTION(BlueprintPure, Category = "Inventory")
	int32 GetItemCount(FName ItemId) const;

	// Копия всех занятых стаков для Blueprint.
	UFUNCTION(BlueprintPure, Category = "Inventory")
	TArray<FInventoryItemStack> GetItemStacks() const
	{
		return ItemStacks;
	}

	// Создаёт предмет перед персонажем. Из рюкзака он удаляется только после
	// успешного создания объекта в мире.
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool DropItem(FName ItemId, int32 Quantity = 1, float ForwardDistance = 150.0f);

	const TArray<FInventoryItemStack>& GetItems() const
	{
		return ItemStacks;
	}

private:
	// Общая внутренняя функция для обоих вариантов AddItem.
	int32 AddItemInternal(FName ItemId, int32 Quantity, int32 MaxStackSize,
		TSubclassOf<AInventoryWorldItem> WorldItemClass, UInventoryItemDefinition* ItemDefinition);

	UPROPERTY(VisibleAnywhere, Category = "Inventory", meta = (AllowPrivateAccess = "true"))
	TArray<FInventoryItemStack> ItemStacks;
};
