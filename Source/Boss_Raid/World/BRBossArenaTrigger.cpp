#include "BRBossArenaTrigger.h"

#include "BRBossBase.h"
#include "Boss_RaidCharacter.h"
#include "Boss_RaidGameMode.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

ABRBossArenaTrigger::ABRBossArenaTrigger()
{
	PrimaryActorTick.bCanEverTick = false;

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	SetRootComponent(TriggerBox);
	TriggerBox->SetBoxExtent(FVector(200.0f, 200.0f, 120.0f));
	TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerBox->SetCollisionObjectType(ECC_WorldDynamic);
	TriggerBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	PreviewMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PreviewMesh"));
	PreviewMesh->SetupAttachment(RootComponent);
	PreviewMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PreviewMesh->SetWorldScale3D(FVector(4.0f, 4.0f, 0.05f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		PreviewMesh->SetStaticMesh(CubeMesh.Object);
	}
}

void ABRBossArenaTrigger::BeginPlay()
{
	Super::BeginPlay();

	if (TriggerBox)
	{
		TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &ABRBossArenaTrigger::OnTriggerBeginOverlap);
	}

	TArray<ABRBossBase*> ManagedBosses;
	BuildManagedBossList(ManagedBosses);
	for (ABRBossBase* Boss : ManagedBosses)
	{
		if (Boss)
		{
			Boss->OnBossDead.AddDynamic(this, &ABRBossArenaTrigger::HandleBossDefeated);
		}
	}

	if (RewardActorToShowOnDefeat)
	{
		RewardActorToShowOnDefeat->SetActorHiddenInGame(true);
		RewardActorToShowOnDefeat->SetActorEnableCollision(false);
	}
}

void ABRBossArenaTrigger::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (bArenaStarted || bArenaCleared || !Cast<ABoss_RaidCharacter>(OtherActor))
	{
		return;
	}

	StartArena();
}

void ABRBossArenaTrigger::StartArena()
{
	bArenaStarted = true;

	if (ABoss_RaidGameMode* BossRaidGameMode = GetWorld() ? GetWorld()->GetAuthGameMode<ABoss_RaidGameMode>() : nullptr)
	{
		BossRaidGameMode->SetActiveBossArena(this);
	}

	TArray<ABRBossBase*> ManagedBosses;
	BuildManagedBossList(ManagedBosses);
	for (ABRBossBase* Boss : ManagedBosses)
	{
		if (!Boss)
		{
			continue;
		}

		if (bResetBossOnEnter)
		{
			Boss->ResetBoss();
		}

		Boss->SetCombatAIEnabled(true);
	}

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(4001, 2.0f, FColor::Red, TEXT("Boss Arena Started"));
	}
}

void ABRBossArenaTrigger::ResetArenaForRetry()
{
	if (bArenaCleared)
	{
		return;
	}

	bArenaStarted = false;

	TArray<ABRBossBase*> ManagedBosses;
	BuildManagedBossList(ManagedBosses);
	for (ABRBossBase* Boss : ManagedBosses)
	{
		if (Boss)
		{
			Boss->SetCombatAIEnabled(false);
			Boss->ResetBoss();
		}
	}

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(4003, 2.0f, FColor::Silver, TEXT("Boss Arena Reset For Retry"));
	}
}

void ABRBossArenaTrigger::HandleBossDefeated()
{
	if (bArenaCleared)
	{
		return;
	}

	if (!AreAllManagedBossesDead())
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(4004, 2.0f, FColor::Orange, TEXT("Boss Down - Team Still Fighting"));
		}
		return;
	}

	bArenaCleared = true;

	TArray<ABRBossBase*> ManagedBosses;
	BuildManagedBossList(ManagedBosses);
	for (ABRBossBase* Boss : ManagedBosses)
	{
		if (Boss)
		{
			Boss->SetCombatAIEnabled(false);
		}
	}

	if (GateActorToHideOnDefeat)
	{
		GateActorToHideOnDefeat->SetActorHiddenInGame(true);
		GateActorToHideOnDefeat->SetActorEnableCollision(false);
	}

	if (RewardActorToShowOnDefeat)
	{
		RewardActorToShowOnDefeat->SetActorHiddenInGame(false);
		RewardActorToShowOnDefeat->SetActorEnableCollision(true);
	}

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(4002, 3.0f, FColor::Green, TEXT("Boss Defeated - Path Opened"));
	}
}

void ABRBossArenaTrigger::BuildManagedBossList(TArray<ABRBossBase*>& OutBosses) const
{
	OutBosses.Reset();

	for (ABRBossBase* Boss : BossActors)
	{
		if (Boss)
		{
			OutBosses.AddUnique(Boss);
		}
	}

	if (BossDummy)
	{
		OutBosses.AddUnique(BossDummy);
	}
}

bool ABRBossArenaTrigger::AreAllManagedBossesDead() const
{
	TArray<ABRBossBase*> ManagedBosses;
	BuildManagedBossList(ManagedBosses);

	if (ManagedBosses.IsEmpty())
	{
		return false;
	}

	for (const ABRBossBase* Boss : ManagedBosses)
	{
		if (Boss && !Boss->IsDead())
		{
			return false;
		}
	}

	return true;
}
