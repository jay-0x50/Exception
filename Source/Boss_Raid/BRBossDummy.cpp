#include "BRBossDummy.h"

#include "BRStatComponent.h"
#include "Components/StaticMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

ABRBossDummy::ABRBossDummy()
{
	PrimaryActorTick.bCanEverTick = true;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	SetRootComponent(MeshComponent);

	MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	MeshComponent->SetCollisionObjectType(ECC_WorldDynamic);
	MeshComponent->SetCollisionResponseToAllChannels(ECR_Block);
	MeshComponent->SetGenerateOverlapEvents(false);
	MeshComponent->SetWorldScale3D(FVector(1.5f, 1.5f, 2.5f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		MeshComponent->SetStaticMesh(CubeMesh.Object);
	}

	StatComponent = CreateDefaultSubobject<UBRStatComponent>(TEXT("StatComponent"));
	StatComponent->ConfigureMaxStats(300.0f, 0.0f, 100.0f);
}

void ABRBossDummy::BeginPlay()
{
	Super::BeginPlay();

	if (StatComponent)
	{
		StatComponent->OnDead.AddDynamic(this, &ABRBossDummy::HandleDead);
		StatComponent->OnGroggy.AddDynamic(this, &ABRBossDummy::HandleGroggy);
	}

	ResetDummy();
}

void ABRBossDummy::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UpdateCombatAI(DeltaSeconds);
	DrawDummyDebug();
}

float ABRBossDummy::TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	const float GroggyDamage = Damage * GroggyDamageMultiplier;
	return ReceiveCombatHit_Implementation(Damage, GroggyDamage, DamageCauser) ? Damage : 0.0f;
}

bool ABRBossDummy::ReceiveCombatHit_Implementation(float Damage, float GroggyDamage, AActor* DamageCauser)
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

	UE_LOG(LogTemp, Log, TEXT("BossDummy hit: Damage=%.1f, GroggyDamage=%.1f, HP=%.1f/%.1f, Groggy=%.1f/%.1f"),
		Damage,
		GroggyDamage,
		StatComponent->GetCurrentHP(),
		StatComponent->GetMaxHP(),
		StatComponent->GetCurrentGroggy(),
		StatComponent->GetMaxGroggy());

	if (GEngine)
	{
		const FString HitText = FString::Printf(TEXT("Boss Dummy Hit! -%.0f HP / +%.0f Groggy"), Damage, GroggyDamage);
		GEngine->AddOnScreenDebugMessage(2002, 1.0f, FColor::Yellow, HitText);
	}

	return true;
}

void ABRBossDummy::ResetDummy()
{
	bIsDead = false;
	bIsGroggy = false;
	bIsAttacking = false;
	LastAttackTime = -1000.0f;
	GetWorldTimerManager().ClearTimer(AttackWindupTimerHandle);
	GetWorldTimerManager().ClearTimer(GroggyTimerHandle);

	if (StatComponent)
	{
		StatComponent->InitializeStats();
	}

	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);

	if (MeshComponent)
	{
		MeshComponent->SetVectorParameterValueOnMaterials(TEXT("Color"), FVector(1.0f, 1.0f, 1.0f));
	}
}

void ABRBossDummy::SetCombatAIEnabled(bool bEnabled)
{
	bCombatAIEnabled = bEnabled;
	bIsAttacking = false;
	GetWorldTimerManager().ClearTimer(AttackWindupTimerHandle);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			2005,
			1.5f,
			bCombatAIEnabled ? FColor::Red : FColor::Silver,
			bCombatAIEnabled ? TEXT("Boss AI Enabled") : TEXT("Boss AI Disabled"));
	}
}

void ABRBossDummy::HandleDead()
{
	bIsDead = true;
	bCombatAIEnabled = false;
	bIsAttacking = false;
	GetWorldTimerManager().ClearTimer(AttackWindupTimerHandle);
	GetWorldTimerManager().ClearTimer(GroggyTimerHandle);
	SetActorEnableCollision(false);
	OnDummyDead.Broadcast();

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(2003, 2.0f, FColor::Red, TEXT("Boss Dummy Dead"));
	}
}

