// Witchery. Copyright Liemander. All Rights Reserved.

#include "Weapons/WTRProjectileGrenade.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"

AWTRProjectileGrenade::AWTRProjectileGrenade()
{
    ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Grenade Mesh"));
    ProjectileMesh->SetupAttachment(RootComponent);
    ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
    ProjectileMovementComponent->bRotationFollowsVelocity = true;
    ProjectileMovementComponent->SetIsReplicated(true);
    ProjectileMovementComponent->bShouldBounce = true;
}

void AWTRProjectileGrenade::BeginPlay()
{
    AActor::BeginPlay();

    SpawnTrailSystem();
    StartDestroyTimer();

    ProjectileMovementComponent->OnProjectileBounce.AddDynamic(this, &AWTRProjectileGrenade::OnBounce);
}

void AWTRProjectileGrenade::LifeSpanExpired()
{
    if (ExplodeDamage())
    {
        // We on the server cause ExplodeDamage() returns true
        Multicast_OnGrenadeDestroyed();
    }

    Super::LifeSpanExpired();
}

void AWTRProjectileGrenade::Multicast_OnGrenadeDestroyed_Implementation() 
{
    // We want play DefaultImpactParticles
    PlayImpact(nullptr);
}

void AWTRProjectileGrenade::OnBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity)
{
    if (BounceSound)
    {
        UGameplayStatics::PlaySoundAtLocation(  //
            this,                               //
            BounceSound,                        //
            GetActorLocation()                  //
        );
    }
}
