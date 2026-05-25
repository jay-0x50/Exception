// Copyright Epic Games, Inc. All Rights Reserved.

#include "ExceptionCharacter.h"
#include "BRCombatInterface.h"
#include "BRBossBase.h"
#include "ExceptionGameMode.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "Animation/AnimInstance.h"
#include "DrawDebugHelpers.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/Engine.h"
#include "Engine/DamageEvents.h"
#include "Kismet/GameplayStatics.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"
#include "Exception.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

AExceptionCharacter::AExceptionCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 500.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = FreeCameraArmLength;
	CameraBoom->TargetOffset = FVector::ZeroVector;
	CameraBoom->SocketOffset = FVector::ZeroVector;
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->bDoCollisionTest = true;

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	// Note: The skeletal mesh and anim blueprint references are set in the derived player Blueprint.
}

void AExceptionCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (CombatState == EBRPlayerCombatState::Dead)
	{
		return;
	}

	const UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	if (CurrentStamina < MaxStamina && World->GetTimeSeconds() - LastStaminaSpendTime >= StaminaRegenDelay)
	{
		CurrentStamina = FMath::Min(MaxStamina, CurrentStamina + (StaminaRegenPerSecond * DeltaSeconds));
		BroadcastStamina();
	}

	UpdateLockOn(DeltaSeconds);
	DrawCombatDebug();
}

void AExceptionCharacter::BeginPlay()
{
	Super::BeginPlay();

	RestoreHPAndStamina();
	RegisterInitialCheckpoint();
}

void AExceptionCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AExceptionCharacter::Move);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &AExceptionCharacter::Look);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AExceptionCharacter::Look);

		if (LightAttackAction)
		{
			EnhancedInputComponent->BindAction(LightAttackAction, ETriggerEvent::Started, this, &AExceptionCharacter::LightAttackPressed);
		}

		if (HeavyAttackAction)
		{
			EnhancedInputComponent->BindAction(HeavyAttackAction, ETriggerEvent::Started, this, &AExceptionCharacter::HeavyAttackPressed);
		}

		if (DodgeAction)
		{
			EnhancedInputComponent->BindAction(DodgeAction, ETriggerEvent::Started, this, &AExceptionCharacter::DodgePressed);
		}

		if (ParryAction)
		{
			EnhancedInputComponent->BindAction(ParryAction, ETriggerEvent::Started, this, &AExceptionCharacter::ParryPressed);
		}

		if (InteractAction)
		{
			EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &AExceptionCharacter::InteractPressed);
		}

		if (LockOnAction)
		{
			EnhancedInputComponent->BindAction(LockOnAction, ETriggerEvent::Started, this, &AExceptionCharacter::LockOnPressed);
		}

		SetupRuntimeCombatInput(EnhancedInputComponent);
	}
	else
	{
		UE_LOG(LogException, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void AExceptionCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	// route the input
	DoMove(MovementVector.X, MovementVector.Y);
}

void AExceptionCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// route the input
	DoLook(LookAxisVector.X, LookAxisVector.Y);
}

void AExceptionCharacter::LightAttackPressed()
{
	DoLightAttack();
}

void AExceptionCharacter::HeavyAttackPressed()
{
	DoHeavyAttack();
}

void AExceptionCharacter::DodgePressed()
{
	DoDodge();
}

void AExceptionCharacter::ParryPressed()
{
	DoParry();
}

void AExceptionCharacter::InteractPressed()
{
	DoInteract();
}

void AExceptionCharacter::LockOnPressed()
{
	ToggleLockOn();
}

