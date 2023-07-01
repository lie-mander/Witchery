// Witchery. Copyright Liemander. All Rights Reserved.

#include "Weapons/WTRProjectile.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "Character/WTRCharacter.h"
#include "WTRTypes.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

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
}

void AWTRProjectile::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
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

    if (HasAuthority() && BoxCollision)
    {
        BoxCollision->OnComponentHit.AddUniqueDynamic(this, &ThisClass::OnHit);
    }
}

void AWTRProjectile::OnHit(
    UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    Multicast_OnDestroyed(OtherActor);
    StartDestroyTimer();
}

void AWTRProjectile::Multicast_OnDestroyed_Implementation(AActor* HitActor)
{
    PlayImpact(HitActor);

    if (ParticleSystemComponent)
    {
        ParticleSystemComponent->Deactivate();
    }
}

void AWTRProjectile::SpawnTrailSystem()
{
    if (TrailSystem)
    {
        TrailComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(  //
            TrailSystem,                                                //
            GetRootComponent(),                                         //
            FName(),                                                    //
            GetActorLocation(),                                         //
            GetActorRotation(),                                         //
            EAttachLocation::KeepWorldPosition,                         //
            false                                                       //
        );
    }
}

void AWTRProjectile::StartDestroyTimer()
{
    SetLifeSpan(DestroyDelay);
}

void AWTRProjectile::LifeSpanExpired()
{
    Super::LifeSpanExpired();
}

void AWTRProjectile::DestroyCosmetic()
{
    if (ProjectileMesh)
    {
        ProjectileMesh->SetVisibility(false);
    }

    if (BoxCollision)
    {
        BoxCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    if (TrailComponent)
    {
        TrailComponent->Deactivate();
    }
}

void AWTRProjectile::PlayImpact(AActor* HitActor)
{
    FTransform CustomParticleTransform = GetActorTransform();
    CustomParticleTransform.SetScale3D(FVector(ImpactParticleScale, ImpactParticleScale, ImpactParticleScale));

    // Different impacts for different actors
    if (HitActor && HitActor->Implements<UInteractWithCrosshairInterface>() && PlayerImpactParticles)
    {
        UGameplayStatics::SpawnEmitterAtLocation(  //
            GetWorld(),                            //
            PlayerImpactParticles,                 //
            CustomParticleTransform                //
        );
    }
    else if (DefaultImpactParticles)
    {
        UGameplayStatics::SpawnEmitterAtLocation(  //
            GetWorld(),                            //
            DefaultImpactParticles,                //
            CustomParticleTransform                //
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

bool AWTRProjectile::ExplodeDamage()
{
    // Calls on the server
    const APawn* OwnerPawn = GetInstigator();
    if (OwnerPawn && HasAuthority())
    {
        AController* OwnerController = OwnerPawn->GetController();
        if (OwnerController)
        {
            UGameplayStatics::ApplyRadialDamageWithFalloff(  //
                this,                                        //
                Damage,                                      // Max damage
                MinimumDamage,                               // Min damage
                GetActorLocation(),                          //
                DamageInnerRadius,                           // In this radius will be max damage
                DamageOutRadius,                             // To this radius damage will be linear down
                1.f,                                         // Linear down falloff
                UDamageType::StaticClass(),                  //
                TArray<AActor*>(),                           // No ignore actors
                this,                                        //
                OwnerController                              //
            );
        }

        if (DebugSphere && GetWorld())
        {
            DrawDebugSphere(GetWorld(), GetActorLocation(), DamageInnerRadius, 12, FColor::Red, true);
            DrawDebugSphere(GetWorld(), GetActorLocation(), DamageOutRadius, 12, FColor::Orange, true);
        }
        return true;
    }
    return false;
}
