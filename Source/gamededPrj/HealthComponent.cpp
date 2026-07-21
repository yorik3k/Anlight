


#include "HealthComponent.h"
#include "gamededPrjCharacter.h"  // ← ТАК РАБОТАЕТ!


UHealthComponent::UHealthComponent()
{
	
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();
	health = maxHealth;
	
	
}
// get health
float UHealthComponent::GetHealth() const
{
	return health;
}
// get max health
float UHealthComponent::GetMaxHealth() const
{
	return maxHealth;
}
// upd. health
void UHealthComponent::updateHealth(float deltaHealth)
{
	health += deltaHealth;
	health = FMath::Clamp(health, 0.0f, maxHealth);
	//UE_LOG(LogTemp, Warning, TEXT("Actual health %f"), health);

	// broadcast trigger
	OnHealthChanged.Broadcast(health, maxHealth);
	if (health == 0.0f)
	{
		OnHealthDepleted.Broadcast();

		if (AActor* Owner = GetOwner())
		{
			if (AgamededPrjCharacter* Character = Cast<AgamededPrjCharacter>(Owner))
			{
				Character->dye();
			}
		}
	}
}


