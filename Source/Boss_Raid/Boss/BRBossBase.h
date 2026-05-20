#pragma once

#include "CoreMinimal.h"
#include "BRCombatInterface.h"
#include "GameFramework/Actor.h"
#include "BRBossBase.generated.h"

class UBRStatComponent;
class UStaticMeshComponent;
class ABRBossTeamCoordinator;

UENUM(BlueprintType)
enum class EBRBossPhase : uint8
{
	Phase1,
	Phase2
};

UENUM(BlueprintType)
enum class EBRBossTeamRole : uint8
{
	Solo,
	Melee,
	Ranged,
	Support
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FBRBossStateEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FBRBossStatChanged, float, CurrentValue, float, MaxValue, float, NormalizedValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FBRBossExecutionEvent, AActor*, Executor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FBRBossPhaseChanged, EBRBossPhase, NewPhase);

UCLASS(Abstract, Blueprintable, BlueprintType)
class BOSS_RAID_API ABRBossBase : public AActor, public IBRCombatInterface
{
	GENERATED_BODY()

public:
	ABRBossBase();

	virtual void Tick(float DeltaSeconds) override;
	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	virtual bool ReceiveCombatHit_Implementation(float Damage, float GroggyDamage, AActor* DamageCauser) override;

	UFUNCTION(BlueprintCallable, Category="BossRaid|Boss")
	virtual void ResetBoss();

	UFUNCTION(BlueprintCallable, Category="BossRaid|AI")
	virtual void SetCombatAIEnabled(bool bEnabled);

	UFUNCTION(BlueprintPure, Category="BossRaid|Boss")
	bool IsDead() const { return bIsDead; }

	UFUNCTION(BlueprintPure, Category="BossRaid|Boss")
	bool IsGroggy() const { return bIsGroggy; }

	UFUNCTION(BlueprintPure, Category="BossRaid|Boss")
	bool IsCombatAIEnabled() const { return bCombatAIEnabled; }

	UFUNCTION(BlueprintPure, Category="BossRaid|Boss")
	EBRBossPhase GetBossPhase() const { return BossPhase; }

	UFUNCTION(BlueprintPure, Category="BossRaid|Team")
	EBRBossTeamRole GetTeamRole() const { return TeamRole; }

	UFUNCTION(BlueprintPure, Category="BossRaid|Team")
	bool IsTeamMateAttacking() const;

	UFUNCTION(BlueprintPure, Category="BossRaid|Team")
	bool IsTeamMateWithin(float Distance) const;

	UFUNCTION(BlueprintCallable, Category="BossRaid|Team")
	void SetTeamCoordinator(ABRBossTeamCoordinator* NewTeamCoordinator);

	UFUNCTION(BlueprintPure, Category="BossRaid|Team")
	ABRBossTeamCoordinator* GetTeamCoordinator() const { return TeamCoordinator; }

	UFUNCTION(BlueprintCallable, Category="BossRaid|Team")
	virtual void ApplyTeamSlot(int32 TeamSlotIndex);

	UFUNCTION(BlueprintPure, Category="BossRaid|Stats")
	float GetMaxHP() const;

	UFUNCTION(BlueprintPure, Category="BossRaid|Stats")
	float GetCurrentHP() const;

	UFUNCTION(BlueprintPure, Category="BossRaid|Stats")
	float GetHPPercent() const;

	UFUNCTION(BlueprintCallable, Category="BossRaid|Stats")
	bool ApplyGroggyDamage(float GroggyDamage, AActor* DamageCauser);

	UFUNCTION(BlueprintPure, Category="BossRaid|Execution")
	bool CanBeExecuted() const;

	UFUNCTION(BlueprintCallable, Category="BossRaid|Execution")
	bool BeginExecution(AActor* Executor);

	UFUNCTION(BlueprintCallable, Category="BossRaid|Execution")
	bool CompleteExecution(float Damage, AActor* Executor);

	UPROPERTY(BlueprintAssignable, Category="BossRaid|Events")
	FBRBossStateEvent OnBossDead;

	UPROPERTY(BlueprintAssignable, Category="BossRaid|Events")
	FBRBossStateEvent OnBossGroggy;

	UPROPERTY(BlueprintAssignable, Category="BossRaid|Events")
	FBRBossStateEvent OnBossRecoveredFromGroggy;

	UPROPERTY(BlueprintAssignable, Category="BossRaid|Events")
	FBRBossStatChanged OnBossHPChanged;

	UPROPERTY(BlueprintAssignable, Category="BossRaid|Events")
	FBRBossStatChanged OnBossGroggyChanged;

	UPROPERTY(BlueprintAssignable, Category="BossRaid|Events")
	FBRBossExecutionEvent OnExecutionStarted;

	UPROPERTY(BlueprintAssignable, Category="BossRaid|Events")
	FBRBossExecutionEvent OnExecutionCompleted;

	UPROPERTY(BlueprintAssignable, Category="BossRaid|Events")
	FBRBossPhaseChanged OnPhaseChanged;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void OnBossReset();
	virtual void OnBossDeadInternal();
	virtual void OnBossGroggyInternal();
	virtual void OnBossRecoveredFromGroggyInternal();
	virtual void OnBossPhaseChanged(EBRBossPhase NewPhase);
	virtual void UpdateBossAI(float DeltaSeconds);
	virtual void DrawBossDebug() const;
	virtual FString GetBossDebugName() const;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UBRStatComponent> StatComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BossRaid|Stats", meta=(ClampMin="1.0"))
	float InitialMaxHP = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BossRaid|Stats", meta=(ClampMin="0.0"))
	float InitialMaxGroggy = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BossRaid|Stats", meta=(ClampMin="0.0"))
	float GroggyDamageMultiplier = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BossRaid|Phase", meta=(ClampMin="0.0", ClampMax="1.0"))
	float Phase2StartHPRatio = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BossRaid|Groggy", meta=(ClampMin="0.1", Units="s"))
	float GroggyDuration = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BossRaid|AI")
	bool bCombatAIEnabled = false;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="BossRaid|Team")
	TObjectPtr<ABRBossTeamCoordinator> TeamCoordinator;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BossRaid|Team")
	EBRBossTeamRole TeamRole = EBRBossTeamRole::Solo;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="BossRaid|State")
	EBRBossPhase BossPhase = EBRBossPhase::Phase1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="BossRaid|State")
	bool bIsDead = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="BossRaid|State")
	bool bIsGroggy = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="BossRaid|State")
	bool bIsAttacking = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="BossRaid|State")
	bool bIsBeingExecuted = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BossRaid|Debug")
	bool bShowDebug = true;

	UPROPERTY(Transient)
	TObjectPtr<AActor> CurrentTarget;

	FTimerHandle GroggyTimerHandle;

	UFUNCTION()
	virtual void HandleDead();

	UFUNCTION()
	virtual void HandleGroggy();

	UFUNCTION()
	void HandleHPChanged(float CurrentValue, float MaxValue, float NormalizedValue);

	UFUNCTION()
	void HandleGroggyChanged(float CurrentValue, float MaxValue, float NormalizedValue);

	void RecoverFromGroggy();
	void RefreshPhaseByHP();
	bool CanStartCoordinatedAttack() const;
	bool NotifyCoordinatedAttackStarted();
	void NotifyCoordinatedAttackFinished();
	virtual void ClearBaseTimers();
};
