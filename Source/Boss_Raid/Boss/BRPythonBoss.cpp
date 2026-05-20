#include "BRPythonBoss.h"

#include "Engine/Engine.h"

ABRPythonBoss::ABRPythonBoss()
{
	InitialMaxHP = 260.0f;
	InitialMaxGroggy = 100.0f;
	GroggyDuration = 3.5f;
	Phase2StartHPRatio = 0.5f;
	DetectionRange = 1900.0f;
	MoveSpeed = 210.0f;
	Phase2MoveSpeedMultiplier = 1.25f;
	Phase2CooldownMultiplier = 0.72f;
	MeleeStandbyDistance = 560.0f;
	RangedStandbyDistance = 950.0f;
	RangedComfortMinDistance = 520.0f;
}

void ABRPythonBoss::ApplyTeamSlot(int32 TeamSlotIndex)
{
	Super::ApplyTeamSlot(TeamSlotIndex);

	if (!bUseTeamSlotRole)
	{
		return;
	}

	PythonDragonRole = TeamSlotIndex == 0
		? EBRPythonDragonRole::MeleeDragon
		: EBRPythonDragonRole::RangedDragon;
	ConfigurePythonPatterns();
}

void ABRPythonBoss::OnBossReset()
{
	Super::OnBossReset();
	ConfigurePythonPatterns();
}

void ABRPythonBoss::OnBossPhaseChanged(EBRBossPhase NewPhase)
{
	Super::OnBossPhaseChanged(NewPhase);
	ConfigurePythonPatterns();

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(2020, 2.0f, FColor::Orange, TEXT("Python Boss Phase Pattern Refresh"));
	}
}

void ABRPythonBoss::ConfigurePythonPatterns()
{
	AttackPatterns.Reset();

	if (PythonDragonRole == EBRPythonDragonRole::MeleeDragon)
	{
		TeamRole = EBRBossTeamRole::Melee;

		FBRBossPatternData Bite;
		Bite.PatternName = TEXT("Python_Bite");
		Bite.PatternType = EBRBossPatternType::Melee;
		Bite.MinRange = 0.0f;
		Bite.MaxRange = 320.0f;
		Bite.Damage = 22.0f * MeleeDragonDamageMultiplier;
		Bite.Windup = 0.5f;
		Bite.Cooldown = 1.5f;
		Bite.Radius = 105.0f;
		Bite.ForwardOffset = 180.0f;
		Bite.bEnableInPhase1 = true;
		Bite.bEnableInPhase2 = true;
		AttackPatterns.Add(Bite);

		FBRBossPatternData ClawRush;
		ClawRush.PatternName = TEXT("Python_ClawRush");
		ClawRush.PatternType = EBRBossPatternType::Dash;
		ClawRush.MinRange = 360.0f;
		ClawRush.MaxRange = 850.0f;
		ClawRush.Damage = 25.0f * MeleeDragonDamageMultiplier;
		ClawRush.Windup = 0.65f;
		ClawRush.Cooldown = 2.6f;
		ClawRush.Radius = 120.0f;
		ClawRush.ForwardOffset = 150.0f;
		ClawRush.DashDistance = 460.0f;
		ClawRush.bEnableInPhase1 = true;
		ClawRush.bEnableInPhase2 = true;
		AttackPatterns.Add(ClawRush);

		FBRBossPatternData TailSweep;
		TailSweep.PatternName = TEXT("Python_TailSweep");
		TailSweep.PatternType = EBRBossPatternType::AOE;
		TailSweep.MinRange = 0.0f;
		TailSweep.MaxRange = 420.0f;
		TailSweep.Damage = 24.0f * MeleeDragonDamageMultiplier;
		TailSweep.Windup = 0.8f;
		TailSweep.Cooldown = 3.2f;
		TailSweep.Radius = 220.0f;
		TailSweep.bEnableInPhase1 = false;
		TailSweep.bEnableInPhase2 = true;
		AttackPatterns.Add(TailSweep);
	}
	else
	{
		TeamRole = EBRBossTeamRole::Ranged;

		FBRBossPatternData Spit;
		Spit.PatternName = TEXT("Python_AcidSpit");
		Spit.PatternType = EBRBossPatternType::Melee;
		Spit.MinRange = 500.0f;
		Spit.MaxRange = 1150.0f;
		Spit.Damage = 18.0f * RangedDragonDamageMultiplier;
		Spit.Windup = 0.75f;
		Spit.Cooldown = 2.1f;
		Spit.Radius = 95.0f;
		Spit.ForwardOffset = 650.0f;
		Spit.bEnableInPhase1 = true;
		Spit.bEnableInPhase2 = true;
		AttackPatterns.Add(Spit);

		FBRBossPatternData RetreatStrike;
		RetreatStrike.PatternName = TEXT("Python_RetreatStrike");
		RetreatStrike.PatternType = EBRBossPatternType::Dash;
		RetreatStrike.MinRange = 0.0f;
		RetreatStrike.MaxRange = 430.0f;
		RetreatStrike.Damage = 15.0f * RangedDragonDamageMultiplier;
		RetreatStrike.Windup = 0.55f;
		RetreatStrike.Cooldown = 3.4f;
		RetreatStrike.Radius = 115.0f;
		RetreatStrike.ForwardOffset = 120.0f;
		RetreatStrike.DashDistance = 520.0f;
		RetreatStrike.bDashAwayFromTarget = true;
		RetreatStrike.bRequiresTeamMateNear = true;
		RetreatStrike.TeamMateNearDistance = 760.0f;
		RetreatStrike.bEnableInPhase1 = true;
		RetreatStrike.bEnableInPhase2 = true;
		AttackPatterns.Add(RetreatStrike);

		FBRBossPatternData SweepBreath;
		SweepBreath.PatternName = TEXT("Python_SweepBreath");
		SweepBreath.PatternType = EBRBossPatternType::AOE;
		SweepBreath.MinRange = 450.0f;
		SweepBreath.MaxRange = 1000.0f;
		SweepBreath.Damage = 20.0f * RangedDragonDamageMultiplier;
		SweepBreath.Windup = 1.0f;
		SweepBreath.Cooldown = 3.0f;
		SweepBreath.Radius = 260.0f;
		SweepBreath.bEnableInPhase1 = true;
		SweepBreath.bEnableInPhase2 = true;
		AttackPatterns.Add(SweepBreath);

		FBRBossPatternData PanicBite;
		PanicBite.PatternName = TEXT("Python_PanicBite");
		PanicBite.PatternType = EBRBossPatternType::Melee;
		PanicBite.MinRange = 0.0f;
		PanicBite.MaxRange = 260.0f;
		PanicBite.Damage = 12.0f * RangedDragonDamageMultiplier;
		PanicBite.Windup = 0.7f;
		PanicBite.Cooldown = 2.2f;
		PanicBite.Radius = 85.0f;
		PanicBite.ForwardOffset = 140.0f;
		PanicBite.bEnableInPhase1 = true;
		PanicBite.bEnableInPhase2 = true;
		AttackPatterns.Add(PanicBite);
	}
}

FString ABRPythonBoss::GetBossDebugName() const
{
	return PythonDragonRole == EBRPythonDragonRole::RangedDragon
		? TEXT("Python Ranged Dragon")
		: TEXT("Python Melee Dragon");
}
