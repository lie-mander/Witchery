// Witchery. Copyright Liemander. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/WTRProjectile.h"
#include "WTRProjectileRocket.generated.h"

class UStaticMeshComponent;
class UNiagaraSystem;
class UNiagaraComponent;

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

    UPROPERTY(EditDefaultsOnly, Category = "Sound")
    USoundCue* ProjectileLoop;

    UPROPERTY()
    UAudioComponent* ProjectileLoopComponent;

    UPROPERTY(EditDefaultsOnly, Category = "Sound")
    USoundAttenuation* ProjectileLoopAttenuation;

private:
    UPROPERTY(VisibleAnywhere, Category = "Rocket Mesh")
    UStaticMeshComponent* RocketMesh;

    UPROPERTY(EditDefaultsOnly, Category = "FX")
    UNiagaraSystem* TrailSystem;

    UNiagaraComponent* TrailComponent;

    UPROPERTY(EditDefaultsOnly, Category = "Hit")
    float MinimumDamage = 10.f;

    UPROPERTY(EditDefaultsOnly, Category = "Hit")
    float DamageInnerRadius = 200.f;

    UPROPERTY(EditDefaultsOnly, Category = "Hit")
    float DamageOutRadius = 500.f;
};
