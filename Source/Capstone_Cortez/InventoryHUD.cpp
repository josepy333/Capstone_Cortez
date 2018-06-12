// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "Capstone_Cortez.h"
#include "InventoryHUD.h"
#include "Engine/Canvas.h"
#include "TextureResource.h"
#include "CanvasItem.h"
#include "Capstone_CortezCharacter.h"
#include "Capstone_CortezGameMode.h"

AInventoryHUD::AInventoryHUD()
{
	// Set the crosshair texture
	static ConstructorHelpers::FObjectFinder<UTexture2D> CrosshairTexObj(TEXT("/Game/Textures/FirstPersonCrosshair"));
	CrosshairTex = CrosshairTexObj.Object;


	// Set the font
	static ConstructorHelpers::FObjectFinder<UFont> HUDFontOb(TEXT("Game/Fonts/RobotoDistanceField"));
	HUDFont = HUDFontOb.Object;
}


void AInventoryHUD::DrawHUD()
{
	Super::DrawHUD();
	// Get reference to character
	ACapstone_CortezCharacter* MyCharacter = Cast<ACapstone_CortezCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
	ACapstone_CortezGameMode* MyGameMode = Cast<ACapstone_CortezGameMode>(UGameplayStatics::GetGameMode(this));

	// Get screen dimensions
	FVector2D ScreenDimensions = FVector2D(Canvas->SizeX, Canvas->SizeY);

	// Only draw HUD if in first person mode
	if (MyCharacter->IsFirstPersonMode())
	{
		// find center of the Canvas
		FVector2D Center(Canvas->ClipX * 0.5f, Canvas->ClipY * 0.5f);

		// offset by half the texture's dimensions so that the center of the texture aligns with the center of the Canvas
		FVector2D CrosshairDrawPosition((Center.X),
			(Center.Y));

		// draw the crosshair
		FCanvasTileItem TileItem(CrosshairDrawPosition, CrosshairTex->Resource, FLinearColor::White);
		TileItem.BlendMode = SE_BLEND_Translucent;
		Canvas->DrawItem(TileItem);
	}

	//GameOver Hud
	if (MyGameMode->GetCurrentPlayState() == EPlayState::GameOver)
	{
		FVector2D GameOverSize;
		GetTextSize(TEXT("Game Over"), GameOverSize.X, GameOverSize.Y, HUDFont);
		DrawText(TEXT("Game Over"), FColor::Red, (ScreenDimensions.X - GameOverSize.X) / 2.0f, (ScreenDimensions.Y - GameOverSize.Y) / 2.0f, HUDFont, 3.0f);
	}

	//GameWon HUD
	if (MyGameMode->GetCurrentPlayState() == EPlayState::GameWon)
	{
		MyGameMode->ChangeHUDState(MyGameMode->HS_InGame);
		FVector2D GameWonSize;
		GetTextSize(TEXT("Congratulations! You found all of the keys!"), GameWonSize.X, GameWonSize.Y, HUDFont);
		DrawText(TEXT("Congratulations! You found all of the keys!"), FColor::Red, (ScreenDimensions.X - GameWonSize.X) / 2.0f, (ScreenDimensions.Y - GameWonSize.Y) / 2.0f, HUDFont, 3.0f);
	}
	
		
}



