// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/CapsuleComponent.h"
#include "Flag.generated.h"

class USkeletalMeshComponent;
class UCapsuleComponent;
class ACTFGameCharacter;

// Flag that saves its spawn location. It also detects player interaction (collided) and sets the flag as taken.
// Attaches to socket on player's back called "FlagSocket".
// If player moves it to its same ETeamColor base, then it scores a point.
UCLASS()
class CTFGAME_API AFlag : public AActor
{
    GENERATED_BODY()

public:
    // Sets default values for this actor's properties
    AFlag();

    // Mesh for the flag
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
    USkeletalMeshComponent* FlagMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
    UCapsuleComponent* CapsuleComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
    FVector CapsuleOffset;

    // Drop the flag
    UFUNCTION(Server, Reliable, WithValidation)
    void ServerDrop();
    void ServerDrop_Implementation();
    bool ServerDrop_Validate();

    UFUNCTION(NetMulticast, Reliable)
    void MulticastDrop();
    void MulticastDrop_Implementation();

    void Drop();
    void EnableCollision();

	// Server Respawn the flag
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerRespawn();
	void ServerRespawn_Implementation();
	bool ServerRespawn_Validate();

    // Respawn the flag
    UFUNCTION(NetMulticast, Reliable)
    void MulticastRespawn();
    void MulticastRespawn_Implementation();
    void Respawn();

    // Score a point
    void ScorePoint();

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

    FTimerHandle CollisionEnableTimer;

    //Save position for respawn
    UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    FVector SpawnLocation;

    // attaches to player
    void AttachToPlayer(ACTFGameCharacter* Player);

    UFUNCTION(NetMulticast, Reliable)
    void MulticastAttachToPlayer(ACTFGameCharacter* Player);
    void MulticastAttachToPlayer_Implementation(ACTFGameCharacter* Player);

public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;

    UFUNCTION()
    void OnFlagOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};