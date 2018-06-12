// Fill out your copyright notice in the Description page of Project Settings.

#include "Capstone_Cortez.h"
#include "Pickup_Key.h"
#include "Capstone_CortezCharacter.h"

APickup_Key::APickup_Key()
{
	AmountOfKeys = 0;
}

void APickup_Key::Interact_Implementation()
{
	ACapstone_CortezCharacter* Character = Cast<ACapstone_CortezCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));

	if (Character->AddItemToInventory(this))
	{
		OnPickedUp();
	}
}

void APickup_Key::Use_Implementation()
{

	ACapstone_CortezCharacter* Character = Cast<ACapstone_CortezCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));


	Character->KeysUsed(AmountOfKeys);

}

