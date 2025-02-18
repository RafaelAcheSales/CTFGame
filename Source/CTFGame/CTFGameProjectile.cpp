// Copyright Epic Games, Inc. All Rights Reserved.

#include "CTFGameProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "../CTFGameCharacter.h"
#include "CTFPlayerState.h"
#include "TeamColors.h"
#include "DrawDebugHelpers.h"
#include "Engine/DamageEvents.h"
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
    // Ensure only the Autonomous Proxy (Owner Client) processes the hit
    if (GetOwner()->GetLocalRole() != ROLE_AutonomousProxy) return;

    if ((OtherActor != nullptr) && (OtherActor != this) && (OtherComp != nullptr))
    {
        if (ACTFGameCharacter* HitCharacter = Cast<ACTFGameCharacter>(OtherActor))
        {
            ACTFGameCharacter* ProjectileOwnerCharacter = Cast<ACTFGameCharacter>(GetOwner());
            ACTFPlayerState* ProjectileOwnerPlayerState = ProjectileOwnerCharacter ? Cast<ACTFPlayerState>(ProjectileOwnerCharacter->GetPlayerState()) : nullptr;
            ACTFPlayerState* HitPlayerState = Cast<ACTFPlayerState>(HitCharacter->GetPlayerState());

            if (ProjectileOwnerPlayerState && HitPlayerState)
            {
                if (ProjectileOwnerPlayerState->GetTeam() == HitPlayerState->GetTeam())
                {
                    UE_LOG(LogTemp, Warning, TEXT("Friendly Fire!"));
                }
                else
                {
                    // Apply damage only once
                    HitCharacter->TakeDamage(20.0f, FDamageEvent(), GetOwner()->GetInstigatorController(), this);
					UE_LOG(LogTemp, Warning, TEXT("Hit %s"), *HitCharacter->GetName());
                }
            }
        }

        if (OtherComp->IsSimulatingPhysics())
        {
            OtherComp->AddImpulseAtLocation(GetVelocity() * 100.0f, GetActorLocation());
        }

        Destroy();
    }
}
