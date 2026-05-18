// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Boss_RaidGameMode.generated.h"

/**
 *  Simple GameMode for a third person game
 */
UCLASS(abstract)
class ABoss_RaidGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	
	/** Constructor */
	ABoss_RaidGameMode();

	UFUNCTION(BlueprintCallable, Category="BossRaid|Checkpoint")
	void SetCheckpointTransform(const FTransform& NewCheckpointTransform);

	UFUNCTION(BlueprintPure, Category="BossRaid|Checkpoint")
	FTransform GetCheckpointTransform() const;

	UFUNCTION(BlueprintPure, Category="BossRaid|Checkpoint")
	bool HasCheckpoint() const { return bHasCheckpoint; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="BossRaid|Checkpoint")
	FTransform CheckpointTransform;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="BossRaid|Checkpoint")
	bool bHasCheckpoint = false;
};



