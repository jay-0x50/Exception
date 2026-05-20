#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BRCheckpoint.generated.h"

class UBillboardComponent;
class USphereComponent;
class UStaticMeshComponent;

UCLASS(Blueprintable, BlueprintType, meta=(DisplayName="Boss Raid Checkpoint"))
class BOSS_RAID_API ABRCheckpoint : public AActor
{
	GENERATED_BODY()

public:
	ABRCheckpoint();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<USphereComponent> ActivationSphere;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BossRaid|Checkpoint")
	bool bRestorePlayerOnActivation = true;

	UFUNCTION()
	void OnActivationBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};