void AExceptionCharacter::SetupRuntimeCombatInput(UEnhancedInputComponent* EnhancedInputComponent)
{
	if (!EnhancedInputComponent)
	{
		return;
	}

	bool bNeedsRuntimeMapping = false;
	if (!RuntimeCombatMappingContext)
	{
		RuntimeCombatMappingContext = NewObject<UInputMappingContext>(this, TEXT("IMC_RuntimeExceptionCombat"));
	}

	if (!LightAttackAction)
	{
		RuntimeLightAttackAction = NewObject<UInputAction>(this, TEXT("IA_RuntimeLightAttack"));
		RuntimeLightAttackAction->ValueType = EInputActionValueType::Boolean;
		RuntimeCombatMappingContext->MapKey(RuntimeLightAttackAction, EKeys::LeftMouseButton);
		EnhancedInputComponent->BindAction(RuntimeLightAttackAction, ETriggerEvent::Started, this, &AExceptionCharacter::LightAttackPressed);
		bNeedsRuntimeMapping = true;
	}

	if (!HeavyAttackAction)
	{
		RuntimeHeavyAttackAction = NewObject<UInputAction>(this, TEXT("IA_RuntimeHeavyAttack"));
		RuntimeHeavyAttackAction->ValueType = EInputActionValueType::Boolean;
		RuntimeCombatMappingContext->MapKey(RuntimeHeavyAttackAction, EKeys::RightMouseButton);
		EnhancedInputComponent->BindAction(RuntimeHeavyAttackAction, ETriggerEvent::Started, this, &AExceptionCharacter::HeavyAttackPressed);
		bNeedsRuntimeMapping = true;
	}

	if (!DodgeAction)
	{
		RuntimeDodgeAction = NewObject<UInputAction>(this, TEXT("IA_RuntimeDodge"));
		RuntimeDodgeAction->ValueType = EInputActionValueType::Boolean;
		RuntimeCombatMappingContext->MapKey(RuntimeDodgeAction, EKeys::LeftShift);
		EnhancedInputComponent->BindAction(RuntimeDodgeAction, ETriggerEvent::Started, this, &AExceptionCharacter::DodgePressed);
		bNeedsRuntimeMapping = true;
	}

	if (!ParryAction)
	{
		RuntimeParryAction = NewObject<UInputAction>(this, TEXT("IA_RuntimeParry"));
		RuntimeParryAction->ValueType = EInputActionValueType::Boolean;
		RuntimeCombatMappingContext->MapKey(RuntimeParryAction, EKeys::F);
		EnhancedInputComponent->BindAction(RuntimeParryAction, ETriggerEvent::Started, this, &AExceptionCharacter::ParryPressed);
		bNeedsRuntimeMapping = true;
	}

	if (!InteractAction)
	{
		RuntimeInteractAction = NewObject<UInputAction>(this, TEXT("IA_RuntimeInteract"));
		RuntimeInteractAction->ValueType = EInputActionValueType::Boolean;
		RuntimeCombatMappingContext->MapKey(RuntimeInteractAction, EKeys::E);
		EnhancedInputComponent->BindAction(RuntimeInteractAction, ETriggerEvent::Started, this, &AExceptionCharacter::InteractPressed);
		bNeedsRuntimeMapping = true;
	}

	if (!LockOnAction)
	{
		RuntimeLockOnAction = NewObject<UInputAction>(this, TEXT("IA_RuntimeLockOn"));
		RuntimeLockOnAction->ValueType = EInputActionValueType::Boolean;
		RuntimeCombatMappingContext->MapKey(RuntimeLockOnAction, EKeys::Tab);
		EnhancedInputComponent->BindAction(RuntimeLockOnAction, ETriggerEvent::Started, this, &AExceptionCharacter::LockOnPressed);
		bNeedsRuntimeMapping = true;
	}

	if (bNeedsRuntimeMapping)
	{
		if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
		{
			if (ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer())
			{
				if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer))
				{
					Subsystem->AddMappingContext(RuntimeCombatMappingContext, 1);
				}
			}
		}
	}
}

void AExceptionCharacter::DoMove(float Right, float Forward)
{
	if (CombatState == EBRPlayerCombatState::Dead || CombatState == EBRPlayerCombatState::Hit || CombatState == EBRPlayerCombatState::Execution)
	{
		return;
	}

	if (GetController() != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = GetController()->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, Forward);
		AddMovementInput(RightDirection, Right);
	}
}

