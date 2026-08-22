#include "Components/Nutrition/NutritionComponent.h"
#include "Components/Health/HealthComponent.h"
#include "Components/EffectManager/EffectManager.h"

UNutritionComponent::UNutritionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 1.0f;
}

void UNutritionComponent::BeginPlay()
{
	Super::BeginPlay();
	CurrentHunger = MaxHunger;
	CurrentThirst = MaxThirst;
}

void UNutritionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	CurrentHunger = FMath::Max(0.0f, CurrentHunger - HungerDrainRate * DeltaTime);
	CurrentThirst = FMath::Max(0.0f, CurrentThirst - ThirstDrainRate * DeltaTime);

	UpdateHungerStage();
	UpdateThirstStage();
	ApplyNutritionEffects();
	CheckDeathConditions();

	OnHungerChanged.Broadcast(CurrentHunger, MaxHunger);
	OnThirstChanged.Broadcast(CurrentThirst, MaxThirst);

	static float LogTimer = 0.0f;
	LogTimer += DeltaTime;
	if (LogTimer >= 5.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("Hunger: %.1f / %.1f (Stage: %d), Thirst: %.1f / %.1f (Stage: %d)"),
			CurrentHunger, MaxHunger, CurrentHungerStage,
			CurrentThirst, MaxThirst, CurrentThirstStage);
		LogTimer = 0.0f;
	}
}

void UNutritionComponent::Eat(float Amount)
{
	CurrentHunger = FMath::Min(MaxHunger, CurrentHunger + Amount);
	UpdateHungerStage();
	ApplyNutritionEffects();
	OnHungerChanged.Broadcast(CurrentHunger, MaxHunger);
}

void UNutritionComponent::Drink(float Amount)
{
	CurrentThirst = FMath::Min(MaxThirst, CurrentThirst + Amount);
	UpdateThirstStage();
	ApplyNutritionEffects();
	OnThirstChanged.Broadcast(CurrentThirst, MaxThirst);
}

void UNutritionComponent::SetHealthComponent(UHealthComponent* InHealth)
{
	HealthComponent = InHealth;
}

void UNutritionComponent::SetEffectManager(UEffectManager* InEffects)
{
	EffectManager = InEffects;
}

void UNutritionComponent::UpdateHungerStage()
{
	int32 NewStage = 0;
	if (CurrentHunger <= RedThreshold)
		NewStage = 2;
	else if (CurrentHunger <= YellowThreshold)
		NewStage = 1;

	if (NewStage != CurrentHungerStage)
	{
		CurrentHungerStage = NewStage;
		OnHungerStageChanged.Broadcast(CurrentHungerStage);
	}
}

void UNutritionComponent::UpdateThirstStage()
{
	int32 NewStage = 0;
	if (CurrentThirst <= RedThreshold)
		NewStage = 2;
	else if (CurrentThirst <= YellowThreshold)
		NewStage = 1;

	if (NewStage != CurrentThirstStage)
	{
		CurrentThirstStage = NewStage;
		OnThirstStageChanged.Broadcast(CurrentThirstStage);
	}
}

void UNutritionComponent::ApplyNutritionEffects()
{
	if (!EffectManager) return;

	// ===== ГОЛОД =====
	if (CurrentHunger <= YellowThreshold)
	{
		// Только восстановление (не запас)
		FStatusEffect HungerStaminaRegenEffect;
		HungerStaminaRegenEffect.EffectID = FName("Hunger_StaminaRegen_Penalty");
		HungerStaminaRegenEffect.AffectedStat = EAnlightStat::StaminaRegenMultiplier;
		HungerStaminaRegenEffect.Value = CurrentHunger <= RedThreshold ? -0.5f : -0.25f;
		HungerStaminaRegenEffect.Duration = 0.0f;
		EffectManager->ApplyEffect(HungerStaminaRegenEffect);
	}
	else
	{
		EffectManager->RemoveEffect(FName("Hunger_StaminaRegen_Penalty"));
	}
	

	// ===== ЖАЖДА =====
	if (CurrentThirst <= YellowThreshold)
	{
		FStatusEffect ThirstStaminaRegenEffect;
		ThirstStaminaRegenEffect.EffectID = FName("Thirst_StaminaRegen_Penalty");
		ThirstStaminaRegenEffect.AffectedStat = EAnlightStat::StaminaRegenMultiplier;
		ThirstStaminaRegenEffect.Value = CurrentThirst <= RedThreshold ? -0.55f : -0.3f;
		ThirstStaminaRegenEffect.Duration = 0.0f;
		EffectManager->ApplyEffect(ThirstStaminaRegenEffect);
	}
	else
	{
		EffectManager->RemoveEffect(FName("Thirst_StaminaRegen_Penalty"));
	}

	if (CurrentHunger <= RedThreshold && CurrentThirst <= RedThreshold)
	{
		FStatusEffect ComboStaminaEffect;
		ComboStaminaEffect.EffectID = FName("Combo_Stamina_Penalty");
		ComboStaminaEffect.AffectedStat = EAnlightStat::StaminaMultiplier;
		ComboStaminaEffect.Value = -0.3f; // -30% к запасу
		ComboStaminaEffect.Duration = 0.0f;
		EffectManager->ApplyEffect(ComboStaminaEffect);
	}
	else
	{
		EffectManager->RemoveEffect(FName("Combo_Stamina_Penalty"));
	}
}

void UNutritionComponent::CheckDeathConditions()
{
	if ((CurrentHunger <= 0.0f || CurrentThirst <= 0.0f) && HealthComponent)
	{
		HealthComponent->OnHealthDepleted.Broadcast();
	}
}

void UNutritionComponent::SetHunger(float NewHunger)
{
	CurrentHunger = FMath::Clamp(NewHunger, 0.0f, MaxHunger);
	UpdateHungerStage();
	ApplyNutritionEffects();
	OnHungerChanged.Broadcast(CurrentHunger, MaxHunger);
}

void UNutritionComponent::SetThirst(float NewThirst)
{
	CurrentThirst = FMath::Clamp(NewThirst, 0.0f, MaxThirst);
	UpdateThirstStage();
	ApplyNutritionEffects();
	OnThirstChanged.Broadcast(CurrentThirst, MaxThirst);
}