#include "BRBossDummy.h"

#include "BRStatComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

ABRBossDummy::ABRBossDummy()
{
	InitialMaxHP = 300.0f;
	InitialMaxGroggy = 100.0f;
	GroggyDuration = 3.0f;

	FBRBossPatternData CloseAttack;
	CloseAttack.PatternName = TEXT("CloseSwing");
	CloseAttack.PatternType = EBRBossPatternType::Melee;
	CloseAttack.MinRange = 0.0f;
	CloseAttack.MaxRange = 280.0f;
	CloseAttack.Damage = 20.0f;
	CloseAttack.Windup = 0.65f;
	CloseAttack.Cooldown = 1.8f;
	CloseAttack.Radius = 90.0f;
	CloseAttack.ForwardOffset = 180.0f;
	CloseAttack.bEnableInPhase1 = true;
	CloseAttack.bEnableInPhase2 = true;
	AttackPatterns.Add(CloseAttack);

	FBRBossPatternData DashAttack;
	DashAttack.PatternName = TEXT("Phase2Dash");
	DashAttack.PatternType = EBRBossPatternType::Dash;
	DashAttack.MinRange = 520.0f;
	DashAttack.MaxRange = 900.0f;
	DashAttack.Damage = 24.0f;
	DashAttack.Windup = 0.55f;
	DashAttack.Cooldown = 3.0f;
	DashAttack.Radius = 110.0f;
	DashAttack.ForwardOffset = 160.0f;
	DashAttack.DashDistance = 420.0f;
	DashAttack.bEnableInPhase1 = false;
	DashAttack.bEnableInPhase2 = true;
	AttackPatterns.Add(DashAttack);

	FBRBossPatternData LongAttack;
	LongAttack.PatternName = TEXT("LongStab");
	LongAttack.PatternType = EBRBossPatternType::Melee;
	LongAttack.MinRange = 220.0f;
	LongAttack.MaxRange = 520.0f;
	LongAttack.Damage = 16.0f;
	LongAttack.Windup = 0.85f;
	LongAttack.Cooldown = 2.4f;
	LongAttack.Radius = 70.0f;
	LongAttack.ForwardOffset = 320.0f;
	LongAttack.bEnableInPhase1 = false;
	LongAttack.bEnableInPhase2 = true;
	AttackPatterns.Add(LongAttack);
}

void ABRBossDummy::ResetDummy()
{
	ResetBoss();
}

void ABRBossDummy::SetCombatAIEnabled(bool bEnabled)
{
	Super::SetCombatAIEnabled(bEnabled);
	if (!bEnabled)
	{
		ClearBaseTimers();
	}
}

void ABRBossDummy::OnBossReset()
{
	LastAttackTime = -1000.0f;
	ActivePatternIndex = INDEX_NONE;
	ClearBaseTimers();

	if (MeshComponent)
	{
		MeshComponent->SetVectorParameterValueOnMaterials(TEXT("Color"), FVector(1.0f, 1.0f, 1.0f));
	}
}

void ABRBossDummy::OnBossDeadInternal()
{
	ClearBaseTimers();
}

void ABRBossDummy::OnBossGroggyInternal()
{
	ClearBaseTimers();
}

void ABRBossDummy::OnBossRecoveredFromGroggyInternal()
{
	ActivePatternIndex = INDEX_NONE;
}

void ABRBossDummy::OnBossPhaseChanged(EBRBossPhase NewPhase)
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(2012, 2.0f, FColor::Orange, TEXT("Dummy pattern table switched to Phase 2"));
	}
}

void ABRBossDummy::UpdateBossAI(float DeltaSeconds)
{
	if (!bCombatAIEnabled || bIsDead || bIsGroggy || bIsAttacking || bIsBeingExecuted)
	{
		return;
	}

	if (!CurrentTarget)
	{
		CurrentTarget = UGameplayStatics::GetPlayerCharacter(this, 0);
	}

	if (!CurrentTarget)
	{
		return;
	}

	const float DistanceToTarget = FVector::Dist(GetActorLocation(), CurrentTarget->GetActorLocation());
	if (DistanceToTarget > DetectionRange)
	{
		return;
	}

	FaceTarget(DeltaSeconds);

	if (IsTeamMateAttacking())
	{
		MoveToTeamStandbyDistance(DeltaSeconds, DistanceToTarget);
		return;
	}

	const int32 PatternIndex = SelectPattern(DistanceToTarget);
	if (PatternIndex != INDEX_NONE)
	{
		StartBossAttack(PatternIndex);
		return;
	}

	if (TeamRole == EBRBossTeamRole::Ranged)
	{
		MoveToTeamStandbyDistance(DeltaSeconds, DistanceToTarget);
		return;
	}

	MoveTowardTarget(DeltaSeconds);
}

