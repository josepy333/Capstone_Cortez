// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "GameFramework/GameModeBase.h"
#include "Capstone_CortezGameMode.generated.h"


UENUM(BlueprintType)
namespace EPlayState
{
	enum Type
	{
		Playing					UMETA(DisplayName = "Playing"),
		GameOver				UMETA(DisplayName = "GameOver"),
		GameWon					UMETA(DisplayName = "GameWon"),

		Unknown					UMETA(Hidden),
	};
}


UCLASS(minimalapi)
class ACapstone_CortezGameMode : public AGameModeBase
{
	GENERATED_BODY()

	virtual void BeginPlay() override;

public:
	ACapstone_CortezGameMode();
	
	// enum to store HUD state
	enum EHUDState : uint8
	{
		HS_InGame,
		HS_Inventory
	};

	/** Checks HUD state and applies correct HUD **/
	void ApplyHUDChanges();

	uint8 GetHUDState();

	/** Sets HUD state and applies new changes **/
	UFUNCTION(BlueprintCallable, Category = "HUD Function")
	void ChangeHUDState(uint8 NewHUDState);

	/** Apply HUD to screen, true if success **/
	bool ApplyHUD(TSubclassOf<class UUserWidget> WidgetToApply, bool bShowMouseCursor, bool EnableClickEvents);

	uint8 GetCurrentPlayState();

	/** Sets current state of the game mode **/
	UFUNCTION(BlueprintCallable, Category = "Game Mode")
	void ChangeCurrentPlayState(uint8 NewPlayState);

	


protected:
	uint8 HUDState;
	uint8 CurrentPlayState;

	/** Apply Play state,**/
	void ApplyPlayState();

	/*/** HUD in the Game **/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "HUD Widgets", META = (BlueprintProtected = "true"))
	TSubclassOf<class UUserWidget> InGameHUDClass;

	/** HUD for the inventory **/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "HUD Widgets", META = (BlueprintProtected = "true"))
	TSubclassOf<class UUserWidget> InventoryHUDClass;

	UPROPERTY()
	class UUserWidget* CurrentWidget;
};



