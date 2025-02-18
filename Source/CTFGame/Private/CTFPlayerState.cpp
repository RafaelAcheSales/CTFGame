// Fill out your copyright notice in the Description page of Project Settings.


#include "CTFPlayerState.h"
#include "../CTFGameCharacter.h"
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
    UE_LOG(LogTemp, Warning, TEXT("OnRep_Team called. Team: %d"), (int32)Team);

    if (AActor* PlayerPawn = GetPawn())
    {
        if (ACTFGameCharacter* Character = Cast<ACTFGameCharacter>(PlayerPawn))
        {
            if (USkeletalMeshComponent* MeshComp = Character->GetMesh3P())
            {
				Character->SetTeamMaterial(Team);
            }
        }
    }
}
void ACTFPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ACTFPlayerState, Team);
}