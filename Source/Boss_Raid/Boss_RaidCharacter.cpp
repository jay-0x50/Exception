// Copyright Epic Games, Inc. All Rights Reserved.

#include "Boss_RaidCharacter.h"
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
#include "Boss_Raid.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

ABoss_RaidCharacter::ABoss_RaidCharacter()
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
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}

void ABoss_RaidCharacter::Tick(float DeltaSeconds)
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

	DrawCombatDebug();
}

void ABoss_RaidCharacter::BeginPlay()
{
	Super::BeginPlay();

	RestoreHPAndStamina();
}

void ABoss_RaidCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ABoss_RaidCharacter::Move);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &ABoss_RaidCharacter::Look);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ABoss_RaidCharacter::Look);

		if (LightAttackAction)
		{
			EnhancedInputComponent->BindAction(LightAttackAction, ETriggerEvent::Started, this, &ABoss_RaidCharacter::LightAttackPressed);
		}

		if (HeavyAttackAction)
		{
			EnhancedInputComponent->BindAction(HeavyAttackAction, ETriggerEvent::Started, this, &ABoss_RaidCharacter::HeavyAttackPressed);
		}

		if (DodgeAction)
		{
			EnhancedInputComponent->BindAction(DodgeAction, ETriggerEvent::Started, this, &ABoss_RaidCharacter::DodgePressed);
		}

		if (ParryAction)
		{
			EnhancedInputComponent->BindAction(ParryAction, ETriggerEvent::Started, this, &ABoss_RaidCharacter::ParryPressed);
		}

		if (InteractAction)
		{
			EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &ABoss_RaidCharacter::InteractPressed);
		}

		SetupRuntimeCombatInput(EnhancedInputComponent);
	}
	else
	{
		UE_LOG(LogBoss_Raid, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void ABoss_RaidCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	// route the input
	DoMove(MovementVector.X, MovementVector.Y);
}

void ABoss_RaidCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// route the input
	DoLook(LookAxisVector.X, LookAxisVector.Y);
}

void ABoss_RaidCharacter::LightAttackPressed()
{
	DoLightAttack();
}

void ABoss_RaidCharacter::HeavyAttackPressed()
{
	DoHeavyAttack();
}

void ABoss_RaidCharacter::DodgePressed()
{
	DoDodge();
}

void ABoss_RaidCharacter::ParryPressed()
{
	DoParry();
}

void ABoss_RaidCharacter::InteractPressed()
{
	DoInteract();
}

void ABoss_RaidCharacter::SetupRuntimeCombatInput(UEnhancedInputComponent* EnhancedInputComponent)
{
	if (!EnhancedInputComponent || LightAttackAction || HeavyAttackAction || DodgeAction || ParryAction || InteractAction)
	{
		return;
	}

	RuntimeCombatMappingContext = NewObject<UInputMappingContext>(this, TEXT("IMC_RuntimeBossRaidCombat"));
	RuntimeLightAttackAction = NewObject<UInputAction>(this, TEXT("IA_RuntimeLightAttack"));
	RuntimeHeavyAttackAction = NewObject<UInputAction>(this, TEXT("IA_RuntimeHeavyAttack"));
	RuntimeDodgeAction = NewObject<UInputAction>(this, TEXT("IA_RuntimeDodge"));
	RuntimeParryAction = NewObject<UInputAction>(this, TEXT("IA_RuntimeParry"));
	RuntimeInteractAction = NewObject<UInputAction>(this, TEXT("IA_RuntimeInteract"));

	RuntimeLightAttackAction->ValueType = EInputActionValueType::Boolean;
	RuntimeHeavyAttackAction->ValueType = EInputActionValueType::Boolean;
	RuntimeDodgeAction->ValueType = EInputActionValueType::Boolean;
	RuntimeParryAction->ValueType = EInputActionValueType::Boolean;
	RuntimeInteractAction->ValueType = EInputActionValueType::Boolean;

	RuntimeCombatMappingContext->MapKey(RuntimeLightAttackAction, EKeys::LeftMouseButton);
	RuntimeCombatMappingContext->MapKey(RuntimeHeavyAttackAction, EKeys::RightMouseButton);
	RuntimeCombatMappingContext->MapKey(RuntimeDodgeAction, EKeys::LeftShift);
	RuntimeCombatMappingContext->MapKey(RuntimeParryAction, EKeys::F);
	RuntimeCombatMappingContext->MapKey(RuntimeInteractAction, EKeys::E);

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

	EnhancedInputComponent->BindAction(RuntimeLightAttackAction, ETriggerEvent::Started, this, &ABoss_RaidCharacter::LightAttackPressed);
	EnhancedInputComponent->BindAction(RuntimeHeavyAttackAction, ETriggerEvent::Started, this, &ABoss_RaidCharacter::HeavyAttackPressed);
	EnhancedInputComponent->BindAction(RuntimeDodgeAction, ETriggerEvent::Started, this, &ABoss_RaidCharacter::DodgePressed);
	EnhancedInputComponent->BindAction(RuntimeParryAction, ETriggerEvent::Started, this, &ABoss_RaidCharacter::ParryPressed);
	EnhancedInputComponent->BindAction(RuntimeInteractAction, ETriggerEvent::Started, this, &ABoss_RaidCharacter::InteractPressed);
}

