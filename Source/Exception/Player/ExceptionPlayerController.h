// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ExceptionPlayerController.generated.h"

class UInputMappingContext;
class UUserWidget;
class UBRBossStatusWidget;
class AExceptionCharacter;

/**
 *  Basic PlayerController class for a third person game
 *  Manages input mappings
 */
UCLASS(abstract)
class AExceptionPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AExceptionPlayerController();

	UFUNCTION(BlueprintCallable, Category="Exception|Boss UI")
	UBRBossStatusWidget* ShowBossStatusWidget();

	UFUNCTION(BlueprintCallable, Category="Exception|Boss UI")
	void HideBossStatusWidget();

	UFUNCTION(BlueprintPure, Category="Exception|Boss UI")
	UBRBossStatusWidget* GetBossStatusWidget() const { return BossStatusWidget; }

	UFUNCTION(BlueprintCallable, Category="Exception|Player UI")
	UUserWidget* ShowPlayerHUDWidget();

	UFUNCTION(BlueprintCallable, Category="Exception|Player UI")
	void RefreshPlayerHUD();
	
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

	UPROPERTY(EditAnywhere, Category="Exception|Boss UI")
	TSubclassOf<UBRBossStatusWidget> BossStatusWidgetClass;

	UPROPERTY()
	TObjectPtr<UBRBossStatusWidget> BossStatusWidget;

	UPROPERTY(EditAnywhere, Category="Exception|Player UI")
	TSubclassOf<UUserWidget> PlayerHUDWidgetClass;

	UPROPERTY()
	TObjectPtr<UUserWidget> PlayerHUDWidget;

	/** If true, the player will use UMG touch controls even if not playing on mobile platforms */
	UPROPERTY(EditAnywhere, Config, Category = "Input|Touch Controls")
	bool bForceTouchControls = false;

	/** Gameplay initialization */
	virtual void BeginPlay() override;

	virtual void SetPawn(APawn* InPawn) override;

	/** Input mapping context setup */
	virtual void SetupInputComponent() override;

	/** Returns true if the player should use UMG touch controls */
	bool ShouldUseTouchControls() const;

	UFUNCTION()
	void HandlePlayerHPChanged(float CurrentValue, float MaxValue, float NormalizedValue);

	UFUNCTION()
	void HandlePlayerStaminaChanged(float CurrentValue, float MaxValue, float NormalizedValue);

	void BindPlayerHUDToPawn();
	void UnbindPlayerHUDFromPawn();
	void UpdateRuntimeGauge(FName GaugeWidgetName, const FText& GaugeText, float NormalizedValue, const FLinearColor& GaugeColor);
	void UpdateHPGauge(float CurrentValue, float MaxValue, float NormalizedValue);
	void UpdateStaminaGauge(float CurrentValue, float MaxValue, float NormalizedValue);
	void UpdateGroggyGauge(float NormalizedValue);

	UPROPERTY()
	TObjectPtr<AExceptionCharacter> BoundHUDCharacter;

};
