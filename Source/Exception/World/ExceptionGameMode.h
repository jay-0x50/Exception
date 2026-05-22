// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ExceptionGameMode.generated.h"

class ABRBossArenaTrigger;

/**
 *  Simple GameMode for a third person game
 */
UCLASS(abstract)
class AExceptionGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	
	/** Constructor */
	AExceptionGameMode();

	UFUNCTION(BlueprintCallable, Category="Exception|Checkpoint")
	void SetCheckpointTransform(const FTransform& NewCheckpointTransform);

	UFUNCTION(BlueprintPure, Category="Exception|Checkpoint")
	FTransform GetCheckpointTransform() const;

	UFUNCTION(BlueprintPure, Category="Exception|Checkpoint")
	bool HasCheckpoint() const { return bHasCheckpoint; }

	UFUNCTION(BlueprintCallable, Category="Exception|Arena")
	void SetActiveBossArena(ABRBossArenaTrigger* NewActiveArena);

	UFUNCTION(BlueprintCallable, Category="Exception|Arena")
	void ResetActiveBossArenaForRetry();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Exception|Checkpoint")
	FTransform CheckpointTransform;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Exception|Checkpoint")
	bool bHasCheckpoint = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Exception|Arena")
	TObjectPtr<ABRBossArenaTrigger> ActiveBossArena;
};



