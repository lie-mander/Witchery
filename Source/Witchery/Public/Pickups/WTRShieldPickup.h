// Witchery. Copyright Liemander. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Pickups/Pickup.h"
#include "WTRShieldPickup.generated.h"

UCLASS()
class WITCHERY_API AWTRShieldPickup : public APickup
{
    GENERATED_BODY()

protected:
    virtual void OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;

private:
    UPROPERTY(EditDefaultsOnly, Category = "WTR | Shield")
    float ShieldAmount = 100.f;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | Shield")
    float ShieldBuffTime = 5.f;
};
