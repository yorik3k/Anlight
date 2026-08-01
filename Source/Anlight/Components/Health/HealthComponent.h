// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHealthDepletedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChangedSignature, float, NewHealth, float, maxHealth);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ANLIGHT_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

	// current health
	UPROPERTY()
	float health;
	UPROPERTY(EditDefaultsOnly, Category = "Character Health")
	float maxHealth;

public:
	UHealthComponent();

	UPROPERTY(BlueprintAssignable, Category = "Character Health")
	FOnHealthChangedSignature OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "Character Health")
	FOnHealthDepletedSignature OnHealthDepleted;

protected:
	virtual void BeginPlay() override;

public:
	UFUNCTION(BlueprintCallable)
	float GetHealth() const;
	UFUNCTION(BlueprintCallable)
	float GetMaxHealth() const;
	UFUNCTION(BlueprintCallable)
	void UpdateHealth(float deltaHealth);
};