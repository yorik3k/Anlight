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

	bool IsValid() const
	{
		// Пустое имя, нулевое количество или неправильный размер стака означают,
		// что такую запись нельзя показывать или подбирать.
		return !ItemId.IsNone() && Quantity > 0 && MaxStackSize > 0;
	}
};
