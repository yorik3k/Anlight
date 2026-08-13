// тестовый персонаж для инвентаря

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InventoryTestCharacter.generated.h"

class UCameraComponent;
class UInventoryComponent;
class UInventoryWidget;
class UInteractionPromptWidget;
class UInventoryItemDefinition;
class AInventoryWorldItem;

// Отдельный персонаж для проверки инвентаря.
// Он не используется на основной карте и не влияет на MainCharacter.
UCLASS()
class ANLIGHT_API AInventoryTestCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AInventoryTestCharacter();

	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<UInventoryComponent> Inventory;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> FirstPersonCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<UInventoryWidget> InventoryWidget;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
	TObjectPtr<UInteractionPromptWidget> InteractionPromptWidget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction", meta = (ClampMin = "50.0"))
	float InteractionDistance = 350.0f;

	// Эти классы можно заменить виджетами из Blueprint.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory|Blueprint")
	TSubclassOf<UInventoryWidget> InventoryWidgetClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory|Blueprint")
	TSubclassOf<UInteractionPromptWidget> InteractionPromptWidgetClass;

	// Первый предмет используется для начальных стаков и клавиш 1/2.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory|Blueprint")
	TObjectPtr<UInventoryItemDefinition> TestItemDefinition;

	// Второй предмет появляется рядом с первым только на тестовой карте.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory|Blueprint")
	TObjectPtr<UInventoryItemDefinition> SecondaryTestItemDefinition;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory|Blueprint")
	TSubclassOf<AInventoryWorldItem> TestWorldItemClass;

	UFUNCTION(BlueprintCallable, Category = "Inventory|Test")
	void AddTestItem();

	UFUNCTION(BlueprintCallable, Category = "Inventory|Test")
	void DropTestItem();

	UFUNCTION(BlueprintCallable, Category = "Inventory|Test")
	void ToggleInventoryUI();

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void PickUpFocusedItem();

protected:
	virtual void BeginPlay() override;

private:
	// Служебные функции этого класса нужны только тестовому полигону.
	void ShowInventoryMessage(const FString& Message) const;
	void SetInventoryUIVisible(bool bVisible);
	void UpdateInteractionFocus();
	void ClearInteractionFocus();
	void SpawnTestPickup(UInventoryItemDefinition* Definition, int32 Quantity, float RightOffset);

	UFUNCTION()
	void HandleInventoryWidgetClosed();

	UPROPERTY()
	TObjectPtr<AInventoryWorldItem> FocusedWorldItem;

	bool bInventoryUIVisible = false;

	static const FName TestItemId;
};
