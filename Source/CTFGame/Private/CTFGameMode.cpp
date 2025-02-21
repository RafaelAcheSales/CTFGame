#include "CTFGameMode.h"
#include "../CTFGameCharacter.h"
#include "TeamManager.h"
#include "EngineUtils.h"
#include "UObject/ConstructorHelpers.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerStart.h"
#include "CTFPlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "CTFGameState.h"

ACTFGameMode::ACTFGameMode()
    : Super()
{
    static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(
        TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter")
    );
    if (PlayerPawnClassFinder.Succeeded())
    {
        DefaultPawnClass = PlayerPawnClassFinder.Class;
    }
}

void ACTFGameMode::PostLogin(APlayerController* NewPlayer)
{
    UE_LOG(LogTemp, Warning, TEXT("PostLogin() called HasAuthority=%d"), HasAuthority());
    Super::PostLogin(NewPlayer);

    // assign the player to a team using your ATeamManager logic
    if (ATeamManager* TeamManager = GetTeamManager())
    {
        TeamManager->AssignPlayerToTeam(NewPlayer->PlayerState);
    }
}

AActor* ACTFGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
    // example: find a PlayerStart tagged "Default"
    for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
    {
        if (It->PlayerStartTag == FName("Default"))
        {
            return *It;
        }
    }
    // fallback: if none found, return first Start you can
    return Super::ChoosePlayerStart_Implementation(Player);
}

ATeamManager* ACTFGameMode::GetTeamManager()
{
    for (TActorIterator<ATeamManager> It(GetWorld()); It; ++It)
    {
        return *It;
    }
    return nullptr;
}

void ACTFGameMode::HandleEndMatch(ETeamColor WinningTeamID)
{
    //show the winning message on all clients via GameState
    if (ACTFGameState* GS = GetGameState<ACTFGameState>())
    {
        GS->MulticastShowEndGameUI(WinningTeamID);
    }

    // set a timer to reset/restart the level after 5 seconds
    GetWorldTimerManager().SetTimer(
        ResetTimerHandle,
        this,
        &ACTFGameMode::CTFResetLevel,
        3.0f,
        false
    );
}

void ACTFGameMode::CTFResetLevel_Implementation()
{
    if (!HasAuthority()) return;


    // for reloading the same map:
    FString CurrentLevel = GetWorld()->GetMapName();
    CurrentLevel.RemoveFromStart(GetWorld()->StreamingLevelsPrefix);

    // listen if you are using a listen-server setup
    FString TravelURL = FString::Printf(TEXT("%s?listen"), *CurrentLevel);
    GetWorld()->ServerTravel(TravelURL);

}
