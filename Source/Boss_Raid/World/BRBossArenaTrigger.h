#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BRBossArenaTrigger.generated.h"

class ABRBossBase;
class UBRBossStatusWidget;
class UBoxComponent;
class UStaticMeshComponent;

UCLASS(Blueprintable, BlueprintType, meta=(DisplayName="Boss Arena Trigger"))
class BOSS_RAID_API ABRBossArenaTrigger : public AActor
{
	GENERATED_BODY()

public:
	ABRBossArenaTrigger();

	UFUNCTION(BlueprintCallable, Category="BossRaid|Arena")
	void ResetArenaForRetry();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UBoxComponent> TriggerBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UStaticMeshComponent> PreviewMesh;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="BossRaid|Arena")
	TObjectPtr<ABRBossBase> BossDummy;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="BossRaid|Arena")
	TArray<TObjectPtr<ABRBossBase>> BossActors;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="BossRaid|Arena")
	TObjectPtr<AActor> GateActorToHideOnDefeat;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="BossRaid|Arena")
	TObjectPtr<AActor> RewardActorToShowOnDefeat;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BossRaid|Arena")
	bool bResetBossOnEnter = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="BossRaid|Arena")
	bool bArenaStarted = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="BossRaid|Arena")
	bool bArenaCleared = false;

	UPROPERTY(Transient)
	TObjectPtr<UBRBossStatusWidget> BossStatusWidget;

	UFUNCTION()
	void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void HandleBossDefeated();

	UFUNCTION()
	void HandleBossStatChanged(float CurrentValue, float MaxValue, float NormalizedValue);

	UFUNCTION()
	void HandleBossStateChanged();

	void StartArena();
	void BuildManagedBossList(TArray<ABRBossBase*>& OutBosses) const;
	bool AreAllManagedBossesDead() const;
	UBRBossStatusWidget* ShowBossStatusWidget();
	void RefreshBossStatusWidget();
	void HideBossStatusWidget();
};
