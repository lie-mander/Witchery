// Witchery. Copyright Liemander. All Rights Reserved.

#include "Weapons/WTRHitScanWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Character/WTRCharacter.h"
#include "Sound/SoundCue.h"

void AWTRHitScanWeapon::Fire(const FVector& HitTarget)
{
    Super::Fire(HitTarget);

    const USkeletalMeshSocket* MuzzleSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
    if (MuzzleSocket)
    {
        const FTransform MuzzleTransform = MuzzleSocket->GetSocketTransform(GetWeaponMesh());
        const FVector Start = MuzzleTransform.GetLocation();
        const FVector End = Start + (HitTarget - Start) * 1.25;

        APawn* OwnerPawn = Cast<APawn>(GetOwner());
        if (!OwnerPawn)
        {
            return;
        }
        AController* InstigatorController = OwnerPawn->GetController();

        if (GetWorld())
        {
            FHitResult FireHit;
            GetWorld()->LineTraceSingleByChannel(  //
                FireHit,                           //
                Start,                             //
                End,                               //
                ECC_Visibility                     //
            );

            AWTRCharacter* WTRCharacter = Cast<AWTRCharacter>(FireHit.GetActor());

            if (HasAuthority() && FireHit.bBlockingHit && WTRCharacter && InstigatorController)
            {
                UGameplayStatics::ApplyDamage(  //
                    FireHit.GetActor(),         //
                    Damage,                     //
                    InstigatorController,       //
                    this,                       //
                    UDamageType::StaticClass()  //
                );
            }

            if (ImpactParticles)
            {
                UGameplayStatics::SpawnEmitterAtLocation(  //
                    GetWorld(),                            //
                    ImpactParticles,                       //
                    FireHit.ImpactPoint,                    //
                    FireHit.ImpactNormal.Rotation()        //
                );
            }

            if (ImpactSound)
            {
                UGameplayStatics::PlaySoundAtLocation(  //
                    this,                               //
                    ImpactSound,                        //
                    FireHit.ImpactPoint                 //
                );
            }
        }
    }
}
