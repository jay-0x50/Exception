#pragma once

#include "CoreMinimal.h"
#include "BRBossDummy.h"
#include "BRMySQLBoss.generated.h"

UCLASS(Blueprintable, BlueprintType, meta=(DisplayName="MySQL Boss"))
class BOSS_RAID_API ABRMySQLBoss : public ABRBossDummy
{
	GENERATED_BODY()

public:
	ABRMySQLBoss();

protected:
	virtual void OnBossReset() override;
	virtual FString GetBossDebugName() const override;
};
