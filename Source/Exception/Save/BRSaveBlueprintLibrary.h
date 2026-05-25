#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "BRSaveBlueprintLibrary.generated.h"

UCLASS()
class EXCEPTION_API UBRSaveBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="Exception|Save", meta=(WorldContext="WorldContextObject"))
	static bool SaveExceptionGame(const UObject* WorldContextObject, const FString& SlotName = TEXT("SaveSlot_0"), int32 UserIndex = 0);

	UFUNCTION(BlueprintCallable, Category="Exception|Save", meta=(WorldContext="WorldContextObject"))
	static bool LoadExceptionGameAndOpenLevel(const UObject* WorldContextObject, const FString& SlotName = TEXT("SaveSlot_0"), int32 UserIndex = 0);

	UFUNCTION(BlueprintCallable, Category="Exception|Save", meta=(WorldContext="WorldContextObject"))
	static bool DeleteExceptionSave(const UObject* WorldContextObject, const FString& SlotName = TEXT("SaveSlot_0"), int32 UserIndex = 0);

	UFUNCTION(BlueprintPure, Category="Exception|Save", meta=(WorldContext="WorldContextObject"))
	static bool DoesExceptionSaveExist(const UObject* WorldContextObject, const FString& SlotName = TEXT("SaveSlot_0"), int32 UserIndex = 0);
};
