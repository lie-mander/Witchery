// Witchery. Copyright Liemander. All Rights Reserved.

#include "Weapons/WTRProjectileBullet.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "GameFramework/ProjectileMovementComponent.h"

AWTRProjectileBullet::AWTRProjectileBullet() 
{
    ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
    ProjectileMovementComponent->bRotationFollowsVelocity = true;
    ProjectileMovementComponent->SetIsReplicated(true);
}

void AWTRProjectileBullet::OnHit(
    UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    const ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter)
    {
        AController* OwnerController = Cast<AController>(OwnerCharacter->Controller);
        if (OwnerController)
        {
            UGameplayStatics::ApplyDamage(OtherActor, Damage, OwnerController, this, UDamageType::StaticClass());
        }
    }

    Super::OnHit(HitComponent, OtherActor, OtherComp, NormalImpulse, Hit);
}
