// Witchery. Copyright Liemander. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WTRProjectile.generated.h"

UCLASS()
class WITCHERY_API AWTRProjectile : public AActor
{
    GENERATED_BODY()

public:
    AWTRProjectile();
    virtual void Tick(float DeltaTime) override;

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY(VisibleAnywhere, Category = "Components")
    class UBoxComponent* BoxCollision;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    class UProjectileMovementComponent* ProjectileMovementComponent;

    UPROPERTY(EditDefaultsOnly, Category = "Projectile")
    class UParticleSystem* Tracer;

    class UParticleSystemComponent* ParticleSystemComponent;
};
