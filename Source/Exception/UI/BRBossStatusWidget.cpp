#include "BRBossStatusWidget.h"

void UBRBossStatusWidget::ClearBosses()
{
	BP_ClearBosses();
}

void UBRBossStatusWidget::SetBossCount(int32 BossCount)
{
	BP_SetBossCount(BossCount);
}

void UBRBossStatusWidget::SetBossHP(int32 BossIndex, FText BossName, float CurrentHP, float MaxHP, float NormalizedHP)
{
	BP_SetBossHP(BossIndex, BossName, CurrentHP, MaxHP, FMath::Clamp(NormalizedHP, 0.0f, 1.0f));
}

void UBRBossStatusWidget::SetBossGroggy(int32 BossIndex, float CurrentGroggy, float MaxGroggy, float NormalizedGroggy)
{
	BP_SetBossGroggy(BossIndex, CurrentGroggy, MaxGroggy, FMath::Clamp(NormalizedGroggy, 0.0f, 1.0f));
}

void UBRBossStatusWidget::SetBossGroggyState(int32 BossIndex, bool bIsGroggy)
{
	BP_SetBossGroggyState(BossIndex, bIsGroggy);
}
