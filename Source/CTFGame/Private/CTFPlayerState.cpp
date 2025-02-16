// Fill out your copyright notice in the Description page of Project Settings.


#include "CTFPlayerState.h"
#include "Net/UnrealNetwork.h"

ACTFPlayerState::ACTFPlayerState()
{
	Team = ETeamColor::None;
}

void ACTFPlayerState::SetTeam_Implementation(ETeamColor NewTeam)
{
    if (HasAuthority())
    {
        Team = NewTeam;
        OnRep_Team();
    }
}
bool ACTFPlayerState::SetTeam_Validate(ETeamColor NewTeam)
{
    return true;
}

void ACTFPlayerState::OnRep_Team()
{
    // UpdateUI
}

void ACTFPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ACTFPlayerState, Team);
}