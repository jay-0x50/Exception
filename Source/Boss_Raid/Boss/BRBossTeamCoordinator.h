#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BRBossTeamCoordinator.generated.h"

class ABRBossBase;

UCLASS(Blueprintable, BlueprintType, meta=(DisplayName="Boss Team Coordinator"))
class BOSS_RAID_API ABRBossTeamCoordinator : public AActor
{
	GENERATED_BODY()

public:
	ABRBossTeamCoordinator();

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category="BossRaid|Team")
	void RegisterBoss(ABRBossBase* Boss);

	UFUNCTION(BlueprintCallable, Category="BossRaid|Team")
	void BindConfiguredTeamMembers();

	UFUNCTION(BlueprintCallable, Category="BossRaid|Team")
	void UnregisterBoss(ABRBossBase* Boss);

	UFUNCTION(BlueprintPure, Category="BossRaid|Team")
	bool CanStartAttack(ABRBossBase* RequestingBoss) const;

	UFUNCTION(BlueprintCallable, Category="BossRaid|Team")
	bool NotifyAttackStarted(ABRBossBase* AttackingBoss);

	UFUNCTION(BlueprintCallable, Category="BossRaid|Team")
	void NotifyAttackFinished(ABRBossBase* AttackingBoss);

	UFUNCTION(BlueprintPure, Category="BossRaid|Team")
	ABRBossBase* GetActiveAttacker() const { return ActiveAttacker; }

	UFUNCTION(BlueprintPure, Category="BossRaid|Team")
	bool IsOtherBossAttacking(ABRBossBase* RequestingBoss) const;

	UFUNCTION(BlueprintPure, Category="BossRaid|Team")
	bool IsOtherBossWithin(ABRBossBase* RequestingBoss, float Distance) const;

protected:
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="BossRaid|Team")
	TArray<TObjectPtr<ABRBossBase>> TeamMembers;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BossRaid|Team")
	bool bAllowSimultaneousAttacks = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BossRaid|Team", meta=(ClampMin="0.0", Units="s"))
	float TeamAttackGap = 0.45f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="BossRaid|Team")
	TObjectPtr<ABRBossBase> ActiveAttacker;

	float LastAttackFinishedTime = -1000.0f;
};
