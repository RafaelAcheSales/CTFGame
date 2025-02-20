#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "TeamColors.h"
#include "../CTFGameGameMode.h"
#include "CTFGameMode.generated.h"

class ATeamManager;

UCLASS()
class CTFGAME_API ACTFGameMode : public ACTFGameGameMode
{
    GENERATED_BODY()

public:
    ACTFGameMode();

    //override postlogin
	virtual void PostLogin(APlayerController* NewPlayer) override;

    ATeamManager* GetTeamManager();

    // Called by GameState when a team has won the match
    void HandleEndMatch(ETeamColor WinningTeamID);
protected:
    // Override the spawning logic so we can pick the correct spawn based on team
    virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;

    // Resets or restarts the level after some delay
	UFUNCTION(Server, Reliable)
    void CTFResetLevel();

    FTimerHandle ResetTimerHandle;
};
