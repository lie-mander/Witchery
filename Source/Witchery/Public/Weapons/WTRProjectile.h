// Witchery. Copyright Liemander. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WTRProjectile.generated.h"

class UBoxComponent;
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
    UPROPERTY(VisibleAnywhere, Category = "WTR | Collision")
    UBoxComponent* BoxCollision;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | Hit")
    float Damage = 20.f;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | Hit")
    float DestroyDelay = 0.01f;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | Hit")
    float ImpactParticleScale = 1.f;

    virtual void BeginPlay() override;

    UFUNCTION()
    virtual void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse,
        const FHitResult& Hit);

private:
    UPROPERTY()
    UParticleSystemComponent* ParticleSystemComponent;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | Movement")
    UParticleSystem* Tracer;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | Hit")
    UParticleSystem* DefaultImpactParticles;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | Hit")
    UParticleSystem* PlayerImpactParticles;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | Hit")
    USoundCue* ImpactSound;

    UFUNCTION(NetMulticast, Reliable)
    virtual void Multicast_OnDestroyed(AActor* HitActor);
};
