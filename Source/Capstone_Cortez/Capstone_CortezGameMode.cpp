// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "Capstone_Cortez.h"
#include "Capstone_CortezGameMode.h"
#include "Capstone_CortezCharacter.h"

ACapstone_CortezGameMode::ACapstone_CortezGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
