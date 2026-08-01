// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/Health/HealthComponent.h"

UHealthComponent::UHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();
	health = maxHealth;
}
float UHealthComponent::GetHealth() const { return health; }
float UHealthComponent::GetMaxHealth() const { return maxHealth; }

void UHealthComponent::UpdateHealth(float deltaHealth)
{
	health += deltaHealth;
	// clamp the health to avoid values less than 0 and bigger than maxHealth
	health = FMath::Clamp(health, 0.0f, maxHealth);
	OnHealthChanged.Broadcast(health, maxHealth);

	if (health == 0.0f) { OnHealthDepleted.Broadcast(); }
}