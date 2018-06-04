// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "GameFramework/GameModeBase.h"
#include "Capstone_CortezGameMode.generated.h"

UCLASS(minimalapi)
class ACapstone_CortezGameMode : public AGameModeBase
{
	GENERATED_BODY()

	virtual void BeginPlay() override;

public:
	ACapstone_CortezGameMode();

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

protected:
	uint8 HUDState;

	/*/** HUD in the Game **/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "HUD Widgets", META = (BlueprintProtected = "true"))
	TSubclassOf<class UUserWidget> InGameHUDClass;

	/** HUD for the inventory **/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "HUD Widgets", META = (BlueprintProtected = "true"))
	TSubclassOf<class UUserWidget> InventoryHUDClass;

	UPROPERTY()
	class UUserWidget* CurrentWidget;
};



