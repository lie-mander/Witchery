// Witchery. Copyright Liemander. All Rights Reserved.

#include "Weapons/WTRBulletShell.h"
#include "Components/StaticMeshComponent.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"

AWTRBulletShell::AWTRBulletShell()
{
    PrimaryActorTick.bCanEverTick = false;

    BulletMesh = CreateDefaultSubobject<UStaticMeshComponent>("BulletShellMesh");
    BulletMesh->SetSimulatePhysics(true);
    BulletMesh->SetEnableGravity(true);
    BulletMesh->SetNotifyRigidBodyCollision(true);
    BulletMesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

    SetRootComponent(BulletMesh);
}

void AWTRBulletShell::BeginPlay()
{
    Super::BeginPlay();

    check(BulletMesh);

    BulletMesh->AddImpulse(GetActorForwardVector() * EjectImpulse);
    BulletMesh->OnComponentHit.AddDynamic(this, &ThisClass::OnHit);
}

void AWTRBulletShell::OnHit(
    UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    if (GroundHitSound && bSingleSound)
    {
        UGameplayStatics::PlaySoundAtLocation(  //
            this,                               //
            GroundHitSound,                     //
            GetActorLocation()                  //
        );

        bSingleSound = false;
    }

    SetLifeSpan(1.5f);
}