void AExceptionCharacter::DoLook(float Yaw, float Pitch)
{
	if (bIsLockedOn)
	{
		LockOnYawOffset = FMath::Clamp(LockOnYawOffset + (Yaw * LockOnLookInputSensitivity), -LockOnYawOffsetLimit, LockOnYawOffsetLimit);
		LockOnPitchOffset = FMath::Clamp(LockOnPitchOffset + (Pitch * LockOnLookInputSensitivity), -LockOnPitchOffsetLimit, LockOnPitchOffsetLimit);
		return;
	}

	if (GetController() != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void AExceptionCharacter::DoJumpStart()
{
	if (CombatState != EBRPlayerCombatState::Idle)
	{
		return;
	}

	// signal the character to jump
	Jump();
}

void AExceptionCharacter::DoJumpEnd()
{
	// signal the character to stop jumping
	StopJumping();
}

bool AExceptionCharacter::DoLightAttack()
{
	if (!CanStartCombatAction() || !SpendStamina(LightAttackStaminaCost))
	{
		return false;
	}

	SetCombatState(EBRPlayerCombatState::LightAttack);
	PlayOptionalMontage(LightAttackMontage);
	PerformAttackTrace(LightAttackDamage, LightAttackGroggyDamage);
	UE_LOG(LogTemplateCharacter, Log, TEXT("LightAttack: Damage=%.1f, HitCount=%d"), LightAttackDamage, LastAttackHitCount);

	GetWorldTimerManager().SetTimer(StateTimerHandle, this, &AExceptionCharacter::FinishCombatAction, LightAttackDuration, false);
	return true;
}

bool AExceptionCharacter::DoHeavyAttack()
{
	if (!CanStartCombatAction() || !SpendStamina(HeavyAttackStaminaCost))
	{
		return false;
	}

	SetCombatState(EBRPlayerCombatState::HeavyAttack);
	PlayOptionalMontage(HeavyAttackMontage);
	PerformAttackTrace(HeavyAttackDamage, HeavyAttackGroggyDamage);
	UE_LOG(LogTemplateCharacter, Log, TEXT("HeavyAttack: Damage=%.1f, HitCount=%d"), HeavyAttackDamage, LastAttackHitCount);

	GetWorldTimerManager().SetTimer(StateTimerHandle, this, &AExceptionCharacter::FinishCombatAction, HeavyAttackDuration, false);
	return true;
}

bool AExceptionCharacter::DoDodge()
{
	if (!CanStartCombatAction() || !SpendStamina(DodgeStaminaCost))
	{
		return false;
	}

	SetCombatState(EBRPlayerCombatState::Dodge);
	PlayOptionalMontage(DodgeMontage);
	UE_LOG(LogTemplateCharacter, Log, TEXT("Dodge: Invincible %.2fs"), DodgeInvincibleDuration);

	bIsInvincible = true;
	const FVector DodgeDirection = GetLastMovementInputVector().IsNearlyZero()
		? GetActorForwardVector()
		: GetLastMovementInputVector().GetSafeNormal();
	LaunchCharacter(DodgeDirection * DodgeImpulseStrength, true, false);

	GetWorldTimerManager().SetTimer(InvincibleTimerHandle, this, &AExceptionCharacter::EndInvincibility, DodgeInvincibleDuration, false);
	GetWorldTimerManager().SetTimer(StateTimerHandle, this, &AExceptionCharacter::FinishCombatAction, DodgeDuration, false);
	return true;
}

bool AExceptionCharacter::DoParry()
{
	if (!CanStartCombatAction() || !SpendStamina(ParryStaminaCost))
	{
		return false;
	}

	SetCombatState(EBRPlayerCombatState::Parry);
	PlayOptionalMontage(ParryMontage);
	UE_LOG(LogTemplateCharacter, Log, TEXT("Parry: Active %.2fs"), ParryActiveDuration);

	bIsParryActive = true;
	BP_ParryWindowStarted();

	GetWorldTimerManager().SetTimer(ParryTimerHandle, this, &AExceptionCharacter::EndParryWindow, ParryActiveDuration, false);
	GetWorldTimerManager().SetTimer(StateTimerHandle, this, &AExceptionCharacter::FinishCombatAction, ParryDuration, false);
	return true;
}

void AExceptionCharacter::DoInteract()
{
	TryExecution();
}

void AExceptionCharacter::ToggleLockOn()
{
	if (bIsLockedOn)
	{
		ClearLockOn();
		return;
	}

	LockOnTarget = FindLockOnTarget();
	if (!LockOnTarget)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(1005, 1.0f, FColor::Silver, TEXT("No Lock-on Target"));
		}
		return;
	}

	bIsLockedOn = true;
	LockOnYawOffset = 0.0f;
	LockOnPitchOffset = 0.0f;
	GetCharacterMovement()->bOrientRotationToMovement = false;
	CameraBoom->TargetArmLength = LockOnCameraArmLength;
	CameraBoom->TargetOffset = LockOnCameraTargetOffset;
	CameraBoom->SocketOffset = LockOnCameraSocketOffset;

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(1006, 1.0f, FColor::Cyan, TEXT("Lock-on Enabled"));
	}
}

