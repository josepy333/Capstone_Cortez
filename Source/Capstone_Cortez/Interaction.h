// Joseph Cortez
// Scalar: An Unreal Engine Game Prototype

#pragma once

#include "GameFramework/Actor.h"
#include "Interaction.generated.h"

UCLASS()
class CAPSTONE_CORTEZ_API AInteraction : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AInteraction();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	

	UFUNCTION(BlueprintNativeEvent)
	void Interact();
	virtual void Interact_Implementation();

	/**Static mesh component for interaction **/
	UPROPERTY(EditAnywhere, Category = "Interaction properties")
	class UStaticMeshComponent* InteractionMesh;

	/**Visible Text for interaction**/
	UPROPERTY(EditAnywhere, Category = "Interaction properties")
	FString InteractionText;
	
};
