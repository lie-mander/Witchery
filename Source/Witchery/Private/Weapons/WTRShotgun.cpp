// Witchery. Copyright Liemander. All Rights Reserved.

#include "Weapons/WTRShotgun.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Character/WTRCharacter.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystemComponent.h"

void AWTRShotgun::FireShotgun(const TArray<FVector_NetQuantize> HitTargets)
{
    AWTRWeapon::Fire(FVector());

    const USkeletalMeshSocket* MuzzleSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
    if (MuzzleSocket)
    {
        const FTransform MuzzleTransform = MuzzleSocket->GetSocketTransform(GetWeaponMesh());
        const FVector Start = MuzzleTransform.GetLocation();

        // Map for characters and number of hits to every character in this map
        TMap<AWTRCharacter*, uint32> HitsMap;

        for (const auto HitTarget : HitTargets)
        {
            // Do hit
            FHitResult FireHit;
            WeaponTraceHit(Start, HitTarget, FireHit);

            // Calculate num of hits
            AWTRCharacter* WTRCharacter = Cast<AWTRCharacter>(FireHit.GetActor());
            if (FireHit.bBlockingHit && WTRCharacter)
            {
                // If we already have character in map - increment hits to him
                if (HitsMap.Contains(WTRCharacter))
                {
                    ++HitsMap[WTRCharacter];
                }
                // Or add him to map with 1 hit
                else
                {
                    HitsMap.Emplace(WTRCharacter, 1);
                }
            }
            else if (FireHit.bBlockingHit)
            {
                if (ImpactParticles)
                {
                    UGameplayStatics::SpawnEmitterAtLocation(  //
                        GetWorld(),                            //
                        ImpactParticles,                       //
                        FireHit.ImpactPoint,                   //
                        FireHit.ImpactNormal.Rotation()        //
                    );
                }

                if (ImpactSound)
                {
                    UGameplayStatics::PlaySoundAtLocation(  //
                        this,                               //
                        ImpactSound,                        //
                        FireHit.ImpactPoint,                //
                        0.5f,                               //
                        FMath::FRandRange(-0.5f, 0.5f)      //
                    );
                }
            }
        }

        AController* InstigatorController = GetOwnerPlayerController();
        if (HasAuthority() && !HitsMap.IsEmpty() && InstigatorController)
        {
            for (auto Pair : HitsMap)
            {
                UGameplayStatics::ApplyDamage(  //
                    Pair.Key,                   // For every character in this map ..
                    Damage * Pair.Value,        // Apply damage multiply by number of hits
                    InstigatorController,       //
                    this,                       //
                    UDamageType::StaticClass()  //
                );
            }
        }
    }
}

void AWTRShotgun::ShotgunTraceEndWithScatter(const FVector& HitTarget, TArray<FVector_NetQuantize>& HitTargets)
{
    if (!GetWeaponMesh()) return;

    const USkeletalMeshSocket* MuzzleSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
    if (!MuzzleSocket) return;

    const FTransform MuzzleTransform = MuzzleSocket->GetSocketTransform(GetWeaponMesh());
    const FVector TraceStart = MuzzleTransform.GetLocation();

    const FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();
    const FVector SphereCenter = TraceStart + ToTargetNormalized * DistanceToSphere;

    for (uint32 i = 0; i < NumberOfShotgunShells; ++i)
    {
        const FVector RandVect = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius);
        const FVector EndLocation = SphereCenter + RandVect;
        const FVector ToEnd = (EndLocation - TraceStart);
        const FVector_NetQuantize ResultRandVector = TraceStart + ToEnd * TRACE_RANGE / ToEnd.Size();

        HitTargets.Add(ResultRandVector);
    }
}
