// Fill out your copyright notice in the Description page of Project Settings.


#include "CTFGameState.h"
#include "Net/UnrealNetwork.h"

void ACTFGameState::OnRep_Score()
{
    // Check if score >= 3 then endgame
	for (const FTeamScore& Team : TeamScores)
	{
		if (Team.Score >= 3)
		{
			// End game
			UE_LOG(LogTemp, Warning, TEXT("Team %d won the game!"), Team.TeamID);

		}
	}
	OnScoreUpdatedEvent.Broadcast(TeamScores);
}

void ACTFGameState::UpdateTeamScore(ETeamColor TeamID, int32 Points)
{
    if (HasAuthority())  // Apenas o servidor pode modificar
    {
        bool bFound = false;
        for (FTeamScore& Team : TeamScores)
        {
            if (Team.TeamID == TeamID)
            {
                Team.Score += Points;
                bFound = true;
                break;
            }
        }

        if (!bFound)
        {
            FTeamScore NewTeamScore;
            NewTeamScore.TeamID = TeamID;
            NewTeamScore.Score = Points;
            TeamScores.Add(NewTeamScore);
        }

        OnRep_Score();
    }
}

void ACTFGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ACTFGameState, TeamScores);
}
