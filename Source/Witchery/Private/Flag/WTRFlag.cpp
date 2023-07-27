// Witchery. Copyright Liemander. All Rights Reserved.

#include "Flag/WTRFlag.h"
#include "Net/UnrealNetwork.h"
#include "Components/SphereComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Character/WTRCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "WTRPlayerState.h"

AWTRFlag::AWTRFlag()
{
    PrimaryActorTick.bCanEverTick = false;

    bReplicates = true;

    SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    SetRootComponent(SceneComponent);

    FlagMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FlagMesh"));
    FlagMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    FlagMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    FlagMesh->SetWorldScale3D(FVector(30.f, 30.f, 30.f));
    FlagMesh->SetupAttachment(RootComponent);

    AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
    AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    AreaSphere->SetSphereRadius(80.f);
    AreaSphere->SetupAttachment(RootComponent);
}

void AWTRFlag::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AWTRFlag, FlagState);
}

void AWTRFlag::BeginPlay()
{
    Super::BeginPlay();

    UMaterialInstance* MaterialToSet = (FlagTeam == ETeam::ET_RedTeam) ? RedTeamMaterial : BlueTeamMaterial;
    FlagMesh->SetMaterial(0, MaterialToSet);

    if (HasAuthority())
    {
        AreaSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
        AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        AreaSphere->OnComponentBeginOverlap.AddUniqueDynamic(this, &AWTRFlag::OnSphereBeginOverlap);
    }
}

void AWTRFlag::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (FlagState == EFlagState::EFS_Picked) return;

    AWTRCharacter* WTRCharacter = Cast<AWTRCharacter>(OtherActor);
    if (!WTRCharacter) return;

    AWTRPlayerState* WTRPlayerState = WTRCharacter->GetPlayerState<AWTRPlayerState>();
    if (!WTRPlayerState || WTRPlayerState->GetTeam() == FlagTeam) return;

    AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    WTRCharacter->PickupFlag(this);
}

void AWTRFlag::Dropped()
{
    FlagState = EFlagState::EFS_Dropped;
    FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
    FlagMesh->DetachFromComponent(DetachRules);

    AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

    SetOwner(nullptr);
}

void AWTRFlag::OnRep_FlagState() 
{

}