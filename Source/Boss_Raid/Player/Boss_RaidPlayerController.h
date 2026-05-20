// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Boss_RaidPlayerController.generated.h"

class UInputMappingContext;
class UUserWidget;
class UBRBossStatusWidget;

/**
 *  Basic PlayerController class for a third person game
 *  Manages input mappings
 */
UCLASS(abstract)
class ABoss_RaidPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ABoss_RaidPlayerController();

	UFUNCTION(BlueprintCallable, Category="BossRaid|Boss UI")
	UBRBossStatusWidget* ShowBossStatusWidget();

	UFUNCTION(BlueprintCallable, Category="BossRaid|Boss UI")
	void HideBossStatusWidget();

	UFUNCTION(BlueprintPure, Category="BossRaid|Boss UI")
	UBRBossStatusWidget* GetBossStatusWidget() const { return BossStatusWidget; }
	
protected:

	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, Category ="Input|Input Mappings")
	TArray<UInputMappingContext*> DefaultMappingContexts;

	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, Category="Input|Input Mappings")
	TArray<UInputMappingContext*> MobileExcludedMappingContexts;

	/** Mobile controls widget to spawn */
	UPROPERTY(EditAnywhere, Category="Input|Touch Controls")
	TSubclassOf<UUserWidget> MobileControlsWidgetClass;

	/** Pointer to the mobile controls widget */
	UPROPERTY()
	TObjectPtr<UUserWidget> MobileControlsWidget;

	UPROPERTY(EditAnywhere, Category="BossRaid|Boss UI")
	TSubclassOf<UBRBossStatusWidget> BossStatusWidgetClass;

	UPROPERTY()
	TObjectPtr<UBRBossStatusWidget> BossStatusWidget;

	/** If true, the player will use UMG touch controls even if not playing on mobile platforms */
	UPROPERTY(EditAnywhere, Config, Category = "Input|Touch Controls")
	bool bForceTouchControls = false;

	/** Gameplay initialization */
	virtual void BeginPlay() override;

	/** Input mapping context setup */
	virtual void SetupInputComponent() override;

	/** Returns true if the player should use UMG touch controls */
	bool ShouldUseTouchControls() const;

};
