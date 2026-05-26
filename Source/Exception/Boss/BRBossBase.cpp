#include "BRBossBase.h"

#include "BRBossTeamCoordinator.h"
#include "BRStatComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

ABRBossBase::ABRBossBase()
{
	PrimaryActorTick.bCanEverTick = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(SceneRoot);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshComponent->SetGenerateOverlapEvents(false);
	MeshComponent->SetWorldScale3D(FVector(1.5f, 1.5f, 2.5f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		MeshComponent->SetStaticMesh(CubeMesh.Object);
	}

	StatComponent = CreateDefaultSubobject<UBRStatComponent>(TEXT("StatComponent"));
}

void ABRBossBase::BeginPlay()
{
	Super::BeginPlay();

	if (StatComponent)
	{
		StatComponent->ConfigureMaxStats(InitialMaxHP, 0.0f, InitialMaxGroggy);
		StatComponent->OnHPChanged.AddDynamic(this, &ABRBossBase::HandleHPChanged);
		StatComponent->OnGroggyChanged.AddDynamic(this, &ABRBossBase::HandleGroggyChanged);
		StatComponent->OnDead.AddDynamic(this, &ABRBossBase::HandleDead);
		StatComponent->OnGroggy.AddDynamic(this, &ABRBossBase::HandleGroggy);
	}

	if (TeamCoordinator)
	{
		TeamCoordinator->RegisterBoss(this);
	}

	ResetBoss();
}

void ABRBossBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (TeamCoordinator)
	{
		TeamCoordinator->UnregisterBoss(this);
	}

	Super::EndPlay(EndPlayReason);
}

void ABRBossBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UpdateBossAI(DeltaSeconds);
	DrawBossDebug();
}

float ABRBossBase::TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	const float GroggyDamage = Damage * GroggyDamageMultiplier;
	return ReceiveCombatHit_Implementation(Damage, GroggyDamage, DamageCauser) ? Damage : 0.0f;
}

bool ABRBossBase::ReceiveCombatHit_Implementation(float Damage, float GroggyDamage, AActor* DamageCauser)
{
	if (!StatComponent || bIsDead || Damage <= 0.0f)
	{
		return false;
	}

	const bool bApplied = StatComponent->ApplyDamageToStats(Damage, GroggyDamage);
	if (!bApplied)
	{
		return false;
	}

	RefreshPhaseByHP();

	UE_LOG(LogTemp, Log, TEXT("%s hit: Damage=%.1f, GroggyDamage=%.1f, HP=%.1f/%.1f, Groggy=%.1f/%.1f"),
		*GetBossDebugName(),
		Damage,
		GroggyDamage,
		StatComponent->GetCurrentHP(),
		StatComponent->GetMaxHP(),
		StatComponent->GetCurrentGroggy(),
		StatComponent->GetMaxGroggy());

	if (GEngine)
	{
		const FString HitText = FString::Printf(TEXT("%s Hit! -%.0f HP / +%.0f Groggy"), *GetBossDebugName(), Damage, GroggyDamage);
		GEngine->AddOnScreenDebugMessage(2002, 1.0f, FColor::Yellow, HitText);
	}

	return true;
}

void ABRBossBase::ResetBoss()
{
	bIsDead = false;
	bIsGroggy = false;
	bIsAttacking = false;
	bIsBeingExecuted = false;
	BossPhase = EBRBossPhase::Phase1;
	ClearBaseTimers();

	if (StatComponent)
	{
		StatComponent->ConfigureMaxStats(InitialMaxHP, 0.0f, InitialMaxGroggy);
	}

	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);

	OnBossReset();
}

void ABRBossBase::SetCombatAIEnabled(bool bEnabled)
{
	bCombatAIEnabled = bEnabled;
	bIsAttacking = false;

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			2005,
			1.5f,
			bCombatAIEnabled ? FColor::Red : FColor::Silver,
			bCombatAIEnabled ? TEXT("Boss AI Enabled") : TEXT("Boss AI Disabled"));
	}
}

