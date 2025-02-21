// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "TeamColors.h"
#include "CTFGameState.generated.h"


USTRUCT(BlueprintType)
struct FTeamScore
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ETeamColor TeamID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Score;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnScoreUpdated, const TArray<FTeamScore>&, UpdatedTeamScores);
// tells UI that game ended by send winning team color name from UEnumHelper
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEndGame, const FString&, WinningTeamName);
/**
 * 
 */
UCLASS()
class CTFGAME_API ACTFGameState : public AGameStateBase
{
	GENERATED_BODY()
	
public:
	UPROPERTY(ReplicatedUsing = OnRep_Score, BlueprintReadOnly, VisibleAnywhere, Category = "Score")
    TArray<FTeamScore> TeamScores;

    UPROPERTY(BlueprintAssignable, Category = "Score")
    FOnScoreUpdated OnScoreUpdatedEvent;

	UPROPERTY(BlueprintAssignable, Category = "EndGame")
	FOnEndGame OnEndGameEvent;

    // multicast function to show the end-game UI on all machines
    UFUNCTION(NetMulticast, Reliable)
    void MulticastShowEndGameUI(ETeamColor WinningTeamID);

    UFUNCTION()
    void OnRep_Score();

    UFUNCTION(BlueprintCallable, Category = "Score")
    void UpdateTeamScore(ETeamColor TeamID, int32 Points);

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};