void ABRBossDummy::FaceTarget(float DeltaSeconds)
{
	if (!CurrentTarget)
	{
		return;
	}

	const FVector ToTarget = CurrentTarget->GetActorLocation() - GetActorLocation();
	if (ToTarget.IsNearlyZero())
	{
		return;
	}

	const FRotator TargetRotation = FRotationMatrix::MakeFromX(FVector(ToTarget.X, ToTarget.Y, 0.0f)).Rotator();
	const FRotator NewRotation = FMath::RInterpTo(GetActorRotation(), TargetRotation, DeltaSeconds, RotationInterpSpeed);
	SetActorRotation(NewRotation);
}

void ABRBossDummy::MoveTowardTarget(float DeltaSeconds)
{
	if (!CurrentTarget)
	{
		return;
	}

	const FVector ToTarget = CurrentTarget->GetActorLocation() - GetActorLocation();
	const FVector MoveDirection = FVector(ToTarget.X, ToTarget.Y, 0.0f).GetSafeNormal();
	if (MoveDirection.IsNearlyZero())
	{
		return;
	}

	AddActorWorldOffset(MoveDirection * GetCurrentMoveSpeed() * DeltaSeconds, true);
}

void ABRBossDummy::MoveToTeamStandbyDistance(float DeltaSeconds, float CurrentDistanceToTarget)
{
	if (!CurrentTarget)
	{
		return;
	}

	const float DesiredDistance = TeamRole == EBRBossTeamRole::Ranged ? RangedStandbyDistance : MeleeStandbyDistance;
	const float DistanceTolerance = 80.0f;
	if (FMath::Abs(CurrentDistanceToTarget - DesiredDistance) <= DistanceTolerance)
	{
		return;
	}

	const FVector ToTarget = CurrentTarget->GetActorLocation() - GetActorLocation();
	const FVector DirectionToTarget = FVector(ToTarget.X, ToTarget.Y, 0.0f).GetSafeNormal();
	if (DirectionToTarget.IsNearlyZero())
	{
		return;
	}

	const FVector MoveDirection = CurrentDistanceToTarget > DesiredDistance ? DirectionToTarget : -DirectionToTarget;
	AddActorWorldOffset(MoveDirection * GetCurrentMoveSpeed() * DeltaSeconds, true);
}

int32 ABRBossDummy::SelectPattern(float DistanceToTarget) const
{
	for (int32 Index = 0; Index < AttackPatterns.Num(); ++Index)
	{
		if (CanStartPattern(AttackPatterns[Index], DistanceToTarget))
		{
			return Index;
		}
	}

	return INDEX_NONE;
}

bool ABRBossDummy::CanStartPattern(const FBRBossPatternData& Pattern, float DistanceToTarget) const
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	const bool bPhaseEnabled = BossPhase == EBRBossPhase::Phase1 ? Pattern.bEnableInPhase1 : Pattern.bEnableInPhase2;
	if (!bPhaseEnabled || DistanceToTarget < Pattern.MinRange || DistanceToTarget > Pattern.MaxRange)
	{
		return false;
	}

	if (Pattern.bRequiresTeamMateNear && !IsTeamMateWithin(Pattern.TeamMateNearDistance))
	{
		return false;
	}

	return CanStartCoordinatedAttack() && World->GetTimeSeconds() - LastAttackTime >= GetPatternCooldown(Pattern);
}

float ABRBossDummy::GetPatternCooldown(const FBRBossPatternData& Pattern) const
{
	const float PhaseMultiplier = BossPhase == EBRBossPhase::Phase2 ? Phase2CooldownMultiplier : 1.0f;
	return Pattern.Cooldown * PhaseMultiplier;
}

float ABRBossDummy::GetCurrentMoveSpeed() const
{
	const float PhaseMultiplier = BossPhase == EBRBossPhase::Phase2 ? Phase2MoveSpeedMultiplier : 1.0f;
	return MoveSpeed * PhaseMultiplier;
}

void ABRBossDummy::StartBossAttack(int32 PatternIndex)
{
	if (!AttackPatterns.IsValidIndex(PatternIndex))
	{
		return;
	}

	if (!NotifyCoordinatedAttackStarted())
	{
		return;
	}

	ActivePatternIndex = PatternIndex;
	bIsAttacking = true;
	LastAttackTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;

	const FBRBossPatternData& Pattern = AttackPatterns[ActivePatternIndex];
	OnPatternStarted.Broadcast(Pattern.PatternName);
	if (GEngine)
	{
		const FString Message = FString::Printf(TEXT("Boss Pattern: %s"), *Pattern.PatternName.ToString());
		GEngine->AddOnScreenDebugMessage(2006, Pattern.Windup, FColor::Orange, Message);
	}

	GetWorldTimerManager().SetTimer(AttackWindupTimerHandle, this, &ABRBossDummy::PerformBossAttack, Pattern.Windup, false);
}

