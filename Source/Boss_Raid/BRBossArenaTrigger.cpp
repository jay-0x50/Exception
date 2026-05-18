#include "BRBossArenaTrigger.h"

#include "BRBossDummy.h"
#include "Boss_RaidCharacter.h"
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

	if (BossDummy)
	{
		BossDummy->OnDummyDead.AddDynamic(this, &ABRBossArenaTrigger::HandleBossDefeated);
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

	if (BossDummy && bResetBossOnEnter)
	{
		BossDummy->ResetDummy();
	}

	if (BossDummy)
	{
		BossDummy->SetCombatAIEnabled(true);
	}

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(4001, 2.0f, FColor::Red, TEXT("Boss Arena Started"));
	}
}

void ABRBossArenaTrigger::HandleBossDefeated()
{
	if (bArenaCleared)
	{
		return;
	}

	bArenaCleared = true;

	if (BossDummy)
	{
		BossDummy->SetCombatAIEnabled(false);
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
