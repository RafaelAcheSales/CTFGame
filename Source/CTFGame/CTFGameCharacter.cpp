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
#include "EngineUtils.h"
#include "Engine/LocalPlayer.h"
#include "CTFPlayerState.h"
#include "GameFramework/PlayerStart.h"
#include "Net/UnrealNetwork.h"


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

	}
}

void ACTFGameCharacter::BeginPlay()
{
	Super::BeginPlay();

	// create dynamic materials for Red and Blue teams
	for (int32 i = 0; i < RedMaterials.Num(); i++)
	{
		RedDynamicMaterials.Add(UMaterialInstanceDynamic::Create(RedMaterials[i], this));
	};
	for (int32 i = 0; i < BlueMaterials.Num(); i++)
	{
		BlueDynamicMaterials.Add(UMaterialInstanceDynamic::Create(BlueMaterials[i], this));
	};

	// register timer to call UpdateTeamMaterial every 3 seconds

	GetWorldTimerManager().SetTimer(RespawnTimerHandle, this, &ACTFGameCharacter::UpdateTeamMaterial, 3.0f, false);
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


void ACTFGameCharacter::UpdateTeamMaterial()
{
	ACTFPlayerState* MyPlayerState = GetPlayerState<ACTFPlayerState>();
	if (!MyPlayerState) return;



	SetTeamMaterial(MyPlayerState->GetTeam());
}


void ACTFGameCharacter::RespawnAtTeamStart()
{
	if (!HasAuthority())
	{
		return;
	}

	ACTFPlayerState* MyPlayerState = GetPlayerState<ACTFPlayerState>();
	if (!MyPlayerState)
	{
		UE_LOG(LogTemp, Warning, TEXT("No valid PlayerState found on this character."));
		return;
	}

	// find the TeamManager in the world
	ATeamManager* TeamManager = Cast<ATeamManager>(
		UGameplayStatics::GetActorOfClass(GetWorld(), ATeamManager::StaticClass())
	);
	if (!TeamManager)
	{
		UE_LOG(LogTemp, Warning, TEXT("No ATeamManager found in the world."));
		return;
	}

	// Get the spawn point for this player’s team
	APlayerStart* MySpawnPoint = TeamManager->GetSpawnPoint(MyPlayerState->GetTeam());
	if (!MySpawnPoint)
	{
		UE_LOG(LogTemp, Warning, TEXT("No spawn point returned for team %d."), MyPlayerState->GetTeam());
		return;
	}

	// teleport the character (location + rotation). 
	// TeleportTo handles potential collision better than SetActorLocation for many Pawns.
	TeleportTo(
		MySpawnPoint->GetActorLocation(),
		MySpawnPoint->GetActorRotation()
	);

	//log rotation

	UE_LOG(LogTemp, Warning, TEXT("Rotation: %s"), *MySpawnPoint->GetActorRotation().ToString());

	// Now force the Controller's rotation to match
	if (AController* MyController = GetController())
	{
		MyController->SetControlRotation(MySpawnPoint->GetActorRotation());
	}
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

bool ACTFGameCharacter::GetHasFlag()
{
	return HasFlag;
}

void ACTFGameCharacter::SetHasFlag(bool FlagStatus) 
{
	HasFlag = FlagStatus;
}


void ACTFGameCharacter::Server_SetTeamMaterial_Implementation(ETeamColor Team)
{
	Multicast_SetTeamMaterial(Team);
}

bool ACTFGameCharacter::Server_SetTeamMaterial_Validate(ETeamColor Team)
{
	return true; // you can add validation logic here if needed
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

		// apply new materials
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


void ACTFGameCharacter::OnRep_Health()
{
	UE_LOG(LogTemp, Warning, TEXT("Health Updated on Client: %f"), Health);

}

float ACTFGameCharacter::TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float DamageApplied = FMath::Min(Health, Damage);
	Health -= DamageApplied;

	UE_LOG(LogTemp, Warning, TEXT("Player %s took damage: %f"), *GetName(), DamageApplied);

	if (Health <= 0)
	{
		if (HasAuthority())
		{
			// drop the flag if the player is holding one
			DropHeldFlag();

			// find the Team Manager for respawn logic
			ATeamManager* TeamManager = nullptr;
			for (TActorIterator<ATeamManager> It(GetWorld()); It; ++It)
			{
				TeamManager = *It;
				break;
			}

			if (TeamManager)
			{
				ACTFPlayerState* PS = GetPlayerState<ACTFPlayerState>();
				if (PS)
				{
					APlayerStart* MySpawnPoint = TeamManager->GetSpawnPoint(PS->GetTeam());
					if (MySpawnPoint)
					{
						SetActorLocation(MySpawnPoint->GetActorLocation());
					}
				}
			}

			// reset health after respawn
			Health = MaxHealth;
		}
	}

	return DamageApplied;
}


void ACTFGameCharacter::SetHeldFlag(AFlag* NewFlag)
{
	HeldFlag = NewFlag;
}

void ACTFGameCharacter::DropHeldFlag()
{
	if (HeldFlag)
	{
		HeldFlag->Drop();
		HeldFlag = nullptr; // clear flag reference after dropping
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
void ACTFGameCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ACTFGameCharacter, Health);
	DOREPLIFETIME(ACTFGameCharacter, MaxHealth);
}