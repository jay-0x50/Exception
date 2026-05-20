#pragma once

#include "CoreMinimal.h"
#include "BRPythonBoss.h"
#include "BRAurathosBoss.generated.h"

UCLASS(Blueprintable, BlueprintType, meta=(DisplayName="Aurathos, Fatal Process"))
class BOSS_RAID_API ABRAurathosBoss : public ABRPythonBoss
{
	GENERATED_BODY()

public:
	ABRAurathosBoss();
};
