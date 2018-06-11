// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "Capstone_Cortez.h"
#include "Capstone_CortezGameMode.h"
#include "Capstone_CortezCharacter.h"
#include "InventoryHUD.h"
#include "Blueprint/UserWidget.h"

void ACapstone_CortezGameMode::BeginPlay()
{
	// Applies HUD to screen when game starts
	ApplyHUDChanges();

}

ACapstone_CortezGameMode::ACapstone_CortezGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}

	// Use custome hud class
	HUDClass = AInventoryHUD::StaticClass();

	HUDState = EHUDState::HS_InGame;
	CurrentPlayState = EPlayState::Playing;
}

void ACapstone_CortezGameMode::ApplyHUDChanges()
{
	if (CurrentWidget != nullptr)
		CurrentWidget->RemoveFromParent();

	switch (HUDState)
	{
		case EHUDState::HS_InGame:
		{
			ApplyHUD(InGameHUDClass, false, false);
			break;
		}
		case EHUDState::HS_Inventory:
		{
			ApplyHUD(InventoryHUDClass, true, true);
			break;
		}
		default:
		{
			ApplyHUD(InGameHUDClass, false, false);
			break;
		}

	}
}

uint8 ACapstone_CortezGameMode::GetHUDState()
{
	return HUDState;
}

void ACapstone_CortezGameMode::ChangeHUDState(uint8 NewHUDState)
{
	HUDState = NewHUDState;
	ApplyHUDChanges();
}

bool ACapstone_CortezGameMode::ApplyHUD(TSubclassOf<class UUserWidget> WidgetToApply, bool bShowMouseCursor, bool EnableClickEvents)
{
	// Get a reference to the character and the controller
	ACapstone_CortezCharacter* MyCharacter = Cast<ACapstone_CortezCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
	APlayerController* MyController = GetWorld()->GetFirstPlayerController();

	if (WidgetToApply != nullptr)
	{
		MyController->bShowMouseCursor = bShowMouseCursor;
		MyController->bEnableClickEvents = EnableClickEvents;

		CurrentWidget = CreateWidget<UUserWidget>(GetWorld(), WidgetToApply);

		if (CurrentWidget != nullptr)
		{
			CurrentWidget->AddToViewport();
			return true;
		}
		else return false;
	}
	else return false;
}

uint8 ACapstone_CortezGameMode::GetCurrentPlayState()
{
	return CurrentPlayState;
}

void ACapstone_CortezGameMode::ChangeCurrentPlayState(uint8 NewPlayState)
{
	CurrentPlayState = NewPlayState;
	ApplyPlayState();

}

void ACapstone_CortezGameMode::ApplyPlayState()
{
	switch (CurrentPlayState)
	{
	case EPlayState::Playing:
		break;
	case EPlayState::GameOver:
	{
		APlayerController * MyController = UGameplayStatics::GetPlayerController(this, 0);
		MyController->SetCinematicMode(true, true, true);
		break;
	}	
	case EPlayState::Unknown:
		break;
	}
}


