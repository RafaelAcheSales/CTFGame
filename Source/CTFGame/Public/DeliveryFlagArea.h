// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "Flag.h"
#include "TeamColors.h"
#include "DeliveryFlagArea.generated.h"

UCLASS()
class CTFGAME_API ADeliveryFlagArea : public AActor
{
    GENERATED_BODY()

public:
    ADeliveryFlagArea();

protected:
    virtual void BeginPlay() override;

public:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UBoxComponent* TriggerBox;

    // Overlap Functions
    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    // function to handle flag delivery logic
    UFUNCTION(Server, Reliable, WithValidation)
    void ServerDeliverFlag(AActor* ActorDelivering);
    void ServerDeliverFlag_Implementation(AActor* ActorDelivering);
    bool ServerDeliverFlag_Validate(AActor* ActorDelivering);

    UFUNCTION(NetMulticast, Reliable)
    void MulticastDeliverFlag(AActor* ActorDelivering);
    void MulticastDeliverFlag_Implementation(AActor* ActorDelivering);

    void DeliverFlag(AActor* ActorDelivering);

    // Mesh for station
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* StationMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Team")
    ETeamColor Team;

    UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    AFlag* Flag;

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
