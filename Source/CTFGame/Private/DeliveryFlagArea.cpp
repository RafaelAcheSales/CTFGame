#include "DeliveryFlagArea.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h"
#include "CTFGameState.h"

// Sets default values
ADeliveryFlagArea::ADeliveryFlagArea()
{
    // Enable ticking if needed
    PrimaryActorTick.bCanEverTick = false;

    // Create and set up TriggerBox component
    TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
    RootComponent = TriggerBox;

    // Set up collision for trigger detection
    TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    TriggerBox->SetCollisionObjectType(ECC_WorldDynamic);
    TriggerBox->SetCollisionResponseToAllChannels(ECR_Ignore);
    TriggerBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    // Bind overlap events
    TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &ADeliveryFlagArea::OnOverlapBegin);
    TriggerBox->OnComponentEndOverlap.AddDynamic(this, &ADeliveryFlagArea::OnOverlapEnd);

	// Create and set up StationMesh component
	StationMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StationMesh"));
	StationMesh->SetupAttachment(RootComponent);


}

// Called when the game starts or when spawned
void ADeliveryFlagArea::BeginPlay()
{
    Super::BeginPlay();
}

// Handle flag delivery when an actor enters the area
void ADeliveryFlagArea::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    if (OtherActor)
    {
        UE_LOG(LogTemp, Warning, TEXT("%s entered the flag delivery area!"), *OtherActor->GetName());
        DeliverFlag(OtherActor);
    }
}

// Handle when an actor leaves the trigger area
void ADeliveryFlagArea::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (OtherActor)
    {
        UE_LOG(LogTemp, Warning, TEXT("%s left the flag delivery area!"), *OtherActor->GetName());
    }
}

// Process flag delivery (customize as needed)
void ADeliveryFlagArea::DeliverFlag(AActor* ActorDelivering)
{
    if (ActorDelivering)
    {
        // Get game state
		ACTFGameState* GameState = Cast<ACTFGameState>(GetWorld()->GetGameState());
        if (GameState)
        {
            GameState->UpdateTeamScore(Team, 1);
            UE_LOG(LogTemp, Warning, TEXT("%s delivered the flag to the base!"), *ActorDelivering->GetName());
        }
    }
}
