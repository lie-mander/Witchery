// Witchery. Copyright Liemander. All Rights Reserved.

#include "Weapons/WTRWeapon.h"
#include "Weapons/WTRBulletShell.h"
#include "Net/UnrealNetwork.h"
#include "Character/WTRCharacter.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/WTRCombatComponent.h"
#include "Animation/AnimSequence.h"
#include "Engine/SkeletalMeshSocket.h"
#include "WTRTools.h"
#include "Kismet/KismetMathLibrary.h"

AWTRWeapon::AWTRWeapon()
{
    PrimaryActorTick.bCanEverTick = false;

    bReplicates = true;
    SetReplicateMovement(true);

    WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
    WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
    WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
    WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    SetRootComponent(WeaponMesh);

    WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_BLUE);
    WeaponMesh->MarkRenderStateDirty();
    EnableCustomDepth(true);

    AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
    AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    AreaSphere->SetSphereRadius(80.f);
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

    if (PickupWidget)
    {
        PickupWidget->SetVisibility(false);
    }

    AreaSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
    AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    AreaSphere->OnComponentBeginOverlap.AddUniqueDynamic(this, &ThisClass::OnSphereBeginOverlap);
    AreaSphere->OnComponentEndOverlap.AddUniqueDynamic(this, &ThisClass::OnSphereEndOverlap);

    WeaponMesh->OnComponentBeginOverlap.AddUniqueDynamic(this, &ThisClass::OnWeaponMeshBeginOverlap);
    WeaponMesh->OnComponentEndOverlap.AddUniqueDynamic(this, &ThisClass::OnWeaponMeshEndOverlap);
}

AController* AWTRWeapon::GetOwnerPlayerController() const
{
    const APawn* OwnerPawn = Cast<APawn>(GetOwner());
    if (!OwnerPawn) return nullptr;

    return OwnerPawn->GetController();
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
        const FTransform AmmoEjectSocketTransform = AmmoEjectSocket->GetSocketTransform(WeaponMesh);

        const float RandRoll = FMath::RandRange(-RandRollForShellsSpawn, RandRollForShellsSpawn);
        const float RandPitch = FMath::RandRange(-RandPitchForShellsSpawn, RandPitchForShellsSpawn);
        const FRotator3d RandRotator = FRotator3d(                               //
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

void AWTRWeapon::StopFire() {}

void AWTRWeapon::DecreaseAmmo()
{
    Ammo = FMath::Clamp(Ammo - 1, 0, MagazineCapacity);
    ++SequenceAmmo;

    WTROwnerCharacter = (WTROwnerCharacter == nullptr) ? Cast<AWTRCharacter>(GetOwner()) : WTROwnerCharacter;
    SetHUDAmmo();

    if (HasAuthority())
    {
        Client_UpdateAmmo(Ammo);
    }
}

void AWTRWeapon::AddAmmo(int32 AmmoToAdd)
{
    Ammo = FMath::Clamp(Ammo + AmmoToAdd, 0, MagazineCapacity);
    SetHUDAmmo();

    if (HasAuthority())
    {
        Client_AddAmmo(AmmoToAdd);
    }
}

void AWTRWeapon::Client_UpdateAmmo_Implementation(int32 NewAmmo)
{
    if (HasAuthority()) return;

    Ammo = NewAmmo;
    --SequenceAmmo;
    Ammo -= SequenceAmmo;
    SetHUDAmmo();
}

void AWTRWeapon::Client_AddAmmo_Implementation(int32 AmmoToAdd)
{
    if (HasAuthority()) return;

    Ammo = FMath::Clamp(Ammo + AmmoToAdd, 0, MagazineCapacity);
    SetHUDAmmo();

    WTROwnerCharacter = (WTROwnerCharacter == nullptr) ? Cast<AWTRCharacter>(GetOwner()) : WTROwnerCharacter;
    const bool bNeedToJump = IsFull() &&                                 //
                             WTROwnerCharacter &&                        //
                             WTROwnerCharacter->GetCombatComponent() &&  //
                             WeaponType == EWeaponType::EWT_Shotgun;
    if (bNeedToJump)
    {
        WTROwnerCharacter->GetCombatComponent()->JumpToShotgunEnd();
    }
}

void AWTRWeapon::SetWeaponState(EWeaponState NewState)
{
    WeaponState = NewState;

    OnWeaponStateChanged();
}

void AWTRWeapon::OnRep_WeaponState()
{
    OnWeaponStateChanged();
}

void AWTRWeapon::OnWeaponStateChanged()
{
    switch (WeaponState)
    {
        case EWeaponState::EWS_Equipped: HandleStateEquipped(); break;
        case EWeaponState::EWS_EquippedSecond: HandleStateEquippedSecond(); break;
        case EWeaponState::EWS_Dropped: HandleStateDropped(); break;
    }
}

void AWTRWeapon::HandleStateEquipped()
{
    AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    WeaponMesh->SetSimulatePhysics(false);
    WeaponMesh->SetEnableGravity(false);
    WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Overlap);
    WeaponMesh->SetGenerateOverlapEvents(true);
    if (WeaponType == EWeaponType::EWT_SubmachineGun)
    {
        WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        WeaponMesh->SetEnableGravity(true);
    }

    SetShowWidget(false);
    EnableCustomDepth(false);
}

void AWTRWeapon::HandleStateEquippedSecond()
{
    HandleStateEquipped();

    WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_BLUE);
    WeaponMesh->MarkRenderStateDirty();
    EnableCustomDepth(true);
}

