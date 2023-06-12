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

        for (uint32 i = 0; i < NumberOfShotgunShells; i++)
        {
            const FVector End = TraceEndWithScatter(Start, HitTarget);
        }
    }
}
