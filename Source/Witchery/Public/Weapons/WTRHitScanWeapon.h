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
    virtual void Fire(const FVector& HitTarget) override;

protected:
    UPROPERTY(EditDefaultsOnly, Category = "WTR | Hit")
    float Damage = 20.f;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | Hit")
    UParticleSystem* ImpactParticles;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | Hit")
    USoundCue* ImpactSound;

    virtual void WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit);

private:
    UPROPERTY(EditDefaultsOnly, Category = "WTR | Shoot")
    UParticleSystem* BeamParticles;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | Shoot")
    UParticleSystem* MuzzleParticles;

    void ApplyDamageIfHasAuthority(const FHitResult& HitResult);
    void HandleEffects(const FHitResult& HitResult, const FTransform& Muzzle);
};
