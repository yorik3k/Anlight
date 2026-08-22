// настройки предмета

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "InventoryItemDefinition.generated.h"

class AInventoryWorldItem;
class UTexture2D;

// Тип использования предмета — определяет, какая логика выполнится.
UENUM(BlueprintType)
enum class EItemUsageType : uint8
{
	None,
	PrimitiveMedkit,    // Аптечка (лечит случайную часть тела)
	TourniquetType,     // Жгут/турникет (останавливает кровотечение)
	PainkillerItem,     // Обезболивающее (снимает дебаффы, может давать отравление)
	Antidote,           // Антидот (снижает токсичность)
	Food,               // Еда (восстанавливает сытость)
	Water               // Вода (утоляет жажду)
};

UCLASS(BlueprintType)
class ANLIGHT_API UInventoryItemDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// Уникальное имя конкретного предмета
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	FName ItemId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item", meta = (MultiLine = "true"))
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	TObjectPtr<UTexture2D> InventoryIcon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	FLinearColor InventoryColor = FLinearColor(0.32f, 0.39f, 0.17f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (ClampMin = "1"))
	int32 MaxStackSize = 1;

	// ===== РАЗМЕРЫ В СЕТКЕ =====
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Grid", meta = (ClampMin = "1"))
	int32 Width = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Grid", meta = (ClampMin = "1"))
	int32 Height = 1;

	// Blueprint предмета в мире
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "World")
	TSubclassOf<AInventoryWorldItem> WorldItemClass;

	// Тип использования
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	EItemUsageType UsageType = EItemUsageType::None;

	// ===== ПАРАМЕТРЫ МЕДИЦИНЫ =====
	// Сила эффекта (лечение, остановка крови, снижение токсичности)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Medicine")
	float MedicinePower = 0.0f;

	// Сколько отравления даёт (для обезболивающих)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Medicine")
	float ToxicityAmount = 0.0f;

	// Длительность эффекта (сек) — для обезболивающих
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Medicine")
	float Duration = 0.0f;

	// ===== ПАРАМЕТРЫ ЕДЫ =====
	// Сколько сытости восстанавливает
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Food")
	float NutritionValue = 0.0f;

	// Сколько жажды утоляет
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Food")
	float HydrationValue = 0.0f;
};