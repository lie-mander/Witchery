// Witchery. Copyright Liemander. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Pickups/Pickup.h"
#include "WTRTypes.h"
#include "WTRAmmoPickup.generated.h"

UCLASS()
class WITCHERY_API AWTRAmmoPickup : public APickup
{
    GENERATED_BODY()

protected:
    virtual void OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;

private:
    UPROPERTY(EditDefaultsOnly, Category = "WTR | Pickup")
    EWeaponType WeaponType;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | Pickup")
    int32 AmmoToAdd = 30;
};
