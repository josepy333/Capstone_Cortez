// Joseph Cortez
// Scalar: An Unreal Engine Game Prototype

#include "Capstone_Cortez.h"
#include "Interaction.h"


// Sets default values
AInteraction::AInteraction()
{
	InteractionText = FString("Press E to interact");
}

// Called when the game starts or when spawned
void AInteraction::BeginPlay()
{
	Super::BeginPlay();
	
}

void AInteraction::Interact_Implementation()
{
	GLog->Log("Interact Base class: Interact()");
}