float ABRBossBase::GetMaxHP() const
{
	return StatComponent ? StatComponent->GetMaxHP() : 0.0f;
}

float ABRBossBase::GetCurrentHP() const
{
	return StatComponent ? StatComponent->GetCurrentHP() : 0.0f;
}

float ABRBossBase::GetHPPercent() const
{
	const float MaxHP = GetMaxHP();
	return MaxHP > 0.0f ? GetCurrentHP() / MaxHP : 0.0f;
}

float ABRBossBase::GetMaxGroggy() const
{
	return StatComponent ? StatComponent->GetMaxGroggy() : 0.0f;
}

float ABRBossBase::GetCurrentGroggy() const
{
	return StatComponent ? StatComponent->GetCurrentGroggy() : 0.0f;
}

float ABRBossBase::GetGroggyPercent() const
{
	const float MaxGroggy = GetMaxGroggy();
	return MaxGroggy > 0.0f ? GetCurrentGroggy() / MaxGroggy : 0.0f;
}

FText ABRBossBase::GetBossDisplayName() const
{
	return FText::FromString(GetBossDebugName());
}

bool ABRBossBase::ApplyGroggyDamage(float GroggyDamage, AActor* DamageCauser)
{
	if (!StatComponent || bIsDead || bIsGroggy || GroggyDamage <= 0.0f)
	{
		return false;
	}

	const bool bApplied = StatComponent->ApplyDamageToStats(0.0f, GroggyDamage);
	if (!bApplied)
	{
		return false;
	}

	if (GEngine)
	{
		const FString GroggyText = FString::Printf(TEXT("%s Parried! +%.0f Groggy"), *GetBossDebugName(), GroggyDamage);
		GEngine->AddOnScreenDebugMessage(2013, 1.2f, FColor::Cyan, GroggyText);
	}

	return true;
}

bool ABRBossBase::IsTeamMateAttacking() const
{
	return TeamCoordinator && TeamCoordinator->IsOtherBossAttacking(const_cast<ABRBossBase*>(this));
}

bool ABRBossBase::IsTeamMateWithin(float Distance) const
{
	return TeamCoordinator && TeamCoordinator->IsOtherBossWithin(const_cast<ABRBossBase*>(this), Distance);
}

void ABRBossBase::SetTeamCoordinator(ABRBossTeamCoordinator* NewTeamCoordinator)
{
	if (TeamCoordinator == NewTeamCoordinator)
	{
		if (TeamCoordinator)
		{
			TeamCoordinator->RegisterBoss(this);
		}
		return;
	}

	if (TeamCoordinator)
	{
		TeamCoordinator->UnregisterBoss(this);
	}

	TeamCoordinator = NewTeamCoordinator;
	if (TeamCoordinator)
	{
		TeamCoordinator->RegisterBoss(this);
	}
}

void ABRBossBase::ApplyTeamSlot(int32 TeamSlotIndex)
{
}

bool ABRBossBase::CanBeExecuted() const
{
	return bIsGroggy && !bIsDead && !bIsBeingExecuted;
}

bool ABRBossBase::BeginExecution(AActor* Executor)
{
	if (!CanBeExecuted())
	{
		return false;
	}

	bIsBeingExecuted = true;
	bIsAttacking = false;
	ClearBaseTimers();
	OnExecutionStarted.Broadcast(Executor);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(2009, 1.5f, FColor::Purple, TEXT("Execution Started"));
	}

	return true;
}

bool ABRBossBase::CompleteExecution(float Damage, AActor* Executor)
{
	if (!bIsBeingExecuted || bIsDead || !StatComponent)
	{
		return false;
	}

	bIsBeingExecuted = false;
	const bool bApplied = StatComponent->ApplyDamageToStats(Damage, 0.0f);
	RefreshPhaseByHP();

	if (!bIsDead)
	{
		bIsGroggy = false;
		StatComponent->ResetGroggy();
	}

	OnExecutionCompleted.Broadcast(Executor);

	if (GEngine)
	{
		const FString ExecutionText = FString::Printf(TEXT("Execution Hit! -%.0f HP"), Damage);
		GEngine->AddOnScreenDebugMessage(2010, 1.5f, FColor::Purple, ExecutionText);
	}

	return bApplied;
}

