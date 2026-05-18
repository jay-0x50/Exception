#pragma once

#include "CoreMinimal.h"
#include "BRCombatInterface.h"
#include "GameFramework/Actor.h"
#include "BRBossDummy.generated.h"

class UBRStatComponent;
class UStaticMeshComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FBRBossDummyStateEvent);

UCLASS(Blueprintable, BlueprintType, meta=(DisplayName="Boss Dummy"))
class BOSS_RAID_API ABRBossDummy : public AActor, public IBRCombatInterface
{
	GENERATED_BODY()

public:
	ABRBossDummy();

	virtual void Tick(float DeltaSeconds) override;
	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	virtual bool ReceiveCombatHit_Implementation(float Damage, float GroggyDamage, AActor* DamageCauser) override;

	UFUNCTION(BlueprintCallable, Category="BossRaid|Dummy")
	void ResetDummy();

	UFUNCTION(BlueprintPure, Category="BossRaid|Dummy")
	bool IsDead() const { return bIsDead; }

	UPROPERTY(BlueprintAssignable, Category="BossRaid|Events")
	FBRBossDummyStateEvent OnDummyDead;

	UPROPERTY(BlueprintAssignable, Category="BossRaid|Events")
	FBRBossDummyStateEvent OnDummyGroggy;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UBRStatComponent> StatComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BossRaid|Stats", meta=(ClampMin="0.0"))
	float GroggyDamageMultiplier = 0.5f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="BossRaid|State")
	bool bIsDead = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="BossRaid|State")
	bool bIsGroggy = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BossRaid|Debug")
	bool bShowDebug = true;

	UFUNCTION()
	void HandleDead();

	UFUNCTION()
	void HandleGroggy();

	void DrawDummyDebug() const;
};
