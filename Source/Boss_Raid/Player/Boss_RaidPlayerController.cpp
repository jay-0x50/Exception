// Copyright Epic Games, Inc. All Rights Reserved.


#include "Boss_RaidPlayerController.h"
#include "BRBossStatusWidget.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "Blueprint/UserWidget.h"
#include "Boss_Raid.h"
#include "Widgets/Input/SVirtualJoystick.h"

ABoss_RaidPlayerController::ABoss_RaidPlayerController()
{
	BossStatusWidgetClass = UBRBossStatusWidget::StaticClass();
}

UBRBossStatusWidget* ABoss_RaidPlayerController::ShowBossStatusWidget()
{
	if (!IsLocalPlayerController())
	{
		return nullptr;
	}

	if (!BossStatusWidgetClass)
	{
		BossStatusWidgetClass = UBRBossStatusWidget::StaticClass();
	}

	if (!BossStatusWidget && BossStatusWidgetClass)
	{
		BossStatusWidget = CreateWidget<UBRBossStatusWidget>(this, BossStatusWidgetClass);
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

void ABoss_RaidPlayerController::HideBossStatusWidget()
{
	if (BossStatusWidget)
	{
		BossStatusWidget->RemoveFromParent();
		BossStatusWidget->ClearBosses();
	}
}

void ABoss_RaidPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// only spawn touch controls on local player controllers
	if (ShouldUseTouchControls() && IsLocalPlayerController())
	{
		// spawn the mobile controls widget
		MobileControlsWidget = CreateWidget<UUserWidget>(this, MobileControlsWidgetClass);

		if (MobileControlsWidget)
		{
			// add the controls to the player screen
			MobileControlsWidget->AddToPlayerScreen(0);

		} else {

			UE_LOG(LogBoss_Raid, Error, TEXT("Could not spawn mobile controls widget."));

		}

	}
}

void ABoss_RaidPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// only add IMCs for local player controllers
	if (IsLocalPlayerController())
	{
		// Add Input Mapping Contexts
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
			{
				Subsystem->AddMappingContext(CurrentContext, 0);
			}

			// only add these IMCs if we're not using mobile touch input
			if (!ShouldUseTouchControls())
			{
				for (UInputMappingContext* CurrentContext : MobileExcludedMappingContexts)
				{
					Subsystem->AddMappingContext(CurrentContext, 0);
				}
			}
		}
	}
}

bool ABoss_RaidPlayerController::ShouldUseTouchControls() const
{
	// are we on a mobile platform? Should we force touch?
	return SVirtualJoystick::ShouldDisplayTouchInterface() || bForceTouchControls;
}
