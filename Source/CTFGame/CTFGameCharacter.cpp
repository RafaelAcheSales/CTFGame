// Copyright Epic Games, Inc. All Rights Reserved.

#include "CTFGameCharacter.h"
#include "CTFGameProjectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Kismet/GameplayStatics.h"
#include "TeamManager.h"
#include "Engine/LocalPlayer.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// ACTFGameCharacter

ACTFGameCharacter::ACTFGameCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);
		
	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-10.f, 0.f, 60.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	Mesh1P->SetRelativeLocation(FVector(-30.f, 0.f, -150.f));

	// Create a mesh component that will be used when being viewed from a '3rd person' view (when controlling other pawns)
	Mesh3P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh3P"));
	Mesh3P->SetOwnerNoSee(true);
	Mesh3P->SetupAttachment(GetCapsuleComponent());
	Mesh3P->bCastDynamicShadow = true;
	Mesh3P->CastShadow = true;
	Mesh3P->SetRelativeLocation(FVector(0.f, 0.f, -90.f));

	SetWeapon(nullptr);

	MaxHealth = 100.0f;
	Health = MaxHealth;

}

//////////////////////////////////////////////////////////////////////////// Input

void ACTFGameCharacter::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();

	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void ACTFGameCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{	
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ACTFGameCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ACTFGameCharacter::Look);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void ACTFGameCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	// CReate team material dynamic
	for (int32 i = 0; i < RedMaterials.Num(); i++)
	{
		RedDynamicMaterials.Add(UMaterialInstanceDynamic::Create(RedMaterials[i], this));
	}
}

//override tick so that


void ACTFGameCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add movement 
		AddMovementInput(GetActorForwardVector(), MovementVector.Y);
		AddMovementInput(GetActorRightVector(), MovementVector.X);
	}
}

void ACTFGameCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

float ACTFGameCharacter::TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float DamageApplied = FMath::Min(Health, Damage); // Don't go below 0
	Health -= DamageApplied;

	UE_LOG(LogTemp, Warning, TEXT("Player %s took damage: %f"), *GetName(), DamageApplied);

	if (Health <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s has died!"), *GetName());

		// Handle Death & Respawn
		GetWorldTimerManager().SetTimerForNextTick(this, &ACTFGameCharacter::Respawn);
	}

	return DamageApplied;
}

void ACTFGameCharacter::Respawn()
{
	Health = MaxHealth;
	UE_LOG(LogTemp, Warning, TEXT("%s has respawned!"), *GetName());



}

void ACTFGameCharacter::SetTeamMaterial(ETeamColor Team)
{
	if (USkeletalMeshComponent* MeshComp = GetMesh3P())
	{
		// Log dos materiais antes da alteração
		UE_LOG(LogTemplateCharacter, Log, TEXT("Materials before change:"));
		for (int32 i = 0; i < MeshComp->GetNumMaterials(); i++)
		{
			UMaterialInterface* CurrentMaterial = MeshComp->GetMaterial(i);
			if (CurrentMaterial)
			{
				UE_LOG(LogTemplateCharacter, Log, TEXT("Slot %d: %s"), i, *CurrentMaterial->GetName());
			}
		}

		// Seleção do array de materiais do time
		TArray<UMaterialInstanceDynamic*> TeamMaterials;
		switch (Team)
		{
		case ETeamColor::Red:
			TeamMaterials = RedDynamicMaterials;
			break;
		case ETeamColor::Blue:
			TeamMaterials = BlueDynamicMaterials;
			break;
		default:
			UE_LOG(LogTemplateCharacter, Error, TEXT("Team color not recognized!"));
			return;
		}

		// Aplicação dos novos materiais
		for (int32 i = 0; i < TeamMaterials.Num(); i++)
		{
			if (TeamMaterials[i])
			{
				UMaterialInstanceDynamic* DynamicMaterial = TeamMaterials[i];
				MeshComp->SetMaterial(i, TeamMaterials[i]);
			}
		}

		// Força a atualização dos materiais na renderização
		MeshComp->MarkRenderStateDirty();

		// Log dos materiais após a alteração
		UE_LOG(LogTemplateCharacter, Log, TEXT("Materials after change:"));
		for (int32 i = 0; i < MeshComp->GetNumMaterials(); i++)
		{
			UMaterialInterface* UpdatedMaterial = MeshComp->GetMaterial(i);
			if (UpdatedMaterial)
			{
				UE_LOG(LogTemplateCharacter, Log, TEXT("Slot %d: %s"), i, *UpdatedMaterial->GetName());
			}
		}
	}
}


AActor* ACTFGameCharacter::GetWeapon() const
{
	return Weapon;
}

void ACTFGameCharacter::SetWeapon(AActor* NewWeapon)
{
	Weapon = NewWeapon;
}