void ABRBossDummy::HandleGroggy()
{
	bIsGroggy = true;
	bIsAttacking = false;
	GetWorldTimerManager().ClearTimer(AttackWindupTimerHandle);
	GetWorldTimerManager().SetTimer(GroggyTimerHandle, this, &ABRBossDummy::RecoverFromGroggy, GroggyDuration, false);
	OnDummyGroggy.Broadcast();

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(2004, 2.0f, FColor::Orange, TEXT("Boss Dummy Groggy"));
	}
}

void ABRBossDummy::RecoverFromGroggy()
{
	if (bIsDead)
	{
		return;
	}

	bIsGroggy = false;

	if (StatComponent)
	{
		StatComponent->ResetGroggy();
	}

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(2008, 1.5f, FColor::Silver, TEXT("Boss Recovered From Groggy"));
	}
}

void ABRBossDummy::UpdateCombatAI(float DeltaSeconds)
{
	if (!bCombatAIEnabled || bIsDead || bIsGroggy || bIsAttacking)
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

	if (CanStartAttack(DistanceToTarget))
	{
		StartBossAttack();
		return;
	}

	if (DistanceToTarget > AttackRange)
	{
		MoveTowardTarget(DeltaSeconds);
	}
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

	AddActorWorldOffset(MoveDirection * MoveSpeed * DeltaSeconds, true);
}

bool ABRBossDummy::CanStartAttack(float DistanceToTarget) const
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	return DistanceToTarget <= AttackRange && World->GetTimeSeconds() - LastAttackTime >= BossAttackCooldown;
}

void ABRBossDummy::StartBossAttack()
{
	bIsAttacking = true;
	LastAttackTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(2006, BossAttackWindup, FColor::Orange, TEXT("Boss Attack Windup"));
	}

	GetWorldTimerManager().SetTimer(AttackWindupTimerHandle, this, &ABRBossDummy::PerformBossAttack, BossAttackWindup, false);
}

void ABRBossDummy::PerformBossAttack()
{
	bIsAttacking = false;

	if (!bCombatAIEnabled || bIsDead || !CurrentTarget)
	{
		return;
	}

	const FVector AttackCenter = GetActorLocation() + (GetActorForwardVector() * BossAttackForwardOffset) + FVector(0.0f, 0.0f, 50.0f);
	const float DistanceToTarget = FVector::Dist(AttackCenter, CurrentTarget->GetActorLocation());
	const bool bHitTarget = DistanceToTarget <= BossAttackRadius;

	if (bDrawAttackDebug)
	{
		DrawDebugSphere(GetWorld(), AttackCenter, BossAttackRadius, 16, bHitTarget ? FColor::Red : FColor::Silver, false, 1.0f, 0, 2.0f);
	}

	if (!bHitTarget)
	{
		return;
	}

	UGameplayStatics::ApplyDamage(CurrentTarget, BossAttackDamage, nullptr, this, UDamageType::StaticClass());

	if (GEngine)
	{
		const FString AttackText = FString::Printf(TEXT("Boss Attack Hit! -%.0f HP"), BossAttackDamage);
		GEngine->AddOnScreenDebugMessage(2007, 1.0f, FColor::Red, AttackText);
	}
}

void ABRBossDummy::DrawDummyDebug() const
{
	if (!bShowDebug || !GEngine)
	{
		return;
	}

	const float CurrentHP = StatComponent ? StatComponent->GetCurrentHP() : 0.0f;
	const float MaxHP = StatComponent ? StatComponent->GetMaxHP() : 0.0f;
	const float CurrentGroggy = StatComponent ? StatComponent->GetCurrentGroggy() : 0.0f;
	const float MaxGroggy = StatComponent ? StatComponent->GetMaxGroggy() : 0.0f;

	const FString DebugText = FString::Printf(
		TEXT("Boss Dummy\nHP: %.0f / %.0f\nGroggy: %.0f / %.0f\nAI: %s\nAttacking: %s\nGroggy State: %s\nDead: %s"),
		CurrentHP,
		MaxHP,
		CurrentGroggy,
		MaxGroggy,
		bCombatAIEnabled ? TEXT("true") : TEXT("false"),
		bIsAttacking ? TEXT("true") : TEXT("false"),
		bIsGroggy ? TEXT("true") : TEXT("false"),
		bIsDead ? TEXT("true") : TEXT("false"));

	GEngine->AddOnScreenDebugMessage(2001, 0.0f, FColor::Orange, DebugText);
}
