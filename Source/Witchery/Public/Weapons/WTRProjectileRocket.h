// Witchery. Copyright Liemander. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/WTRProjectile.h"
#include "WTRProjectileRocket.generated.h"

class UStaticMeshComponent;
class UNiagaraSystem;
class UNiagaraComponent;
class UWTRRocketMovementComponent;
class UParticleSystem;

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
    UPROPERTY(VisibleAnywhere, Category = "WTR | Rocket Mesh")
    UStaticMeshComponent* RocketMesh;

    UPROPERTY(VisibleAnywhere, Category = "WTR | Movement")
    UWTRRocketMovementComponent* RocketMovementComponent;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | FX")
    UNiagaraSystem* TrailSystem;

    UNiagaraComponent* TrailComponent;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | Hit")
    float MinimumDamage = 10.f;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | Hit")
    float DamageInnerRadius = 200.f;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | Hit")
    float DamageOutRadius = 500.f;
};
