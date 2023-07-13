// Witchery. Copyright Liemander. All Rights Reserved.

#include "Weapons/WTRProjectileWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Weapons/WTRProjectile.h"

void AWTRProjectileWeapon::Fire(const FVector& HitTarget)
{
    Super::Fire(HitTarget);

    APawn* InstigatorPawn = Cast<APawn>(GetOwner());
    if (GetWorld() && InstigatorPawn)
    {
        const FVector TraceStart = GetTraceStartFromMuzzleSocket();
        const FVector ToTargetVector = HitTarget - TraceStart;
        FRotator ToTargetRotation = ToTargetVector.Rotation();

        FActorSpawnParameters ProjSpawnParams;
        ProjSpawnParams.Owner = InstigatorPawn;
        ProjSpawnParams.Instigator = InstigatorPawn;

        // We want to spawn projectile reverce if out weapon overlapping with static mesh (such as walls, boxes, doors)
        if (bOverlapOtherStaticMeshes)
        {
            ToTargetRotation = FRotator(ToTargetRotation.Pitch, -ToTargetRotation.Yaw, ToTargetRotation.Roll);
        }

        AWTRProjectile* SpawnedProjectile = nullptr;
        // Check SSR
        if (bUseServerSideRewind)
        {
            if (InstigatorPawn->HasAuthority())  // server
            {
                if (InstigatorPawn->IsLocallyControlled())  // server locally controlled - not SSR, replicated projectile
                {
                    SpawnedProjectile =
                        GetWorld()->SpawnActor<AWTRProjectile>(ProjectileClass, TraceStart, ToTargetRotation, ProjSpawnParams);
                    SpawnedProjectile->bUseServerSideRewind = false;
                    SpawnedProjectile->SetDamage(Damage);
                }
                else  // server not locally controlled - SSR, not replicated
                {
                    SpawnedProjectile = GetWorld()->SpawnActor<AWTRProjectile>(
                        ServerSideRewindProjectileClass, TraceStart, ToTargetRotation, ProjSpawnParams);
                    SpawnedProjectile->bUseServerSideRewind = true;
                }
            }
            else  // client
            {
                if (InstigatorPawn->IsLocallyControlled())  // client locally controlled - SSR, not replicated
                {
                    SpawnedProjectile = GetWorld()->SpawnActor<AWTRProjectile>(
                        ServerSideRewindProjectileClass, TraceStart, ToTargetRotation, ProjSpawnParams);
                    SpawnedProjectile->bUseServerSideRewind = true;
                    SpawnedProjectile->SetDamage(Damage);
                    SpawnedProjectile->TraceStart = TraceStart;
                    SpawnedProjectile->LaunchVelocity = SpawnedProjectile->GetActorForwardVector() * SpawnedProjectile->GetInitialSpeed();
                }
                else // client not locally controlled - not SSR, not replicated
                {
                    SpawnedProjectile = GetWorld()->SpawnActor<AWTRProjectile>(
                        ServerSideRewindProjectileClass, TraceStart, ToTargetRotation, ProjSpawnParams);
                    SpawnedProjectile->bUseServerSideRewind = false;
                }
            }
        }
        else // spawn only on server`s characters replication projectiles
        {
            if (InstigatorPawn->HasAuthority())
            {
                SpawnedProjectile = GetWorld()->SpawnActor<AWTRProjectile>(ProjectileClass, TraceStart, ToTargetRotation, ProjSpawnParams);
                SpawnedProjectile->bUseServerSideRewind = false;
                SpawnedProjectile->SetDamage(Damage);
            }
        }
    }
}