void AExceptionCharacter::ClearLockOn()
{
	if (!bIsLockedOn && !LockOnTarget)
	{
		return;
	}

	bIsLockedOn = false;
	LockOnTarget = nullptr;
	LockOnYawOffset = 0.0f;
	LockOnPitchOffset = 0.0f;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	CameraBoom->TargetArmLength = FreeCameraArmLength;
	CameraBoom->TargetOffset = FVector::ZeroVector;
	CameraBoom->SocketOffset = FVector::ZeroVector;

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(1007, 1.0f, FColor::Silver, TEXT("Lock-on Disabled"));
	}
}

void AExceptionCharacter::PerformAttackTrace(float Damage, float GroggyDamage)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	LastAttackHitCount = 0;
	LastAttackDebugTime = World->GetTimeSeconds();

	const FVector TraceStart = GetActorLocation() + FVector(0.0f, 0.0f, 45.0f);
	const FVector TraceEnd = TraceStart + (GetActorForwardVector() * AttackTraceDistance);

	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(ECC_Pawn);
	ObjectParams.AddObjectTypesToQuery(ECC_WorldDynamic);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(ExceptionPlayerAttackTrace), false, this);
	QueryParams.AddIgnoredActor(this);

	TArray<FHitResult> Hits;
	const FCollisionShape Shape = FCollisionShape::MakeSphere(AttackTraceRadius);
	const bool bHit = World->SweepMultiByObjectType(Hits, TraceStart, TraceEnd, FQuat::Identity, ObjectParams, Shape, QueryParams);

	if (bDrawAttackTraceDebug)
	{
		DrawDebugLine(World, TraceStart, TraceEnd, bHit ? FColor::Green : FColor::Red, false, 1.0f, 0, 2.0f);
		DrawDebugSphere(World, TraceEnd, AttackTraceRadius, 16, bHit ? FColor::Green : FColor::Red, false, 1.0f);
	}

	if (!bHit)
	{
		return;
	}

	TSet<AActor*> DamagedActors;
	for (const FHitResult& Hit : Hits)
	{
		AActor* HitActor = Hit.GetActor();
		if (!HitActor || DamagedActors.Contains(HitActor))
		{
			continue;
		}

		DamagedActors.Add(HitActor);
		if (HitActor->GetClass()->ImplementsInterface(UBRCombatInterface::StaticClass()))
		{
			IBRCombatInterface::Execute_ReceiveCombatHit(HitActor, Damage, GroggyDamage, this);
		}
		else
		{
			UGameplayStatics::ApplyDamage(HitActor, Damage, GetController(), this, UDamageType::StaticClass());
		}
		BP_AttackHit(HitActor, Damage);
		++LastAttackHitCount;

		if (bDrawAttackTraceDebug)
		{
			DrawDebugSphere(World, Hit.ImpactPoint, 18.0f, 12, FColor::Yellow, false, 1.0f);
		}
	}
}

bool AExceptionCharacter::SpendStamina(float Amount)
{
	if (Amount <= 0.0f)
	{
		return true;
	}

	if (CurrentStamina < Amount)
	{
		return false;
	}

	CurrentStamina -= Amount;
	LastStaminaSpendTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	BroadcastStamina();
	return true;
}

void AExceptionCharacter::RestoreHPAndStamina()
{
	CurrentHP = MaxHP;
	CurrentStamina = MaxStamina;
	GetWorldTimerManager().ClearTimer(StateTimerHandle);
	GetWorldTimerManager().ClearTimer(InvincibleTimerHandle);
	GetWorldTimerManager().ClearTimer(ParryTimerHandle);
	GetWorldTimerManager().ClearTimer(RespawnTimerHandle);
	GetWorldTimerManager().ClearTimer(ExecutionTimerHandle);
	SetCombatState(EBRPlayerCombatState::Idle);
	bIsInvincible = false;
	bIsParryActive = false;
	PendingExecutionTarget = nullptr;
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	GetCharacterMovement()->StopMovementImmediately();
	ClearLockOn();
	BroadcastHP();
	BroadcastStamina();
}

