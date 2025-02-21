#include "CTFGameState.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "UEnumHelper.h"
#include "GameFramework/GameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "CTFGameMode.h"

void ACTFGameState::OnRep_Score()
{
    //check if any team has reached or exceeded 3 points
    for (const FTeamScore& Team : TeamScores)
    {
        if (Team.Score >= 3)
        {
            OnScoreUpdatedEvent.Broadcast(TeamScores);
            // Ornly the server should handle the "end match" logic
            if (HasAuthority())
            {
                // get the GameMode and tell it who won
                if (ACTFGameMode* GM = Cast<ACTFGameMode>(GetWorld()->GetAuthGameMode()))
                {
                    GM->HandleEndMatch(Team.TeamID);
                }
            }
            //once a winner is found, we can stop checking
            return;
        }
    }

    // if nobody has 3 yet, just broadcast the updated scores for UI
    OnScoreUpdatedEvent.Broadcast(TeamScores);
}

void ACTFGameState::MulticastShowEndGameUI_Implementation(ETeamColor WinningTeamID)
{
	OnEndGameEvent.Broadcast(UEnumHelper::EnumToString(WinningTeamID));
    
}

void ACTFGameState::UpdateTeamScore(ETeamColor TeamID, int32 Points)
{
    if (HasAuthority())
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
            FTeamScore NewTeam;
            NewTeam.TeamID = TeamID;
            NewTeam.Score = Points;
            TeamScores.Add(NewTeam);
        }
        OnRep_Score();
    }
}

void ACTFGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ACTFGameState, TeamScores);
}
