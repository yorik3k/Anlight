// данные инвентаря

#pragma once

#include "CoreMinimal.h"
#include "InventoryTypes.generated.h"

class AInventoryWorldItem;
class UInventoryItemDefinition;

// Один занятый слот инвентаря. Например: пять бинтов в одном стаке.
USTRUCT(BlueprintType)
struct ANLIGHT_API FInventoryItemStack
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
	FName ItemId = NAME_None;

	// Сколько предметов сейчас лежит в этом стаке.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (ClampMin = "1"))
	int32 Quantity = 1;

	// Больше этого количества в один стак положить нельзя.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (ClampMin = "1"))
	int32 MaxStackSize = 1;

	// Какой объект появится в мире при выбрасывании предмета.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
	TSubclassOf<AInventoryWorldItem> WorldItemClass;

	// Data Asset с названием, описанием, иконкой и остальными настройками.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<UInventoryItemDefinition> Definition;

	//== сетка

	// размер предмета в клетках
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Grid", meta = (ClampMin = "1"))
	int32 Width = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Grid", meta = (ClampMin = "1"))
	int32 Height = 1;

	// позиция левого верхнего угла в сетке
		// -1 = предмет не размещён (например, только что добавлен, но место не найдено)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Grid")
	int32 PositionX = -1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Grid")
	int32 PositionY = -1;

	// В каком контейнере лежит (0 = рюкзак, 1 = карманы и т.д.)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Grid")
	int32 ContainerIndex = 0;

	bool IsValid() const
	{
		// Пустое имя, нулевое количество или неправильный размер стака означают,
		// что такую запись нельзя показывать или подбирать.
		return !ItemId.IsNone() && Quantity > 0 && MaxStackSize > 0;
	}
};
	USTRUCT(BlueprintType)
		struct ANLIGHT_API FContainerData
	{
		GENERATED_BODY()

		// unical name - pokets, backpack, etc.
		UPROPERTY(EditAnywhere,BlueprintReadOnly,Category = "Container")
		FName ContainerId = NAME_None;

		// name for UI
		UPROPERTY(EditAnywhere, BlueprintReadOnly,Category = "Container")
		FText DisplayName;

		// размер сетки в клетках
		UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Container", meta = (ClampMin = "1"))
		int32 Width = 6;

		UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Container", meta = (ClampMin = "1"))
		int32 Height = 5;

		// Какие типы предметов можно класть (пусто = всё)
		UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Container")
		TArray<FName> AllowedItemTypes;
	};
