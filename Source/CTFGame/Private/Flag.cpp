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
        UE_LOG(LogTemp, Warning, TEXT("Flag attached to player: %s"), *Player->GetName());
        //Disable overlapp event
        CapsuleComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
}

void AFlag::Drop()
{
    DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
    SetOwner(nullptr);
    UE_LOG(LogTemp, Warning, TEXT("Flag dropped!"));
    //Enable overlapp event
    CapsuleComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void AFlag::Respawn()
{
    DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
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
