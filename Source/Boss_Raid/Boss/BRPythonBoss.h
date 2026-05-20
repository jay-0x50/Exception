#pragma once

#include "CoreMinimal.h"
#include "BRBossDummy.h"
#include "BRPythonBoss.generated.h"

UENUM(BlueprintType)
enum class EBRPythonBossIdentity : uint8
{
	Vethara,
	Aurathos
};

UCLASS(Blueprintable, BlueprintType, meta=(DisplayName="Python Twin Boss"))
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
	EBRPythonBossIdentity PythonBossIdentity = EBRPythonBossIdentity::Vethara;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BossRaid|Python", meta=(ClampMin="0.0"))
	float VetharaDamageMultiplier = 0.85f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BossRaid|Python", meta=(ClampMin="0.0"))
	float AurathosDamageMultiplier = 1.15f;
};