void ABRBossDummy::PerformBossAttack()
{
	bIsAttacking = false;

	if (!bCombatAIEnabled || bIsDead || bIsGroggy || bIsBeingExecuted || !CurrentTarget || !AttackPatterns.IsValidIndex(ActivePatternIndex))
	{
		ActivePatternIndex = INDEX_NONE;
		NotifyCoordinatedAttackFinished();
		return;
	}

	const FBRBossPatternData Pattern = AttackPatterns[ActivePatternIndex];
	ActivePatternIndex = INDEX_NONE;

	const FVector PreDashLocation = GetActorLocation();
	const FVector PreDashForward = GetActorForwardVector();
	if (Pattern.PatternType == EBRBossPatternType::Dash)
	{
		const FVector RawDashDirection = CurrentTarget
			? FVector(CurrentTarget->GetActorLocation() - GetActorLocation()).GetSafeNormal2D()
			: GetActorForwardVector();
		const FVector DashDirection = Pattern.bDashAwayFromTarget ? -RawDashDirection : RawDashDirection;
		const FVector FinalDashDirection = DashDirection.IsNearlyZero() ? GetActorForwardVector() : DashDirection;
		AddActorWorldOffset(FinalDashDirection * Pattern.DashDistance, true);
		FaceTarget(0.0f);
	}

	const FVector AttackCenter = Pattern.PatternType == EBRBossPatternType::AOE
		? GetActorLocation() + FVector(0.0f, 0.0f, 50.0f)
		: Pattern.bDashAwayFromTarget
			? PreDashLocation + (PreDashForward * Pattern.ForwardOffset) + FVector(0.0f, 0.0f, 50.0f)
			: GetActorLocation() + (GetActorForwardVector() * Pattern.ForwardOffset) + FVector(0.0f, 0.0f, 50.0f);
	const float DistanceToTarget = FVector::Dist(AttackCenter, CurrentTarget->GetActorLocation());
	const bool bHitTarget = DistanceToTarget <= Pattern.Radius;

	if (bDrawAttackDebug)
	{
		DrawDebugSphere(GetWorld(), AttackCenter, Pattern.Radius, 16, bHitTarget ? FColor::Red : FColor::Silver, false, 1.0f, 0, 2.0f);
	}

	if (!bHitTarget)
	{
		OnPatternFinished.Broadcast(Pattern.PatternName);
		NotifyCoordinatedAttackFinished();
		return;
	}

	UGameplayStatics::ApplyDamage(CurrentTarget, Pattern.Damage, nullptr, this, UDamageType::StaticClass());
	OnPatternHit.Broadcast(Pattern.PatternName);
	OnPatternFinished.Broadcast(Pattern.PatternName);
	NotifyCoordinatedAttackFinished();

	if (GEngine)
	{
		const FString AttackText = FString::Printf(TEXT("%s Hit! -%.0f HP"), *Pattern.PatternName.ToString(), Pattern.Damage);
		GEngine->AddOnScreenDebugMessage(2007, 1.0f, FColor::Red, AttackText);
	}
}

void ABRBossDummy::ClearBaseTimers()
{
	Super::ClearBaseTimers();
	GetWorldTimerManager().ClearTimer(AttackWindupTimerHandle);
	bIsAttacking = false;
	ActivePatternIndex = INDEX_NONE;
	NotifyCoordinatedAttackFinished();
}

void ABRBossDummy::DrawBossDebug() const
{
	if (!bShowDebug || !GEngine)
	{
		return;
	}

	const float CurrentHP = StatComponent ? StatComponent->GetCurrentHP() : 0.0f;
	const float MaxHP = StatComponent ? StatComponent->GetMaxHP() : 0.0f;
	const float CurrentGroggy = StatComponent ? StatComponent->GetCurrentGroggy() : 0.0f;
	const float MaxGroggy = StatComponent ? StatComponent->GetMaxGroggy() : 0.0f;
	const FString PatternText = AttackPatterns.IsValidIndex(ActivePatternIndex)
		? AttackPatterns[ActivePatternIndex].PatternName.ToString()
		: TEXT("None");

	const FString DebugText = FString::Printf(
		TEXT("Boss Dummy\nPhase: %s\nRole: %s\nHP: %.0f / %.0f\nGroggy: %.0f / %.0f\nAI: %s\nTeamMate Attacking: %s\nAttacking: %s\nPattern: %s\nGroggy State: %s\nExecution: %s\nDead: %s"),
		BossPhase == EBRBossPhase::Phase2 ? TEXT("Phase2") : TEXT("Phase1"),
		TeamRole == EBRBossTeamRole::Ranged ? TEXT("Ranged") : TeamRole == EBRBossTeamRole::Melee ? TEXT("Melee") : TeamRole == EBRBossTeamRole::Support ? TEXT("Support") : TEXT("Solo"),
		CurrentHP,
		MaxHP,
		CurrentGroggy,
		MaxGroggy,
		bCombatAIEnabled ? TEXT("true") : TEXT("false"),
		IsTeamMateAttacking() ? TEXT("true") : TEXT("false"),
		bIsAttacking ? TEXT("true") : TEXT("false"),
		*PatternText,
		bIsGroggy ? TEXT("true") : TEXT("false"),
		bIsBeingExecuted ? TEXT("true") : TEXT("false"),
		bIsDead ? TEXT("true") : TEXT("false"));

	GEngine->AddOnScreenDebugMessage(2001, 0.0f, FColor::Orange, DebugText);
}

FString ABRBossDummy::GetBossDebugName() const
{
	return TEXT("Boss Dummy");
}
