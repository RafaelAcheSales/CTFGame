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
    // Aqui chamamos a função no Pawn para mudar a cor do Mesh
    if (AActor* PlayerPawn = GetPawn())
    {
        if (ACTFGameCharacter* Character = Cast<ACTFGameCharacter>(PlayerPawn))
        {
            if (USkeletalMeshComponent* MeshComp = Character->GetMesh3P())
            {
                UMaterialInstanceDynamic* DynMaterial = MeshComp->CreateAndSetMaterialInstanceDynamic(0);
                if (DynMaterial)
                {
                    FLinearColor NewColor;
                    switch (Team)
                    {
                    case ETeamColor::Red:
                        NewColor = FLinearColor::Red;
                        break;
                    case ETeamColor::Blue:
                        NewColor = FLinearColor::Blue;
                        break;
                    case ETeamColor::Yellow:
                        NewColor = FLinearColor::Yellow;
                        break;
                    case ETeamColor::Green:
                        NewColor = FLinearColor::Green;
                        break;
                    default:
                        NewColor = FLinearColor::White;
                        break;
                    }
                    DynMaterial->SetVectorParameterValue("Color", NewColor);
                }
            }
        }
    }
}

void ACTFPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ACTFPlayerState, Team);
}