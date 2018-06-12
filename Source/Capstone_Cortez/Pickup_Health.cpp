// Fill out your copyright notice in the Description page of Project Settings.

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


	Character->UpdateCharacterHealth(AmountOfHealth);

	//Destroy();
	
}
