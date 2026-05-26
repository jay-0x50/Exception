#pragma once

#include "CoreMinimal.h"
#include "BRPatternBossBase.h"
#include "BRMySQLBoss.generated.h"

UCLASS(Blueprintable, BlueprintType, meta=(DisplayName="MySQL Boss"))
class EXCEPTION_API ABRMySQLBoss : public ABRPatternBossBase
{
	GENERATED_BODY()

public:
	ABRMySQLBoss();

protected:
	virtual void OnBossReset() override;
	virtual FString GetBossDebugName() const override;
};
