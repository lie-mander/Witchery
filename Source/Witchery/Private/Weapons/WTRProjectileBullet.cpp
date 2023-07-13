// Witchery. Copyright Liemander. All Rights Reserved.

#include "Weapons/WTRProjectileBullet.h"
#include "Kismet/GameplayStatics.h"
#include "Character/WTRCharacter.h"
#include "Character/WTRPlayerController.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/WTRLagCompensationComponent.h"

AWTRProjectileBullet::AWTRProjectileBullet()
{
    ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
    ProjectileMovementComponent->bRotationFollowsVelocity = true;
    ProjectileMovementComponent->SetIsReplicated(true);
    ProjectileMovementComponent->InitialSpeed = InitialSpeed;
    ProjectileMovementComponent->MaxSpeed = InitialSpeed;
}

#if WITH_EDITOR
void AWTRProjectileBullet::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);

    FName PropertyName = PropertyChangedEvent.Property ? PropertyChangedEvent.Property->GetFName() : NAME_None;
    if (GET_MEMBER_NAME_CHECKED(AWTRProjectileBullet, InitialSpeed) == PropertyName)
    {
        if (ProjectileMovementComponent)
        {
            ProjectileMovementComponent->InitialSpeed = InitialSpeed;
            ProjectileMovementComponent->MaxSpeed = InitialSpeed;
        }
    }
}
#endif

void AWTRProjectileBullet::OnHit(
    UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    const AWTRCharacter* OwnerCharacter = Cast<AWTRCharacter>(GetOwner());
    if (OwnerCharacter)
    {
        AWTRPlayerController* OwnerController = Cast<AWTRPlayerController>(OwnerCharacter->Controller);
        if (                                                                     //
            (!bUseServerSideRewind || OwnerCharacter->IsLocallyControlled()) &&  //
            OwnerCharacter->HasAuthority() &&                                    //
            Hit.bBlockingHit &&                                                  //
            OtherActor &&                                                        //
            OwnerController                                                      //
        )
        {
            UGameplayStatics::ApplyDamage(OtherActor, Damage, OwnerController, this, UDamageType::StaticClass());
            Super::OnHit(HitComponent, OtherActor, OtherComp, NormalImpulse, Hit);
            return;
        }
        else if (bUseServerSideRewind && !OwnerCharacter->HasAuthority() && Hit.bBlockingHit && OtherActor && OwnerController)
        {
            if (OwnerCharacter && OwnerCharacter->IsLocallyControlled() && OwnerController && OwnerCharacter->GetLagCompensation())
            {
                AWTRCharacter* WTRCharacter = Cast<AWTRCharacter>(Hit.GetActor());
                OwnerCharacter->GetLagCompensation()->Server_ProjectileScoreRequest(    //
                    WTRCharacter,                                                       //
                    TraceStart,                                                         //
                    LaunchVelocity,                                                     //
                    OwnerController->GetServerTime() - OwnerController->SingleTripTime  //
                );
            }
        }
    }

    Super::OnHit(HitComponent, OtherActor, OtherComp, NormalImpulse, Hit);
}
