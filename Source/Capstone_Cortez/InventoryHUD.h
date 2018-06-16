// Joseph Cortez
// Scalar: An Unreal Engine Game Prototype

// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameFramework/HUD.h"
#include "InventoryHUD.generated.h"

/**
 * 
 */
UCLASS()
class CAPSTONE_CORTEZ_API AInventoryHUD : public AHUD
{
	GENERATED_BODY()
	
public:
	AInventoryHUD();
	/** Primary draw call for the HUD */
	virtual void DrawHUD() override;

public:
	/** Crosshair asset pointer */
	UPROPERTY()
	class UTexture2D* CrosshairTex;

	/** Font asset pointer */
	UPROPERTY()
	class UFont* HUDFont;
	
	
};