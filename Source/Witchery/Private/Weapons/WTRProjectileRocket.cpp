// Witchery. Copyright Liemander. All Rights Reserved.

#include "Weapons/WTRProjectileRocket.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Components/BoxComponent.h"
#include "Components/AudioComponent.h"
#include "Components/WTRRocketMovementComponent.h"
#include "Sound/SoundCue.h"

AWTRProjectileRocket::AWTRProjectileRocket()
{
    RocketMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Rocket Mesh"));
    RocketMesh->SetupAttachment(RootComponent);
    RocketMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    RocketMovementComponent = CreateDefaultSubobject<UWTRRocketMovementComponent>(TEXT("RocketMovementComponent"));
    RocketMovementComponent->bRotationFollowsVelocity = true;
    RocketMovementComponent->SetIsReplicated(true);
}

void AWTRProjectileRocket::BeginPlay()
{
    Super::BeginPlay();

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

    BoxCollision->OnComponentHit.AddDynamic(this, &AWTRProjectileRocket::OnHit);
}

void AWTRProjectileRocket::OnHit(
    UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    if (OtherActor == GetOwner())
    {
        UE_LOG(LogTemp, Warning, TEXT("Hit self"));
        return;
    }

    // Calls on the server
    APawn* OwnerPawn = GetInstigator();
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

        Super::OnHit(HitComponent, OtherActor, OtherComp, NormalImpulse, Hit);
    }

    // Calls on all machines
    if (RocketMesh)
    {
        RocketMesh->SetVisibility(false);
    }

    if (BoxCollision)
    {
        BoxCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    if (TrailComponent)
    {
        TrailComponent->Deactivate();
    }

    if (ProjectileLoopComponent && ProjectileLoopComponent->IsPlaying())
    {
        ProjectileLoopComponent->Stop();
    }
}
