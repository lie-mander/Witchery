// Witchery. Copyright Liemander. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WTRProjectile.generated.h"

class UBoxComponent;
class UProjectileMovementComponent;
class UParticleSystemComponent;
class UParticleSystem;
class USoundCue;

UCLASS()
class WITCHERY_API AWTRProjectile : public AActor
{
    GENERATED_BODY()

public:
    AWTRProjectile();
    virtual void Tick(float DeltaTime) override;

protected:
    virtual void BeginPlay() override;

    UFUNCTION()
    virtual void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse,
        const FHitResult& Hit);

    UPROPERTY(EditDefaultsOnly, Category = "Hit")
    float Damage = 20.f;

private:
    UPROPERTY(VisibleAnywhere, Category = "Collision")
    UBoxComponent* BoxCollision;

    UPROPERTY(VisibleAnywhere, Category = "Movement")
    UProjectileMovementComponent* ProjectileMovementComponent;

    UPROPERTY()
    UParticleSystemComponent* ParticleSystemComponent;

    UPROPERTY(EditDefaultsOnly, Category = "Movement")
    UParticleSystem* Tracer;

    UPROPERTY(EditDefaultsOnly, Category = "Hit")
    UParticleSystem* DefaultImpactParticles;

    UPROPERTY(EditDefaultsOnly, Category = "Hit")
    UParticleSystem* PlayerImpactParticles;

    UPROPERTY(EditDefaultsOnly, Category = "Hit")
    USoundCue* ImpactSound;

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_OnDestroyed(AActor* HitActor);
};
