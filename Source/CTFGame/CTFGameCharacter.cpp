#include "CTFGameCharacter.h"
#include "CTFGameProjectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "TeamManager.h"
#include "Engine/LocalPlayer.h"
#include "CTFPlayerState.h"  // Include the PlayerState header
#include "GameFramework/PlayerStart.h"


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
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-10.f, 0.f, 60.f));
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create first-person mesh component
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	Mesh1P->SetRelativeLocation(FVector(-30.f, 0.f, -150.f));

	// Create third-person mesh component
	Mesh3P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh3P"));
	Mesh3P->SetOwnerNoSee(true);
	Mesh3P->SetupAttachment(GetCapsuleComponent());
	Mesh3P->bCastDynamicShadow = true;
	Mesh3P->CastShadow = true;
	Mesh3P->SetRelativeLocation(FVector(0.f, 0.f, -90.f));
	//Make material dynamic
	

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
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ACTFGameCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ACTFGameCharacter::Look);

		// Change Team
		EnhancedInputComponent->BindAction(ChangeTeamAction, ETriggerEvent::Triggered, this, &ACTFGameCharacter::ChangeTeam);
	}
}

void ACTFGameCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Create dynamic materials for Red and Blue teams
	for (int32 i = 0; i < RedMaterials.Num(); i++)
	{
		RedDynamicMaterials.Add(UMaterialInstanceDynamic::Create(RedMaterials[i], this));
	}
	for (int32 i = 0; i < BlueMaterials.Num(); i++)
	{
		BlueDynamicMaterials.Add(UMaterialInstanceDynamic::Create(BlueMaterials[i], this));
	}

	//Register timer to call UpdateTeamMaterial every 3 seconds

	GetWorldTimerManager().SetTimer(RespawnTimerHandle, this, &ACTFGameCharacter::UpdateTeamMaterial, 3.0f, true);
}

void ACTFGameCharacter::Move(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		AddMovementInput(GetActorForwardVector(), MovementVector.Y);
		AddMovementInput(GetActorRightVector(), MovementVector.X);
	}
}

void ACTFGameCharacter::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void ACTFGameCharacter::ChangeTeam()
{
	//ACTFPlayerState* MyPlayerState = GetPlayerState<ACTFPlayerState>();
	//if (!MyPlayerState) return;

	//// Toggle team and update PlayerState
	////ETeamColor NewTeam = (MyPlayerState->GetTeam() == ETeamColor::Red) ? ETeamColor::Blue : ETeamColor::Red;
	////MyPlayerState->SetTeam(NewTeam);

	//// Apply new team material
	//UpdateTeamMaterial();	

	////UE_LOG(LogTemplateCharacter, Warning, TEXT("Changed team to %s"), (NewTeam == ETeamColor::Red) ? TEXT("Red") : TEXT("Blue"));
}


void ACTFGameCharacter::UpdateTeamMaterial()
{
	ACTFPlayerState* MyPlayerState = GetPlayerState<ACTFPlayerState>();
	if (!MyPlayerState) return;

	SetTeamMaterial(MyPlayerState->GetTeam());
}

void ACTFGameCharacter::SetTeamMaterial(ETeamColor Team)
{
	if (HasAuthority())
	{
		Multicast_SetTeamMaterial(Team);
	}
	else
	{
		Server_SetTeamMaterial(Team);
	}
}


void ACTFGameCharacter::Server_SetTeamMaterial_Implementation(ETeamColor Team)
{
	Multicast_SetTeamMaterial(Team);
}

bool ACTFGameCharacter::Server_SetTeamMaterial_Validate(ETeamColor Team)
{
	return true; // You can add validation logic here if needed
}

void ACTFGameCharacter::Multicast_SetTeamMaterial_Implementation(ETeamColor Team)
{
	if (USkeletalMeshComponent* MeshComp = GetMesh3P())
	{
		TArray<UMaterialInstanceDynamic*>* TeamMaterials = nullptr;

		switch (Team)
		{
		case ETeamColor::Red:
			TeamMaterials = &RedDynamicMaterials;
			break;
		case ETeamColor::Blue:
			TeamMaterials = &BlueDynamicMaterials;
			break;
		default:
			UE_LOG(LogTemplateCharacter, Error, TEXT("Invalid team color!"));
			return;
		}

		// Apply new materials
		if (TeamMaterials && TeamMaterials->Num() > 0)
		{
			for (int32 i = 0; i < TeamMaterials->Num(); i++)
			{
				if ((*TeamMaterials)[i])
				{
					MeshComp->SetMaterial(i, (*TeamMaterials)[i]);
				}
			}

			MeshComp->MarkRenderStateDirty();

		}
		else
		{
			UE_LOG(LogTemplateCharacter, Error, TEXT("No dynamic materials found for the selected team!"));
		}
	}
}



float ACTFGameCharacter::TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float DamageApplied = FMath::Min(Health, Damage);
	Health -= DamageApplied;

	UE_LOG(LogTemp, Warning, TEXT("Player %s took damage: %f"), *GetName(), DamageApplied);

	if (Health <= 0)
	{
		MulticastOnDeath_Implementation();
	}

	return DamageApplied;
}
void ACTFGameCharacter::MulticastOnDeath_Implementation()
{
	HandleDeath();
}
void ACTFGameCharacter::HandleDeath()
{
	UE_LOG(LogTemp, Warning, TEXT("%s has died!"), *GetName());

	PlayDeathAnimation();
	//DisableCharacter();
	//StartRespawnTimer();
}

void ACTFGameCharacter::PlayDeathAnimation()
{
	if (DeathAnimation)
	{
		PlayAnimMontage(DeathAnimation);
	}
}

void ACTFGameCharacter::DisableCharacter()
{
	// Desativar entrada do jogador
	AController* PlayerController = GetController();
	if (PlayerController)
	{
		PlayerController->DisableInput(nullptr);
	}

	// Desativar movimento
	GetCharacterMovement()->DisableMovement();

	// Remover colisão para evitar interação
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ACTFGameCharacter::StartRespawnTimer()
{
	GetWorldTimerManager().SetTimer(RespawnTimerHandle, this, &ACTFGameCharacter::Respawn, RespawnDelay, false);
}

void ACTFGameCharacter::Respawn()
{
	RestoreCharacter();
	RespawnAtSpawnPoint();

	UE_LOG(LogTemp, Warning, TEXT("%s has respawned!"), *GetName());
}

void ACTFGameCharacter::RestoreCharacter()
{
	Health = MaxHealth;

	// Restaurar entrada e movimento
	AController* PlayerController = GetController();
	if (PlayerController)
	{
		PlayerController->EnableInput(nullptr);
	}

	GetCharacterMovement()->SetMovementMode(MOVE_Walking);

	// Restaurar colisão
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
}

void ACTFGameCharacter::RespawnAtSpawnPoint()
{
	
	if (SpawnPoint)
	{
		SetActorLocation(SpawnPoint->GetActorLocation());
		SetActorRotation(SpawnPoint->GetActorRotation());
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
