// Fill out your copyright notice in the Description page of Project Settings.

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
	
	UPROPERTY(EditAnywhere, Category = "HealthProperties")
	uint8 AmountOfHealth;
	
};
