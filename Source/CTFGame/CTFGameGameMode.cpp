// Copyright Epic Games, Inc. All Rights Reserved.

#include "CTFGameGameMode.h"
#include "CTFGameCharacter.h"
#include "UObject/ConstructorHelpers.h"

ACTFGameGameMode::ACTFGameGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

}
