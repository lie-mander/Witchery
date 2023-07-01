// Witchery. Copyright Liemander. All Rights Reserved.

#include "Weapons/WTRHitScanWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Character/WTRCharacter.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystemComponent.h"
#include "DrawDebugHelpers.h"
#include "WTRTypes.h"

void AWTRHitScanWeapon::Fire(const FVector& HitTarget)
{
    Super::Fire(HitTarget);

    if (bOverlapOtherStaticMeshes) return;

    const USkeletalMeshSocket* MuzzleSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
    if (MuzzleSocket)
    {
        const FTransform MuzzleTransform = MuzzleSocket->GetSocketTransform(GetWeaponMesh());
        const FVector Start = MuzzleTransform.GetLocation();
        FHitResult FireHit;

        if (GetWorld())
        {
            WeaponTraceHit(Start, HitTarget, FireHit);
            ApplyDamageIfHasAuthority(FireHit);
            HandleEffects(FireHit, MuzzleTransform);
        }
    }
}

void AWTRHitScanWeapon::WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit)
{
    // Do hit
    const FVector End = bUseScatter ? TraceEndWithScatter(TraceStart, HitTarget) : TraceStart + (HitTarget - TraceStart) * 1.25;

    GetWorld()->LineTraceSingleByChannel(  //
        OutHit,                            //
        TraceStart,                        //
        End,                               //
        ECC_Visibility                     //
    );

    // Spawn beam particles
    FVector BeamEnd = End;
    if (OutHit.bBlockingHit)
    {
        BeamEnd = OutHit.ImpactPoint;
    }

    if (BeamParticles)
    {
        UParticleSystemComponent* BeamSystemComponent = UGameplayStatics::SpawnEmitterAtLocation(  //
            GetWorld(),                                                                            //
            BeamParticles,                                                                         //
            TraceStart,                                                                            //
            FRotator::ZeroRotator,                                                                 //
            true                                                                                   //
        );

        if (BeamSystemComponent)
        {
            BeamSystemComponent->SetVectorParameter(FName("Target"), BeamEnd);
        }
    }
}

void AWTRHitScanWeapon::ApplyDamageIfHasAuthority(const FHitResult& HitResult)
{
    AController* InstigatorController = GetOwnerPlayerController();
    const AWTRCharacter* WTRCharacter = Cast<AWTRCharacter>(HitResult.GetActor());

    if (HasAuthority() && HitResult.bBlockingHit && WTRCharacter && InstigatorController)
    {
        UGameplayStatics::ApplyDamage(  //
            HitResult.GetActor(),       //
            Damage,                     //
            InstigatorController,       //
            this,                       //
            UDamageType::StaticClass()  //
        );
    }
}

void AWTRHitScanWeapon::HandleEffects(const FHitResult& HitResult, const FTransform& Muzzle)
{
    if (MuzzleParticles)
    {
        UGameplayStatics::SpawnEmitterAtLocation(  //
            GetWorld(),                            //
            MuzzleParticles,                       //
            Muzzle.GetLocation(),                  //
            Muzzle.GetRotation().Rotator()         //
        );
    }

    if (ShootSound)
    {
        UGameplayStatics::PlaySoundAtLocation(  //
            this,                               //
            ShootSound,                         //
            Muzzle.GetLocation()                //
        );
    }

    if (HitResult.bBlockingHit)
    {
        if (ImpactParticles)
        {
            UGameplayStatics::SpawnEmitterAtLocation(  //
                GetWorld(),                            //
                ImpactParticles,                       //
                HitResult.ImpactPoint,                 //
                HitResult.ImpactNormal.Rotation()      //
            );
        }

        if (ImpactSound)
        {
            UGameplayStatics::PlaySoundAtLocation(  //
                this,                               //
                ImpactSound,                        //
                HitResult.ImpactPoint               //
            );
        }
    }
}

FVector AWTRHitScanWeapon::TraceEndWithScatter(const FVector& TraceStart, const FVector& HitTarget)
{
    const FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();
    const FVector SphereCenter = TraceStart + ToTargetNormalized * DistanceToSphere;
    const FVector RandVect = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius);
    const FVector EndLocation = SphereCenter + RandVect;
    const FVector ToEnd = (EndLocation - TraceStart);

    /* DrawDebugSphere(GetWorld(), SphereCenter, SphereRadius, 12, FColor::Red, true);
    DrawDebugSphere(GetWorld(), EndLocation, 3.f, 12, FColor::Orange, true);
    DrawDebugLine(GetWorld(), TraceStart, TraceStart + ToEnd * TRACE_RANGE / ToEnd.Size(), FColor::Orange, true);*/

    return FVector(TraceStart + ToEnd * TRACE_RANGE / ToEnd.Size());
}
