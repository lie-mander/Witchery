// Witchery. Copyright Liemander. All Rights Reserved.

#include "Pickups/WTRJumpPickup.h"
#include "Character/WTRCharacter.h"
#include "Components/WTRBuffComponent.h"

void AWTRJumpPickup::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    Super::OnSphereBeginOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

    const AWTRCharacter* WTRCharacter = Cast<AWTRCharacter>(OtherActor);
    if (WTRCharacter)
    {
        UWTRBuffComponent* WTRBuffComponent = WTRCharacter->GetBuffComponent();
        if (WTRBuffComponent)
        {
            WTRBuffComponent->JumpBuff(BuffJumpZVelocity, BuffJumpTime);
        }
        Destroy();
    }
}