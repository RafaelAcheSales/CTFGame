#include "TeamManager.h"
#include "Net/UnrealNetwork.h"
#include "CTFPlayerState.h"

ATeamManager::ATeamManager()
{
    bReplicates = true;

    // Inicializa os times disponíveis
    TeamColors.Add(ETeamColor::Red);
    TeamColors.Add(ETeamColor::Blue);
    TeamColors.Add(ETeamColor::Yellow);
    TeamColors.Add(ETeamColor::Green);
}

void ATeamManager::AssignPlayerToTeam(APlayerState* PlayerState)
{
    if (!HasAuthority() || !PlayerState) return;

    ACTFPlayerState* PS = Cast<ACTFPlayerState>(PlayerState);
    if (PS)
    {
        ETeamColor AssignedTeam = GetBalancedTeam();
        PS->SetTeam(AssignedTeam);

        TeamPlayers.Add(PlayerState);
    }
}

ETeamColor ATeamManager::GetBalancedTeam()
{
    ETeamColor BestTeam = ETeamColor::Red;
    int32 MinPlayers = MAX_int32;
    TMap<ETeamColor, int32> TeamCount;

    for (ETeamColor Team : TeamColors)
    {
        TeamCount.Add(Team, 0);
    }

    for (APlayerState* Player : TeamPlayers)
    {
        ACTFPlayerState* PS = Cast<ACTFPlayerState>(Player);
        if (PS)
        {
            TeamCount[PS->GetTeam()]++;
        }
    }

    for (const auto& Pair : TeamCount)
    {
        if (Pair.Value < MinPlayers)
        {
            MinPlayers = Pair.Value;
            BestTeam = Pair.Key;
        }
    }

    return BestTeam;
}

void ATeamManager::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ATeamManager, TeamColors);
    DOREPLIFETIME(ATeamManager, TeamPlayers);
}
