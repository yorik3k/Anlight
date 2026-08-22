#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NutritionComponent.generated.h"

class UHealthComponent;
class UEffectManager;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHungerChanged, float, CurrentHunger, float, MaxHunger);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHungerStageChanged, int32, Stage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnThirstChanged, float, CurrentThirst, float, MaxThirst);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnThirstStageChanged, int32, Stage);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ANLIGHT_API UNutritionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UNutritionComponent();

	// ===== ÍÀÑÒÐÎÉÊÈ =====
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Nutrition")
	float MaxHunger = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Nutrition")
	float MaxThirst = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Nutrition")
	float HungerDrainRate = 0.02f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Nutrition")
	float ThirstDrainRate = 0.03f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Nutrition|Thresholds")
	float YellowThreshold = 66.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Nutrition|Thresholds")
	float RedThreshold = 33.0f;

	// ===== ÄÅËÅÃÀÒÛ =====
	UPROPERTY(BlueprintAssignable)
	FOnHungerChanged OnHungerChanged;

	UPROPERTY(BlueprintAssignable)
	FOnHungerStageChanged OnHungerStageChanged;

	UPROPERTY(BlueprintAssignable)
	FOnThirstChanged OnThirstChanged;

	UPROPERTY(BlueprintAssignable)
	FOnThirstStageChanged OnThirstStageChanged;

	// ===== ÌÅÒÎÄÛ =====
	UFUNCTION(BlueprintCallable, Category = "Nutrition")
	void Eat(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Nutrition")
	void Drink(float Amount);

	UFUNCTION(BlueprintPure, Category = "Nutrition")
	float GetHunger() const { return CurrentHunger; }

	UFUNCTION(BlueprintPure, Category = "Nutrition")
	float GetThirst() const { return CurrentThirst; }

	UFUNCTION(BlueprintPure, Category = "Nutrition")
	float GetHungerPercent() const { return MaxHunger > 0 ? CurrentHunger / MaxHunger : 0.0f; }

	UFUNCTION(BlueprintPure, Category = "Nutrition")
	float GetThirstPercent() const { return MaxThirst > 0 ? CurrentThirst / MaxThirst : 0.0f; }

	UFUNCTION(BlueprintCallable, Category = "Nutrition")
	void SetHealthComponent(UHealthComponent* InHealth);

	UFUNCTION(BlueprintCallable, Category = "Nutrition")
	void SetEffectManager(UEffectManager* InEffects);

	UFUNCTION(BlueprintCallable, Category = "Nutrition|Debug")
	void SetHunger(float NewHunger);

	UFUNCTION(BlueprintCallable, Category = "Nutrition|Debug")
	void SetThirst(float NewThirst);

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	float CurrentHunger = 100.0f;
	float CurrentThirst = 100.0f;
	int32 CurrentHungerStage = 0;
	int32 CurrentThirstStage = 0;

	UPROPERTY()
	UHealthComponent* HealthComponent = nullptr;

	UPROPERTY()
	UEffectManager* EffectManager = nullptr;

	void UpdateHungerStage();
	void UpdateThirstStage();
	void ApplyNutritionEffects();
	void CheckDeathConditions();
};