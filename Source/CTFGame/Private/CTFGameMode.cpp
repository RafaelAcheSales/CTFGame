// Fill out your copyright notice in the Description page of Project Settings.

#include "CTFGameMode.h"
#include "../CTFGameCharacter.h"
#include "TeamManager.h"
#include "EngineUtils.h"
#include "UObject/ConstructorHelpers.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerStart.h"
#include <CTFPlayerState.h>

ACTFGameMode::ACTFGameMode()
    : Super()
{
    // Set default pawn class to your Blueprinted character
    static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(
        TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter")
    );
    DefaultPawnClass = PlayerPawnClassFinder.Class;
}

void ACTFGameMode::PostLogin(APlayerController* NewPlayer)
{
	//log if has authority
	UE_LOG(LogTemp, Warning, TEXT("PostLogin() called has auth %d"), HasAuthority());
	Super::PostLogin(NewPlayer);
	// Assign the player to a team
	ATeamManager* TeamManager = GetTeamManager();
	if (TeamManager)
	{
		TeamManager->AssignPlayerToTeam(NewPlayer->PlayerState);
	}
}

ATeamManager* ACTFGameMode::GetTeamManager()
{
	for (TActorIterator<ATeamManager> It(GetWorld()); It; ++It)
	{
		return *It;
	}
	return nullptr;
}

AActor* ACTFGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	//return the one with player start tag = "Default"

	AActor* BestStart = nullptr;
	for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
	{
		if (It->PlayerStartTag == "Default")
		{
			BestStart = *It;
			break;
		}
	}
	return BestStart; 
}
