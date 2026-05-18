#include "BRBossDummy.h"

#include "BRStatComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"
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

void ABRBossDummy::HandleDead()
{
	bIsDead = true;
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
	OnDummyGroggy.Broadcast();

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(2004, 2.0f, FColor::Orange, TEXT("Boss Dummy Groggy"));
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
		TEXT("Boss Dummy\nHP: %.0f / %.0f\nGroggy: %.0f / %.0f\nGroggy State: %s\nDead: %s"),
		CurrentHP,
		MaxHP,
		CurrentGroggy,
		MaxGroggy,
		bIsGroggy ? TEXT("true") : TEXT("false"),
		bIsDead ? TEXT("true") : TEXT("false"));

	GEngine->AddOnScreenDebugMessage(2001, 0.0f, FColor::Orange, DebugText);
}
