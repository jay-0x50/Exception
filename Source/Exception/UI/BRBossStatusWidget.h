#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BRBossStatusWidget.generated.h"

UCLASS(Blueprintable, BlueprintType)
class EXCEPTION_API UBRBossStatusWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="Exception|Boss UI")
	void ClearBosses();

	UFUNCTION(BlueprintCallable, Category="Exception|Boss UI")
	void SetBossCount(int32 BossCount);

	UFUNCTION(BlueprintCallable, Category="Exception|Boss UI")
	void SetBossHP(int32 BossIndex, FText BossName, float CurrentHP, float MaxHP, float NormalizedHP);

	UFUNCTION(BlueprintCallable, Category="Exception|Boss UI")
	void SetBossGroggy(int32 BossIndex, float CurrentGroggy, float MaxGroggy, float NormalizedGroggy);

	UFUNCTION(BlueprintCallable, Category="Exception|Boss UI")
	void SetBossGroggyState(int32 BossIndex, bool bIsGroggy);

protected:
	UFUNCTION(BlueprintImplementableEvent, Category="Exception|Boss UI", meta=(DisplayName="Clear Bosses"))
	void BP_ClearBosses();

	UFUNCTION(BlueprintImplementableEvent, Category="Exception|Boss UI", meta=(DisplayName="Set Boss Count"))
	void BP_SetBossCount(int32 BossCount);

	UFUNCTION(BlueprintImplementableEvent, Category="Exception|Boss UI", meta=(DisplayName="Set Boss HP"))
	void BP_SetBossHP(int32 BossIndex, const FText& BossName, float CurrentHP, float MaxHP, float NormalizedHP);

	UFUNCTION(BlueprintImplementableEvent, Category="Exception|Boss UI", meta=(DisplayName="Set Boss Groggy"))
	void BP_SetBossGroggy(int32 BossIndex, float CurrentGroggy, float MaxGroggy, float NormalizedGroggy);

	UFUNCTION(BlueprintImplementableEvent, Category="Exception|Boss UI", meta=(DisplayName="Set Boss Groggy State"))
	void BP_SetBossGroggyState(int32 BossIndex, bool bIsGroggy);
};
