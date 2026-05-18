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

	UFUNCTION(BlueprintCallable, Category="BossRaid|AI")
	void SetCombatAIEnabled(bool bEnabled);

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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BossRaid|AI")
	bool bCombatAIEnabled = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BossRaid|AI", meta=(ClampMin="0.0", Units="cm"))
	float DetectionRange = 1600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BossRaid|AI", meta=(ClampMin="0.0", Units="cm"))
	float AttackRange = 260.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BossRaid|AI", meta=(ClampMin="0.0", Units="cm/s"))
	float MoveSpeed = 180.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BossRaid|AI", meta=(ClampMin="0.0"))
	float RotationInterpSpeed = 4.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BossRaid|Attack", meta=(ClampMin="0.0"))
	float BossAttackDamage = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BossRaid|Attack", meta=(ClampMin="0.01", Units="s"))
	float BossAttackWindup = 0.65f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BossRaid|Attack", meta=(ClampMin="0.01", Units="s"))
	float BossAttackCooldown = 1.8f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BossRaid|Attack", meta=(ClampMin="1.0", Units="cm"))
	float BossAttackRadius = 90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BossRaid|Attack", meta=(ClampMin="0.0", Units="cm"))
	float BossAttackForwardOffset = 180.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BossRaid|Groggy", meta=(ClampMin="0.1", Units="s"))
	float GroggyDuration = 3.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="BossRaid|State")
	bool bIsDead = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="BossRaid|State")
	bool bIsGroggy = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="BossRaid|State")
	bool bIsAttacking = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BossRaid|Debug")
	bool bShowDebug = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BossRaid|Debug")
	bool bDrawAttackDebug = true;

	UPROPERTY(Transient)
	TObjectPtr<AActor> CurrentTarget;

	FTimerHandle AttackWindupTimerHandle;
	FTimerHandle GroggyTimerHandle;
	float LastAttackTime = -1000.0f;

	UFUNCTION()
	void HandleDead();

	UFUNCTION()
	void HandleGroggy();

	void RecoverFromGroggy();
	void UpdateCombatAI(float DeltaSeconds);
	void FaceTarget(float DeltaSeconds);
	void MoveTowardTarget(float DeltaSeconds);
	bool CanStartAttack(float DistanceToTarget) const;
	void StartBossAttack();
	void PerformBossAttack();
	void DrawDummyDebug() const;
};