void ABoss_RaidCharacter::DoMove(float Right, float Forward)
{
	if (CombatState == EBRPlayerCombatState::Dead)
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

void ABoss_RaidCharacter::DoLook(float Yaw, float Pitch)
{
	if (GetController() != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void ABoss_RaidCharacter::DoJumpStart()
{
	if (CombatState != EBRPlayerCombatState::Idle)
	{
		return;
	}

	// signal the character to jump
	Jump();
}

void ABoss_RaidCharacter::DoJumpEnd()
{
	// signal the character to stop jumping
	StopJumping();
}

bool ABoss_RaidCharacter::DoLightAttack()
{
	if (!CanStartCombatAction() || !SpendStamina(LightAttackStaminaCost))
	{
		return false;
	}

	SetCombatState(EBRPlayerCombatState::LightAttack);
	PlayOptionalMontage(LightAttackMontage);
	PerformAttackTrace(LightAttackDamage);
	UE_LOG(LogTemplateCharacter, Log, TEXT("LightAttack: Damage=%.1f, HitCount=%d"), LightAttackDamage, LastAttackHitCount);

	GetWorldTimerManager().SetTimer(StateTimerHandle, this, &ABoss_RaidCharacter::FinishCombatAction, LightAttackDuration, false);
	return true;
}

bool ABoss_RaidCharacter::DoHeavyAttack()
{
	if (!CanStartCombatAction() || !SpendStamina(HeavyAttackStaminaCost))
	{
		return false;
	}

	SetCombatState(EBRPlayerCombatState::HeavyAttack);
	PlayOptionalMontage(HeavyAttackMontage);
	PerformAttackTrace(HeavyAttackDamage);
	UE_LOG(LogTemplateCharacter, Log, TEXT("HeavyAttack: Damage=%.1f, HitCount=%d"), HeavyAttackDamage, LastAttackHitCount);

	GetWorldTimerManager().SetTimer(StateTimerHandle, this, &ABoss_RaidCharacter::FinishCombatAction, HeavyAttackDuration, false);
	return true;
}

bool ABoss_RaidCharacter::DoDodge()
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

	GetWorldTimerManager().SetTimer(InvincibleTimerHandle, this, &ABoss_RaidCharacter::EndInvincibility, DodgeInvincibleDuration, false);
	GetWorldTimerManager().SetTimer(StateTimerHandle, this, &ABoss_RaidCharacter::FinishCombatAction, DodgeDuration, false);
	return true;
}

bool ABoss_RaidCharacter::DoParry()
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

	GetWorldTimerManager().SetTimer(ParryTimerHandle, this, &ABoss_RaidCharacter::EndParryWindow, ParryActiveDuration, false);
	GetWorldTimerManager().SetTimer(StateTimerHandle, this, &ABoss_RaidCharacter::FinishCombatAction, ParryDuration, false);
	return true;
}

void ABoss_RaidCharacter::DoInteract()
{
	UE_LOG(LogTemplateCharacter, Log, TEXT("Interact pressed. Groggy execution target check will be connected with the boss implementation."));
}

void ABoss_RaidCharacter::PerformAttackTrace(float Damage)
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

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(BossRaidPlayerAttackTrace), false, this);
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
		UGameplayStatics::ApplyDamage(HitActor, Damage, GetController(), this, UDamageType::StaticClass());
		BP_AttackHit(HitActor, Damage);
		++LastAttackHitCount;

		if (bDrawAttackTraceDebug)
		{
			DrawDebugSphere(World, Hit.ImpactPoint, 18.0f, 12, FColor::Yellow, false, 1.0f);
		}
	}
}

