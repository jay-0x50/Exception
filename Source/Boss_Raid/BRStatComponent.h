#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BRStatComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FBRStatValueChanged, float, CurrentValue, float, MaxValue, float, NormalizedValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FBRStatStateEvent);

UCLASS(ClassGroup=(BossRaid), meta=(BlueprintSpawnableComponent))
class BOSS_RAID_API UBRStatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UBRStatComponent();

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category="BossRaid|Stats")
	void InitializeStats();

	UFUNCTION(BlueprintCallable, Category="BossRaid|Stats")
	void ConfigureMaxStats(float InMaxHP, float InMaxStamina, float InMaxGroggy);

	UFUNCTION(BlueprintCallable, Category="BossRaid|Stats")
	bool ApplyDamageToStats(float Damage, float GroggyDamage);

	UFUNCTION(BlueprintCallable, Category="BossRaid|Stats")
	bool SpendStamina(float Amount);

	UFUNCTION(BlueprintCallable, Category="BossRaid|Stats")
	void RecoverStamina(float Amount);

	UFUNCTION(BlueprintCallable, Category="BossRaid|Stats")
	void ResetGroggy();

	UFUNCTION(BlueprintPure, Category="BossRaid|Stats")
	float GetCurrentHP() const { return CurrentHP; }

	UFUNCTION(BlueprintPure, Category="BossRaid|Stats")
	float GetMaxHP() const { return MaxHP; }

	UFUNCTION(BlueprintPure, Category="BossRaid|Stats")
	float GetCurrentStamina() const { return CurrentStamina; }

	UFUNCTION(BlueprintPure, Category="BossRaid|Stats")
	float GetMaxStamina() const { return MaxStamina; }

	UFUNCTION(BlueprintPure, Category="BossRaid|Stats")
	float GetCurrentGroggy() const { return CurrentGroggy; }

	UFUNCTION(BlueprintPure, Category="BossRaid|Stats")
	float GetMaxGroggy() const { return MaxGroggy; }

	UFUNCTION(BlueprintPure, Category="BossRaid|Stats")
	bool IsDead() const { return bIsDead; }

	UFUNCTION(BlueprintPure, Category="BossRaid|Stats")
	bool IsGroggy() const { return bIsGroggy; }

	UPROPERTY(BlueprintAssignable, Category="BossRaid|Stats")
	FBRStatValueChanged OnHPChanged;

	UPROPERTY(BlueprintAssignable, Category="BossRaid|Stats")
	FBRStatValueChanged OnStaminaChanged;

	UPROPERTY(BlueprintAssignable, Category="BossRaid|Stats")
	FBRStatValueChanged OnGroggyChanged;

	UPROPERTY(BlueprintAssignable, Category="BossRaid|Stats")
	FBRStatStateEvent OnDead;

	UPROPERTY(BlueprintAssignable, Category="BossRaid|Stats")
	FBRStatStateEvent OnGroggy;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BossRaid|Stats", meta=(ClampMin="1.0"))
	float MaxHP = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BossRaid|Stats", meta=(ClampMin="0.0"))
	float MaxStamina = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BossRaid|Stats", meta=(ClampMin="0.0"))
	float MaxGroggy = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="BossRaid|Stats")
	float CurrentHP = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="BossRaid|Stats")
	float CurrentStamina = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="BossRaid|Stats")
	float CurrentGroggy = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="BossRaid|State")
	bool bIsDead = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="BossRaid|State")
	bool bIsGroggy = false;

	void BroadcastAllStats();
	void BroadcastHP();
	void BroadcastStamina();
	void BroadcastGroggy();
};
