// Witchery. Copyright Liemander. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/WTRProjectile.h"
#include "WTRProjectileRocket.generated.h"

class UWTRRocketMovementComponent;

UCLASS()
class WITCHERY_API AWTRProjectileRocket : public AWTRProjectile
{
    GENERATED_BODY()

public:
    AWTRProjectileRocket();

protected:
    virtual void BeginPlay() override;

    virtual void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse,
        const FHitResult& Hit) override;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | Sound")
    USoundCue* ProjectileLoop;

    UPROPERTY()
    UAudioComponent* ProjectileLoopComponent;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | Sound")
    USoundAttenuation* ProjectileLoopAttenuation;

private:
    UPROPERTY(VisibleAnywhere, Category = "WTR | Movement")
    UWTRRocketMovementComponent* RocketMovementComponent;

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_OnRocketDestroyed();
};
