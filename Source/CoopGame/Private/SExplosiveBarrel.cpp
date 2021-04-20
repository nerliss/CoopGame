// Fill out your copyright notice in the Description page of Project Settings.


#include "SExplosiveBarrel.h"
#include "Components/SHealthComponent.h"
#include "Kismet/GameplayStatics.h"
#include "PhysicsEngine/RadialForceComponent.h"
#include "Net/UnrealNetwork.h"

// Sets default values
ASExplosiveBarrel::ASExplosiveBarrel()
{
    HealthComp = CreateDefaultSubobject<USHealthComponent>(TEXT("HealthComp"));
    HealthComp->OnHealthChanged.AddDynamic(this, &ASExplosiveBarrel::OnHealthChanged);

    MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
    MeshComp->SetSimulatePhysics(true);
    MeshComp->SetCollisionObjectType(ECC_PhysicsBody); // radial component now affects barrel 
    RootComponent = MeshComp;

    RadialForceComp = CreateDefaultSubobject<URadialForceComponent>(TEXT("RadialForceComp"));
    RadialForceComp->SetupAttachment(MeshComp);
    RadialForceComp->Radius = 250.f;
    RadialForceComp->bImpulseVelChange = true;
    RadialForceComp->bAutoActivate = false; // prevent component from ticking, activate only on FireImpulse()
    RadialForceComp->bIgnoreOwningActor = true;

    ExplosionImpulse = 400.f;
    ExpDamage = 120.f;
    ExpRadius = 500.f;

    SetReplicates(true);
    SetReplicateMovement(true);
}

void ASExplosiveBarrel::OnHealthChanged(USHealthComponent* OwningHealthComp, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
    if (bExploded)
    {
        return;
    }

    // Explosion
    if (Health <= 0.f)
    {
        bExploded = true;
        OnRep_Exploded();

        // Boost the barrel up
        FVector BoostIntensity = FVector::UpVector * ExplosionImpulse;
        MeshComp->AddImpulse(BoostIntensity, NAME_None, true);

        // Blast away nearby physics actors
        RadialForceComp->FireImpulse();

        // Apply radial damage
		TArray<AActor*> IgnoredActors;
		IgnoredActors.Add(this);
		UGameplayStatics::ApplyRadialDamage(GetWorld(), ExpDamage, GetActorLocation(), ExpRadius, RadialDamageType, IgnoredActors, this, GetInstigatorController(), true, ECC_Visibility);
    }
}

// Replicate visuals
void ASExplosiveBarrel::OnRep_Exploded()
{
	// Play FX and change material
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionFX, GetActorLocation());
	MeshComp->SetMaterial(0, ExplodedMaterial);
}

// Replication rule for CurrentWeapon variable
void ASExplosiveBarrel::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ASExplosiveBarrel, bExploded);
}