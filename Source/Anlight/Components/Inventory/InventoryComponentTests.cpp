// тесты инвентаря

#if WITH_DEV_AUTOMATION_TESTS

#include "Components/Inventory/InventoryComponent.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FInventoryStackingTest,
	"Anlight.Inventory.StackingAndRemoval",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FInventoryStackingTest::RunTest(const FString& Parameters)
{
	// Этот тест проверяет базовый путь: заполнение двух стаков и удаление с конца.
	UInventoryComponent* Inventory = NewObject<UInventoryComponent>();
	Inventory->MaxSlots = 2;

	TestEqual(TEXT("All seven items are added"), Inventory->AddItem(TEXT("Bandage"), 7, 5), 7);
	TestEqual(TEXT("Two stacks are created"), Inventory->GetItems().Num(), 2);
	TestEqual(TEXT("First stack is full"), Inventory->GetItems()[0].Quantity, 5);
	TestEqual(TEXT("Second stack contains the remainder"), Inventory->GetItems()[1].Quantity, 2);
	TestEqual(TEXT("Total count is seven"), Inventory->GetItemCount(TEXT("Bandage")), 7);

	TestEqual(TEXT("Six items are removed"), Inventory->RemoveItem(TEXT("Bandage"), 6), 6);
	TestEqual(TEXT("One item remains"), Inventory->GetItemCount(TEXT("Bandage")), 1);
	TestEqual(TEXT("Empty stack is removed"), Inventory->GetItems().Num(), 1);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FInventoryDifferentItemsAndCapacityTest,
	"Anlight.Inventory.DifferentItemsAndCapacity",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FInventoryDifferentItemsAndCapacityTest::RunTest(const FString& Parameters)
{
	// Здесь два разных Item Id не должны объединиться, а третий стак создать нельзя.
	UInventoryComponent* Inventory = NewObject<UInventoryComponent>();
	Inventory->MaxSlots = 2;

	TestEqual(TEXT("Five bandages fill the first slot"), Inventory->AddItem(TEXT("Bandage"), 5, 5), 5);
	TestEqual(TEXT("Only three medkits fit in the last slot"), Inventory->AddItem(TEXT("Medkit"), 4, 3), 3);
	TestEqual(TEXT("Different items use different stacks"), Inventory->GetItems().Num(), 2);
	TestEqual(TEXT("Bandage count remains unchanged"), Inventory->GetItemCount(TEXT("Bandage")), 5);
	TestEqual(TEXT("Medkit count is three"), Inventory->GetItemCount(TEXT("Medkit")), 3);

	return true;
}

#endif
