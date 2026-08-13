// предмет в игровом мире

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/Inventory/InventoryTypes.h"
#include "InventoryWorldItem.generated.h"

class UInventoryComponent;
class UInventoryItemDefinition;
class UStaticMeshComponent;

// Предмет, который лежит в игровом мире и может быть подобран.
// Если своей модели пока нет, используется маленький тестовый куб.
UCLASS(Blueprintable)
class ANLIGHT_API AInventoryWorldItem : public AActor
{
	GENERATED_BODY()

public:
	AInventoryWorldItem();
	virtual void OnConstruction(const FTransform& Transform) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<UStaticMeshComponent> ItemMesh;

	// Количество и служебные данные именно этого объекта в мире.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
	FInventoryItemStack ItemStack;

	// Это можно назначить в Blueprint или прямо на объекте карты.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<UInventoryItemDefinition> ItemDefinition;

	void InitializeItem(FName ItemId, int32 Quantity, int32 MaxStackSize);

	// Заполняет ItemStack данными из Data Asset.
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void InitializeItemDefinition(UInventoryItemDefinition* Definition, int32 Quantity = 1);
	void SetDroppedPhysicsEnabled(bool bEnabled);

	// Переносит столько предметов, сколько поместится в целевой инвентарь.
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 PickUp(UInventoryComponent* TargetInventory);
};
