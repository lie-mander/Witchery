// Witchery. Copyright Liemander. All Rights Reserved.

#include "Pickups/WTRAmmoPickup.h"
#include "Components/WTRCombatComponent.h"
#include "Character/WTRCharacter.h"

void AWTRAmmoPickup::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    Super::OnSphereBeginOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

    AWTRCharacter* WTRCharacter = Cast<AWTRCharacter>(OtherActor);
    if (WTRCharacter)
    {
        UWTRCombatComponent* CombatComponent = WTRCharacter->GetCombatComponent();
        if (CombatComponent)
        {
            CombatComponent->AddPickupAmmo(WeaponType, AmmoToAdd);
        }
    }

    Destroy();
}
