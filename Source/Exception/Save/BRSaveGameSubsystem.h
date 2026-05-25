#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "BRSaveGameSubsystem.generated.h"

class UBRSaveGame;

UCLASS(BlueprintType)
class EXCEPTION_API UBRSaveGameSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="Exception|Save")
	bool SaveCurrentGame(const FString& SlotName = TEXT("SaveSlot_0"), int32 UserIndex = 0);

	UFUNCTION(BlueprintCallable, Category="Exception|Save")
	bool LoadGameFromSlotAndOpenLevel(const FString& SlotName = TEXT("SaveSlot_0"), int32 UserIndex = 0);

	UFUNCTION(BlueprintCallable, Category="Exception|Save")
	bool ApplyPendingLoadedGame();

	UFUNCTION(BlueprintPure, Category="Exception|Save")
	bool DoesSaveExist(const FString& SlotName = TEXT("SaveSlot_0"), int32 UserIndex = 0) const;

	UFUNCTION(BlueprintCallable, Category="Exception|Save")
	bool DeleteSave(const FString& SlotName = TEXT("SaveSlot_0"), int32 UserIndex = 0);

	UFUNCTION(BlueprintPure, Category="Exception|Save")
	UBRSaveGame* GetPendingSaveGame() const { return PendingSaveGame; }

private:
	UPROPERTY(Transient)
	TObjectPtr<UBRSaveGame> PendingSaveGame;
};