void AExceptionCharacter::ApplySavedStats(float SavedHP, float SavedStamina)
{
	GetWorldTimerManager().ClearTimer(StateTimerHandle);
	GetWorldTimerManager().ClearTimer(InvincibleTimerHandle);
	GetWorldTimerManager().ClearTimer(ParryTimerHandle);
	GetWorldTimerManager().ClearTimer(RespawnTimerHandle);
	GetWorldTimerManager().ClearTimer(ExecutionTimerHandle);

	CurrentHP = FMath::Clamp(SavedHP, 1.0f, MaxHP);
	CurrentStamina = FMath::Clamp(SavedStamina, 0.0f, MaxStamina);
	SetCombatState(EBRPlayerCombatState::Idle);
	bIsInvincible = false;
	bIsParryActive = false;
	PendingExecutionTarget = nullptr;
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	GetCharacterMovement()->StopMovementImmediately();
	ClearLockOn();
	BroadcastHP();
	BroadcastStamina();
}

void AExceptionCharacter::RespawnAtCheckpoint()
{
	AExceptionGameMode* ExceptionGameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AExceptionGameMode>() : nullptr;
	FTransform RespawnTransform = ExceptionGameMode && ExceptionGameMode->HasCheckpoint()
		? ExceptionGameMode->GetCheckpointTransform()
		: GetActorTransform();
	RespawnTransform.SetScale3D(GetActorScale3D());

	if (ExceptionGameMode)
	{
		ExceptionGameMode->ResetActiveBossArenaForRetry();
	}

	SetActorTransform(RespawnTransform, false, nullptr, ETeleportType::TeleportPhysics);
	RestoreHPAndStamina();

	if (AController* CurrentController = GetController())
	{
		CurrentController->SetControlRotation(RespawnTransform.GetRotation().Rotator());
	}

	UE_LOG(LogTemplateCharacter, Log, TEXT("Player respawned at checkpoint: %s"), *RespawnTransform.GetLocation().ToString());
}

float AExceptionCharacter::TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (CombatState == EBRPlayerCombatState::Dead || bIsInvincible || Damage <= 0.0f)
	{
		return 0.0f;
	}

	if (bIsParryActive)
	{
		EndParryWindow();

		if (ABRBossBase* BossCauser = Cast<ABRBossBase>(DamageCauser))
		{
			BossCauser->ApplyGroggyDamage(ParrySuccessGroggyDamage, this);
		}

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(1004, 1.2f, FColor::Cyan, TEXT("Parry Success"));
		}

		return 0.0f;
	}

	CurrentHP = FMath::Max(0.0f, CurrentHP - Damage);
	BroadcastHP();
	BP_DamageReceived(Damage);

	if (CurrentHP <= 0.0f)
	{
		GetWorldTimerManager().ClearTimer(StateTimerHandle);
		GetWorldTimerManager().ClearTimer(InvincibleTimerHandle);
		GetWorldTimerManager().ClearTimer(ParryTimerHandle);
		GetWorldTimerManager().ClearTimer(ExecutionTimerHandle);
		ClearLockOn();
		SetCombatState(EBRPlayerCombatState::Dead);
		GetCharacterMovement()->DisableMovement();
		GetWorldTimerManager().SetTimer(RespawnTimerHandle, this, &AExceptionCharacter::RespawnAtCheckpoint, RespawnDelay, false);
		return Damage;
	}

	GetWorldTimerManager().ClearTimer(StateTimerHandle);
	GetWorldTimerManager().ClearTimer(InvincibleTimerHandle);
	GetWorldTimerManager().ClearTimer(ParryTimerHandle);
	EndParryWindow();
	bIsInvincible = false;
	SetCombatState(EBRPlayerCombatState::Hit);
	PlayOptionalMontage(HitMontage);

	if (DamageCauser)
	{
		const FVector AwayFromCauser = FVector(GetActorLocation() - DamageCauser->GetActorLocation()).GetSafeNormal2D();
		const FVector KnockbackDirection = AwayFromCauser.IsNearlyZero() ? -GetActorForwardVector() : AwayFromCauser;
		LaunchCharacter(KnockbackDirection * HitKnockbackStrength, true, false);
	}

	GetWorldTimerManager().SetTimer(StateTimerHandle, this, &AExceptionCharacter::FinishCombatAction, HitStunDuration, false);
	return Damage;
}

