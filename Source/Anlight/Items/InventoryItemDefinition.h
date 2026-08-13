// настройки предмета

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "InventoryItemDefinition.generated.h"

class AInventoryWorldItem;
class UTexture2D;

// Набор настраиваемых данных одного вида предмета.
// Благодаря Data Asset новые обычные предметы создаются без изменения C++.
UCLASS(BlueprintType)
class ANLIGHT_API UInventoryItemDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// Уникальное внутреннее имя. У разных видов предметов оно не должно совпадать.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	FName ItemId = NAME_None;

	// Название и описание, которые увидит игрок в интерфейсе.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item", meta = (MultiLine = "true"))
	FText Description;

	// Если иконка не назначена, интерфейс покажет цветной квадрат.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	TObjectPtr<UTexture2D> InventoryIcon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	FLinearColor InventoryColor = FLinearColor(0.32f, 0.39f, 0.17f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (ClampMin = "1"))
	int32 MaxStackSize = 1;

	// Blueprint предмета, который лежит на карте или появляется после выбрасывания.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "World")
	TSubclassOf<AInventoryWorldItem> WorldItemClass;
};
