// Witchery. Copyright Liemander. All Rights Reserved.

#include "Weapons/WTRWeapon.h"
#include "Weapons/WTRBulletShell.h"
#include "Net/UnrealNetwork.h"
#include "Character/WTRCharacter.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimSequence.h"
#include "Engine/SkeletalMeshSocket.h"

AWTRWeapon::AWTRWeapon()
{
    PrimaryActorTick.bCanEverTick = false;

    bReplicates = true;

    WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
    WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
    WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
    WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    SetRootComponent(WeaponMesh);

    AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
    AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    AreaSphere->SetSphereRadius(60.f);
    AreaSphere->SetupAttachment(RootComponent);

    PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidget"));
    PickupWidget->SetWidgetSpace(EWidgetSpace::Screen);
    PickupWidget->SetDrawAtDesiredSize(true);
    PickupWidget->SetupAttachment(RootComponent);
}

void AWTRWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AWTRWeapon, WeaponState);
}

void AWTRWeapon::BeginPlay()
{
    Super::BeginPlay();

    check(WeaponMesh);
    check(AreaSphere);
    check(PickupWidget);
    check(FireAnimation);
    check(BulletShellClass);

    if (PickupWidget)
    {
        PickupWidget->SetVisibility(false);
    }

    if (HasAuthority())
    {
        AreaSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
        AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnSphereBeginOverlap);
        AreaSphere->OnComponentEndOverlap.AddDynamic(this, &ThisClass::OnSphereEndOverlap);
    }
}

void AWTRWeapon::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AWTRWeapon::Fire(const FVector& HitTarget)
{
    if (!WeaponMesh || !FireAnimation || !BulletShellClass) return;

    WeaponMesh->PlayAnimation(FireAnimation, false);

    const USkeletalMeshSocket* AmmoEjectSocket = WeaponMesh->GetSocketByName(FName(AmmoEjectSocketName));
    if (AmmoEjectSocket && GetWorld())
    {
        FTransform AmmoEjectSocketTransform = AmmoEjectSocket->GetSocketTransform(WeaponMesh);

        float RandRoll = FMath::RandRange(-RandRollForShellsSpawn, RandRollForShellsSpawn);
        float RandPitch = FMath::RandRange(-RandPitchForShellsSpawn, RandPitchForShellsSpawn);
        FRotator3d RandRotator = FRotator3d(                                     //
            AmmoEjectSocketTransform.GetRotation().Rotator().Pitch + RandPitch,  //
            AmmoEjectSocketTransform.GetRotation().Rotator().Yaw,                //
            AmmoEjectSocketTransform.GetRotation().Rotator().Roll + RandRoll     //
        );

        GetWorld()->SpawnActor<AWTRBulletShell>(     //
            BulletShellClass,                        //
            AmmoEjectSocketTransform.GetLocation(),  //
            RandRotator                              //
        );
    }
}

void AWTRWeapon::SetWeaponState(EWeaponState NewState)
{
    WeaponState = NewState;

    switch (WeaponState)
    {
        case EWeaponState::EWS_Initial: break;
        case EWeaponState::EWS_Equipped:
            AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            SetShowWidget(false);
            break;
        case EWeaponState::EWS_Dropped: break;
        case EWeaponState::EWS_MAX: break;
    }
}

void AWTRWeapon::OnRep_WeaponState()
{
    switch (WeaponState)
    {
        case EWeaponState::EWS_Initial: break;
        case EWeaponState::EWS_Equipped:  //
            SetShowWidget(false);
            break;
        case EWeaponState::EWS_Dropped: break;
        case EWeaponState::EWS_MAX: break;
    }
}

void AWTRWeapon::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    AWTRCharacter* WTRCharacter = Cast<AWTRCharacter>(OtherActor);
    if (WTRCharacter)
    {
        WTRCharacter->SetOverlappingWeapon(this);
    }
}

void AWTRWeapon::OnSphereEndOverlap(
    UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    AWTRCharacter* WTRCharacter = Cast<AWTRCharacter>(OtherActor);
    if (WTRCharacter)
    {
        WTRCharacter->SetOverlappingWeapon(nullptr);
    }
}

void AWTRWeapon::SetShowWidget(bool bShowWidget)
{
    if (PickupWidget)
    {
        PickupWidget->SetVisibility(bShowWidget);
    }
}
