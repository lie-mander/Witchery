// Witchery. Copyright Liemander. All Rights Reserved.

#include "Weapons/WTRProjectileRocket.h"
#include "Kismet/GameplayStatics.h"

AWTRProjectileRocket::AWTRProjectileRocket() 
{
    RocketMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Rocket Mesh"));
    RocketMesh->SetupAttachment(RootComponent);
    RocketMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AWTRProjectileRocket::OnHit(
    UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    APawn* OwnerPawn = GetInstigator();
    if (OwnerPawn)
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
    }

    Super::OnHit(HitComponent, OtherActor, OtherComp, NormalImpulse, Hit);
}
