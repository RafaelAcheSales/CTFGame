// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "TeamColors.h"
#include "CTFGameCharacter.generated.h"

class UInputComponent;
class USkeletalMeshComponent;
class UCameraComponent;
class UInputAction;
class UInputMappingContext;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(config=Game)
class ACTFGameCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Pawn mesh: 1st person view (arms; seen only by self) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Mesh, meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* Mesh1P;

	/** Other Player mesh: 3rd person view (body; seen by everyone) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Mesh, meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* Mesh3P;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCameraComponent;

	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* LookAction;

	/** Change Team Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* ChangeTeamAction;
	
public:
	ACTFGameCharacter();

protected:
	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	void ChangeTeam();

	AActor* Weapon;

protected:
	/** Server RPC to handle material setting */
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SetTeamMaterial(ETeamColor Team);
	void Server_SetTeamMaterial_Implementation(ETeamColor Team);
	bool Server_SetTeamMaterial_Validate(ETeamColor Team);

	/** Multicast RPC to update the material on all clients */
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SetTeamMaterial(ETeamColor Team);
	void Multicast_SetTeamMaterial_Implementation(ETeamColor Team);
	// APawn interface
	virtual void NotifyControllerChanged() override;
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	// End of APawn interface

	virtual void BeginPlay() override;

public:

	// CurrentHealth
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
	float Health;

	// MaxHealth
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
	float MaxHealth;

	// override TakeDamage
	virtual float TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	//respawn
	void Respawn();

	void UpdateTeamMaterial();


	/** Returns Mesh1P subobject **/
	USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }

	/** Returns Mesh3P subobject **/
	USkeletalMeshComponent* GetMesh3P() const { return Mesh3P; }

	/** Function to set material based on TeamColors **/
	UFUNCTION(BlueprintCallable, Category = "Team")
	void SetTeamMaterial(ETeamColor Team);

	/** Array of Materials (1 and 2 for Red team) **/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Team")
	TArray<UMaterialInstance*> RedMaterials;

	/** dynamic material instance for team color **/
	UPROPERTY()
	TArray<UMaterialInstanceDynamic*> RedDynamicMaterials;

	/** Array of Materials (1 and 2 for Blue team) **/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Team")
	TArray<UMaterialInstance*> BlueMaterials;

	/** dynamic material instance for team color **/
	UPROPERTY()
	TArray<UMaterialInstanceDynamic*> BlueDynamicMaterials;




	/** Returns FirstPersonCameraComponent subobject **/
	UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

	/** Returns Weapon Pointer if its in socket **/
	AActor* GetWeapon() const;

	/** Sets Weapon Pointer **/
	void SetWeapon(AActor* Weapon);

};

