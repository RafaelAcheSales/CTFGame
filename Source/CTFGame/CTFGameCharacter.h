// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "GameFramework/PlayerStart.h"
#include "TeamColors.h"
#include "Flag.h"
#include "CTFGameCharacter.generated.h"

class UInputComponent;
class USkeletalMeshComponent;
class UCameraComponent;
class UInputAction;
class UInputMappingContext;
class APlayerStart;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(config = Game)
class ACTFGameCharacter : public ACharacter
{
	GENERATED_BODY()

private:
	/** Pawn mesh: 1st person view (arms; seen only by self) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Mesh, meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* Mesh1P;

	/** Other Player mesh: 3rd person view (body; seen by everyone) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Mesh, meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* Mesh3P;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCameraComponent;

	/** Input Mapping Context */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	/** Input Actions */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* ChangeTeamAction;

	/** Weapon Pointer */
	AActor* Weapon;

protected:
	virtual void BeginPlay() override;

	//Has flag boolean
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
	bool HasFlag;

	AFlag* HeldFlag;  // Reference to the flag the player is holding
	/** Movement and Look Functions */
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);

	/** RPCs for Team Material Handling */
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SetTeamMaterial(ETeamColor Team);
	void Server_SetTeamMaterial_Implementation(ETeamColor Team);
	bool Server_SetTeamMaterial_Validate(ETeamColor Team);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SetTeamMaterial(ETeamColor Team);
	void Multicast_SetTeamMaterial_Implementation(ETeamColor Team);


	/** APawn Interface Overrides */
	virtual void NotifyControllerChanged() override;
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;

public:

	ACTFGameCharacter();

	void SetHeldFlag(AFlag* NewFlag);
	void DropHeldFlag();
	/** Health Properties */
	UPROPERTY(ReplicatedUsing = OnRep_Health)
	float Health;
	UFUNCTION()
	void OnRep_Health();

	UPROPERTY(Replicated)
	float MaxHealth;

	//RespawnTimerHandle
	FTimerHandle RespawnTimerHandle;

	// RespawnDelay
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
	float RespawnDelay;

	// SpawnPoint
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
	APlayerStart* SpawnPoint;

	/** Damage Handling */
	virtual float TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	void UpdateTeamMaterial();

	/** Mesh and Camera Getters */
	USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }
	USkeletalMeshComponent* GetMesh3P() const { return Mesh3P; }
	UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

	/** Weapon Handling */
	AActor* GetWeapon() const;
	void SetWeapon(AActor* NewWeapon);

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void RespawnAtTeamStart();

	/** Team Material Handling */
	UFUNCTION(BlueprintCallable, Category = "Team")
	void SetTeamMaterial(ETeamColor Team);

	/** Team Materials Arrays */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Team")
	TArray<UMaterialInstance*> RedMaterials;

	UPROPERTY()
	TArray<UMaterialInstanceDynamic*> RedDynamicMaterials;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Team")
	TArray<UMaterialInstance*> BlueMaterials;

	UPROPERTY()
	TArray<UMaterialInstanceDynamic*> BlueDynamicMaterials;

	///////////////////////////////////////////
	// Animations
	///////////////////////////////////////////

	/** Animation Montages */

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	UAnimMontage* DeathAnimation;


	///////////////////////////////////////////
	// Flag
	///////////////////////////////////////////

	UFUNCTION(BlueprintCallable)
	bool GetHasFlag();

	UFUNCTION(BlueprintCallable)
	void SetHasFlag(bool FlagStatus);
};
