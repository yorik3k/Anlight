// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"

class AgamededPrjCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHealthDepletedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChangedSignature, float, NewHealth, float, MaxHealth);


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GAMEDEDPRJ_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

	// curr. health
	UPROPERTY(EditDefaultsOnly, Category = "Default")
	float health;
	// max health
	UPROPERTY(EditDefaultsOnly, Category = "Default")
	float maxHealth;

public:	
	UHealthComponent();

	UPROPERTY(BlueprintAssignable,Category = "Default")
	FOnHealthChangedSignature OnHealthChanged;
	UPROPERTY(BlueprintAssignable, Category = "Default")
	FOnHealthDepletedSignature OnHealthDepleted;

protected:
	virtual void BeginPlay() override;

public:	
	// get health
	UFUNCTION(BlueprintCallable)
	float GetHealth() const;
	// get max health
	UFUNCTION(BlueprintCallable)
	float GetMaxHealth() const;
	UFUNCTION(BlueprintCallable)
	void updateHealth(float deltaHealth);
	

		
};
