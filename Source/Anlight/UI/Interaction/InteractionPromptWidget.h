// подсказка подбора предмета

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InteractionPromptWidget.generated.h"

class UTextBlock;
struct FInventoryItemStack;

// Показывает название, количество и кнопку подбора при наведении на предмет.
UCLASS(Blueprintable)
class ANLIGHT_API UInteractionPromptWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// ShowItem обновляет текст и открывает подсказку, HidePrompt её скрывает.
	void ShowItem(const FInventoryItemStack& ItemStack);
	void HidePrompt();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interaction UI")
	FText PickupText = FText::FromString(TEXT("ПОДОБРАТЬ"));

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interaction UI")
	FText InteractionKeyText = FText::FromString(TEXT("E"));

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interaction UI")
	FLinearColor AccentColor = FLinearColor(0.62f, 0.7f, 0.3f, 1.0f);

protected:
	virtual void NativeOnInitialized() override;

private:
	// Создаёт простой рабочий вид подсказки, если Blueprint не создал свой.
	void BuildWidgetTree();

	UPROPERTY()
	TObjectPtr<UTextBlock> ItemNameText;

	UPROPERTY()
	TObjectPtr<UTextBlock> ActionText;
};
