// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SExplosiveBarrel.generated.h"

class USHealthComponent;
class UStaticMeshComponent;
class URadialForceComponent;
class UParticleSystem;

UCLASS()
class COOPGAME_API ASExplosiveBarrel : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASExplosiveBarrel();

protected:
	
	UPROPERTY(VisibleAnywhere, Category = "Components")
	USHealthComponent* HealthComp;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* MeshComp;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	URadialForceComponent* RadialForceComp;

	UFUNCTION()
	void OnHealthChanged(USHealthComponent* OwningHealthComp, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	UPROPERTY(ReplicatedUsing=OnRep_Exploded)
	bool bExploded;

	UFUNCTION()
	void OnRep_Exploded();

	/* Declare the damage type */
	UPROPERTY(EditDefaultsOnly, Category = "RadialDamage")
	TSubclassOf<class UDamageType> RadialDamageType;

	/* Explosion damage */
	UPROPERTY(EditDefaultsOnly, Category = "RadialDamage")
	float ExpDamage;

	/* Explosion radius */
	UPROPERTY(EditDefaultsOnly, Category = "RadialDamage")
	float ExpRadius;

	/* The impulse applied to barrel mesh that boosts it up a little once exploded */
	UPROPERTY(EditDefaultsOnly, Category = "FX")
	float ExplosionImpulse;

	/* The particle to play at zero HP */
	UPROPERTY(EditDefaultsOnly, Category = "FX")
	UParticleSystem* ExplosionFX;

	/* The material to replace the original one after the explosion */
	UPROPERTY(EditDefaultsOnly, Category = "FX")
	UMaterialInterface* ExplodedMaterial;

};
