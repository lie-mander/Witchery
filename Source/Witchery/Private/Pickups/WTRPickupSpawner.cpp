// Witchery. Copyright Liemander. All Rights Reserved.

#include "Pickups/WTRPickupSpawner.h"
#include "Pickups/Pickup.h"

AWTRPickupSpawner::AWTRPickupSpawner()
{
    PrimaryActorTick.bCanEverTick = false;

    bReplicates = true;
}

void AWTRPickupSpawner::BeginPlay()
{
    Super::BeginPlay();

    SpawnPickupTimerStart((AActor*)nullptr);
}

void AWTRPickupSpawner::SpawnPickup()
{
    if (HasAuthority() && GetWorld() && !PickupClasses.IsEmpty())
    {
        const int RandPickupIndex = FMath::RandRange(0, PickupClasses.Num() - 1);
        SpawnedPickup = GetWorld()->SpawnActor<APickup>(PickupClasses[RandPickupIndex], GetActorTransform());

        if (SpawnedPickup)
        {
            SpawnedPickup->OnDestroyed.AddDynamic(this, &AWTRPickupSpawner::SpawnPickupTimerStart);
        }
    }
}

void AWTRPickupSpawner::SpawnPickupTimerStart(AActor* DestroyedActor)
{
    if (HasAuthority())
    {
        const float RandSpawnTime = FMath::RandRange(SpawnPickupTimeMin, SpawnPickupTimeMax);

        GetWorldTimerManager().SetTimer(                   //
            SpawnPickupTimerHandle,                        //
            this,                                          //
            &AWTRPickupSpawner::SpawnPickupTimerFinished,  //
            RandSpawnTime                                  //
        );
    }
}

void AWTRPickupSpawner::SpawnPickupTimerFinished() 
{
    if (HasAuthority())
    {
        SpawnPickup();
    }
}
