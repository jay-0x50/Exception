// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "Boss_RaidCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputAction;
class UInputMappingContext;
class UAnimMontage;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UENUM(BlueprintType)
enum class EBRPlayerCombatState : uint8
{
	Idle,
	LightAttack,
	HeavyAttack,
	Dodge,
	Parry,
	Hit,
	Dead
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FBRPlayerStatChanged, float, CurrentValue, float, MaxValue, float, NormalizedValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FBRPlayerStateChanged, EBRPlayerCombatState, NewState);

/**
 *  A simple player-controllable third person character
 *  Implements a controllable orbiting camera
 */
UCLASS(abstract)
class ABoss_RaidCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;
	
protected:

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* LookAction;

	/** Mouse Look Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* MouseLookAction;

	/** Light Attack Input Action */
	UPROPERTY(EditAnywhere, Category="Input|Combat")
	UInputAction* LightAttackAction;

	/** Heavy Attack Input Action */
	UPROPERTY(EditAnywhere, Category="Input|Combat")
	UInputAction* HeavyAttackAction;

	/** Dodge Input Action */
	UPROPERTY(EditAnywhere, Category="Input|Combat")
	UInputAction* DodgeAction;

	/** Parry Input Action */
	UPROPERTY(EditAnywhere, Category="Input|Combat")
	UInputAction* ParryAction;

	/** Interact Input Action, reserved for groggy execution later */
	UPROPERTY(EditAnywhere, Category="Input|Combat")
	UInputAction* InteractAction;

	/** Max player HP for the boss raid demo */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BossRaid|Stats", meta=(ClampMin="1.0"))
	float MaxHP = 100.0f;

	/** Max stamina used by attack, dodge, and parry */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BossRaid|Stats", meta=(ClampMin="1.0"))
	float MaxStamina = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BossRaid|Stats", meta=(ClampMin="0.0"))
	float StaminaRegenPerSecond = 35.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BossRaid|Stats", meta=(ClampMin="0.0"))
	float StaminaRegenDelay = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BossRaid|Cost", meta=(ClampMin="0.0"))
	float LightAttackStaminaCost = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BossRaid|Cost", meta=(ClampMin="0.0"))
	float HeavyAttackStaminaCost = 25.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BossRaid|Cost", meta=(ClampMin="0.0"))
	float DodgeStaminaCost = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BossRaid|Cost", meta=(ClampMin="0.0"))
	float ParryStaminaCost = 15.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BossRaid|Timing", meta=(ClampMin="0.01", Units="s"))
	float LightAttackDuration = 0.45f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BossRaid|Timing", meta=(ClampMin="0.01", Units="s"))
	float HeavyAttackDuration = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BossRaid|Timing", meta=(ClampMin="0.01", Units="s"))
	float DodgeDuration = 0.55f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BossRaid|Timing", meta=(ClampMin="0.01", Units="s"))
	float DodgeInvincibleDuration = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BossRaid|Timing", meta=(ClampMin="0.01", Units="s"))
	float ParryDuration = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BossRaid|Timing", meta=(ClampMin="0.01", Units="s"))
	float ParryActiveDuration = 0.15f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BossRaid|Attack", meta=(ClampMin="0.0"))
	float LightAttackDamage = 15.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BossRaid|Attack", meta=(ClampMin="0.0"))
	float HeavyAttackDamage = 35.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BossRaid|Attack", meta=(ClampMin="0.0", Units="cm"))
	float AttackTraceDistance = 160.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BossRaid|Attack", meta=(ClampMin="1.0", Units="cm"))
	float AttackTraceRadius = 55.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BossRaid|Dodge", meta=(ClampMin="0.0"))
	float DodgeImpulseStrength = 650.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BossRaid|Animation")
	TObjectPtr<UAnimMontage> LightAttackMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BossRaid|Animation")
	TObjectPtr<UAnimMontage> HeavyAttackMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BossRaid|Animation")
	TObjectPtr<UAnimMontage> DodgeMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BossRaid|Animation")
	TObjectPtr<UAnimMontage> ParryMontage;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="BossRaid|Stats")
	float CurrentHP = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="BossRaid|Stats")
	float CurrentStamina = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="BossRaid|State")
	EBRPlayerCombatState CombatState = EBRPlayerCombatState::Idle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="BossRaid|State")
	bool bIsInvincible = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="BossRaid|State")
	bool bIsParryActive = false;

	float LastStaminaSpendTime = -1000.0f;

	FTimerHandle StateTimerHandle;
	FTimerHandle InvincibleTimerHandle;
	FTimerHandle ParryTimerHandle;

	UPROPERTY(Transient)
	TObjectPtr<UInputMappingContext> RuntimeCombatMappingContext;

	UPROPERTY(Transient)
	TObjectPtr<UInputAction> RuntimeLightAttackAction;

	UPROPERTY(Transient)
	TObjectPtr<UInputAction> RuntimeHeavyAttackAction;

	UPROPERTY(Transient)
	TObjectPtr<UInputAction> RuntimeDodgeAction;

	UPROPERTY(Transient)
	TObjectPtr<UInputAction> RuntimeParryAction;

	UPROPERTY(Transient)
	TObjectPtr<UInputAction> RuntimeInteractAction;

