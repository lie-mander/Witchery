// Witchery. Copyright Liemander. All Rights Reserved.

#include "Pickups/Pickup.h"
#include "Components/SphereComponent.h"
#include "Sound/SoundCue.h"
#include "WTRTypes.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"

APickup::APickup()
{
    PrimaryActorTick.bCanEverTick = true;

    RootComponent = CreateDefaultSubobject<USceneComponent>("Root");

    AreaSphere = CreateDefaultSubobject<USphereComponent>("AreaSphere");
    AreaSphere->SetupAttachment(GetRootComponent());
    AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    AreaSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    AreaSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    AreaSphere->SetSphereRadius(150.f);

    PickupMesh = CreateDefaultSubobject<UStaticMeshComponent>("PickupMesh");
    PickupMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
    PickupMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_PURPLE);
    PickupMesh->MarkRenderStateDirty();
    PickupMesh->SetupAttachment(AreaSphere);
    EnableCustomDepth(true);

    NiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>("NiagaraComponent");
    NiagaraComponent->SetupAttachment(GetRootComponent());

    bReplicates = true;
}

void APickup::BeginPlay()
{
    Super::BeginPlay();

    if (HasAuthority())
    {
        AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnSphereBeginOverlap);
    }
}

void APickup::Destroyed()
{
    if (PickupSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, PickupSound, GetActorLocation());
    }

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

void APickup::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
}

void APickup::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (PickupMesh)
    {
        PickupMesh->AddWorldRotation(FRotator(0.f, BaseTurnRate * DeltaTime, 0.f));
    }
}

void APickup::EnableCustomDepth(bool bEnable)
{
    if (PickupMesh)
    {
        PickupMesh->SetRenderCustomDepth(bEnable);
    }
}
