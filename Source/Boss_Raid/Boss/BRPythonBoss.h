#pragma once

#include "CoreMinimal.h"
#include "BRBossDummy.h"
#include "BRPythonBoss.generated.h"

UENUM(BlueprintType)
enum class EBRPythonDragonRole : uint8
{
	MeleeDragon,
	RangedDragon
};

UCLASS(Blueprintable, BlueprintType, meta=(DisplayName="Python Boss"))
class BOSS_RAID_API ABRPythonBoss : public ABRBossDummy
{
	GENERATED_BODY()

public:
	ABRPythonBoss();

	virtual void ApplyTeamSlot(int32 TeamSlotIndex) override;

	UFUNCTION(BlueprintCallable, Category="BossRaid|Python")
	void ConfigurePythonPatterns();

protected:
	virtual void OnBossReset() override;
	virtual void OnBossPhaseChanged(EBRBossPhase NewPhase) override;
	virtual FString GetBossDebugName() const override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BossRaid|Python")
	bool bUseTeamSlotRole = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BossRaid|Python")
	EBRPythonDragonRole PythonDragonRole = EBRPythonDragonRole::MeleeDragon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BossRaid|Python", meta=(ClampMin="0.0"))
	float MeleeDragonDamageMultiplier = 1.15f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BossRaid|Python", meta=(ClampMin="0.0"))
	float RangedDragonDamageMultiplier = 0.85f;
};
