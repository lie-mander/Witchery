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

protected:
    UPROPERTY(EditDefaultsOnly, Category = "WTR | Scatter")
    float SphereRadius = 75.f;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | Scatter")
    float DistanceToSphere = 800.f;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | Scatter")
    bool bUseScatter = false;

    virtual FVector TraceEndWithScatter(const FVector& TraceStart, const FVector& HitTarget);

private:
    UPROPERTY(EditDefaultsOnly, Category = "WTR | Hit")
    float Damage = 20.f;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | Hit")
    UParticleSystem* ImpactParticles;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | Hit")
    USoundCue* ImpactSound;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | Shoot")
    UParticleSystem* BeamParticles;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | Shoot")
    UParticleSystem* MuzzleParticles;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | Shoot")
    USoundCue* ShootSound;

    void ApplyDamageIfHasAuthority(FHitResult& HitResult, FVector& Beam);
    void HandleEffects(const FHitResult& HitResult, const FVector& Beam, const FTransform& Muzzle);
};
