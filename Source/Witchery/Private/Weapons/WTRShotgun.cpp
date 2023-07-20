// Witchery. Copyright Liemander. All Rights Reserved.

#include "Weapons/WTRShotgun.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Character/WTRCharacter.h"
#include "Character/WTRPlayerController.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/WTRLagCompensationComponent.h"

void AWTRShotgun::FireShotgun(const TArray<FVector_NetQuantize> HitTargets)
{
    AWTRWeapon::Fire(FVector());

    const FVector TraceStart = GetTraceStartFromMuzzleSocket();

    // Map for characters and number of hits to every character in this map
    TMap<AWTRCharacter*, uint32> HitsMap;
    TMap<AWTRCharacter*, uint32> HeadShotsHitsMap;

    for (const auto HitTarget : HitTargets)
    {
        FHitResult FireHit;

        CalculateNumOfHits(TraceStart, HitTarget, HitsMap, HeadShotsHitsMap, FireHit);
        HandleShotgunEffects(FireHit);
    }

    AController* InstigatorController = GetOwnerPlayerController();
    WTROwnerCharacter = (WTROwnerCharacter == nullptr) ? Cast<AWTRCharacter>(GetOwner()) : WTROwnerCharacter;
    TArray<AWTRCharacter*> HitCharacters;
    TMap<AWTRCharacter*, float> DamageMap;

    // Body shots, calculate damage to DamageMap
    for (auto Pair : HitsMap)
    {
        if (Pair.Key)
        {
            DamageMap.Emplace(Pair.Key, Pair.Value * Damage);  // Num of body hits * default damage

            HitCharacters.AddUnique(Pair.Key);
        }
    }

    // Head shots, calculate damage to DamageMap
    for (auto Pair : HeadShotsHitsMap)
    {
        if (Pair.Key)
        {
            if (DamageMap.Contains(Pair.Key))
                DamageMap[Pair.Key] += Pair.Value * HeadShotDamage;  // num of head hits * headshot damage
            else
                DamageMap.Emplace(Pair.Key, Pair.Value * HeadShotDamage);

            HitCharacters.AddUnique(Pair.Key);
        }
    }

    ApplyDamageWithoutSSR(DamageMap, InstigatorController);
    ApplyDamageWithSSR(HitCharacters, TraceStart, HitTargets, InstigatorController);
}

void AWTRShotgun::ApplyDamageWithoutSSR(TMap<AWTRCharacter*, float>& DamageMap, AController* InstigatorController)
{
    if ((!bUseServerSideRewind || WTROwnerCharacter->IsLocallyControlled()) &&  //
        HasAuthority() &&                                                       //
        InstigatorController                                                    //
    )
    {
        for (auto Pair : DamageMap)
        {
            if (Pair.Key)
            {
                UGameplayStatics::ApplyDamage(  //
                    Pair.Key,                   // For every character in this map ..
                    Pair.Value,                 // Apply damage calculated early in DamageMap
                    InstigatorController,       //
                    this,                       //
                    UDamageType::StaticClass()  //
                );
            }
        }
    }
}

void AWTRShotgun::ApplyDamageWithSSR(const TArray<AWTRCharacter*>& HitCharacters, const FVector& TraceStart,
    const TArray<FVector_NetQuantize> HitTargets, AController* InstigatorController)
{
    if (bUseServerSideRewind && !HasAuthority() && InstigatorController)
    {
        WTROwnerPlayerController =
            (WTROwnerPlayerController == nullptr) ? Cast<AWTRPlayerController>(InstigatorController) : WTROwnerPlayerController;

        if (WTROwnerCharacter && WTROwnerCharacter->IsLocallyControlled() && WTROwnerPlayerController &&
            WTROwnerCharacter->GetLagCompensation())
        {
            WTROwnerCharacter->GetLagCompensation()->Server_ShotgunScoreRequest(                      //
                HitCharacters,                                                                        //
                TraceStart,                                                                           //
                HitTargets,                                                                           //
                WTROwnerPlayerController->GetServerTime() - WTROwnerPlayerController->SingleTripTime  //
            );
        }
    }
}

void AWTRShotgun::CalculateNumOfHits(const FVector& TraceStart, const FVector_NetQuantize& HitTarget, TMap<AWTRCharacter*, uint32>& HitsMap,
    TMap<AWTRCharacter*, uint32>& HeadShotsHitsMap, FHitResult& FireHit)
{
    // Do hit
    WeaponTraceHit(TraceStart, HitTarget, FireHit);

    // Calculate num of hits
    AWTRCharacter* WTRCharacter = Cast<AWTRCharacter>(FireHit.GetActor());
    if (FireHit.bBlockingHit && WTRCharacter)
    {
        const bool bHeadShotHit = FireHit.BoneName.ToString() == FString("head");
        if (bHeadShotHit)
        {
            // If we already have character in map - increment hits to him
            // Or add him to map with 1 hit
            if (HeadShotsHitsMap.Contains(WTRCharacter))
                ++HeadShotsHitsMap[WTRCharacter];
            else
                HeadShotsHitsMap.Emplace(WTRCharacter, 1);
        }
        else
        {
            // If we already have character in map - increment hits to him
            // Or add him to map with 1 hit
            if (HitsMap.Contains(WTRCharacter))
                ++HitsMap[WTRCharacter];
            else
                HitsMap.Emplace(WTRCharacter, 1);
        }
    }
}

void AWTRShotgun::HandleShotgunEffects(const FHitResult& FireHit)
{
    if (FireHit.bBlockingHit)
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

void AWTRShotgun::ShotgunTraceEndWithScatter(const FVector& HitTarget, TArray<FVector_NetQuantize>& HitTargets)
{
    const FVector TraceStart = GetTraceStartFromMuzzleSocket();

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
