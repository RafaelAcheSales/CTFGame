#include "CTFGameWeaponComponent.h"
#include "CTFGameCharacter.h"
#include "CTFGameProjectile.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "Kismet/GameplayStatics.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Animation/AnimInstance.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"

UCTFGameWeaponComponent::UCTFGameWeaponComponent()
{
    SetIsReplicated(true);

    // Default offset from the character location for projectiles to spawn
    MuzzleOffset = FVector(100.0f, 0.0f, 10.0f);
}

void UCTFGameWeaponComponent::Fire()
{
    UE_LOG(LogTemp, Warning, TEXT("Fire() called"));
    // If we�re a client (or non-authority), call the Server function:
    // The server will do the actual projectile spawn.
    if (!GetOwner() || !GetOwner()->HasAuthority())
    {
        ServerFire();
    }
    else
    {
        // If we **are** the server already, just call directly
        ServerFire();
    }
}

void UCTFGameWeaponComponent::ServerFire_Implementation()
{
    // Validate we have a character + controller
    if (!Character || !Character->GetController())
    {
        return;
    }

    // *** SPAWN THE PROJECTILE ON THE SERVER ***
    if (ProjectileClass)
    {
        UWorld* const World = GetWorld();
        if (World)
        {
            APlayerController* PlayerController = Cast<APlayerController>(Character->GetController());
            if (PlayerController)
            {
                // Determine spawn transform from camera rotation + MuzzleOffset
                const FRotator SpawnRotation = PlayerController->PlayerCameraManager->GetCameraRotation();
                const FVector SpawnLocation = Character->GetActorLocation() + SpawnRotation.RotateVector(MuzzleOffset);

                FActorSpawnParameters ActorSpawnParams;
                ActorSpawnParams.SpawnCollisionHandlingOverride =
                    ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;
                ActorSpawnParams.Owner = Character; // helps with damage ownership, etc.

                // Actually spawn the projectile
                World->SpawnActor<ACTFGameProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, ActorSpawnParams);
            }
        }
    }

    // *** Tell everyone (including the Server) to play the sound/animation ***
    MulticastFireEffects();
}

bool UCTFGameWeaponComponent::ServerFire_Validate()
{
    // You can do any security checks here.
    // Return false if you want to reject the call.
    return true;
}

void UCTFGameWeaponComponent::MulticastFireEffects_Implementation()
{
    // If a FireSound is set, play it at the weapon�s (or Character�s) location
    if (FireSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetComponentLocation());
    }

    // Attempt to play the firing animation. Each client can do its own logic
    // about which mesh (1P or 3P) to play on. If you only want local players
    // to see the 1P arms montage, do an IsLocallyControlled() check.
    if (FireAnimation && Character)
    {
        if (Character->IsLocallyControlled())
        {
            // Use the 1P mesh
            UAnimInstance* AnimInstance = Character->GetMesh1P()->GetAnimInstance();
            if (AnimInstance)
            {
                AnimInstance->Montage_Play(FireAnimation, 1.f);
            }
        }
        else
        {
            // Optionally play on 3P mesh for other clients, if you have a 3P montage
            UAnimInstance* AnimInstance = Character->GetMesh3P()->GetAnimInstance();
            // AnimInstance->Montage_Play(FireAnimation, 1.f);
        }
    }
}

bool UCTFGameWeaponComponent::AttachWeapon(ACTFGameCharacter* TargetCharacter)
{
    Character = TargetCharacter;
    UE_LOG(LogTemp, Warning, TEXT("AttachWeapon() called to character %s"), *Character->GetName());

    // Check that the character is valid, and has no weapon component yet
    if (!Character || Character->GetInstanceComponents().FindItemByClass<UCTFGameWeaponComponent>())
    {
        return false;
    }

    GetOwner()->SetOwner(Character);

    // Attach the weapon to the appropriate mesh
    FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, true);
    bool isLocallyControlled = Character->IsLocallyControlled();
    UE_LOG(LogTemp, Warning, TEXT("isLocallyControlled: %d"), isLocallyControlled);
    if (isLocallyControlled)
    {
        UE_LOG(LogTemp, Warning, TEXT("Attaching to 1P mesh"));
        AttachToComponent(Character->GetMesh1P(), AttachmentRules, FName(TEXT("GripPoint")));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Attaching to 3P mesh"));
        AttachToComponent(Character->GetMesh3P(), AttachmentRules, FName(TEXT("GripPoint")));
    }


    // Set up action bindings
    if (APlayerController* PlayerController = Cast<APlayerController>(Character->GetController()))
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
            ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
        {
            Subsystem->AddMappingContext(FireMappingContext, 1);
        }

        if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerController->InputComponent))
        {
            EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Triggered, this, &UCTFGameWeaponComponent::Fire);
        }
    }

    return true;
}

void UCTFGameWeaponComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // Remove the input mapping context if needed
    if (Character)
    {
        if (APlayerController* PlayerController = Cast<APlayerController>(Character->GetController()))
        {
            if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
                ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
            {
                Subsystem->RemoveMappingContext(FireMappingContext);
            }
        }
    }

    Super::EndPlay(EndPlayReason);
}
