#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BRStatComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FBRStatValueChanged, float, CurrentValue, float, MaxValue, float, NormalizedValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FBRStatStateEvent);

UCLASS(ClassGroup=(Exception), meta=(BlueprintSpawnableComponent))
class EXCEPTION_API UBRStatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UBRStatComponent();

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category="Exception|Stats")
	void InitializeStats();

	UFUNCTION(BlueprintCallable, Category="Exception|Stats")
	void ConfigureMaxStats(float InMaxHP, float InMaxStamina, float InMaxGroggy);

	UFUNCTION(BlueprintCallable, Category="Exception|Stats")
	bool ApplyDamageToStats(float Damage, float GroggyDamage);

	UFUNCTION(BlueprintCallable, Category="Exception|Stats")
	bool SpendStamina(float Amount);

	UFUNCTION(BlueprintCallable, Category="Exception|Stats")
	void RecoverStamina(float Amount);

	UFUNCTION(BlueprintCallable, Category="Exception|Stats")
	void ResetGroggy();

	UFUNCTION(BlueprintPure, Category="Exception|Stats")
	float GetCurrentHP() const { return CurrentHP; }

	UFUNCTION(BlueprintPure, Category="Exception|Stats")
	float GetMaxHP() const { return MaxHP; }

	UFUNCTION(BlueprintPure, Category="Exception|Stats")
	float GetCurrentStamina() const { return CurrentStamina; }

	UFUNCTION(BlueprintPure, Category="Exception|Stats")
	float GetMaxStamina() const { return MaxStamina; }

	UFUNCTION(BlueprintPure, Category="Exception|Stats")
	float GetCurrentGroggy() const { return CurrentGroggy; }

	UFUNCTION(BlueprintPure, Category="Exception|Stats")
	float GetMaxGroggy() const { return MaxGroggy; }

	UFUNCTION(BlueprintPure, Category="Exception|Stats")
	bool IsDead() const { return bIsDead; }

	UFUNCTION(BlueprintPure, Category="Exception|Stats")
	bool IsGroggy() const { return bIsGroggy; }

	UPROPERTY(BlueprintAssignable, Category="Exception|Stats")
	FBRStatValueChanged OnHPChanged;

	UPROPERTY(BlueprintAssignable, Category="Exception|Stats")
	FBRStatValueChanged OnStaminaChanged;

	UPROPERTY(BlueprintAssignable, Category="Exception|Stats")
	FBRStatValueChanged OnGroggyChanged;

	UPROPERTY(BlueprintAssignable, Category="Exception|Stats")
	FBRStatStateEvent OnDead;

	UPROPERTY(BlueprintAssignable, Category="Exception|Stats")
	FBRStatStateEvent OnGroggy;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Exception|Stats", meta=(ClampMin="1.0"))
	float MaxHP = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Exception|Stats", meta=(ClampMin="0.0"))
	float MaxStamina = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Exception|Stats", meta=(ClampMin="0.0"))
	float MaxGroggy = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Exception|Stats")
	float CurrentHP = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Exception|Stats")
	float CurrentStamina = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Exception|Stats")
	float CurrentGroggy = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Exception|State")
	bool bIsDead = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Exception|State")
	bool bIsGroggy = false;

	void BroadcastAllStats();
	void BroadcastHP();
	void BroadcastStamina();
	void BroadcastGroggy();
};
