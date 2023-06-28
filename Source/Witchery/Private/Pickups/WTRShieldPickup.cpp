// Witchery. Copyright Liemander. All Rights Reserved.

#include "Pickups/WTRShieldPickup.h"
#include "Character/WTRCharacter.h"
#include "Components/WTRBuffComponent.h"

void AWTRShieldPickup::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    Super::OnSphereBeginOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

    AWTRCharacter* WTRCharacter = Cast<AWTRCharacter>(OtherActor);
    if (WTRCharacter && !WTRCharacter->IsFullShield())
    {
        UWTRBuffComponent* WTRBuffComponent = WTRCharacter->GetBuffComponent();
        if (WTRBuffComponent)
        {
            WTRBuffComponent->ShieldBuff(ShieldAmount, ShieldBuffTime);
        }
        Destroy();
    }
}
