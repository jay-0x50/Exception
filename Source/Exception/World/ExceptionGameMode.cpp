// Copyright Epic Games, Inc. All Rights Reserved.

#include "ExceptionGameMode.h"

#include "BRBossArenaTrigger.h"
#include "BRSaveGameSubsystem.h"
#include "TimerManager.h"

AExceptionGameMode::AExceptionGameMode()
{
	// stub
}

void AExceptionGameMode::BeginPlay()
{
	Super::BeginPlay();

	GetWorldTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this, [this]()
	{
		if (UGameInstance* GameInstance = GetGameInstance())
		{
			if (UBRSaveGameSubsystem* SaveSubsystem = GameInstance->GetSubsystem<UBRSaveGameSubsystem>())
			{
				SaveSubsystem->ApplyPendingLoadedGame();
			}
		}
	}));
}

void AExceptionGameMode::SetCheckpointTransform(const FTransform& NewCheckpointTransform)
{
	CheckpointTransform = NewCheckpointTransform;
	bHasCheckpoint = true;

	UE_LOG(LogTemp, Log, TEXT("Checkpoint saved: %s"), *CheckpointTransform.GetLocation().ToString());
}

FTransform AExceptionGameMode::GetCheckpointTransform() const
{
	return CheckpointTransform;
}

void AExceptionGameMode::SetActiveBossArena(ABRBossArenaTrigger* NewActiveArena)
{
	ActiveBossArena = NewActiveArena;
}

void AExceptionGameMode::ResetActiveBossArenaForRetry()
{
	if (ActiveBossArena)
	{
		ActiveBossArena->ResetArenaForRetry();
	}
}
