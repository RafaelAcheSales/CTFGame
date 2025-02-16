#pragma once

#include "CoreMinimal.h"
#include "Components/SkeletalMeshComponent.h"
#include "CTFGameWeaponComponent.generated.h"

class ACTFGameCharacter;
class ACTFGameProjectile;
class UInputAction;
class UInputMappingContext;
class USoundBase;
class UAnimMontage;

/**
 * Weapon component that handles firing projectiles.
 * Uses RPCs so that the server spawns the projectile,
 * and all clients get the firing effects.
 */
UCLASS(Blueprintable, BlueprintType, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class CTFGAME_API UCTFGameWeaponComponent : public USkeletalMeshComponent
{
	GENERATED_BODY()

public:

	/** Projectile class to spawn */
	UPROPERTY(EditDefaultsOnly, Category = Projectile)
	TSubclassOf<ACTFGameProjectile> ProjectileClass;

	/** Sound to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	USoundBase* FireSound;

	/** AnimMontage to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	UAnimMontage* FireAnimation;

	/** Gun muzzle's offset from the character's location (in camera space) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	FVector MuzzleOffset;

	/** MappingContext for input */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* FireMappingContext;

	/** Fire input action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* FireAction;

	/** Constructor */
	UCTFGameWeaponComponent();

	/**
	 * Attach the weapon to a CTFGameCharacter.
	 * @param TargetCharacter - The owning character
	 * @return true if successfully attached, false otherwise
	 */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	bool AttachWeapon(ACTFGameCharacter* TargetCharacter);

	/** Called on the local client when the fire input is triggered */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void Fire();

protected:

	/** Ends gameplay for this component */
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/**
	 * Server RPC for spawning the projectile.
	 * Marking Reliable for simplicity.
	 * WithValidation allows a Validate() function for security checks.
	 */
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerFire();

	/**
	 * Multicast RPC to play firing effects (sound, animation) on all clients.
	 */
	UFUNCTION(NetMulticast, Reliable)
	void MulticastFireEffects();

private:

	/** The character holding this weapon */
	ACTFGameCharacter* Character;
};