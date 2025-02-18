#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "TeamColors.h"
#include "DeliveryFlagArea.generated.h"

UCLASS()
class CTFGAME_API ADeliveryFlagArea : public AActor
{
    GENERATED_BODY()

public:
    // Sets default values for this actor's properties
    ADeliveryFlagArea();

protected:
    virtual void BeginPlay() override;

public:
    // Trigger Box Component
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

    // Function to handle flag delivery logic
    void DeliverFlag(AActor* ActorDelivering);

	//MEsh for station
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* StationMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Team")
	ETeamColor Team;
};