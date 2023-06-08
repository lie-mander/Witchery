// Witchery. Copyright Liemander. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/WTRProjectile.h"
#include "WTRProjectileRocket.generated.h"

class UStaticMeshComponent;

UCLASS()
class WITCHERY_API AWTRProjectileRocket : public AWTRProjectile
{
    GENERATED_BODY()

public:
    AWTRProjectileRocket();

protected:
    virtual void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse,
        const FHitResult& Hit) override;

private:
    UPROPERTY(VisibleAnywhere, Category = "Rocket Mesh")
    UStaticMeshComponent* RocketMesh;

    UPROPERTY(EditDefaultsOnly, Category = "Hit")
    float MinimumDamage = 10.f;

    UPROPERTY(EditDefaultsOnly, Category = "Hit")
    float DamageInnerRadius = 200.f;

    UPROPERTY(EditDefaultsOnly, Category = "Hit")
    float DamageOutRadius = 500.f;
};
