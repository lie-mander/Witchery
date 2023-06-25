// Witchery. Copyright Liemander. All Rights Reserved.

#include "Pickups/WTRHealthPickup.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Character/WTRCharacter.h"
#include "Components/WTRBuffComponent.h"

AWTRHealthPickup::AWTRHealthPickup()
{
    NiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>("NiagaraComponent");
    NiagaraComponent->SetupAttachment(GetRootComponent());

    bReplicates = true;
}

void AWTRHealthPickup::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    Super::OnSphereBeginOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

    AWTRCharacter* WTRCharacter = Cast<AWTRCharacter>(OtherActor);
    if (WTRCharacter)
    {

    }

    Destroy();
}

void AWTRHealthPickup::Destroyed()
{
    if (PickupFX)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(  //
            this,                                        //
            PickupFX,                                    //
            GetActorLocation(),                          //
            GetActorRotation()                           //
        );
    }

    Super::Destroyed();
}