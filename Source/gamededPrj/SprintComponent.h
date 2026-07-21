// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SprintComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class GAMEDEDPRJ_API USprintComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    USprintComponent();

    UPROPERTY(EditDefaultsOnly, Category = "Sprint")
    float sprintSpeedMultiplier = 1.5f;

    UPROPERTY(EditDefaultsOnly, Category = "Sprint")
    float staminaCost = 15.0f;

    UFUNCTION(BlueprintCallable)
    void StartSprint();

    UFUNCTION(BlueprintCallable)
    void StopSprint();

    UFUNCTION(BlueprintCallable)
    bool IsSprinting() const { return bIsSprinting; }  // ← ИСПРАВЛЕНО!

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    bool bIsSprinting = false;
    float originalWalkSpeed = 0.0f;

    class ACharacter* OwnerCharacter = nullptr;
    class UStaminaComponent* StaminaComponent = nullptr;

    void UpdateSpeed(float DeltaTime);
};