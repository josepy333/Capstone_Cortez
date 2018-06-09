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
}

void ACapstone_Cortez::Tick(float DeltaSeconds)
{
	ACapstone_CortezCharacter* MyCharacter = Cast<ACapstone_CortezCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));

	if (MyCharacter->CharacterHealth = 0)
		ChangeCurrentPlayState(EPlayState::GameOver);

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

EPlayState ACapstone_CortezGameMode::GetCurrentPlayState() const
{
	return CurrentPlayState;
}

void ACapstone_CortezGameMode::ChangeCurrentState(EPlayState NewPlayState)
{
	CurrentPlayState = NewPlayState;
	ApplyPlayStateChanges(NewPlayState);

}

void ACapstone_CortezGameMode::ApplyPlayState(EPlayState NewPlayState)
{
	switch (NewPlayState)
	{
	case EPlayState::EPlaying:
		break;
	case EPlayState::EGameOver:
		break;
	case EPlayState::Unknown:
		break;
	}
}


