// Joseph Cortez
// Scalar: An Unreal Engine Game Prototype

#pragma once

#include "Pickup.h"
#include "Pickup_Key.generated.h"

/**
 * 
 */
UCLASS()
class CAPSTONE_CORTEZ_API APickup_Key : public APickup
{
	GENERATED_BODY()
	
public:
	APickup_Key();

	virtual void Interact_Implementation() override;
	virtual void Use_Implementation() override;

	UPROPERTY(EditAnywhere, Category = "KeyProperties")
		uint8 AmountOfKeys;
	
	
};
