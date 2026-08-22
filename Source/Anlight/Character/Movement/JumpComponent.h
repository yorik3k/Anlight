#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "JumpComponent.generated.h"

class ACharacter;
class UStaminaComponent;
class UEffectManager;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ANLIGHT_API UJumpComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UJumpComponent();

protected:
	virtual void BeginPlay() override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jump|Settings")
	float JumpCost = 15.0f;

	UFUNCTION(BlueprintCallable, Category = "Jump")
	void StartJump();

	UFUNCTION(BlueprintCallable, Category = "Jump")
	void StopJump();

	UFUNCTION(BlueprintCallable, Category = "Jump")
	bool CanJump() const;

	// Ñâÿçü ñ EffectManager
	UFUNCTION(BlueprintCallable, Category = "Jump")
	void SetEffectManager(UEffectManager* InEffectManager);

protected:
	UPROPERTY()
	ACharacter* OwnerCharacter;

	UPROPERTY()
	UStaminaComponent* StaminaComponent;

	UPROPERTY()
	UEffectManager* EffectManager = nullptr;
};