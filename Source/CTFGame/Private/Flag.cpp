// Fill out your copyright notice in the Description page of Project Settings.

#include "Flag.h"
#include "../CTFGameCharacter.h"
#include "CTFPlayerState.h"
#include "CTFGameState.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerState.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

// Sets default values
AFlag::AFlag()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;
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
    CapsuleComponent->SetCollisionResponseToChannel(ECC_Pawn, ECollisionResponse::ECR_Overlap);
    CapsuleComponent->SetCollisionResponseToChannel(ECC_WorldDynamic, ECollisionResponse::ECR_Overlap);
    CapsuleComponent->SetupAttachment(RootComponent);

    CapsuleOffset = FVector(0.0f, 0.0f, 50.0f);
    CapsuleComponent->SetRelativeLocation(CapsuleOffset);

    CapsuleComponent->OnComponentBeginOverlap.AddDynamic(this, &AFlag::OnFlagOverlap);
}

void AFlag::BeginPlay()
{
    Super::BeginPlay();
    if (HasAuthority()) // Apenas o servidor define a posição inicial
    {
        SpawnLocation = GetActorLocation();
    }
}

void AFlag::Tick(float DeltaTime)
{
}

void AFlag::OnFlagOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!HasAuthority()) return; // Apenas o servidor pode processar capturas

    ACTFGameCharacter* Character = Cast<ACTFGameCharacter>(OtherActor);
    if (Character)
    {
        AttachToPlayer(Character);
    }
}

void AFlag::AttachToPlayer(ACTFGameCharacter* Player)
{
    if (HasAuthority() && Player)
    {
        SetOwner(Player);
        Player->SetHeldFlag(this);
        CapsuleComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Player->SetHasFlag(true);
        MulticastAttachToPlayer(Player);
    }
}

void AFlag::MulticastAttachToPlayer_Implementation(ACTFGameCharacter* Player)
{
    if (Player)
    {
        AttachToComponent(Player->GetMesh3P(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("FlagSocket"));
    }
}

void AFlag::Drop()
{
    if (!HasAuthority())
    {
        ServerDrop();
        return;
    }

    ACTFGameCharacter* Player = Cast<ACTFGameCharacter>(GetOwner());
    if (Player)
    {
        Player->SetHasFlag(false);
        Player->SetHeldFlag(nullptr);
    }

    SetOwner(nullptr);
    DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
    SetActorRotation(FRotator(0.f, 0.f, 0.f), ETeleportType::ResetPhysics);
    SetActorLocation(FVector(GetActorLocation().X, GetActorLocation().Y, SpawnLocation.Z));
    CapsuleComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    GetWorldTimerManager().SetTimer(CollisionEnableTimer, this, &AFlag::EnableCollision, 1.0f, false);
    MulticastDrop();
}

void AFlag::ServerDrop_Implementation()
{
    Drop();
}

bool AFlag::ServerDrop_Validate()
{
    return true;
}

void AFlag::MulticastDrop_Implementation()
{
    DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
    SetActorRotation(FRotator(0.f, 0.f, 0.f), ETeleportType::ResetPhysics);
    SetActorLocation(FVector(GetActorLocation().X, GetActorLocation().Y, SpawnLocation.Z));
}

void AFlag::EnableCollision()
{
    CapsuleComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    UE_LOG(LogTemp, Warning, TEXT("Flag collision re-enabled!"));
}

void AFlag::ServerRespawn_Implementation()
{
	Respawn();
}

bool AFlag::ServerRespawn_Validate()
{
	return true;
}

void AFlag::Respawn()
{
    if (!HasAuthority()) {
		ServerRespawn();
		return;
    }
    ACTFGameCharacter* Player = Cast<ACTFGameCharacter>(GetOwner());
    if (Player) {
        Player->SetHasFlag(false);
		Player->SetHeldFlag(nullptr);
    }
    SetOwner(nullptr);

    DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
    SetActorRotation(FRotator(0.f, 0.f, 0.f), ETeleportType::ResetPhysics);
    SetActorLocation(SpawnLocation);
    UE_LOG(LogTemp, Warning, TEXT("Flag respawned at spawn location!"));
    CapsuleComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    MulticastRespawn();
}

void AFlag::MulticastRespawn_Implementation()
{
    DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
    SetActorRotation(FRotator(0.f, 0.f, 0.f), ETeleportType::ResetPhysics);
    SetActorLocation(SpawnLocation);
}

void AFlag::ScorePoint()
{
    if (!HasAuthority()) return;

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

void AFlag::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AFlag, SpawnLocation);
}
