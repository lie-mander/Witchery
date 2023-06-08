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
#include "WTRTools.h"

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
    DOREPLIFETIME(AWTRWeapon, Ammo);
}

void AWTRWeapon::BeginPlay()
{
    Super::BeginPlay();

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
    if (!WeaponMesh) return;

    if (FireAnimation)
    {
        WeaponMesh->PlayAnimation(FireAnimation, false);
    }

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

        if (BulletShellClass)
        {
            GetWorld()->SpawnActor<AWTRBulletShell>(     //
                BulletShellClass,                        //
                AmmoEjectSocketTransform.GetLocation(),  //
                RandRotator                              //
            );
        }
    }

    DecreaseAmmo();
}

void AWTRWeapon::DecreaseAmmo()
{
    Ammo = FMath::Clamp(Ammo - 1, 0, MagazineCapacity);

    SetHUDAmmo();
}

void AWTRWeapon::OnRep_Ammo()
{
    SetHUDAmmo();
}

void AWTRWeapon::SetWeaponState(EWeaponState NewState)
{
    WeaponState = NewState;

    switch (WeaponState)
    {
        case EWeaponState::EWS_Initial: break;

        case EWeaponState::EWS_Equipped:
            AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            WeaponMesh->SetSimulatePhysics(false);
            WeaponMesh->SetEnableGravity(false);
            WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            SetShowWidget(false);
            break;

        case EWeaponState::EWS_Dropped:
            if (HasAuthority())
            {
                AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
            }
            WeaponMesh->SetSimulatePhysics(true);
            WeaponMesh->SetEnableGravity(true);
            WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            break;

        case EWeaponState::EWS_MAX: break;
    }
}

void AWTRWeapon::OnRep_WeaponState()
{
    switch (WeaponState)
    {
        case EWeaponState::EWS_Initial: break;
        case EWeaponState::EWS_Equipped:
            WeaponMesh->SetSimulatePhysics(false);
            WeaponMesh->SetEnableGravity(false);
            WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            SetShowWidget(false);
            break;
        case EWeaponState::EWS_Dropped:
            WeaponMesh->SetSimulatePhysics(true);
            WeaponMesh->SetEnableGravity(true);
            WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            break;
        case EWeaponState::EWS_MAX: break;
    }
}

void AWTRWeapon::OnRep_Owner()
{
    Super::OnRep_Owner();

    if (!Owner)
    {
        WTROwnerPlayerController = nullptr;
    }
    else
    {
        SetHUDAmmo();
    }
}

void AWTRWeapon::Dropped()
{
    SetWeaponState(EWeaponState::EWS_Dropped);
    FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
    WeaponMesh->DetachFromComponent(DetachRules);

    SetOwner(nullptr);
    WTROwnerPlayerController = nullptr;
}

void AWTRWeapon::AddAmmo(int32 AmmoToAdd) 
{
    Ammo = FMath::Clamp(Ammo + AmmoToAdd, 0, MagazineCapacity);

    SetHUDAmmo();
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

void AWTRWeapon::SetHUDAmmo()
{
    WTROwnerPlayerController =
        (WTROwnerPlayerController == nullptr) ? UWTRTools::GetPlayerControllerByActor(GetOwner()) : WTROwnerPlayerController;

    if (WTROwnerPlayerController)
    {
        WTROwnerPlayerController->SetHUDWeaponAmmo(Ammo);
    }
}
