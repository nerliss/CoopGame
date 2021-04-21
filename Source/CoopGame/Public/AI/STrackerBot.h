// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "STrackerBot.generated.h"

class USHealthComponent;
class USphereComponent;

UCLASS()
class COOPGAME_API ASTrackerBot : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ASTrackerBot();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleDefaultsOnly, Category = "Components")
	UStaticMeshComponent* MeshComp;

	UPROPERTY(VisibleDefaultsOnly, Category = "Components")
	USHealthComponent* HealthComp;

	UPROPERTY(VisibleDefaultsOnly, Category = "Components")
	USphereComponent* SphereComp;

	UFUNCTION()
	void HandleTakeDamage(USHealthComponent* OwningHealthComp, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	FVector GetNextPathPoint();

	// Next point in navigation path
	FVector NextPathPoint;

	UPROPERTY(EditDefaultsOnly, Category = "TracerBot")
	float MovementForce;

	UPROPERTY(EditDefaultsOnly, Category = "TracerBot")
	bool bUseVelocityChange;

	UPROPERTY(EditDefaultsOnly, Category = "TracerBot")
	float RequiredDistanceToTarget;

	// Dynamic material to pulse on damage taken
	UMaterialInstanceDynamic* MatInst;

	void SelfDestruct();

	UPROPERTY(EditDefaultsOnly, Category = "TracerBot")
	UParticleSystem* ExplosionFX;

	bool bExploded;

	bool bStartetSelfDestruction;

	UPROPERTY(EditDefaultsOnly, Category = "TracerBot")
	float ExplosionRadius;

	UPROPERTY(EditDefaultsOnly, Category = "TracerBot")
	float ExplosionDamage;

	FTimerHandle TimerHandle_SelfDamage;

	void DamageSelf();

public:	

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
};
