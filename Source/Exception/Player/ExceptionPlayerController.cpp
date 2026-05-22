// Copyright Epic Games, Inc. All Rights Reserved.


#include "ExceptionPlayerController.h"
#include "BRBossStatusWidget.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "Blueprint/UserWidget.h"
#include "Exception.h"
#include "Widgets/Input/SVirtualJoystick.h"

AExceptionPlayerController::AExceptionPlayerController()
{
	BossStatusWidgetClass = UBRBossStatusWidget::StaticClass();
}

UBRBossStatusWidget* AExceptionPlayerController::ShowBossStatusWidget()
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

void AExceptionPlayerController::HideBossStatusWidget()
{
	if (BossStatusWidget)
	{
		BossStatusWidget->RemoveFromParent();
		BossStatusWidget->ClearBosses();
	}
}

void AExceptionPlayerController::BeginPlay()
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

			UE_LOG(LogException, Error, TEXT("Could not spawn mobile controls widget."));

		}

	}
}

void AExceptionPlayerController::SetupInputComponent()
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

bool AExceptionPlayerController::ShouldUseTouchControls() const
{
	// are we on a mobile platform? Should we force touch?
	return SVirtualJoystick::ShouldDisplayTouchInterface() || bForceTouchControls;
}
