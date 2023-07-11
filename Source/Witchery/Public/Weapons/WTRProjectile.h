// Witchery. Copyright Liemander. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WTRProjectile.generated.h"

class UBoxComponent;
class UParticleSystemComponent;
class UParticleSystem;
class USoundCue;
class UNiagaraSystem;
class UNiagaraComponent;
class UStaticMeshComponent;
class UProjectileMovementComponent;

UCLASS()
class WITCHERY_API AWTRProjectile : public AActor
{
    GENERATED_BODY()

public:
    AWTRProjectile();
    virtual void Tick(float DeltaTime) override;
    virtual void LifeSpanExpired() override;

protected:
    UPROPERTY(VisibleAnywhere, Category = "WTR | Movement")
    UProjectileMovementComponent* ProjectileMovementComponent;

    UPROPERTY(EditAnywhere, Category = "WTR | Movement")
    float InitialSpeed = 50000.f;

    UPROPERTY(VisibleAnywhere, Category = "WTR | Projectile Mesh")
    UStaticMeshComponent* ProjectileMesh;

    UPROPERTY(VisibleAnywhere, Category = "WTR | Collision")
    UBoxComponent* BoxCollision;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | FX")
    UNiagaraSystem* TrailSystem;

    UPROPERTY()
    UNiagaraComponent* TrailComponent;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | Hit")
    float Damage = 20.f;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | Hit")
    float DestroyDelay = 0.01f;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | Hit")
    float ImpactParticleScale = 1.f;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | Radial hit")
    float MinimumDamage = 10.f;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | Radial hit")
    float DamageInnerRadius = 200.f;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | Radial hit")
    float DamageOutRadius = 500.f;

    /*
     * For server-side rewind using
     */
    UPROPERTY(EditAnywhere, Category = "WTR | SSR")
    bool bUseServerSideRewind = false;

    FVector_NetQuantize TraceStart;
    FVector_NetQuantize100 LaunchVelocity;


    virtual void BeginPlay() override;
    virtual void SpawnTrailSystem();
    virtual void StartDestroyTimer();
    void DestroyCosmetic();
    void PlayImpact(AActor* HitActor);
    bool ExplodeDamage();

    UFUNCTION()
    virtual void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse,
        const FHitResult& Hit);

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_OnDestroyed(AActor* HitActor);

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

    UPROPERTY(EditDefaultsOnly, Category = "WTR | Debug")
    bool DebugSphere = false;
};