bool ABoss_RaidCharacter::SpendStamina(float Amount)
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

void ABoss_RaidCharacter::RestoreHPAndStamina()
{
	CurrentHP = MaxHP;
	CurrentStamina = MaxStamina;
	SetCombatState(EBRPlayerCombatState::Idle);
	bIsInvincible = false;
	bIsParryActive = false;
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	BroadcastHP();
	BroadcastStamina();
}

float ABoss_RaidCharacter::TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (CombatState == EBRPlayerCombatState::Dead || bIsInvincible || Damage <= 0.0f)
	{
		return 0.0f;
	}

	CurrentHP = FMath::Max(0.0f, CurrentHP - Damage);
	BroadcastHP();
	BP_DamageReceived(Damage);

	if (CurrentHP <= 0.0f)
	{
		SetCombatState(EBRPlayerCombatState::Dead);
		GetCharacterMovement()->DisableMovement();
	}

	return Damage;
}

bool ABoss_RaidCharacter::CanStartCombatAction() const
{
	return CombatState == EBRPlayerCombatState::Idle && !GetCharacterMovement()->IsFalling();
}

void ABoss_RaidCharacter::SetCombatState(EBRPlayerCombatState NewState)
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

void ABoss_RaidCharacter::FinishCombatAction()
{
	if (CombatState == EBRPlayerCombatState::Dead)
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

void ABoss_RaidCharacter::EndInvincibility()
{
	bIsInvincible = false;
	GetWorldTimerManager().ClearTimer(InvincibleTimerHandle);
}

void ABoss_RaidCharacter::EndParryWindow()
{
	if (!bIsParryActive)
	{
		return;
	}

	bIsParryActive = false;
	GetWorldTimerManager().ClearTimer(ParryTimerHandle);
	BP_ParryWindowEnded();
}

void ABoss_RaidCharacter::PlayOptionalMontage(UAnimMontage* Montage)
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

void ABoss_RaidCharacter::BroadcastHP()
{
	OnHPChanged.Broadcast(CurrentHP, MaxHP, MaxHP > 0.0f ? CurrentHP / MaxHP : 0.0f);
}

void ABoss_RaidCharacter::BroadcastStamina()
{
	OnStaminaChanged.Broadcast(CurrentStamina, MaxStamina, MaxStamina > 0.0f ? CurrentStamina / MaxStamina : 0.0f);
}

void ABoss_RaidCharacter::DrawCombatDebug() const
{
	if (!bShowCombatDebug || !GEngine)
	{
		return;
	}

	const float StaminaPercent = MaxStamina > 0.0f ? CurrentStamina / MaxStamina : 0.0f;
	const float HPPercent = MaxHP > 0.0f ? CurrentHP / MaxHP : 0.0f;
	const FString DebugText = FString::Printf(
		TEXT("Player Debug\nState: %s\nHP: %.0f / %.0f (%.0f%%)\nStamina: %.0f / %.0f (%.0f%%)\nInvincible: %s\nParry Active: %s\nLast Attack Hits: %d"),
		*GetCombatStateName(),
		CurrentHP,
		MaxHP,
		HPPercent * 100.0f,
		CurrentStamina,
		MaxStamina,
		StaminaPercent * 100.0f,
		bIsInvincible ? TEXT("true") : TEXT("false"),
		bIsParryActive ? TEXT("true") : TEXT("false"),
		LastAttackHitCount);

	GEngine->AddOnScreenDebugMessage(1001, 0.0f, FColor::Cyan, DebugText);
}

FString ABoss_RaidCharacter::GetCombatStateName() const
{
	const UEnum* Enum = StaticEnum<EBRPlayerCombatState>();
	return Enum ? Enum->GetNameStringByValue(static_cast<int64>(CombatState)) : TEXT("Unknown");
}