bool AExceptionCharacter::CanStartCombatAction() const
{
	return CombatState == EBRPlayerCombatState::Idle && !GetCharacterMovement()->IsFalling();
}

void AExceptionCharacter::SetCombatState(EBRPlayerCombatState NewState)
{
	if (CombatState == NewState)
	{
		return;
	}

	const EBRPlayerCombatState PreviousState = CombatState;
	CombatState = NewState;
	OnCombatStateChanged.Broadcast(CombatState);

	if (NewState == EBRPlayerCombatState::Idle)
	{
		BP_CombatActionEnded(PreviousState);
	}
	else
	{
		BP_CombatActionStarted(NewState);
	}
}

void AExceptionCharacter::FinishCombatAction()
{
	if (CombatState == EBRPlayerCombatState::Dead || CombatState == EBRPlayerCombatState::Execution)
	{
		return;
	}

	if (bIsInvincible)
	{
		EndInvincibility();
	}

	if (bIsParryActive)
	{
		EndParryWindow();
	}

	SetCombatState(EBRPlayerCombatState::Idle);
}

void AExceptionCharacter::EndInvincibility()
{
	bIsInvincible = false;
	GetWorldTimerManager().ClearTimer(InvincibleTimerHandle);
}

void AExceptionCharacter::EndParryWindow()
{
	if (!bIsParryActive)
	{
		return;
	}

	bIsParryActive = false;
	GetWorldTimerManager().ClearTimer(ParryTimerHandle);
	BP_ParryWindowEnded();
}

void AExceptionCharacter::PlayOptionalMontage(UAnimMontage* Montage)
{
	if (!Montage)
	{
		return;
	}

	if (UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr)
	{
		AnimInstance->Montage_Play(Montage);
	}
}

void AExceptionCharacter::BroadcastHP()
{
	OnHPChanged.Broadcast(CurrentHP, MaxHP, MaxHP > 0.0f ? CurrentHP / MaxHP : 0.0f);
}

void AExceptionCharacter::BroadcastStamina()
{
	OnStaminaChanged.Broadcast(CurrentStamina, MaxStamina, MaxStamina > 0.0f ? CurrentStamina / MaxStamina : 0.0f);
}

void AExceptionCharacter::DrawCombatDebug() const
{
	if (!bShowCombatDebug || !GEngine)
	{
		return;
	}

	const float StaminaPercent = MaxStamina > 0.0f ? CurrentStamina / MaxStamina : 0.0f;
	const float HPPercent = MaxHP > 0.0f ? CurrentHP / MaxHP : 0.0f;
	const FString DebugText = FString::Printf(
		TEXT("Player Debug\nState: %s\nHP: %.0f / %.0f (%.0f%%)\nStamina: %.0f / %.0f (%.0f%%)\nLock-on: %s\nInvincible: %s\nParry Active: %s\nLast Attack Hits: %d"),
		*GetCombatStateName(),
		CurrentHP,
		MaxHP,
		HPPercent * 100.0f,
		CurrentStamina,
		MaxStamina,
		StaminaPercent * 100.0f,
		bIsLockedOn ? TEXT("true") : TEXT("false"),
		bIsInvincible ? TEXT("true") : TEXT("false"),
		bIsParryActive ? TEXT("true") : TEXT("false"),
		LastAttackHitCount);

	GEngine->AddOnScreenDebugMessage(1001, 0.0f, FColor::Cyan, DebugText);
}

FString AExceptionCharacter::GetCombatStateName() const
{
	const UEnum* Enum = StaticEnum<EBRPlayerCombatState>();
	return Enum ? Enum->GetNameStringByValue(static_cast<int64>(CombatState)) : TEXT("Unknown");
}

