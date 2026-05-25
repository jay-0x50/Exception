#include "BRSaveBlueprintLibrary.h"

#include "BRSaveGameSubsystem.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"

namespace
{
	UBRSaveGameSubsystem* GetSaveSubsystem(const UObject* WorldContextObject)
	{
		if (!WorldContextObject)
		{
			return nullptr;
		}

		if (UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(WorldContextObject))
		{
			return GameInstance->GetSubsystem<UBRSaveGameSubsystem>();
		}

		return nullptr;
	}
}

bool UBRSaveBlueprintLibrary::SaveExceptionGame(const UObject* WorldContextObject, const FString& SlotName, int32 UserIndex)
{
	if (UBRSaveGameSubsystem* SaveSubsystem = GetSaveSubsystem(WorldContextObject))
	{
		return SaveSubsystem->SaveCurrentGame(SlotName, UserIndex);
	}

	return false;
}

bool UBRSaveBlueprintLibrary::LoadExceptionGameAndOpenLevel(const UObject* WorldContextObject, const FString& SlotName, int32 UserIndex)
{
	if (UBRSaveGameSubsystem* SaveSubsystem = GetSaveSubsystem(WorldContextObject))
	{
		return SaveSubsystem->LoadGameFromSlotAndOpenLevel(SlotName, UserIndex);
	}

	return false;
}

bool UBRSaveBlueprintLibrary::DeleteExceptionSave(const UObject* WorldContextObject, const FString& SlotName, int32 UserIndex)
{
	if (UBRSaveGameSubsystem* SaveSubsystem = GetSaveSubsystem(WorldContextObject))
	{
		return SaveSubsystem->DeleteSave(SlotName, UserIndex);
	}

	return false;
}

bool UBRSaveBlueprintLibrary::DoesExceptionSaveExist(const UObject* WorldContextObject, const FString& SlotName, int32 UserIndex)
{
	if (UBRSaveGameSubsystem* SaveSubsystem = GetSaveSubsystem(WorldContextObject))
	{
		return SaveSubsystem->DoesSaveExist(SlotName, UserIndex);
	}

	return false;
}
