// Joseph Cortez
// Scalar: An Unreal Engine Game Prototype

#pragma once

#include "Pickup.h"
#include "Pickup_Health.generated.h"

/**
 * 
 */
UCLASS()
class CAPSTONE_CORTEZ_API APickup_Health : public APickup
{
	GENERATED_BODY()
	
public:
	APickup_Health();

	virtual void Interact_Implementation() override;
	virtual void Use_Implementation() override;
	
	UPROPERTY(EditAnywhere, Category = "HealthProperties")
	float AmountOfHealth;
	
};
