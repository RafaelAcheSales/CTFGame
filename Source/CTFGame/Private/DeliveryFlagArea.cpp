#include "DeliveryFlagArea.h"
#include "../CTFGameCharacter.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "CTFGameState.h"
#include "Net/UnrealNetwork.h"

ADeliveryFlagArea::ADeliveryFlagArea()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;

    // create and set up TriggerBox component
    TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
    RootComponent = TriggerBox;

    //set up collision for trigger detection
    TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    TriggerBox->SetCollisionObjectType(ECC_WorldDynamic);
    TriggerBox->SetCollisionResponseToAllChannels(ECR_Ignore);
    TriggerBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &ADeliveryFlagArea::OnOverlapBegin);
    TriggerBox->OnComponentEndOverlap.AddDynamic(this, &ADeliveryFlagArea::OnOverlapEnd);

    // create and set up StationMesh component
    StationMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StationMesh"));
    StationMesh->SetupAttachment(RootComponent);
}

void ADeliveryFlagArea::BeginPlay()
{
    Super::BeginPlay();
    if (HasAuthority())
    {
        Flag = Cast<AFlag>(UGameplayStatics::GetActorOfClass(GetWorld(), AFlag::StaticClass()));
        if (!Flag)
        {
            UE_LOG(LogTemp, Error, TEXT("Flag object not found by delivery area"));
        }
    }
}

void ADeliveryFlagArea::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    if (!HasAuthority()) return;

    ACTFGameCharacter* ScoringPlayer = Cast<ACTFGameCharacter>(OtherActor);
    if (ScoringPlayer && ScoringPlayer->GetHasFlag())
    {
        ServerDeliverFlag(ScoringPlayer);
    }
}

void ADeliveryFlagArea::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (OtherActor)
    {
        UE_LOG(LogTemp, Warning, TEXT("%s left the flag delivery area!"), *OtherActor->GetName());
    }
}

void ADeliveryFlagArea::ServerDeliverFlag_Implementation(AActor* ActorDelivering)
{
    if (ActorDelivering && HasAuthority())
    {
        MulticastDeliverFlag(ActorDelivering);
        if (Flag)
        {
            Flag->Respawn();
        }
    }
}

bool ADeliveryFlagArea::ServerDeliverFlag_Validate(AActor* ActorDelivering)
{
    return true;
}

void ADeliveryFlagArea::MulticastDeliverFlag_Implementation(AActor* ActorDelivering)
{
    if (ActorDelivering)
    {
        ACTFGameState* GameState = GetWorld()->GetGameState<ACTFGameState>();
        if (GameState)
        {
            GameState->UpdateTeamScore(Team, 1);
            UE_LOG(LogTemp, Warning, TEXT("%s delivered the flag to the base!"), *ActorDelivering->GetName());

        }
    }
}

void ADeliveryFlagArea::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ADeliveryFlagArea, Flag);
}
