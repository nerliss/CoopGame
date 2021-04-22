// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/SHealthComponent.h"
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
USHealthComponent::USHealthComponent()
{
	DefaultHealth = 100.f;

	SetIsReplicatedByDefault(true);
}


// Called when the game starts
void USHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	// Only use if we are server
	if (GetOwnerRole() == ROLE_Authority)
	{
		AActor* MyOwner = GetOwner();
		if (MyOwner)
		{
			MyOwner->OnTakeAnyDamage.AddDynamic(this, &USHealthComponent::HandleTakeAnyDamage);
		}
	}

	Health = DefaultHealth;
}

void USHealthComponent::OnRep_Health(float OldHealth)
{
	float Damage = Health - OldHealth;

	OnHealthChanged.Broadcast(this, Health, Damage, nullptr, nullptr, nullptr);
}

void USHealthComponent::HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	if (Damage <= 0.f)
	{
		return;
	}

	Health = FMath::Clamp(Health - Damage, 0.f, DefaultHealth);

	UE_LOG(LogTemp, Log, TEXT("Health changed: %s"), *FString::SanitizeFloat(Health)); // log

	// Blueprint assignable event that changes health
	OnHealthChanged.Broadcast(this, Health, Damage, DamageType, InstigatedBy, DamageCauser);

	ClientOnHealthChanged(this, Health, Damage, DamageType, InstigatedBy, DamageCauser);
}

void USHealthComponent::ClientOnHealthChanged_Implementation(USHealthComponent* HealthComponent, float SHealth, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	OnHealthChanged.Broadcast(this, SHealth, HealthDelta, DamageType, InstigatedBy, DamageCauser);
}

// Replication rule for Health variable
void USHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(USHealthComponent, Health);
}
