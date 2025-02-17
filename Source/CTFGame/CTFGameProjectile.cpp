// Copyright Epic Games, Inc. All Rights Reserved.

#include "CTFGameProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "../CTFGameCharacter.h"
#include "CTFPlayerState.h"
#include "TeamColors.h"
#include "DrawDebugHelpers.h"
#include "Components/SphereComponent.h"

ACTFGameProjectile::ACTFGameProjectile() 
{
	// Use a sphere as a simple collision representation
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(5.0f);
	CollisionComp->BodyInstance.SetCollisionProfileName("Projectile");
	CollisionComp->OnComponentHit.AddDynamic(this, &ACTFGameProjectile::OnHit);		// set up a notification for when this component hits something blocking

	// Players can't walk on it
	CollisionComp->SetWalkableSlopeOverride(FWalkableSlopeOverride(WalkableSlope_Unwalkable, 0.f));
	CollisionComp->CanCharacterStepUpOn = ECB_No;

	// Set as root component
	RootComponent = CollisionComp;

	// Use a ProjectileMovementComponent to govern this projectile's movement
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->InitialSpeed = 3000.f;
	ProjectileMovement->MaxSpeed = 3000.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = true;

	// Die after 3 seconds by default
	InitialLifeSpan = 3.0f;
}

void ACTFGameProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// Only add impulse and destroy projectile if` we hit a physics
	if ((OtherActor != nullptr) && (OtherActor != this) && (OtherComp != nullptr))
	{
		//try
		//	{
		//	UE_LOG(LogTemp, Warning, TEXT("Owner of Projectile: %s"), *GetOwner()->GetName());
		//	UE_LOG(LogTemp, Warning, TEXT("Owner of HitCharacter: %s"), *OtherActor->GetName());
		//}
		//catch (const std::exception& e)
		//{
		//	UE_LOG(LogTemp, Warning, TEXT("Error: %s"), *FString(e.what()));
		//}

		// if it hit a player, deal damage based on ETeamColor
		// debugs on screen if it hit a player and both projectile + player owners's playerstate colors

		if (ACTFGameCharacter* HitCharacter = Cast<ACTFGameCharacter>(OtherActor))
		{
			ACTFGameCharacter* ProjectileOwnerCharacter = Cast<ACTFGameCharacter>(GetOwner());
			ACTFPlayerState* ProjectileOwnerPlayerState = Cast<ACTFPlayerState>(ProjectileOwnerCharacter->GetPlayerState());
			ACTFPlayerState* HitPlayerState = Cast<ACTFPlayerState>(HitCharacter->GetPlayerState());

			if (ProjectileOwnerPlayerState && HitPlayerState)
			{
				if (ProjectileOwnerPlayerState->GetTeam() == HitPlayerState->GetTeam())
				{
					UE_LOG(LogTemp, Warning, TEXT("Friendly Fire!"));
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("Enemy Hit!"));
				}
			}
		}

		if (OtherComp->IsSimulatingPhysics())
		{
			OtherComp->AddImpulseAtLocation(GetVelocity() * 100.0f, GetActorLocation());

			Destroy();
		}
	}
}