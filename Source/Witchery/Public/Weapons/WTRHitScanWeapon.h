// Witchery. Copyright Liemander. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/WTRWeapon.h"
#include "WTRHitScanWeapon.generated.h"

class UParticleSystem;
class USoundCue;

UCLASS()
class WITCHERY_API AWTRHitScanWeapon : public AWTRWeapon
{
    GENERATED_BODY()

public:
    virtual void Fire(const FVector& HitTarget);

private:
    UPROPERTY(EditDefaultsOnly, Category = "Hit")
    float Damage = 20.f;

    UPROPERTY(EditDefaultsOnly, Category = "Hit")
    UParticleSystem* ImpactParticles;

    UPROPERTY(EditDefaultsOnly, Category = "Hit")
    USoundCue* ImpactSound;
};
