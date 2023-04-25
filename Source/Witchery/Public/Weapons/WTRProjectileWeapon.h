// Witchery. Copyright Liemander. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/WTRWeapon.h"
#include "WTRProjectileWeapon.generated.h"

UCLASS()
class WITCHERY_API AWTRProjectileWeapon : public AWTRWeapon
{
    GENERATED_BODY()

public:
    virtual void Fire(const FVector& HitTarget) override;

private:
    UPROPERTY(EditDefaultsOnly, Category = "Weapon properties")
    TSubclassOf<class AWTRProjectile> ProjectileClass;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon properties")
    FName MuzzleSocketName = "MuzzleFlash";
};
