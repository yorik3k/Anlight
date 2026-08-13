// логика предмета в игровом мире

#include "World/Items/InventoryWorldItem.h"

#include "Components/Inventory/InventoryComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Items/InventoryItemDefinition.h"
#include "UObject/ConstructorHelpers.h"

AInventoryWorldItem::AInventoryWorldItem()
{
	// Предмет не выполняет постоянную логику, поэтому Tick ему не нужен.
	PrimaryActorTick.bCanEverTick = false;

	ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh"));
	SetRootComponent(ItemMesh);
	ItemMesh->SetCollisionProfileName(TEXT("PhysicsActor"));
	ItemMesh->SetSimulatePhysics(false);
	ItemMesh->SetRelativeScale3D(FVector(0.25f));

	// Куб нужен как безопасная временная модель, пока художник не назначил свою.
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		ItemMesh->SetStaticMesh(CubeMesh.Object);
	}
}

void AInventoryWorldItem::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (ItemDefinition)
	{
		// Обновляем стак после изменения Data Asset или свойств объекта в редакторе.
		InitializeItemDefinition(ItemDefinition, FMath::Max(1, ItemStack.Quantity));
	}
}

void AInventoryWorldItem::SetDroppedPhysicsEnabled(const bool bEnabled)
{
	ItemMesh->SetSimulatePhysics(bEnabled);
}

void AInventoryWorldItem::InitializeItem(const FName ItemId, const int32 Quantity, const int32 MaxStackSize)
{
	ItemStack.ItemId = ItemId;
	ItemStack.Quantity = FMath::Max(1, Quantity);
	ItemStack.MaxStackSize = FMath::Max(1, MaxStackSize);
	ItemStack.WorldItemClass = GetClass();
}

void AInventoryWorldItem::InitializeItemDefinition(UInventoryItemDefinition* Definition, const int32 Quantity)
{
	if (!Definition || Definition->ItemId.IsNone())
	{
		return;
	}

	// Копируем в стак только данные, нужные во время игры. Полное описание
	// остаётся в Definition и используется интерфейсом.
	ItemDefinition = Definition;
	ItemStack.ItemId = Definition->ItemId;
	ItemStack.Quantity = FMath::Max(1, Quantity);
	ItemStack.MaxStackSize = FMath::Max(1, Definition->MaxStackSize);
	if (Definition->WorldItemClass)
	{
		ItemStack.WorldItemClass = Definition->WorldItemClass;
	}
	else
	{
		ItemStack.WorldItemClass = GetClass();
	}
	ItemStack.Definition = Definition;
}

int32 AInventoryWorldItem::PickUp(UInventoryComponent* TargetInventory)
{
	if (!TargetInventory || !ItemStack.IsValid())
	{
		return 0;
	}

	// Компонент может принять всё количество или только часть, если рюкзак заполнен.
	const int32 Added = ItemDefinition
		? TargetInventory->AddItemDefinition(ItemDefinition, ItemStack.Quantity)
		: TargetInventory->AddItem(
			ItemStack.ItemId,
			ItemStack.Quantity,
			ItemStack.MaxStackSize,
			GetClass());

	// В мире остаётся только то количество, которое не поместилось.
	ItemStack.Quantity -= Added;
	if (ItemStack.Quantity <= 0)
	{
		Destroy();
	}

	return Added;
}