static float GetExecutionDistanceSq2D(const AExceptionCharacter* Player, const ABRBossBase* Boss)
{
	if (!Player || !Boss)
	{
		return TNumericLimits<float>::Max();
	}

	const FVector PlayerLocation = Player->GetActorLocation();
	const FBox BossBounds = Boss->GetComponentsBoundingBox(true);
	const FVector ClosestPoint = BossBounds.IsValid ? BossBounds.GetClosestPointTo(PlayerLocation) : Boss->GetActorLocation();
	return FVector::DistSquared2D(PlayerLocation, ClosestPoint);
}

void AExceptionCharacter::RegisterInitialCheckpoint()
{
	if (AExceptionGameMode* ExceptionGameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AExceptionGameMode>() : nullptr)
	{
		if (!ExceptionGameMode->HasCheckpoint())
		{
			ExceptionGameMode->SetCheckpointTransform(GetActorTransform());
		}
	}
}

bool AExceptionCharacter::TryExecution()
{
	if (!CanStartCombatAction())
	{
		return false;
	}

	ABRBossBase* ExecutionTarget = FindExecutionTarget();
	if (!ExecutionTarget)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(1010, 1.0f, FColor::Silver, TEXT("No Execution Target"));
		}
		return false;
	}

	StartExecution(ExecutionTarget);
	return true;
}

AActor* AExceptionCharacter::FindLockOnTarget() const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	TArray<AActor*> Bosses;
	UGameplayStatics::GetAllActorsOfClass(this, ABRBossBase::StaticClass(), Bosses);

	AActor* BestTarget = nullptr;
	float BestDistanceSq = FMath::Square(LockOnRange);
	const FVector TraceStart = FollowCamera ? FollowCamera->GetComponentLocation() : GetActorLocation();

	for (AActor* Candidate : Bosses)
	{
		ABRBossBase* Boss = Cast<ABRBossBase>(Candidate);
		if (!Boss || Boss->IsDead())
		{
			continue;
		}

		const FVector TargetFocus = Boss->GetActorLocation() + FVector(0.0f, 0.0f, LockOnTargetHeightOffset);
		const float DistanceSq = FVector::DistSquared(GetActorLocation(), Boss->GetActorLocation());
		if (DistanceSq > BestDistanceSq)
		{
			continue;
		}

		FHitResult Hit;
		FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(ExceptionLockOnTrace), false, this);
		QueryParams.AddIgnoredActor(Boss);
		const bool bBlocked = World->LineTraceSingleByChannel(Hit, TraceStart, TargetFocus, ECC_Visibility, QueryParams);
		if (bBlocked)
		{
			continue;
		}

		BestTarget = Boss;
		BestDistanceSq = DistanceSq;
	}

	return BestTarget;
}

ABRBossBase* AExceptionCharacter::FindExecutionTarget() const
{
	ABRBossBase* BestTarget = Cast<ABRBossBase>(LockOnTarget);
	if (BestTarget && BestTarget->CanBeExecuted() && GetExecutionDistanceSq2D(this, BestTarget) <= FMath::Square(ExecutionRange))
	{
		return BestTarget;
	}

	TArray<AActor*> Bosses;
	UGameplayStatics::GetAllActorsOfClass(this, ABRBossBase::StaticClass(), Bosses);

	float BestDistanceSq = FMath::Square(ExecutionRange);
	BestTarget = nullptr;
	for (AActor* Candidate : Bosses)
	{
		ABRBossBase* Boss = Cast<ABRBossBase>(Candidate);
		if (!Boss || !Boss->CanBeExecuted())
		{
			continue;
		}

		const float DistanceSq = GetExecutionDistanceSq2D(this, Boss);
		if (DistanceSq <= BestDistanceSq)
		{
			BestTarget = Boss;
			BestDistanceSq = DistanceSq;
		}
	}

	return BestTarget;
}

