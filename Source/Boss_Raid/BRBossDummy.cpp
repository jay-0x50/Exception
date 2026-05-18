#include "BRBossDummy.h"

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
}

void ABRBossDummy::BeginPlay()
{
	Super::BeginPlay();

	ResetDummy();
}

void ABRBossDummy::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	DrawDummyDebug();
}

float ABRBossDummy::TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (bIsDead || Damage <= 0.0f)
	{
		return 0.0f;
	}

	CurrentHP = FMath::Max(0.0f, CurrentHP - Damage);
	CurrentGroggy = FMath::Min(MaxGroggy, CurrentGroggy + (Damage * GroggyDamageMultiplier));

	UE_LOG(LogTemp, Log, TEXT("BossDummy hit: Damage=%.1f, HP=%.1f/%.1f, Groggy=%.1f/%.1f"),
		Damage,
		CurrentHP,
		MaxHP,
		CurrentGroggy,
		MaxGroggy);

	if (GEngine)
	{
		const FString HitText = FString::Printf(TEXT("Boss Dummy Hit! -%.0f HP"), Damage);
		GEngine->AddOnScreenDebugMessage(2002, 1.0f, FColor::Yellow, HitText);
	}

	if (CurrentGroggy >= MaxGroggy && !bIsGroggy)
	{
		SetGroggy();
	}

	if (CurrentHP <= 0.0f)
	{
		SetDead();
	}

	return Damage;
}

void ABRBossDummy::ResetDummy()
{
	CurrentHP = MaxHP;
	CurrentGroggy = 0.0f;
	bIsDead = false;
	bIsGroggy = false;

	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);

	if (MeshComponent)
	{
		MeshComponent->SetVectorParameterValueOnMaterials(TEXT("Color"), FVector(1.0f, 1.0f, 1.0f));
	}
}

void ABRBossDummy::SetDead()
{
	bIsDead = true;
	SetActorEnableCollision(false);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(2003, 2.0f, FColor::Red, TEXT("Boss Dummy Dead"));
	}
}

void ABRBossDummy::SetGroggy()
{
	bIsGroggy = true;

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
