// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "TeamColors.h"
#include "CTFPlayerState.generated.h"

/**
 *
 */
UCLASS()
class CTFGAME_API ACTFPlayerState : public APlayerState
{
    GENERATED_BODY()


public:
    ACTFPlayerState();

protected:
    UPROPERTY(ReplicatedUsing = OnRep_Team)
    ETeamColor Team;

public:
    UFUNCTION(BlueprintCallable, Category = "Team")
    ETeamColor GetTeam() const { return Team; }

    UFUNCTION(Server, Reliable, WithValidation)
    void SetTeam(ETeamColor NewTeam);

protected:
    UFUNCTION()
    void OnRep_Team();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
