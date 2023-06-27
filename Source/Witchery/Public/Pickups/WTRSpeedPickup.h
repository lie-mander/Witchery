// Witchery. Copyright Liemander. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Pickups/Pickup.h"
#include "WTRSpeedPickup.generated.h"

UCLASS()
class WITCHERY_API AWTRSpeedPickup : public APickup
{
    GENERATED_BODY()

protected:
    virtual void OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;

private:
    UPROPERTY(EditDefaultsOnly, Category = "WTR | Speed")
    float BuffBaseSpeed = 1600.f;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | Speed")
    float BuffCrouchSpeed = 850.f;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | Speed")
    float BuffSpeedTime = 30.f;
};