void AExceptionCharacter::StartExecution(ABRBossBase* Target)
{
	if (!Target || !Target->BeginExecution(this))
	{
		return;
	}

	PendingExecutionTarget = Target;
	GetWorldTimerManager().ClearTimer(StateTimerHandle);
	GetWorldTimerManager().ClearTimer(InvincibleTimerHandle);
	GetWorldTimerManager().ClearTimer(ParryTimerHandle);
	bIsInvincible = true;
	bIsParryActive = false;
	SetCombatState(EBRPlayerCombatState::Execution);
	ClearLockOn();
	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->SetMovementMode(MOVE_None);

	const FVector ToPlayer = (GetActorLocation() - Target->GetActorLocation()).GetSafeNormal2D();
	const FVector SnapDirection = ToPlayer.IsNearlyZero() ? -Target->GetActorForwardVector() : ToPlayer;
	const FVector SnapLocation = Target->GetActorLocation() + (SnapDirection * ExecutionSnapDistance);
	SetActorLocation(SnapLocation, false, nullptr, ETeleportType::TeleportPhysics);

	const FVector ToTarget = Target->GetActorLocation() - GetActorLocation();
	SetActorRotation(FRotationMatrix::MakeFromX(FVector(ToTarget.X, ToTarget.Y, 0.0f)).Rotator());
	Target->SetActorRotation(FRotationMatrix::MakeFromX(FVector(-ToTarget.X, -ToTarget.Y, 0.0f)).Rotator());

	PlayOptionalMontage(ExecutionMontage);
	BP_ExecutionStarted(Target);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(1011, 1.2f, FColor::Purple, TEXT("Player Execution"));
	}

	GetWorldTimerManager().SetTimer(ExecutionTimerHandle, this, &AExceptionCharacter::FinishExecution, ExecutionDuration, false);
}

void AExceptionCharacter::FinishExecution()
{
	ABRBossBase* Target = PendingExecutionTarget.Get();
	float AppliedDamage = 0.0f;
	if (Target && !Target->IsDead())
	{
		AppliedDamage = Target->GetMaxHP() * ExecutionDamageMaxHPRatio;
		Target->CompleteExecution(AppliedDamage, this);
	}

	PendingExecutionTarget = nullptr;
	bIsInvincible = false;
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	SetCombatState(EBRPlayerCombatState::Idle);
	BP_ExecutionFinished(Target, AppliedDamage);
}

void AExceptionCharacter::UpdateLockOn(float DeltaSeconds)
{
	if (!bIsLockedOn)
	{
		return;
	}

	ABRBossBase* Boss = Cast<ABRBossBase>(LockOnTarget);
	if (!Boss || Boss->IsDead() || CombatState == EBRPlayerCombatState::Dead)
	{
		ClearLockOn();
		return;
	}

	const float DistanceToTarget = FVector::Dist(GetActorLocation(), Boss->GetActorLocation());
	if (DistanceToTarget > LockOnBreakRange)
	{
		ClearLockOn();
		return;
	}

	const FVector TargetFocus = Boss->GetActorLocation() + FVector(0.0f, 0.0f, LockOnTargetHeightOffset);
	const FVector CameraLocation = FollowCamera ? FollowCamera->GetComponentLocation() : GetActorLocation();
	FRotator DesiredControlRotation = (TargetFocus - CameraLocation).Rotation();
	DesiredControlRotation.Yaw += LockOnYawOffset;
	DesiredControlRotation.Pitch = FMath::Clamp(DesiredControlRotation.Pitch + LockOnPitchOffset, -85.0f, 85.0f);

	if (AController* CurrentController = GetController())
	{
		const FRotator NewControlRotation = FMath::RInterpTo(CurrentController->GetControlRotation(), DesiredControlRotation, DeltaSeconds, LockOnRotationInterpSpeed);
		CurrentController->SetControlRotation(NewControlRotation);
	}

	LockOnYawOffset = FMath::FInterpTo(LockOnYawOffset, 0.0f, DeltaSeconds, LockOnOffsetReturnSpeed);
	LockOnPitchOffset = FMath::FInterpTo(LockOnPitchOffset, 0.0f, DeltaSeconds, LockOnOffsetReturnSpeed);

	const FVector ToTarget = Boss->GetActorLocation() - GetActorLocation();
	const FRotator DesiredActorRotation = FRotationMatrix::MakeFromX(FVector(ToTarget.X, ToTarget.Y, 0.0f)).Rotator();
	const FRotator NewActorRotation = FMath::RInterpTo(GetActorRotation(), DesiredActorRotation, DeltaSeconds, LockOnCharacterRotationInterpSpeed);
	SetActorRotation(NewActorRotation);
}
