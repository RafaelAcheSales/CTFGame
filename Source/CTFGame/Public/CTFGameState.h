// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "TeamColors.h"
#include "CTFGameState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnScoreUpdated);

USTRUCT(BlueprintType)
struct FTeamScore
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ETeamColor TeamID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Score;
};

/**
 * 
 */
UCLASS()
class CTFGAME_API ACTFGameState : public AGameState
{
	GENERATED_BODY()
	
public:
    UPROPERTY(ReplicatedUsing = OnRep_Score)
    TArray<FTeamScore> TeamScores;

    UPROPERTY(BlueprintAssignable, Category = "Score")
    FOnScoreUpdated OnScoreUpdatedEvent;

    UFUNCTION()
    void OnRep_Score();

    void UpdateTeamScore(ETeamColor TeamID, int32 Points);

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};

