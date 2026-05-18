#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BRBossDummy.generated.h"

class UStaticMeshComponent;

UCLASS()
class BOSS_RAID_API ABRBossDummy : public AActor
{
	GENERATED_BODY()

public:
	ABRBossDummy();

	virtual void Tick(float DeltaSeconds) override;
	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION(BlueprintCallable, Category="BossRaid|Dummy")
	void ResetDummy();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BossRaid|Stats", meta=(ClampMin="1.0"))
	float MaxHP = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BossRaid|Stats", meta=(ClampMin="1.0"))
	float MaxGroggy = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BossRaid|Stats", meta=(ClampMin="0.0"))
	float GroggyDamageMultiplier = 0.5f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="BossRaid|Stats")
	float CurrentHP = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="BossRaid|Stats")
	float CurrentGroggy = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="BossRaid|State")
	bool bIsDead = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="BossRaid|State")
	bool bIsGroggy = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BossRaid|Debug")
	bool bShowDebug = true;

	void SetDead();
	void SetGroggy();
	void DrawDummyDebug() const;
};
