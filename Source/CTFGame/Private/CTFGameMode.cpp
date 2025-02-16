// Fill out your copyright notice in the Description page of Project Settings.


#include "CTFGameMode.h"
#include "../CTFGameCharacter.h"
#include "TeamManager.h"
#include "EngineUtils.h"
#include "UObject/ConstructorHelpers.h"

ACTFGameMode::ACTFGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

}
void ACTFGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
    if (NewPlayer && NewPlayer->PlayerState)
    {
        ATeamManager* TeamManager = nullptr;

        for (TActorIterator<ATeamManager> It(GetWorld()); It; ++It)
        {
            TeamManager = *It;
            break;
        }

        if (TeamManager)
        {
            TeamManager->AssignPlayerToTeam(NewPlayer->PlayerState);
        }
    }
}

