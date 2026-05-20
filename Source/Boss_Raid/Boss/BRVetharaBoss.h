#pragma once

#include "CoreMinimal.h"
#include "BRPythonBoss.h"
#include "BRVetharaBoss.generated.h"

UCLASS(Blueprintable, BlueprintType, meta=(DisplayName="Vethara, Unhandled Exception"))
class BOSS_RAID_API ABRVetharaBoss : public ABRPythonBoss
{
	GENERATED_BODY()

public:
	ABRVetharaBoss();
};
