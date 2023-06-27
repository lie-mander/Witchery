// Witchery. Copyright Liemander. All Rights Reserved.

#include "Pickups/WTRHealthPickup.h"
#include "Character/WTRCharacter.h"
#include "Components/WTRBuffComponent.h"

void AWTRHealthPickup::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    Super::OnSphereBeginOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

    AWTRCharacter* WTRCharacter = Cast<AWTRCharacter>(OtherActor);
    if (WTRCharacter && !WTRCharacter->IsFullHealth())
    {
        UWTRBuffComponent* WTRBuffComponent = WTRCharacter->GetBuffComponent();
        if (WTRBuffComponent)
        {
            WTRBuffComponent->Heal(HealAmount, HealTime);
        }
        Destroy();
    }
}
