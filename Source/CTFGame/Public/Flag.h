// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Flag.generated.h"


//Flag that saves its spawn location. it also detects player interaction (collided) and sets the flag as taken.
//Attaches to socket on player's back called "FlagSocket".
//If player moves it to its same ETeamColor base then it scores a point.
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


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Save position for respawn
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	FVector SpawnLocation;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