void AWTRWeapon::HandleStateDropped()
{
    AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

    WeaponMesh->SetSimulatePhysics(true);
    WeaponMesh->SetEnableGravity(true);
    WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
    WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
    WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Overlap);
    WeaponMesh->SetGenerateOverlapEvents(false);

    WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_BLUE);
    WeaponMesh->MarkRenderStateDirty();
    EnableCustomDepth(true);
}

void AWTRWeapon::OnRep_Owner()
{
    Super::OnRep_Owner();

    if (!Owner)
    {
        WTROwnerPlayerController = nullptr;
        WTROwnerCharacter = nullptr;
    }
}

void AWTRWeapon::Dropped()
{
    SetWeaponState(EWeaponState::EWS_Dropped);
    FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
    WeaponMesh->DetachFromComponent(DetachRules);

    SetOwner(nullptr);
    WTROwnerPlayerController = nullptr;
    WTROwnerCharacter = nullptr;
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

void AWTRWeapon::OnWeaponMeshBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    // If we overlapped weapon we don`t want to block shooting
    const AWTRWeapon* OverlapWeapon = Cast<AWTRWeapon>(OtherActor);
    if (OverlapWeapon) return;

    bOverlapOtherStaticMeshes = true;
}

void AWTRWeapon::OnWeaponMeshEndOverlap(
    UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    // If we overlapped weapon we don`t want to block shooting
    const AWTRWeapon* OverlapWeapon = Cast<AWTRWeapon>(OtherActor);
    if (OverlapWeapon) return;

    bOverlapOtherStaticMeshes = false;
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

void AWTRWeapon::EnableCustomDepth(bool bEnable)
{
    if (WeaponMesh)
    {
        WeaponMesh->SetRenderCustomDepth(bEnable);
    }
}

FVector AWTRWeapon::TraceEndWithScatter(const FVector& HitTarget)
{
    const FVector TraceStart = GetTraceStartFromMuzzleSocket();

    const FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();
    const FVector SphereCenter = TraceStart + ToTargetNormalized * DistanceToSphere;
    const FVector RandVect = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius);
    const FVector EndLocation = SphereCenter + RandVect;
    const FVector ToEnd = (EndLocation - TraceStart);

    /* DrawDebugSphere(GetWorld(), SphereCenter, SphereRadius, 12, FColor::Red, true);
    DrawDebugSphere(GetWorld(), EndLocation, 3.f, 12, FColor::Orange, true);
    DrawDebugLine(GetWorld(), TraceStart, TraceStart + ToEnd * TRACE_RANGE / ToEnd.Size(), FColor::Orange, true);*/

    return FVector(TraceStart + ToEnd * TRACE_RANGE / ToEnd.Size());
}

FVector AWTRWeapon::GetTraceStartFromMuzzleSocket() const
{
    if (!WeaponMesh) return FVector();

    const USkeletalMeshSocket* MuzzleSocket = WeaponMesh->GetSocketByName("MuzzleFlash");
    if (!MuzzleSocket) return FVector();

    const FTransform MuzzleTransform = MuzzleSocket->GetSocketTransform(WeaponMesh);
    return MuzzleTransform.GetLocation();
}
