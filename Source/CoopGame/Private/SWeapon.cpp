// Fill out your copyright notice in the Description page of Project Settings.


#include "SWeapon.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Components/SkeletalMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "CoopGame/CoopGame.h"
#include "TimerManager.h"
#include "Net/UnrealNetwork.h"

// Draw debug linetrace for weapon
static int32 DebugWeaponDrawing = 0;
FAutoConsoleVariableRef CVARDebugWeaponDrawing (
	TEXT("COOP.DebugWeapons"), 
	DebugWeaponDrawing, 
	TEXT("Draw debug lines for weapons"), 
	ECVF_Cheat);


// Sets default values
ASWeapon::ASWeapon()
{
	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;

	MuzzleSocketName = "MuzzleSocket";
	TracerTargetName = "Target";

	BaseDamage = 20.f;

	FireRate = 600;

	// Set the replication of the rifle to true to properly synchronize between server and client
	SetReplicates(true);

	// Actor network tick
	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;
}

void ASWeapon::BeginPlay()
{
	Super::BeginPlay();

	TimeBetweenShots = 60 / FireRate; // convert FireRate to timed version
}

void ASWeapon::Fire()
{
	// Line trace the world from camera position to crosshair location

	if (GetLocalRole() < ROLE_Authority)
	{
		ServerFire();
	}

	AActor* MyOwner = GetOwner();
	if (MyOwner)
	{
		FVector EyeLocation;
		FRotator EyeRotation;
		MyOwner->GetActorEyesViewPoint(EyeLocation, EyeRotation);

		FVector ShotDirection = EyeRotation.Vector();

		FVector TraceEnd = EyeLocation + (ShotDirection * 11000);

		FCollisionQueryParams CQP;
		CQP.AddIgnoredActor(MyOwner); // ignore character
		CQP.AddIgnoredActor(this); // ignore weapon
		CQP.bTraceComplex = true; // gets exact result of hit result, using complex collision
		CQP.bReturnPhysicalMaterial = true; // returns physical material

		// Particle "Target" parameter
		FVector TracerEndPoint = TraceEnd;

		EPhysicalSurface SurfaceType = SurfaceType_Default;

		FHitResult Hit;
		if (GetWorld()->LineTraceSingleByChannel(Hit, EyeLocation, TraceEnd, COLLISION_WEAPON, CQP))
		{
			// Blocking hit process damage
			AActor* HitActor = Hit.GetActor();

			SurfaceType = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());

			// Headshot bonus damage
			float ActualDamage = BaseDamage;
			if (SurfaceType == SURFACE_FLESHVULNERABLE)
			{
				ActualDamage *= 4.f;
			}

			// Apply damage
			UGameplayStatics::ApplyPointDamage(HitActor, ActualDamage, ShotDirection, Hit, MyOwner->GetInstigatorController(), this, DamageType); 

			PlayImpactFX(SurfaceType, Hit.ImpactPoint);

			TracerEndPoint = Hit.ImpactPoint;

		}

		// CVARDebugWeaponDrawing
		if (DebugWeaponDrawing > 0)
		{
			DrawDebugLine(GetWorld(), EyeLocation, TraceEnd, FColor::White, false, 1.f, 0, 1.f);
		}

		PlayFireFX(TracerEndPoint);

		// Update replication of cosmetic FX for clients on server
		if (GetLocalRole() == ROLE_Authority)
		{
			HitScanTrace.TraceTo = TracerEndPoint;
			HitScanTrace.SurfaceType = SurfaceType;
			HitScanTrace.Seed = (HitScanTrace.Seed + 1) % 3;
		}

		LastFireTime = GetWorld()->TimeSeconds;
	}
}

void ASWeapon::ServerFire_Implementation()
{
	Fire();
}

bool ASWeapon::ServerFire_Validate()
{
	return true;
}

void ASWeapon::OnRep_HitScanTrace()
{
	// Play cosmetic FX
	PlayFireFX(HitScanTrace.TraceTo);

	PlayImpactFX(HitScanTrace.SurfaceType, HitScanTrace.TraceTo);
}

void ASWeapon::StartFire()
{
	float FirstDelay = FMath::Max(LastFireTime + TimeBetweenShots - GetWorld()->TimeSeconds, 0.0f); // use FMath::Max not to allow FirstDelay be less than 0

	GetWorldTimerManager().SetTimer(TimerHandle_TimeBetweenShots, this, &ASWeapon::Fire, TimeBetweenShots, true, FirstDelay);
	
	Fire();
}

void ASWeapon::StopFire()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_TimeBetweenShots);
}

void ASWeapon::PlayFireFX(FVector TraceEnd)
{
	if (MuzzleFX)
	{
		UGameplayStatics::SpawnEmitterAttached(MuzzleFX, MeshComp, MuzzleSocketName); // spawn muzzle emitter once shot is fired
	}

	// Spawn trace emitter once shot is fired
	if (TracerFX)
	{
		FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);

		UParticleSystemComponent* TracerComp = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), TracerFX, MuzzleLocation);
		if (TracerComp)
		{
			TracerComp->SetVectorParameter(TracerTargetName, TraceEnd);
		}
	}

	// Play camera shake effect
	APawn* MyOwner = Cast<APawn>(GetOwner());
	if (MyOwner)
	{
		APlayerController* PlayerController = Cast<APlayerController>(MyOwner->GetController());
		if (PlayerController)
		{
			PlayerController->ClientPlayCameraShake(FireCamShake);
		}
	}
}

void ASWeapon::PlayImpactFX(EPhysicalSurface SurfaceType, FVector ImpactPoint)
{
	// Surface type switch statement - applies different FX based on Hit surface type
	UParticleSystem* SelectedEffect = nullptr;
	switch (SurfaceType)
	{
	case SURFACE_FLESHDEFAULT:
	case SURFACE_FLESHVULNERABLE:
		SelectedEffect = FleshImpactFX;
		break;
	default:
		SelectedEffect = DefaultImpactFX;
		break;
	}

	if (SelectedEffect)
	{
		FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);

		FVector ShotDirection = ImpactPoint - MuzzleLocation;
		ShotDirection.Normalize();

		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SelectedEffect, ImpactPoint, ShotDirection.Rotation()); // spawn impact emitter once hit is done
	}
}

// Replication rule for HitScanTrace structure
void ASWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ASWeapon, HitScanTrace, COND_SkipOwner); // replicate to everyone but owner
}
