#include "CTFGameState.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "GameFramework/GameModeBase.h"
#include "Kismet/GameplayStatics.h"

// Include your custom GameMode header
#include "CTFGameMode.h"

void ACTFGameState::OnRep_Score()
{
    // Check if any team has reached or exceeded 3 points
    for (const FTeamScore& Team : TeamScores)
    {
        if (Team.Score >= 3)
        {
            OnScoreUpdatedEvent.Broadcast(TeamScores);
            // Only the server should handle the "end match" logic
            if (HasAuthority())
            {
                // Get the GameMode and tell it who won
                if (ACTFGameMode* GM = Cast<ACTFGameMode>(GetWorld()->GetAuthGameMode()))
                {
                    GM->HandleEndMatch(Team.TeamID);
                }
            }
            // Once a winner is found, we can stop checking
            return;
        }
    }

    // If nobody has 3 yet, just broadcast the updated scores for UI
    OnScoreUpdatedEvent.Broadcast(TeamScores);
}

void ACTFGameState::MulticastShowEndGameUI_Implementation(ETeamColor WinningTeamID)
{
    // This runs on ALL machines (server + clients).
    // Show "Team X Won!" in your UI. Implementation depends on your UI setup.
    UE_LOG(LogTemp, Warning, TEXT("Team %d won the game! (UI shown on all clients)"), static_cast<uint8>(WinningTeamID));

    // Example pseudo-code (if using UMG):
    //APlayerController* PC = GetWorld()->GetFirstPlayerController();
    //if (PC && EndGameWidgetClass)
    //{
    //    UYourEndGameWidget* Widget = CreateWidget<UYourEndGameWidget>(PC, EndGameWidgetClass);
    //    if (Widget)
    //    {
    //        Widget->Setup(WinningTeamID);
    //        Widget->AddToViewport();
    //    }
    //}
    
}

void ACTFGameState::UpdateTeamScore(ETeamColor TeamID, int32 Points)
{
    if (HasAuthority())  // Only the server can modify
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
            // If this is the first time we see this team, add a new entry
            FTeamScore NewTeam;
            NewTeam.TeamID = TeamID;
            NewTeam.Score = Points;
            TeamScores.Add(NewTeam);
        }

        // Force a replication update (calls OnRep_Score on clients)
        OnRep_Score();
    }
}

void ACTFGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ACTFGameState, TeamScores);
}
