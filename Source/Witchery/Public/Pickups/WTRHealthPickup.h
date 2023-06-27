// Witchery. Copyright Liemander. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Pickups/Pickup.h"
#include "WTRHealthPickup.generated.h"

UCLASS()
class WITCHERY_API AWTRHealthPickup : public APickup
{
    GENERATED_BODY()

protected:
    virtual void OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;

private:
    UPROPERTY(EditDefaultsOnly, Category = "WTR | Healing")
    float HealAmount = 100.f;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | Healing")
    float HealTime = 5.f;
};
