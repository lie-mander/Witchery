// Witchery. Copyright Liemander. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/WTRProjectile.h"
#include "WTRProjectileGrenade.generated.h"

class USoundCue;

UCLASS()
class WITCHERY_API AWTRProjectileGrenade : public AWTRProjectile
{
    GENERATED_BODY()

public:
    AWTRProjectileGrenade();
    virtual void LifeSpanExpired() override;

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY(EditDefaultsOnly, Category = "WTR | Bounce")
    USoundCue* BounceSound;

    UFUNCTION()
    void OnBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity);

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_OnGrenadeDestroyed();
};
