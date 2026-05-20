#include "BRMySQLBoss.h"

ABRMySQLBoss::ABRMySQLBoss()
{
	InitialMaxHP = 320.0f;
	InitialMaxGroggy = 120.0f;
	GroggyDuration = 3.0f;
	Phase2StartHPRatio = 0.5f;
}

void ABRMySQLBoss::OnBossReset()
{
	Super::OnBossReset();
	// MySQL boss-specific pattern table will be filled after Python is complete.
}

FString ABRMySQLBoss::GetBossDebugName() const
{
	return TEXT("MySQL Boss");
}
