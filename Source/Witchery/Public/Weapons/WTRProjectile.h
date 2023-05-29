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

    UFUNCTION()
    virtual void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse,
        const FHitResult& Hit);

    UPROPERTY(EditDefaultsOnly, Category = "Hit")
    float Damage = 20.f;

private:
    UPROPERTY(VisibleAnywhere, Category = "Collision")
    class UBoxComponent* BoxCollision;

    UPROPERTY(VisibleAnywhere, Category = "Movement")
    class UProjectileMovementComponent* ProjectileMovementComponent;

    class UParticleSystemComponent* ParticleSystemComponent;

    UPROPERTY(EditDefaultsOnly, Category = "Movement")
    class UParticleSystem* Tracer;

    UPROPERTY(EditDefaultsOnly, Category = "Hit")
    UParticleSystem* DefaultImpactParticles;

    UPROPERTY(EditDefaultsOnly, Category = "Hit")
    UParticleSystem* PlayerImpactParticles;

    UPROPERTY(EditDefaultsOnly, Category = "Hit")
    class USoundCue* ImpactSound;

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_OnDestroyed(AActor* HitActor);
};