void ABRBossBase::HandleDead()
{
	bIsDead = true;
	bCombatAIEnabled = false;
	bIsAttacking = false;
	bIsBeingExecuted = false;
	ClearBaseTimers();
	NotifyCoordinatedAttackFinished();
	SetActorEnableCollision(false);
	OnBossDead.Broadcast();
	OnBossDeadInternal();

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(2003, 2.0f, FColor::Red, TEXT("Boss Dead"));
	}
}

void ABRBossBase::HandleGroggy()
{
	bIsGroggy = true;
	bIsAttacking = false;
	ClearBaseTimers();
	OnBossGroggy.Broadcast();
	OnBossGroggyInternal();
	GetWorldTimerManager().SetTimer(GroggyTimerHandle, this, &ABRBossBase::RecoverFromGroggy, GroggyDuration, false);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(2004, 2.0f, FColor::Orange, TEXT("Boss Groggy"));
	}
}

void ABRBossBase::HandleHPChanged(float CurrentValue, float MaxValue, float NormalizedValue)
{
	OnBossHPChanged.Broadcast(CurrentValue, MaxValue, NormalizedValue);
}

void ABRBossBase::HandleGroggyChanged(float CurrentValue, float MaxValue, float NormalizedValue)
{
	OnBossGroggyChanged.Broadcast(CurrentValue, MaxValue, NormalizedValue);
}

void ABRBossBase::RecoverFromGroggy()
{
	if (bIsDead)
	{
		return;
	}

	if (bIsBeingExecuted)
	{
		GetWorldTimerManager().SetTimer(GroggyTimerHandle, this, &ABRBossBase::RecoverFromGroggy, 0.1f, false);
		return;
	}

	bIsGroggy = false;

	if (StatComponent)
	{
		StatComponent->ResetGroggy();
	}

	OnBossRecoveredFromGroggy.Broadcast();
	OnBossRecoveredFromGroggyInternal();

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(2008, 1.5f, FColor::Silver, TEXT("Boss Recovered From Groggy"));
	}
}

void ABRBossBase::RefreshPhaseByHP()
{
	if (BossPhase == EBRBossPhase::Phase2 || bIsDead || Phase2StartHPRatio <= 0.0f)
	{
		return;
	}

	if (GetHPPercent() <= Phase2StartHPRatio)
	{
		BossPhase = EBRBossPhase::Phase2;
		OnPhaseChanged.Broadcast(BossPhase);
		OnBossPhaseChanged(BossPhase);

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(2011, 2.0f, FColor::Orange, TEXT("Boss Phase 2"));
		}
	}
}

bool ABRBossBase::CanStartCoordinatedAttack() const
{
	return !TeamCoordinator || TeamCoordinator->CanStartAttack(const_cast<ABRBossBase*>(this));
}

bool ABRBossBase::NotifyCoordinatedAttackStarted()
{
	return !TeamCoordinator || TeamCoordinator->NotifyAttackStarted(this);
}

void ABRBossBase::NotifyCoordinatedAttackFinished()
{
	if (TeamCoordinator)
	{
		TeamCoordinator->NotifyAttackFinished(this);
	}
}

void ABRBossBase::ClearBaseTimers()
{
	GetWorldTimerManager().ClearTimer(GroggyTimerHandle);
}

void ABRBossBase::OnBossReset()
{
}

void ABRBossBase::OnBossDeadInternal()
{
}

void ABRBossBase::OnBossGroggyInternal()
{
}

void ABRBossBase::OnBossRecoveredFromGroggyInternal()
{
}

void ABRBossBase::OnBossPhaseChanged(EBRBossPhase NewPhase)
{
}

void ABRBossBase::UpdateBossAI(float DeltaSeconds)
{
}

void ABRBossBase::DrawBossDebug() const
{
}

FString ABRBossBase::GetBossDebugName() const
{
	return GetName();
}
