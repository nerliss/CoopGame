// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SGameMode.generated.h"

enum class EWaveState : uint8;

UCLASS()
class COOPGAME_API ASGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	ASGameMode();

protected:

	FTimerHandle TimerHandle_BotSpawner;

	FTimerHandle TimerHande_NextWaveStart;

	// Bots to spawn in current wave
	int32 NumOfBotsToSpawn;

	int32 WaveCount;

	UPROPERTY(EditDefaultsOnly, Category = "Gamemode")
	float TimeBetweenWaves;

	// Hook for BP to spawn a single bot
	UFUNCTION(BlueprintImplementableEvent, Category = "Gamemode")
	void SpawnNewBot();

	void SpawnBotTimerElapsed();

	// Start spawning bots
	void StartWave();

	// Stop spawning bots
	void EndWave();

	// Set timer for next wave
	void PrepareForNextWave();

	void CheckWaveState();

	void CheckAnyPlayerAlive();

	void GameOver();

	void SetWaveState(EWaveState NewState);

public:

	virtual void StartPlay() override;

	virtual void Tick(float DeltaSeconds) override;
};
