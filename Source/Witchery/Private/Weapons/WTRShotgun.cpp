// Witchery. Copyright Liemander. All Rights Reserved.

#include "Weapons/WTRShotgun.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Character/WTRCharacter.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystemComponent.h"

void AWTRShotgun::Fire(const FVector& HitTarget)
{
    AWTRWeapon::Fire(HitTarget);

    const USkeletalMeshSocket* MuzzleSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
    if (MuzzleSocket)
    {
        const FTransform MuzzleTransform = MuzzleSocket->GetSocketTransform(GetWeaponMesh());
        const FVector Start = MuzzleTransform.GetLocation();

        AController* InstigatorController = GetOwnerPlayerController();

        TMap<AWTRCharacter*, uint32> HitsMap;
        for (uint32 i = 0; i < NumberOfShotgunShells; i++)
        {
            // Do hit
            FHitResult FireHit;
            const FVector End = TraceEndWithScatter(HitTarget);
            WeaponTraceHit(Start, End, FireHit);

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

        if (HasAuthority() && !HitsMap.IsEmpty() && InstigatorController)
        {
            for (auto Pair : HitsMap)
            {
                UGameplayStatics::ApplyDamage(  //
                    Pair.Key,                   //
                    Damage * Pair.Value,        //
                    InstigatorController,       //
                    this,                       //
                    UDamageType::StaticClass()  //
                );
            }
        }
    }
}
