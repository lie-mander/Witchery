// Witchery. Copyright Liemander. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/WTRHitScanWeapon.h"
#include "WTRShotgun.generated.h"

UCLASS()
class WITCHERY_API AWTRShotgun : public AWTRHitScanWeapon
{
    GENERATED_BODY()

public:
    virtual void Fire(const FVector& HitTarget);

private:
    uint32 NumberOfShotgunShells = 10;
};
