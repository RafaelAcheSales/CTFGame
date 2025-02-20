// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "Delegates/Delegate.h"
#include "TeamColors.h"
#include "CTFPlayerState.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTeamChanged, ETeamColor, NewTeam);
/**
 *
 */
UCLASS()
class CTFGAME_API ACTFPlayerState : public APlayerState
{
    GENERATED_BODY()


public:
    ACTFPlayerState();
    UPROPERTY(BlueprintAssignable,Category = "Team")
    FOnTeamChanged OnTeamChanged;

protected:
    UPROPERTY(ReplicatedUsing = OnRep_Team, VisibleAnywhere, BlueprintReadOnly, Category = "Team")
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
