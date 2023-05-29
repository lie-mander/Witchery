// Witchery. Copyright Liemander. All Rights Reserved.

#include "Weapons/WTRProjectile.h"
#include "Components/BoxComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "Character/WTRCharacter.h"
#include "WTRTypes.h"

AWTRProjectile::AWTRProjectile()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;

    BoxCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxCollision"));
    BoxCollision->SetCollisionObjectType(ECC_WorldDynamic);
    BoxCollision->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    BoxCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
    BoxCollision->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
    BoxCollision->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
    BoxCollision->SetCollisionResponseToChannel(ECC_SkeletalMesh, ECR_Block);

    SetRootComponent(BoxCollision);

    ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
    ProjectileMovementComponent->bRotationFollowsVelocity = true;
}

void AWTRProjectile::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AWTRProjectile::BeginPlay()
{
    Super::BeginPlay();

    check(Tracer);
    check(DefaultImpactParticles);
    check(PlayerImpactParticles);
    check(ImpactSound);

    if (Tracer)
    {
        // Spawn projectile tracer
        ParticleSystemComponent = UGameplayStatics::SpawnEmitterAttached(  //
            Tracer,                                                        //
            BoxCollision,                                                  //
            FName(),                                                       //
            GetActorLocation(),                                            //
            GetActorRotation(),                                            //
            EAttachLocation::KeepWorldPosition                             //
        );
    }

    if (HasAuthority())
    {
        BoxCollision->OnComponentHit.AddDynamic(this, &ThisClass::OnHit);
    }
}

void AWTRProjectile::OnHit(
    UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    Multicast_OnDestroyed(OtherActor);
    Destroy();
}

void AWTRProjectile::Multicast_OnDestroyed_Implementation(AActor* HitActor)
{
    // Different impacts for different actors
    if (HitActor->Implements<UInteractWithCrosshairInterface>() && PlayerImpactParticles)
    {
        UGameplayStatics::SpawnEmitterAtLocation(  //
            GetWorld(),                            //
            PlayerImpactParticles,                 //
            GetActorTransform()                    //
        );
    }
    else if (DefaultImpactParticles)
    {
        UGameplayStatics::SpawnEmitterAtLocation(  //
            GetWorld(),                            //
            DefaultImpactParticles,                //
            GetActorTransform()                    //
        );
    }

    if (ImpactSound)
    {
        UGameplayStatics::PlaySoundAtLocation(  //
            this,                               //
            ImpactSound,                        //
            GetActorLocation()                  //
        );
    }
}
