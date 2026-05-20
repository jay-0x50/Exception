#include "BRBossArenaTrigger.h"

#include "BRBossBase.h"
#include "BRBossStatusWidget.h"
#include "Boss_RaidCharacter.h"
#include "Boss_RaidGameMode.h"
#include "Boss_RaidPlayerController.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"
#include "Kismet/GameplayStatics.h"
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
			Boss->OnBossHPChanged.AddDynamic(this, &ABRBossArenaTrigger::HandleBossStatChanged);
			Boss->OnBossGroggyChanged.AddDynamic(this, &ABRBossArenaTrigger::HandleBossStatChanged);
			Boss->OnBossGroggy.AddDynamic(this, &ABRBossArenaTrigger::HandleBossStateChanged);
			Boss->OnBossRecoveredFromGroggy.AddDynamic(this, &ABRBossArenaTrigger::HandleBossStateChanged);
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

	if (UBRBossStatusWidget* ActiveBossStatusWidget = ShowBossStatusWidget())
	{
		ActiveBossStatusWidget->ClearBosses();
		ActiveBossStatusWidget->SetBossCount(ManagedBosses.Num());
	}

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

	RefreshBossStatusWidget();

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
	HideBossStatusWidget();

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

	RefreshBossStatusWidget();

	if (!AreAllManagedBossesDead())
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(4004, 2.0f, FColor::Orange, TEXT("Boss Down - Team Still Fighting"));
		}
		return;
	}

	bArenaCleared = true;
	HideBossStatusWidget();

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

void ABRBossArenaTrigger::HandleBossStatChanged(float CurrentValue, float MaxValue, float NormalizedValue)
{
	RefreshBossStatusWidget();
}

void ABRBossArenaTrigger::HandleBossStateChanged()
{
	RefreshBossStatusWidget();
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

UBRBossStatusWidget* ABRBossArenaTrigger::ShowBossStatusWidget()
{
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	if (!PlayerController)
	{
		return nullptr;
	}

	if (ABoss_RaidPlayerController* BossRaidPC = Cast<ABoss_RaidPlayerController>(PlayerController))
	{
		if (UBRBossStatusWidget* PlayerControllerWidget = BossRaidPC->ShowBossStatusWidget())
		{
			return PlayerControllerWidget;
		}
	}

	if (!BossStatusWidget)
	{
		BossStatusWidget = CreateWidget<UBRBossStatusWidget>(PlayerController, UBRBossStatusWidget::StaticClass());
	}

	if (BossStatusWidget && !BossStatusWidget->IsInViewport())
	{
		BossStatusWidget->AddToPlayerScreen(10);
		BossStatusWidget->SetAlignmentInViewport(FVector2D(0.0f, 0.0f));
		BossStatusWidget->SetPositionInViewport(FVector2D(40.0f, 32.0f), false);
		BossStatusWidget->SetDesiredSizeInViewport(FVector2D(760.0f, 180.0f));
	}

	return BossStatusWidget;
}

void ABRBossArenaTrigger::RefreshBossStatusWidget()
{
	if (!bArenaStarted || bArenaCleared)
	{
		return;
	}

	UBRBossStatusWidget* ActiveBossStatusWidget = ShowBossStatusWidget();
	if (!ActiveBossStatusWidget)
	{
		return;
	}

	TArray<ABRBossBase*> ManagedBosses;
	BuildManagedBossList(ManagedBosses);
	ActiveBossStatusWidget->SetBossCount(ManagedBosses.Num());

	for (int32 BossIndex = 0; BossIndex < ManagedBosses.Num(); ++BossIndex)
	{
		const ABRBossBase* Boss = ManagedBosses[BossIndex];
		if (!Boss)
		{
			continue;
		}

		ActiveBossStatusWidget->SetBossHP(
			BossIndex,
			Boss->GetBossDisplayName(),
			Boss->GetCurrentHP(),
			Boss->GetMaxHP(),
			Boss->GetHPPercent());

		ActiveBossStatusWidget->SetBossGroggy(
			BossIndex,
			Boss->GetCurrentGroggy(),
			Boss->GetMaxGroggy(),
			Boss->GetGroggyPercent());

		ActiveBossStatusWidget->SetBossGroggyState(BossIndex, Boss->IsGroggy());
	}
}

void ABRBossArenaTrigger::HideBossStatusWidget()
{
	if (ABoss_RaidPlayerController* BossRaidPC = Cast<ABoss_RaidPlayerController>(UGameplayStatics::GetPlayerController(this, 0)))
	{
		BossRaidPC->HideBossStatusWidget();
	}

	if (BossStatusWidget)
	{
		BossStatusWidget->RemoveFromParent();
		BossStatusWidget->ClearBosses();
	}
}
