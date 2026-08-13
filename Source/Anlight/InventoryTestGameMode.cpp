// логика игрового режима для теста инвентаря

#include "InventoryTestGameMode.h"

#include "Character/InventoryTestCharacter.h"

AInventoryTestGameMode::AInventoryTestGameMode()
{
	DefaultPawnClass = AInventoryTestCharacter::StaticClass();
}
