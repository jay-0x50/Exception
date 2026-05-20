#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "BRCombatInterface.generated.h"

UINTERFACE(BlueprintType)
class BOSS_RAID_API UBRCombatInterface : public UInterface
{
	GENERATED_BODY()
};

class BOSS_RAID_API IBRCombatInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="BossRaid|Combat")
	bool ReceiveCombatHit(float Damage, float GroggyDamage, AActor* DamageCauser);
};
