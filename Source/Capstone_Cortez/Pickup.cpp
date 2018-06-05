// Joseph Cortez
// Capstone project

#include "Capstone_Cortez.h"
#include "Capstone_CortezCharacter.h"
#include "Pickup.h"

APickup::APickup()
{
	// Setup mesh for pickup
	InteractionMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PickupMesh"));
	InteractionMesh->SetSimulatePhysics(true);

	// Name of mesh
	ItemName = FString("Enter item name here");

	// Interaction text of mesh
	InteractionText = FString("Enter interaction text here");

	// Value of mesh
	Value = 0;
}

void APickup::BeginPlay()
{
	InteractionText = FString::Printf(TEXT("Press E to pickup %s"), *ItemName);
}

void APickup::Interact_Implementation()
{
	ACapstone_CortezCharacter* Character = Cast<ACapstone_CortezCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));

	if (Character->AddItemToInventory(this))
	{
		OnPickedUp();
	}

	
}

void APickup::Use_Implementation()
{
	GLog->Log("Use() from base pickup class");
}

void APickup::OnPickedUp()
{
	// Make mesh disappear once picked up but not destroyed
	InteractionMesh->SetVisibility(false);
	InteractionMesh->SetSimulatePhysics(false);
	InteractionMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}