public:

	/** Constructor */
	ABoss_RaidCharacter();	

	virtual void Tick(float DeltaSeconds) override;

protected:

	/** Initialize input action bindings */
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void BeginPlay() override;

protected:

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	void LightAttackPressed();
	void HeavyAttackPressed();
	void DodgePressed();
	void ParryPressed();
	void InteractPressed();
	void SetupRuntimeCombatInput(class UEnhancedInputComponent* EnhancedInputComponent);

public:

	/** Handles move inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoMove(float Right, float Forward);

	/** Handles look inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoLook(float Yaw, float Pitch);

	/** Handles jump pressed inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpStart();

	/** Handles jump pressed inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpEnd();

	UFUNCTION(BlueprintCallable, Category="BossRaid|Combat")
	virtual bool DoLightAttack();

	UFUNCTION(BlueprintCallable, Category="BossRaid|Combat")
	virtual bool DoHeavyAttack();

	UFUNCTION(BlueprintCallable, Category="BossRaid|Combat")
	virtual bool DoDodge();

	UFUNCTION(BlueprintCallable, Category="BossRaid|Combat")
	virtual bool DoParry();

	UFUNCTION(BlueprintCallable, Category="BossRaid|Combat")
	virtual void DoInteract();

	UFUNCTION(BlueprintCallable, Category="BossRaid|Combat")
	virtual void PerformAttackTrace(float Damage);

	UFUNCTION(BlueprintCallable, Category="BossRaid|Stats")
	bool SpendStamina(float Amount);

	UFUNCTION(BlueprintCallable, Category="BossRaid|Stats")
	void RestoreHPAndStamina();

	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	UPROPERTY(BlueprintAssignable, Category="BossRaid|Events")
	FBRPlayerStatChanged OnHPChanged;

	UPROPERTY(BlueprintAssignable, Category="BossRaid|Events")
	FBRPlayerStatChanged OnStaminaChanged;

	UPROPERTY(BlueprintAssignable, Category="BossRaid|Events")
	FBRPlayerStateChanged OnCombatStateChanged;

	UFUNCTION(BlueprintImplementableEvent, Category="BossRaid|Events")
	void BP_CombatActionStarted(EBRPlayerCombatState NewState);

	UFUNCTION(BlueprintImplementableEvent, Category="BossRaid|Events")
	void BP_CombatActionEnded(EBRPlayerCombatState PreviousState);

	UFUNCTION(BlueprintImplementableEvent, Category="BossRaid|Events")
	void BP_AttackHit(AActor* HitActor, float Damage);

	UFUNCTION(BlueprintImplementableEvent, Category="BossRaid|Events")
	void BP_DamageReceived(float Damage);

	UFUNCTION(BlueprintImplementableEvent, Category="BossRaid|Events")
	void BP_ParryWindowStarted();

	UFUNCTION(BlueprintImplementableEvent, Category="BossRaid|Events")
	void BP_ParryWindowEnded();

protected:
	bool CanStartCombatAction() const;
	void SetCombatState(EBRPlayerCombatState NewState);
	void FinishCombatAction();
	void EndInvincibility();
	void EndParryWindow();
	void PlayOptionalMontage(UAnimMontage* Montage);
	void BroadcastHP();
	void BroadcastStamina();

public:

	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};

