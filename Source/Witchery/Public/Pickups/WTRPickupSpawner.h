// Witchery. Copyright Liemander. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WTRPickupSpawner.generated.h"

class APickup;

UCLASS()
class WITCHERY_API AWTRPickupSpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	AWTRPickupSpawner();

protected:
	virtual void BeginPlay() override;

private:	
	UPROPERTY(EditAnywhere, Category = "WTR | Pickup spawner")
    TArray<TSubclassOf<APickup>> PickupClasses;

	UPROPERTY(EditAnywhere, Category = "WTR | Pickup spawner")
	float SpawnPickupTimeMin = 5.f;

	UPROPERTY(EditAnywhere, Category = "WTR | Pickup spawner")
    float SpawnPickupTimeMax = 15.f;

	UPROPERTY()
    APickup* SpawnedPickup;

	FTimerHandle SpawnPickupTimerHandle;

	void SpawnPickup();
    void SpawnPickupTimerFinished();

    UFUNCTION()
    void SpawnPickupTimerStart(AActor* DestroyedActor);
};
