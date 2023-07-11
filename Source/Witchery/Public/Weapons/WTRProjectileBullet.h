// Witchery. Copyright Liemander. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/WTRProjectile.h"
#include "WTRProjectileBullet.generated.h"

UCLASS()
class WITCHERY_API AWTRProjectileBullet : public AWTRProjectile
{
    GENERATED_BODY()

public:
    AWTRProjectileBullet();

protected:
    virtual void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse,
        const FHitResult& Hit) override;

    virtual void BeginPlay() override;

private:
};
