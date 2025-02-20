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
    MuzzleOffset = FVector(100.0f, 0.0f, 10.0f);
}

void UCTFGameWeaponComponent::Fire()
{
    if (GetOwner() && GetOwner()->HasAuthority())
    {
        ServerFire_Implementation(); // Call directly if server
    }
    else
    {
        ServerFire(); // Call via RPC if client
    }
}

void UCTFGameWeaponComponent::ServerFire_Implementation()
{
    if (!Character || !Character->GetController() || !ProjectileClass)
    {
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    APlayerController* PlayerController = Cast<APlayerController>(Character->GetController());
    if (!PlayerController || !PlayerController->PlayerCameraManager)
    {
        return;
    }

    FRotator SpawnRotation = PlayerController->PlayerCameraManager->GetCameraRotation();
    FVector SpawnLocation = Character->GetActorLocation() + SpawnRotation.RotateVector(MuzzleOffset);

    FActorSpawnParameters ActorSpawnParams;
    ActorSpawnParams.SpawnCollisionHandlingOverride =
        ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;
    ActorSpawnParams.Owner = Character;

    World->SpawnActor<ACTFGameProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, ActorSpawnParams);

    MulticastFireEffects();
}

bool UCTFGameWeaponComponent::ServerFire_Validate()
{
    return true;
}

void UCTFGameWeaponComponent::MulticastFireEffects_Implementation()
{
    if (FireSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetComponentLocation());
    }

    if (!Character || !FireAnimation)
    {
        return;
    }

    if (Character->IsLocallyControlled())
    {
        UAnimInstance* AnimInstance = Character->GetMesh1P()->GetAnimInstance();
        if (AnimInstance)
        {
            AnimInstance->Montage_Play(FireAnimation, 1.f);
        }
    }
    else
    {
        UAnimInstance* AnimInstance = Character->GetMesh3P()->GetAnimInstance();
        if (AnimInstance)
        {
            AnimInstance->Montage_Play(FireAnimation, 1.f);
        }
    }
}

bool UCTFGameWeaponComponent::AttachWeapon(ACTFGameCharacter* TargetCharacter)
{
    Character = TargetCharacter;
    UE_LOG(LogTemp, Warning, TEXT("AttachWeapon() called to character %s"), *Character->GetName());

    if (!Character || Character->FindComponentByClass<UCTFGameWeaponComponent>())
    {
        return false;
    }

    Character->SetWeapon(GetOwner());
    GetOwner()->SetOwner(Character);

    FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, true);

    ENetRole CurrentRole = Character->GetLocalRole();
    UE_LOG(LogTemp, Warning, TEXT("AttachWeapon() Role: %d"), static_cast<int32>(CurrentRole));

    if (CurrentRole == ROLE_AutonomousProxy)
    {
        AttachToComponent(Character->GetMesh1P(), AttachmentRules, FName(TEXT("GripPoint")));
    }
    else if (CurrentRole == ROLE_SimulatedProxy)
    {
        AttachToComponent(Character->GetMesh3P(), AttachmentRules, FName(TEXT("GripPoint")));
    }

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

void UCTFGameWeaponComponent::DetachWeapon()
{
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
        Character = nullptr;
    }
}

void UCTFGameWeaponComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    DetachWeapon();
    Super::EndPlay(EndPlayReason);
}

void UCTFGameWeaponComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UCTFGameWeaponComponent, MuzzleOffset);
}
