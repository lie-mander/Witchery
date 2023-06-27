// Witchery. Copyright Liemander. All Rights Reserved.

#include "Pickups/WTRSpeedPickup.h"
#include "Character/WTRCharacter.h"
#include "Components/WTRBuffComponent.h"

void AWTRSpeedPickup::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    Super::OnSphereBeginOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

    AWTRCharacter* WTRCharacter = Cast<AWTRCharacter>(OtherActor);
    if (WTRCharacter)
    {
        UWTRBuffComponent* WTRBuffComponent = WTRCharacter->GetBuffComponent();
        if (WTRBuffComponent)
        {
            WTRBuffComponent->SpeedBuff(BuffBaseSpeed, BuffCrouchSpeed, BuffSpeedTime);
        }
        Destroy();
    }
}
