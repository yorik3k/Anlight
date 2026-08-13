// игровой режим для теста инвентаря

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "InventoryTestGameMode.generated.h"

// запускает отдельного тестового персонажа
UCLASS()
class ANLIGHT_API AInventoryTestGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AInventoryTestGameMode();
};
