// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SprintComponent.generated.h"

class ACharacter;
class UStaminaComponent;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ANLIGHT_API USprintComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USprintComponent();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ===== НАСТРОЙКИ =====

	// Множитель скорости при беге
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sprint|Settings")
	float SprintSpeedMultiplier = 1.8f;

	// Расход стамины в секунду
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sprint|Settings")
	float SprintDrainRate = 25.0f;

	// ===== ССЫЛКИ =====

	UPROPERTY()
	ACharacter* OwnerCharacter;

	UPROPERTY()
	UStaminaComponent* StaminaComponent;

	// Оригинальная скорость ходьбы
	float OriginalWalkSpeed = 0.0f;

public:
	// ===== МЕТОДЫ =====

	UFUNCTION(BlueprintCallable, Category = "Sprint")
	void StartSprint();

	UFUNCTION(BlueprintCallable, Category = "Sprint")
	void StopSprint();

	UFUNCTION(BlueprintCallable, Category = "Sprint")
	bool IsSprinting() const { return bIsSprinting; }

protected:
	bool bIsSprinting = false;

private:
	void UpdateSpeed();
};