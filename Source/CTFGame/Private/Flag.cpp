// Fill out your copyright notice in the Description page of Project Settings.


#include "Flag.h"
#include "../CTFGameCharacter.h"

// Sets default values
AFlag::AFlag()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	SpawnLocation = GetActorLocation();

	// Adds a mesh component to the flag
	FlagMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FlagMesh"));
	RootComponent = FlagMesh;

	//Adds capsule collider
	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CollisionCapsule"));
	CapsuleComponent->InitCapsuleSize(50.0f, 75.0f);
	CapsuleComponent->SetCollisionProfileName(TEXT("Trigger"));

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

	UE_LOG(LogTemp, Warning, TEXT("Flag overlapped with: %s"), *OtherActor->GetName())
	ACTFGameCharacter* Character = Cast<ACTFGameCharacter>(OtherActor);
	if (Character == nullptr) { return; }
}

