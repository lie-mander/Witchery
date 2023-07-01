// Witchery. Copyright Liemander. All Rights Reserved.

#include "Weapons/WTRProjectileWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Weapons/WTRProjectile.h"

void AWTRProjectileWeapon::Fire(const FVector& HitTarget)
{
    Super::Fire(HitTarget);

    if (!HasAuthority()) return;

    const USkeletalMeshSocket* MuzzleSocket = GetWeaponMesh()->GetSocketByName(FName(MuzzleSocketName));
    if (MuzzleSocket)
    {
        const FTransform MuzzleSocketTransform = MuzzleSocket->GetSocketTransform(GetWeaponMesh());
        const FVector ToTargetVector = HitTarget - MuzzleSocketTransform.GetLocation();
        const FRotator ToTargetRotation = ToTargetVector.Rotation();

        APawn* InstigatorPawn = Cast<APawn>(GetOwner());

        if (GetWorld() && InstigatorPawn)
        {
            FActorSpawnParameters ProjSpawnParams;
            ProjSpawnParams.Owner = GetOwner();
            ProjSpawnParams.Instigator = InstigatorPawn;

            // We want to spawn projectile reverce if out weapon overlapping with static mesh (such as walls, boxes, doors)
            if (bOverlapOtherStaticMeshes)
            {
                const FRotator ToTargetRotationReverse = FRotator(ToTargetRotation.Pitch, -ToTargetRotation.Yaw, ToTargetRotation.Roll);

                GetWorld()->SpawnActor<AWTRProjectile>(   //
                    ProjectileClass,                      //
                    MuzzleSocketTransform.GetLocation(),  //
                    ToTargetRotationReverse,              //
                    ProjSpawnParams                       //
                );
            }
            else
            {
                GetWorld()->SpawnActor<AWTRProjectile>(   //
                    ProjectileClass,                      //
                    MuzzleSocketTransform.GetLocation(),  //
                    ToTargetRotation,                     //
                    ProjSpawnParams                       //
                );
            }
        }
    }
}
