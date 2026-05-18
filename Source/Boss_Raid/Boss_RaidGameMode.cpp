// Copyright Epic Games, Inc. All Rights Reserved.

#include "Boss_RaidGameMode.h"

ABoss_RaidGameMode::ABoss_RaidGameMode()
{
	// stub
}

void ABoss_RaidGameMode::SetCheckpointTransform(const FTransform& NewCheckpointTransform)
{
	CheckpointTransform = NewCheckpointTransform;
	bHasCheckpoint = true;

	UE_LOG(LogTemp, Log, TEXT("Checkpoint saved: %s"), *CheckpointTransform.GetLocation().ToString());
}

FTransform ABoss_RaidGameMode::GetCheckpointTransform() const
{
	return CheckpointTransform;
}
