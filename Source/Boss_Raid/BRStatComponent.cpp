#include "BRStatComponent.h"

UBRStatComponent::UBRStatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UBRStatComponent::BeginPlay()
{
	Super::BeginPlay();

	InitializeStats();
}

void UBRStatComponent::InitializeStats()
{
	CurrentHP = MaxHP;
	CurrentStamina = MaxStamina;
	CurrentGroggy = 0.0f;
	bIsDead = false;
	bIsGroggy = false;

	BroadcastAllStats();
}

void UBRStatComponent::ConfigureMaxStats(float InMaxHP, float InMaxStamina, float InMaxGroggy)
{
	MaxHP = FMath::Max(1.0f, InMaxHP);
	MaxStamina = FMath::Max(0.0f, InMaxStamina);
	MaxGroggy = FMath::Max(0.0f, InMaxGroggy);
	InitializeStats();
}

bool UBRStatComponent::ApplyDamageToStats(float Damage, float GroggyDamage)
{
	if (bIsDead || Damage <= 0.0f)
	{
		return false;
	}

	CurrentHP = FMath::Max(0.0f, CurrentHP - Damage);
	BroadcastHP();

	if (MaxGroggy > 0.0f && GroggyDamage > 0.0f && !bIsGroggy)
	{
		CurrentGroggy = FMath::Min(MaxGroggy, CurrentGroggy + GroggyDamage);
		BroadcastGroggy();

		if (CurrentGroggy >= MaxGroggy)
		{
			bIsGroggy = true;
			OnGroggy.Broadcast();
		}
	}

	if (CurrentHP <= 0.0f)
	{
		bIsDead = true;
		OnDead.Broadcast();
	}

	return true;
}

bool UBRStatComponent::SpendStamina(float Amount)
{
	if (Amount <= 0.0f)
	{
		return true;
	}

	if (CurrentStamina < Amount)
	{
		return false;
	}

	CurrentStamina = FMath::Max(0.0f, CurrentStamina - Amount);
	BroadcastStamina();
	return true;
}

void UBRStatComponent::RecoverStamina(float Amount)
{
	if (Amount <= 0.0f || MaxStamina <= 0.0f)
	{
		return;
	}

	CurrentStamina = FMath::Min(MaxStamina, CurrentStamina + Amount);
	BroadcastStamina();
}

void UBRStatComponent::ResetGroggy()
{
	CurrentGroggy = 0.0f;
	bIsGroggy = false;
	BroadcastGroggy();
}

void UBRStatComponent::BroadcastAllStats()
{
	BroadcastHP();
	BroadcastStamina();
	BroadcastGroggy();
}

void UBRStatComponent::BroadcastHP()
{
	OnHPChanged.Broadcast(CurrentHP, MaxHP, MaxHP > 0.0f ? CurrentHP / MaxHP : 0.0f);
}

void UBRStatComponent::BroadcastStamina()
{
	OnStaminaChanged.Broadcast(CurrentStamina, MaxStamina, MaxStamina > 0.0f ? CurrentStamina / MaxStamina : 0.0f);
}

void UBRStatComponent::BroadcastGroggy()
{
	OnGroggyChanged.Broadcast(CurrentGroggy, MaxGroggy, MaxGroggy > 0.0f ? CurrentGroggy / MaxGroggy : 0.0f);
}
