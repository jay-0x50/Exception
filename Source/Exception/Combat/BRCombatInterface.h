#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "BRCombatInterface.generated.h"

UINTERFACE(BlueprintType)
class EXCEPTION_API UBRCombatInterface : public UInterface
{
	GENERATED_BODY()
};

class EXCEPTION_API IBRCombatInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Exception|Combat")
	bool ReceiveCombatHit(float Damage, float GroggyDamage, AActor* DamageCauser);
};
