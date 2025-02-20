#include "TeamManager.h"
#include "Net/UnrealNetwork.h"
#include "TeamColors.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerStart.h"
#include "CTFPlayerState.h"

ATeamManager::ATeamManager()
{
    bReplicates = true;

    // Inicializa os times disponíveis
    TeamColors.Add(ETeamColor::Red);
    TeamColors.Add(ETeamColor::Blue);
    // TeamColors.Add(ETeamColor::Yellow);
    // TeamColors.Add(ETeamColor::Green);
}

ETeamColor GetTeamColorFromTag(const FName& Tag)
{
    if (Tag == "Red") return ETeamColor::Red;
    if (Tag == "Blue") return ETeamColor::Blue;
    if (Tag == "Yellow") return ETeamColor::Yellow;
    if (Tag == "Green") return ETeamColor::Green;
    return ETeamColor::None;
}

void ATeamManager::BeginPlay()
{
    Super::BeginPlay();

    PopulateTeams();
}

void ATeamManager::PopulateTeams()
{
	TeamStartPoints.Empty();
    for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
    {
        ETeamColor TeamColor = GetTeamColorFromTag(It->PlayerStartTag);

        if (TeamColor != ETeamColor::None)
        {
            TeamStartPoints.Add(*It);
        }
    }
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

APlayerStart* ATeamManager::GetSpawnPoint(ETeamColor TeamColor)
{
    for (APlayerStart* StartPoint : TeamStartPoints)
    {
        if (GetTeamColorFromTag(StartPoint->PlayerStartTag) == TeamColor)
        {
            //logs all info about spawning
			UE_LOG(LogTemp, Warning, TEXT("SpawnPoint: %s"), *StartPoint->GetName());
			UE_LOG(LogTemp, Warning, TEXT("SpawnPoint Tag: %s"), *StartPoint->PlayerStartTag.ToString());
			UE_LOG(LogTemp, Warning, TEXT("SpawnPoint Team: %d"), (int32)GetTeamColorFromTag(StartPoint->PlayerStartTag));
			UE_LOG(LogTemp, Warning, TEXT("SpawnPoint Player Start Tag: %s"), *StartPoint->PlayerStartTag.ToString());
            //current role net

            return StartPoint;
        }
    }
    return nullptr;
}

void ATeamManager::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ATeamManager, TeamColors);
    DOREPLIFETIME(ATeamManager, TeamPlayers);
    DOREPLIFETIME(ATeamManager, TeamStartPoints);
}
