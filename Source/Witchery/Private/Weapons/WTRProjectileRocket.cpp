// Witchery. Copyright Liemander. All Rights Reserved.

#include "Weapons/WTRProjectileRocket.h"
#include "Kismet/GameplayStatics.h"
#include "Components/BoxComponent.h"
#include "Components/AudioComponent.h"
#include "Components/WTRRocketMovementComponent.h"
#include "Sound/SoundCue.h"

AWTRProjectileRocket::AWTRProjectileRocket()
{
    ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Rocket Mesh"));
    ProjectileMesh->SetupAttachment(RootComponent);
    ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    RocketMovementComponent = CreateDefaultSubobject<UWTRRocketMovementComponent>(TEXT("RocketMovementComponent"));
    RocketMovementComponent->bRotationFollowsVelocity = true;
    RocketMovementComponent->SetIsReplicated(true);
}

void AWTRProjectileRocket::BeginPlay()
{
    Super::BeginPlay();

    SpawnTrailSystem();

    if (ProjectileLoop && ProjectileLoopAttenuation)
    {
        ProjectileLoopComponent = UGameplayStatics::SpawnSoundAttached(  //
            ProjectileLoop,                                              //
            GetRootComponent(),                                          //
            FName(),                                                     //
            GetActorLocation(),                                          //
            EAttachLocation::KeepWorldPosition,                          //
            false,                                                       //
            1.f,                                                         //
            1.f,                                                         //
            0.f,                                                         //
            ProjectileLoopAttenuation,                                   //
            (USoundConcurrency*)nullptr,                                 //
            false                                                        //
        );
    }

    if (HasAuthority() && BoxCollision)
    {
        BoxCollision->OnComponentHit.AddUniqueDynamic(this, &AWTRProjectileRocket::OnHit);
    }
}

void AWTRProjectileRocket::OnHit(
    UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    if (OtherActor == GetOwner())
    {
        return;
    }

    if (ExplodeDamage())
    {
        Super::OnHit(HitComponent, OtherActor, OtherComp, NormalImpulse, Hit);
    }

    // We on the server cause bind was in BeginPlay with HasAuthority()
    Multicast_OnRocketDestroyed();
}

void AWTRProjectileRocket::Multicast_OnRocketDestroyed_Implementation()
{
    // Calls on all machines
    DestroyCosmetic();

    if (ProjectileLoopComponent && ProjectileLoopComponent->IsPlaying())
    {
        ProjectileLoopComponent->Stop();
    }
}
