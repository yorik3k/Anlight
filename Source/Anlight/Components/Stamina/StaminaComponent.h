// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "StaminaComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStaminaDepletedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStaminaChangedSignature, float, NewStamina, float, MaxStamina);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMaxStaminaChangedSignature, float, NewMaxStamina);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ANLIGHT_API UStaminaComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UStaminaComponent();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ===== BASE =====

	// Текущая выносливость
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stamina")
	float Stamina;

	// Базовая максимальная выносливость (настройка в BP)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina")
	float BaseMaxStamina = 100.0f;

	// Актуальная максимальная выносливость (с учётом модификаторов)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stamina")
	float CurrentMaxStamina;

	// ===== REGEN =====

	// Скорость восстановления (в секунду)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina|Regen")
	float StaminaRegenRate = 15.0f;

	// Задержка перед восстановлением (в секундах)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina|Regen")
	float StaminaRegenDelay = 1.5f;

	// Таймер задержки
	float RegenDelayTimer = 0.0f;

	// ===== DRAIN =====

	// Расход при беге (в секунду)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina|Drain")
	float SprintDrainRate = 25.0f;

	// Минимальный порог для бега
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina|Settings")
	float MinStaminaForSprint = 5.0f;

	// ===== STATE =====

	// Флаг: бежит ли персонаж
	bool bIsSprinting = false;

public:
	// ===== СОБЫТИЯ =====

	UPROPERTY(BlueprintAssignable, Category = "Stamina|Events")
	FOnStaminaChangedSignature OnStaminaChanged;

	UPROPERTY(BlueprintAssignable, Category = "Stamina|Events")
	FOnStaminaDepletedSignature OnStaminaDepleted;

	UPROPERTY(BlueprintAssignable, Category = "Stamina|Events")
	FOnMaxStaminaChangedSignature OnMaxStaminaChanged;

	// ===== ГЕТТЕРЫ =====

	UFUNCTION(BlueprintCallable, Category = "Stamina")
	float GetStamina() const { return Stamina; }

	UFUNCTION(BlueprintCallable, Category = "Stamina")
	float GetMaxStamina() const { return CurrentMaxStamina; }

	UFUNCTION(BlueprintCallable, Category = "Stamina")
	float GetStaminaPercent() const;

	UFUNCTION(BlueprintCallable, Category = "Stamina")
	bool IsStaminaFull() const;

	UFUNCTION(BlueprintCallable, Category = "Stamina")
	bool IsStaminaEmpty() const;

	// ===== ПРОВЕРКИ =====

	UFUNCTION(BlueprintCallable, Category = "Stamina")
	bool CanSprint() const;

	// CanJump удалён, так как проверка теперь в JumpComponent

	// ===== УПРАВЛЕНИЕ =====

	// Изменить стамину (положительно или отрицательно)
	UFUNCTION(BlueprintCallable, Category = "Stamina")
	void UpdateStamina(float DeltaStamina);

	// Установить флаг бега (вызывается из SprintComponent)
	UFUNCTION(BlueprintCallable, Category = "Stamina")
	void SetIsSprinting(bool bInSprinting);

	// ===== МОДИФИКАТОРЫ (дебаффы/баффы) =====

	// Добавить модификатор к максимальной стамине
	UFUNCTION(BlueprintCallable, Category = "Stamina|Modifiers")
	void AddMaxStaminaModifier(float ModifierAmount, float Duration);

	// Убрать модификатор (по значению)
	UFUNCTION(BlueprintCallable, Category = "Stamina|Modifiers")
	void RemoveMaxStaminaModifier(float ModifierAmount);

private:
	// Структура модификатора
	struct FStaminaModifier
	{
		float Amount;
		float Duration;
		float TimeRemaining;
	};

	// Список активных модификаторов
	TArray<FStaminaModifier> ActiveModifiers;

	// Пересчитать максимальную стамину
	void RecalculateMaxStamina();

	// Обновить модификаторы (убрать истёкшие)
	void UpdateModifiers(float DeltaTime);
};