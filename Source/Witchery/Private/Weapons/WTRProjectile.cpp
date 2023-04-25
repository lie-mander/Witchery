// Witchery. Copyright Liemander. All Rights Reserved.

#include "Weapons/WTRProjectile.h"
#include "Components/BoxComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"

AWTRProjectile::AWTRProjectile()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;

    BoxCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxCollision"));
    SetRootComponent(BoxCollision);
    BoxCollision->SetCollisionObjectType(ECC_WorldDynamic);
    BoxCollision->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    BoxCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
    BoxCollision->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
    BoxCollision->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);

    ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
    ProjectileMovementComponent->bRotationFollowsVelocity = true;
}

void AWTRProjectile::BeginPlay()
{
    Super::BeginPlay();

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
}

void AWTRProjectile::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}
