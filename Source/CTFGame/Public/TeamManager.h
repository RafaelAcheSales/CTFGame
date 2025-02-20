#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TeamColors.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/PlayerStart.h"
#include "Net/UnrealNetwork.h"
#include "TeamManager.generated.h"

UCLASS()
class CTFGAME_API ATeamManager : public AActor
{
    GENERATED_BODY()

public:
    ATeamManager();

protected:
    UPROPERTY(Replicated)
    TArray<ETeamColor> TeamColors;

    UPROPERTY(Replicated)
    TArray<APlayerState*> TeamPlayers;

    UPROPERTY(Replicated)
    TArray<APlayerStart*> TeamStartPoints;

public:
    UFUNCTION(BlueprintCallable, Category = "Teams")
    void AssignPlayerToTeam(APlayerState* PlayerState);

    UFUNCTION(BlueprintCallable, Category = "Teams")
    ETeamColor GetBalancedTeam();

    UFUNCTION(BlueprintCallable, Category = "Teams")
    APlayerStart* GetSpawnPoint(ETeamColor TeamColor);

protected:
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void BeginPlay() override;
};
