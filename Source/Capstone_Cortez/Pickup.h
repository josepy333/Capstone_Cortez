// Joseph Cortez
// Capstone project

#pragma once

#include "Interaction.h"
#include "Pickup.generated.h"

/**
 * 
 */
UCLASS()
class CAPSTONE_CORTEZ_API APickup : public AInteraction
{
	GENERATED_BODY()
	
public:

	APickup();

	virtual void BeginPlay() override;

	virtual void Interact_Implementation() override;

	UFUNCTION(BlueprintNativeEvent)
	void Use();
	virtual void Use_Implementation();

	UPROPERTY(EditAnywhere, Category = "Pickup Properties")
	UTexture2D* PickupThumbnail;

	UPROPERTY(EditAnywhere, Category = "Pickup Properties")
	FString ItemName;

	UPROPERTY(EditAnywhere, Category = "Pickup Properties")
	int32 Value;

	/** Called when item is picked up and needs to be removed from the level**/
	void OnPickedUp();
	
	
};
