// Fill out your copyright notice in the Description page of Project Settings.


#include "Flag.h"
#include "../CTFGameCharacter.h"
#include "CTFPlayerState.h"
#include "CTFGameState.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerState.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AFlag::AFlag()
{
    PrimaryActorTick.bCanEverTick = true;
    SpawnLocation = GetActorLocation();

    // Adds a mesh component to the flag
    FlagMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FlagMesh"));
    RootComponent = FlagMesh;

    // Adds capsule collider
    CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CollisionCapsule"));
    CapsuleComponent->InitCapsuleSize(50.0f, 75.0f);
    CapsuleComponent->SetCollisionProfileName(TEXT("Trigger"));
    CapsuleComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    CapsuleComponent->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
    CapsuleComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    CapsuleComponent->SetCollisionResponseToChannel(ECC_Pawn, ECollisionResponse::ECR_Overlap); // Detects players
    CapsuleComponent->SetCollisionResponseToChannel(ECC_WorldDynamic, ECollisionResponse::ECR_Overlap); // Detects Stuff
    CapsuleComponent->SetupAttachment(RootComponent);

    CapsuleOffset = FVector(0.0f, 0.0f, 50.0f);
    CapsuleComponent->SetRelativeLocation(CapsuleOffset);

    CapsuleComponent->OnComponentBeginOverlap.AddDynamic(this, &AFlag::OnFlagOverlap);
}

// Called when the game starts or when spawned
void AFlag::BeginPlay()
{
    Super::BeginPlay();
    SpawnLocation = GetActorLocation();
}

// Called every frame
void AFlag::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AFlag::OnFlagOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    ACTFGameCharacter* Character = Cast<ACTFGameCharacter>(OtherActor);
    if (Character == nullptr) { return; }

    UE_LOG(LogTemp, Warning, TEXT("Flag overlapped with: %s"), *OtherActor->GetName());
    AttachToPlayer(Character);
}

void AFlag::AttachToPlayer(ACTFGameCharacter* Player)
{
    if (Player)
    {
        AttachToComponent(Player->GetMesh3P(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("FlagSocket"));
        SetOwner(Player);
        Player->SetHeldFlag(this); // Store flag reference in the player
        UE_LOG(LogTemp, Warning, TEXT("Flag attached to player: %s"), *Player->GetName());

        // Disable overlap event
        CapsuleComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Player->SetHasFlag(true);
    }
}


void AFlag::Drop()
{
    ACTFGameCharacter* Player = Cast<ACTFGameCharacter>(GetOwner());
    if (Player) {
        Player->SetHasFlag(false);
        Player->SetHeldFlag(nullptr); // Clear reference in player
    }

    DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
    SetActorRotation(FRotator(0.f, 0.f, 0.f), ETeleportType::ResetPhysics);
    SetActorLocation(FVector(GetActorLocation().X, GetActorLocation().Y, SpawnLocation.Z));

    SetOwner(nullptr);
    UE_LOG(LogTemp, Warning, TEXT("Flag dropped!"));

    // Temporarily disable collision
    CapsuleComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Re-enable collision after a short delay (e.g., 1 second)
    GetWorldTimerManager().SetTimer(CollisionEnableTimer, this, &AFlag::EnableCollision, 1.0f, false);
}

void AFlag::EnableCollision()
{
    CapsuleComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    UE_LOG(LogTemp, Warning, TEXT("Flag collision re-enabled!"));
}


void AFlag::Respawn()
{
    ACTFGameCharacter* Player = Cast<ACTFGameCharacter>(GetOwner());
    if (Player) {
        Player->SetHasFlag(false);
    }
    DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
    SetActorRotation(FRotator(0.f, 0.f, 0.f), ETeleportType::ResetPhysics);
    SetActorLocation(SpawnLocation);
    SetOwner(nullptr);
    UE_LOG(LogTemp, Warning, TEXT("Flag respawned at spawn location!"));
    //Enable overlapp event
    CapsuleComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);


}

void AFlag::ScorePoint()
{
    ACTFGameCharacter* FlagHolder = Cast<ACTFGameCharacter>(GetOwner());
    if (FlagHolder)
    {
        ACTFGameState* GameState = GetWorld()->GetGameState<ACTFGameState>();
        if (GameState)
        {
			ACTFPlayerState* MyPlayerState = Cast<ACTFPlayerState>(FlagHolder->GetPlayerState());
			if (MyPlayerState)
			{
				ETeamColor TeamID = MyPlayerState->GetTeam();
				GameState->UpdateTeamScore(TeamID, 1);
				Respawn();
			}
        }
    }
}
