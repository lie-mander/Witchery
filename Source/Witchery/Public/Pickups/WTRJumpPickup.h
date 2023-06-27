// Witchery. Copyright Liemander. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Pickups/Pickup.h"
#include "WTRJumpPickup.generated.h"

UCLASS()
class WITCHERY_API AWTRJumpPickup : public APickup
{
	GENERATED_BODY()

protected:
    virtual void OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;

private:
    UPROPERTY(EditDefaultsOnly, Category = "WTR | Jump")
    float BuffJumpZVelocity = 4000.f;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | Jump")
    float BuffJumpTime = 30.f;
};
