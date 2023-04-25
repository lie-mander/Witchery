// Witchery. Copyright Liemander. All Rights Reserved.

#include "Weapons/WTRProjectileWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Weapons/WTRProjectile.h"

void AWTRProjectileWeapon::Fire(const FVector& HitTarget)
{
    Super::Fire(HitTarget);

    const USkeletalMeshSocket* MuzzleSocket = GetWeaponMesh()->GetSocketByName(FName(MuzzleSocketName));
    if (MuzzleSocket)
    {
        FTransform MuzzleSocketTransform = MuzzleSocket->GetSocketTransform(GetWeaponMesh());
        FVector ToTargetVector = HitTarget - MuzzleSocketTransform.GetLocation();
        FRotator ToTargetRotation = ToTargetVector.Rotation();

        UWorld* World = GetWorld();
        APawn* InstigatorPawn = Cast<APawn>(GetOwner());

        if (World && InstigatorPawn)
        {
            FActorSpawnParameters ProjSpawnParams;
            ProjSpawnParams.Owner = GetOwner();
            ProjSpawnParams.Instigator = InstigatorPawn;

            World->SpawnActor<AWTRProjectile>(        //
                ProjectileClass,                      //
                MuzzleSocketTransform.GetLocation(),  //
                ToTargetRotation,                     //
                ProjSpawnParams                       //
            );
        }
    }
}
