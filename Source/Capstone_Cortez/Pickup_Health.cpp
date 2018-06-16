// Joseph Cortez
// Scalar: An Unreal Engine Game Prototype

#include "Capstone_Cortez.h"
#include "Pickup_Health.h"
#include "Capstone_CortezCharacter.h"

APickup_Health::APickup_Health()
{
	AmountOfHealth = 0.0f;
}

void APickup_Health::Interact_Implementation()
{
	ACapstone_CortezCharacter* Character = Cast<ACapstone_CortezCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));

	if (Character->AddItemToInventory(this))
	{
		OnPickedUp();
	}
}

void APickup_Health::Use_Implementation()
{
	
	ACapstone_CortezCharacter* Character = Cast<ACapstone_CortezCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));


	Character->IncreaseCharacterHealth(AmountOfHealth);

	//Destroy();
	
}
