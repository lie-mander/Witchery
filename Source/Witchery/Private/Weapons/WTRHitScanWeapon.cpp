// Witchery. Copyright Liemander. All Rights Reserved.

#include "Weapons/WTRHitScanWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Character/WTRCharacter.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystemComponent.h"

void AWTRHitScanWeapon::Fire(const FVector& HitTarget)
{
    Super::Fire(HitTarget);

    const USkeletalMeshSocket* MuzzleSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
    if (MuzzleSocket)
    {
        const FTransform MuzzleTransform = MuzzleSocket->GetSocketTransform(GetWeaponMesh());
        const FVector Start = MuzzleTransform.GetLocation();
        const FVector End = Start + (HitTarget - Start) * 1.25;

        if (GetWorld())
        {
            FHitResult FireHit;
            GetWorld()->LineTraceSingleByChannel(  //
                FireHit,                           //
                Start,                             //
                End,                               //
                ECC_Visibility                     //
            );

            FVector BeamEnd = End;

            ApplyDamageIfHasAuthority(FireHit, BeamEnd); 
            HandleEffects(FireHit, BeamEnd, MuzzleTransform);
        }
    }
}

void AWTRHitScanWeapon::ApplyDamageIfHasAuthority(FHitResult& HitResult, FVector& Beam) 
{
    AController* InstigatorController = GetOwnerPlayerController();
    AWTRCharacter* WTRCharacter = Cast<AWTRCharacter>(HitResult.GetActor());

    if (HasAuthority() && HitResult.bBlockingHit && WTRCharacter && InstigatorController)
    {
        Beam = HitResult.ImpactPoint;

        UGameplayStatics::ApplyDamage(  //
            HitResult.GetActor(),       //
            Damage,                     //
            InstigatorController,       //
            this,                       //
            UDamageType::StaticClass()  //
        );
    }
}

void AWTRHitScanWeapon::HandleEffects(const FHitResult& HitResult, const FVector& Beam, const FTransform& Muzzle)
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

    if (BeamParticles)
    {
        UParticleSystemComponent* BeamSystemComponent = UGameplayStatics::SpawnEmitterAtLocation(  //
            GetWorld(),                                                                            //
            BeamParticles,                                                                         //
            Muzzle                                                                                 //
        );

        if (BeamSystemComponent)
        {
            BeamSystemComponent->SetVectorParameter(FName("Target"), Beam);
        }
    }
}
