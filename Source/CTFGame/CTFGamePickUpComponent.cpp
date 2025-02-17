// Copyright Epic Games, Inc. All Rights Reserved.

#include "CTFGamePickUpComponent.h"

UCTFGamePickUpComponent::UCTFGamePickUpComponent()
{
	// Setup the Sphere Collision
	SphereRadius = 32.f;
}

void UCTFGamePickUpComponent::BeginPlay()
{
	Super::BeginPlay();

	// Register our Overlap Event
	OnComponentBeginOverlap.AddDynamic(this, &UCTFGamePickUpComponent::OnSphereBeginOverlap);
}

void UCTFGamePickUpComponent::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Checking if it is a First Person Character overlapping
	ACTFGameCharacter* Character = Cast<ACTFGameCharacter>(OtherActor);
	if(Character != nullptr)
	{
		// Return in case player already has Weapon Attached
		if(Character->GetWeapon() != nullptr)
		{
			return;
		}

		// Notify that the actor is being picked up
		OnPickUp.Broadcast(Character);

		// Unregister from the Overlap Event so it is no longer triggered
		OnComponentBeginOverlap.RemoveAll(this);
	}
}
