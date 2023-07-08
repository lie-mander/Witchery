// Witchery. Copyright Liemander. All Rights Reserved.

#include "Components/WTRCombatComponent.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Weapons/WTRWeapon.h"
#include "Weapons/WTRProjectile.h"
#include "Weapons/WTRShotgun.h"
#include "Character/WTRCharacter.h"
#include "Character/WTRPlayerController.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "HUD/WTR_HUD.h"
#include "Camera/CameraComponent.h"
#include "TimerManager.h"
#include "Sound/SoundCue.h"
#include "GameModes/WTRGameMode.h"

UWTRCombatComponent::UWTRCombatComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UWTRCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(UWTRCombatComponent, EquippedWeapon);
    DOREPLIFETIME(UWTRCombatComponent, SecondWeapon);
    DOREPLIFETIME(UWTRCombatComponent, bIsAiming);
    DOREPLIFETIME(UWTRCombatComponent, CombatState);
    DOREPLIFETIME(UWTRCombatComponent, Grenades);
    DOREPLIFETIME_CONDITION(UWTRCombatComponent, CarriedAmmo, COND_OwnerOnly);
}

void UWTRCombatComponent::BeginPlay()
{
    Super::BeginPlay();

    if (Character && Character->GetCameraComponent())
    {
        DefaultZoomFOV = Character->GetCameraComponent()->FieldOfView;
        CurrentZoomFOV = DefaultZoomFOV;

        if (Character->HasAuthority())
        {
            InitCarriedAmmoMap();
        }
    }

    SpawnAndEquipDefaultWeapon();
    UpdateHUDGrenades();

    bCanFire = true;
}

void UWTRCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (Character && Character->IsLocallyControlled())
    {
        TraceFromScreen(TraceHitResult);
        HitTarget = TraceHitResult.ImpactPoint;

        DrawCrosshair(DeltaTime);
        InterpFOV(DeltaTime);
    }
}

void UWTRCombatComponent::InitCarriedAmmoMap()
{
    CarriedAmmoByWeaponTypeMap.Emplace(EWeaponType::EWT_AssaultRifle, AssaultRifleCarrAmmo);
    CarriedAmmoByWeaponTypeMap.Emplace(EWeaponType::EWT_RocketLauncher, RocketLauncherCarrAmmo);
    CarriedAmmoByWeaponTypeMap.Emplace(EWeaponType::EWT_Pistol, PistolCarrAmmo);
    CarriedAmmoByWeaponTypeMap.Emplace(EWeaponType::EWT_SubmachineGun, SubmachineGunCarrAmmo);
    CarriedAmmoByWeaponTypeMap.Emplace(EWeaponType::EWT_Shotgun, ShotgunCarrAmmo);
    CarriedAmmoByWeaponTypeMap.Emplace(EWeaponType::EWT_SniperRifle, SniperRifleCarrAmmo);
    CarriedAmmoByWeaponTypeMap.Emplace(EWeaponType::EWT_GrenadeLauncher, GrenadeLauncherCarrAmmo);
    CarriedAmmoByWeaponTypeMap.Emplace(EWeaponType::EWT_Flamethrower, FlamethrowerCarrAmmo);
}

void UWTRCombatComponent::DrawCrosshair(float DeltaTime)
{
    if (!Character || !Controller || !HUD) return;

    if (Character->IsDisableGameplay() && EquippedWeapon)
    {
        FCrosshairHUDPackage EmptyHUDPackage;
        HUD->SetCrosshairHUDPackage(EmptyHUDPackage);
        return;
    }

    if (EquippedWeapon)
    {
        HUDPackage.CrosshairsCenter = EquippedWeapon->CrosshairsCenter;
        HUDPackage.CrosshairsLeft = EquippedWeapon->CrosshairsLeft;
        HUDPackage.CrosshairsRight = EquippedWeapon->CrosshairsRight;
        HUDPackage.CrosshairsTop = EquippedWeapon->CrosshairsTop;
        HUDPackage.CrosshairsBottom = EquippedWeapon->CrosshairsBottom;
    }

    if (Character->GetCharacterMovement())
    {
        const FVector2D CharacterVelocityRange(0.f, Character->GetCharacterMovement()->MaxWalkSpeed);
        const FVector2D OutputVelocityRange(0.f, 1.f);
        FVector CurrentVelocity = Character->GetVelocity();
        CurrentVelocity.Z = 0.f;

        CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(CharacterVelocityRange, OutputVelocityRange, CurrentVelocity.Size());
    }

    if (Character->GetCharacterMovement()->IsFalling())
    {
        CrosshairAirFactor = FMath::FInterpTo(CrosshairAirFactor, AirFactorSpread, DeltaTime, AirFactorSpeedUp);
    }
    else
    {
        CrosshairAirFactor = FMath::FInterpTo(CrosshairAirFactor, 0.f, DeltaTime, AirFactorSpeedDown);
    }

    if (bIsAiming)
    {
        CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, AimFactorSpread, DeltaTime, AimFactorSpeedUp);
    }
    else
    {
        CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.f, DeltaTime, AimFactorSpeedDown);
    }

    if (!FMath::IsNearlyZero(CrosshairShootingFactor))
    {
        CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.f, DeltaTime, ShootingFactorSpeedDown);
    }

    if (Character->bIsCrouched)
    {
        CrosshairCrouchingFactor = FMath::FInterpTo(CrosshairCrouchingFactor, CrouchingFactorSpread, DeltaTime, CrouchingFactorSpeedUp);
    }
    else
    {
        CrosshairCrouchingFactor = FMath::FInterpTo(CrosshairCrouchingFactor, 0.f, DeltaTime, CrouchingFactorSpeedDown);
    }

    if (TraceHitResult.GetActor() && TraceHitResult.GetActor()->Implements<UInteractWithCrosshairInterface>())
    {
        HUDPackage.CrosshairColor = CrosshairColorWithTarget;
        CrosshairHasEnemyFactor = FMath::FInterpTo(CrosshairHasEnemyFactor, HasEnemyFactorSpread, DeltaTime, HasEnemyFactorSpeedUp);
    }
    else
    {
        HUDPackage.CrosshairColor = CrosshairColorWithoutTarget;
        CrosshairHasEnemyFactor = FMath::FInterpTo(CrosshairHasEnemyFactor, 0.f, DeltaTime, HasEnemyFactorSpeedDown);
    }

    HUDPackage.CrosshairSpread =   //
        CrosshairSpread +          //
        CrosshairVelocityFactor +  //
        CrosshairAirFactor +       //
        CrosshairShootingFactor -  //
        CrosshairHasEnemyFactor -  //
        CrosshairAimFactor -       //
        CrosshairCrouchingFactor;

    HUD->SetCrosshairHUDPackage(HUDPackage);
}

void UWTRCombatComponent::InterpFOV(float DeltaTime)
{
    if (!EquippedWeapon || !Character->GetCameraComponent()) return;

    if (bIsAiming && CombatState == ECombatState::ECS_Unoccupied)
    {
        CurrentZoomFOV = FMath::FInterpTo(CurrentZoomFOV, EquippedWeapon->GetZoomedFOV(), DeltaTime, EquippedWeapon->GetZoomInterpSpeed());
    }
    else if (!bIsAiming)
    {
        CurrentZoomFOV = FMath::FInterpTo(CurrentZoomFOV, DefaultZoomFOV, DeltaTime, ZoomInterpSpeed);
    }

    Character->GetCameraComponent()->SetFieldOfView(CurrentZoomFOV);
}

void UWTRCombatComponent::EquipWeapon(AWTRWeapon* WeaponToEquip)
{
    if (!Character || !WeaponToEquip) return;

    if (EquippedWeapon && !SecondWeapon)
    {
        EquipSecondWeapon(WeaponToEquip);
    }
    else
    {
        EquipFirstWeapon(WeaponToEquip);
    }
}

void UWTRCombatComponent::EquipFirstWeapon(AWTRWeapon* WeaponToEquip)
{
    if (!WeaponToEquip) return;

    DropOrDestroyWeapon(EquippedWeapon);

    EquippedWeapon = WeaponToEquip;
    EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
    EquippedWeapon->SetOwner(Character);

    StopReloadWhileEquip();
    UpdateCarriedAmmoAndHUD();
    PlayPickupSound(EquippedWeapon);
    ReloadEmptyWeapon();
    SetCharacterSettingsWhenEquip();
    AttachWeaponByTypeToRightHand(EquippedWeapon);
    UpdateHUDAmmo();
}

void UWTRCombatComponent::EquipSecondWeapon(AWTRWeapon* WeaponToEquip)
{
    if (!WeaponToEquip) return;

    SecondWeapon = WeaponToEquip;
    SecondWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecond);
    SecondWeapon->SetOwner(Character);

    StopReloadWhileEquip();
    PlayPickupSound(SecondWeapon);
    AttachActorToBackpack(SecondWeapon);
}

void UWTRCombatComponent::OnRep_EquippedWeapon()
{
    if (!Character || !EquippedWeapon) return;

    EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
    PlayPickupSound(EquippedWeapon);
    SetCharacterSettingsWhenEquip();
    AttachWeaponByTypeToRightHand(EquippedWeapon);
    UpdateHUDWeaponType();
    UpdateHUDAmmo();
}

void UWTRCombatComponent::OnRep_SecondWeapon()
{
    if (!Character || !SecondWeapon) return;

    SecondWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecond);
    PlayPickupSound(SecondWeapon);
    AttachActorToBackpack(SecondWeapon);
}

void UWTRCombatComponent::DroppedWeapon(AWTRWeapon* Weapon)
{
    if (Weapon)
    {
        Weapon->Dropped();

        if (Weapon == EquippedWeapon)
        {
            EquippedWeapon = nullptr;
        }
    }
}

void UWTRCombatComponent::UpdateCarriedAmmoAndHUD()
{
    if (!EquippedWeapon) return;

    // Set CarriedAmmo and WeaponType on HUD by EquippedWeapon weapon type
    if (CarriedAmmoByWeaponTypeMap.Contains(EquippedWeapon->GetWeaponType()))
    {
        CarriedAmmo = CarriedAmmoByWeaponTypeMap[EquippedWeapon->GetWeaponType()];

        Controller = (Controller == nullptr) ? Cast<AWTRPlayerController>(Character->Controller) : Controller;
        if (Controller)
        {
            Controller->SetHUDCarriedAmmo(CarriedAmmo);
            Controller->SetHUDWeaponType(EquippedWeapon->GetWeaponType());
        }
    }
}

void UWTRCombatComponent::PlayPickupSound(AWTRWeapon* WeaponToPickup)
{
    if (!WeaponToPickup || !WeaponToPickup->PickupSound || !Character) return;

    // Play pickup sound (for server, for client will play in OnRep_Notify)
    UGameplayStatics::PlaySoundAtLocation(  //
        this,                               //
        WeaponToPickup->PickupSound,        //
        Character->GetActorLocation()       //
    );
}

void UWTRCombatComponent::ReloadEmptyWeapon()
{
    if (!EquippedWeapon) return;

    // Want to reload if equipped weapon is empty
    if (EquippedWeapon->IsEmpty())
    {
        Reload();
    }
}

void UWTRCombatComponent::SetCharacterSettingsWhenEquip()
{
    if (!Character || !Character->GetCharacterMovement() || !Character->GetSpringArm()) return;

    Character->GetCharacterMovement()->bOrientRotationToMovement = false;
    Character->bUseControllerRotationYaw = true;
    Character->GetSpringArm()->SocketOffset = SpringArmOffsetWhileEquipped;
}

void UWTRCombatComponent::AttachWeaponByTypeToRightHand(AWTRWeapon* WeaponToAttach)
{
    if (WeaponToAttach && WeaponToAttach->GetWeaponType() == EWeaponType::EWT_Flamethrower)
    {
        AttachActorToFlamethrowerRightHand(WeaponToAttach);
    }
    else if (WeaponToAttach)
    {
        AttachActorToRightHand(WeaponToAttach);
    }
}

void UWTRCombatComponent::AttachActorToRightHand(AActor* ActorToAttach)
{
    if (!Character || !Character->GetMesh() || !ActorToAttach) return;

    const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));
    if (HandSocket)
    {
        HandSocket->AttachActor(ActorToAttach, Character->GetMesh());
    }
}

void UWTRCombatComponent::AttachActorToFlamethrowerRightHand(AActor* ActorToAttach)
{
    if (!Character || !Character->GetMesh() || !ActorToAttach) return;

    const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("FlamethrowerRightHandSocket"));
    if (HandSocket)
    {
        HandSocket->AttachActor(ActorToAttach, Character->GetMesh());
    }
}

void UWTRCombatComponent::AttachActorToLeftHand(AActor* ActorToAttach)
{
    if (!Character || !Character->GetMesh() || !EquippedWeapon || !ActorToAttach) return;

    const bool bUsePistolSocket =                                      //
        EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Pistol ||  //
        EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SubmachineGun;

    const FName SocketName = bUsePistolSocket ? FName("PistolSocket") : FName("LeftHandSocket");

    const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(SocketName);
    if (HandSocket)
    {
        HandSocket->AttachActor(ActorToAttach, Character->GetMesh());
    }
}

void UWTRCombatComponent::AttachActorToBackpack(AActor* ActorToAttach)
{
    if (!Character || !Character->GetMesh() || !ActorToAttach) return;

    const USkeletalMeshSocket* BackpackSocket = Character->GetMesh()->GetSocketByName(FName("BackpackSocket"));
    if (BackpackSocket)
    {
        BackpackSocket->AttachActor(ActorToAttach, Character->GetMesh());
    }
}

void UWTRCombatComponent::SpawnAndEquipDefaultWeapon()
{
    const AWTRGameMode* WTRGameMode = Cast<AWTRGameMode>(UGameplayStatics::GetGameMode(this));
    if (WTRGameMode && GetWorld() && DefautlWeaponClass && Character && !Character->IsElimmed())
    {
        AWTRWeapon* StartWeapon = GetWorld()->SpawnActor<AWTRWeapon>(DefautlWeaponClass);
        EquipWeapon(StartWeapon);
        StartWeapon->bNeedDestroy = true;
    }
}

void UWTRCombatComponent::DropOrDestroyWeapon(AWTRWeapon* Weapon)
{
    if (Weapon && Weapon->bNeedDestroy)
    {
        Weapon->Destroy();
    }
    else
    {
        DroppedWeapon(Weapon);
    }
}

void UWTRCombatComponent::SwapWeapon()
{
    if (!CanSwapWeapon()) return;

    AWTRWeapon* TempPtr = EquippedWeapon;
    EquippedWeapon = SecondWeapon;
    SecondWeapon = TempPtr;

    HandleSwapWeapon();
}

void UWTRCombatComponent::HandleSwapWeapon()
{
    EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
    StopReloadWhileEquip();
    UpdateCarriedAmmoAndHUD();
    PlayPickupSound(EquippedWeapon);
    ReloadEmptyWeapon();
    AttachWeaponByTypeToRightHand(EquippedWeapon);
    UpdateHUDAmmo();

    SecondWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecond);
    AttachActorToBackpack(SecondWeapon);
}

void UWTRCombatComponent::UpdateHUDWeaponType()
{
    Controller = (Controller == nullptr) ? Cast<AWTRPlayerController>(Character->Controller) : Controller;
    if (Controller)
    {
        Controller->SetHUDWeaponType(EquippedWeapon->GetWeaponType());
    }
}

void UWTRCombatComponent::UpdateHUDAmmo()
{
    if (!EquippedWeapon) return;

    // Need to know weapon owner, must be set after SetOwner() function
    EquippedWeapon->SetHUDAmmo();
}

void UWTRCombatComponent::UpdateHUDGrenades()
{
    Controller = (Controller == nullptr) ? Cast<AWTRPlayerController>(Character->Controller) : Controller;
    if (Controller)
    {
        Controller->SetHUDGrenades(Grenades);
    }
}

void UWTRCombatComponent::OnRep_CarriedAmmo()
{
    SetHUDCarriedAmmo();

    if (CarriedAmmo == 0 && EquippedWeapon && EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Shotgun &&
        CombatState == ECombatState::ECS_Reloading)
    {
        JumpToShotgunEnd();
    }
}

void UWTRCombatComponent::SetHUDCarriedAmmo()
{
    if (Character)
    {
        Controller = (Controller == nullptr) ? Cast<AWTRPlayerController>(Character->Controller) : Controller;
        if (Controller)
        {
            Controller->SetHUDCarriedAmmo(CarriedAmmo);
        }
    }
}

void UWTRCombatComponent::OnFireButtonPressed(bool bPressed)
{
    bFireButtonPressed = bPressed;
    if (bFireButtonPressed)
    {
        Fire();

        const bool bHideScope = Character && Character->IsLocallyControlled() && bIsAiming && EquippedWeapon &&
                                EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle;
        if (bHideScope)
        {
            SetAiming(false);
            Character->SetShowScopeAnimation(false);
        }
    }
    else
    {
        LocalStopFire();
        Server_StopFire();
    }
}

void UWTRCombatComponent::Fire()
{
    if (CanFire())
    {
        bCanFire = false;

        FireByWeaponFireType();

        if (EquippedWeapon)
        {
            CrosshairShootingFactor = ShootingFactorSpread;
        }

        FireTimerStart();
    }
    else if (EquippedWeapon && EquippedWeapon->IsEmpty() && CarriedAmmo == 0)
    {
        UGameplayStatics::PlaySoundAtLocation(this, EmptyWeapon, EquippedWeapon->GetActorLocation());
        LocalStopFire();
        Server_StopFire();
    }
    else if (EquippedWeapon && EquippedWeapon->IsEmpty())
    {
        LocalStopFire();
        Server_StopFire();
        Reload();
    }
}

void UWTRCombatComponent::FireByWeaponFireType()
{
    if (!EquippedWeapon) return;

    switch (EquippedWeapon->GetFireType())
    {
        case EFireType::EFT_HitScan: HandleHitScanWeaponFire(); break;
        case EFireType::EFT_Projectile: HandleProjectileWeaponFire(); break;
        case EFireType::EFT_Shotgun: HandleShotgunWeaponFire(); break;
        case EFireType::EFT_Flamethrower: HandleFlamethrowerWeaponFire(); break;
    }
}

void UWTRCombatComponent::HandleHitScanWeaponFire()
{
    if (!EquippedWeapon) return;

    HitTarget = (EquippedWeapon->bUseScatter) ? EquippedWeapon->TraceEndWithScatter(HitTarget) : HitTarget;
    LocalFire(HitTarget);
    Server_Fire(HitTarget);
}

void UWTRCombatComponent::HandleProjectileWeaponFire()
{
    if (!EquippedWeapon) return;

    HitTarget = (EquippedWeapon->bUseScatter) ? EquippedWeapon->TraceEndWithScatter(HitTarget) : HitTarget;
    LocalFire(HitTarget);
    Server_Fire(HitTarget);
}

void UWTRCombatComponent::HandleShotgunWeaponFire()
{
    if (!EquippedWeapon) return;

    AWTRShotgun* Shotgun = Cast<AWTRShotgun>(EquippedWeapon);
    if (Shotgun)
    {
        TArray<FVector_NetQuantize> HitTargets;
        Shotgun->ShotgunTraceEndWithScatter(HitTarget, HitTargets);

        LocalFireShotgun(HitTargets);
        Server_FireShotgun(HitTargets);
    }
}

void UWTRCombatComponent::HandleFlamethrowerWeaponFire()
{
    LocalFire(HitTarget);
    Server_Fire(HitTarget);
}

void UWTRCombatComponent::FireTimerStart()
{
    if (!Character || !EquippedWeapon) return;

    Character->GetWorldTimerManager().SetTimer(  //
        FireTimerHandle,                         //
        this,                                    //
        &UWTRCombatComponent::FireTimerUpdate,   //
        EquippedWeapon->GetWeaponFiringDelay()   //
    );
}

void UWTRCombatComponent::FireTimerUpdate()
{
    bCanFire = true;

    if (bFireButtonPressed && EquippedWeapon && EquippedWeapon->IsAutomatic())
    {
        Fire();
    }
}

void UWTRCombatComponent::LocalFire(const FVector_NetQuantize& TraceHitTarget)
{
    if (Character && EquippedWeapon && EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Shotgun &&
        CombatState == ECombatState::ECS_Reloading)
    {
        Character->PlayFireMontage(bIsAiming);
        EquippedWeapon->Fire(TraceHitTarget);
        CombatState = ECombatState::ECS_Unoccupied;
    }
    else if (Character && EquippedWeapon && EquippedWeapon->GetWeaponType() != EWeaponType::EWT_Flamethrower &&
             CombatState == ECombatState::ECS_Unoccupied)
    {
        Character->PlayFireMontage(bIsAiming);
        EquippedWeapon->Fire(TraceHitTarget);
    }
    else if (Character && EquippedWeapon && CombatState == ECombatState::ECS_Unoccupied)
    {
        EquippedWeapon->Fire(TraceHitTarget);
    }
    else if (EquippedWeapon && EquippedWeapon->IsEmpty())
    {
        Reload();
    }
}

void UWTRCombatComponent::LocalFireShotgun(const TArray<FVector_NetQuantize>& TraceHitTargets)
{
    AWTRShotgun* Shotgun = Cast<AWTRShotgun>(EquippedWeapon);
    if (!Shotgun || !Character) return;

    if (CombatState == ECombatState::ECS_Reloading)
    {
        Character->PlayFireMontage(bIsAiming);
        Shotgun->FireShotgun(TraceHitTargets);
        CombatState = ECombatState::ECS_Unoccupied;
    }
    else if (CombatState == ECombatState::ECS_Unoccupied)
    {
        Shotgun->FireShotgun(TraceHitTargets);
    }
    else if (Shotgun->IsEmpty())
    {
        Reload();
    }
}

void UWTRCombatComponent::Server_Fire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
    Multicast_Fire(TraceHitTarget);
}

void UWTRCombatComponent::Multicast_Fire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
    // if Locally controlled character - return, cause we already played LocalFire() in Fire()
    if (Character && Character->IsLocallyControlled()) return;

    /*
     * if we here, we on the server or on the simulated proxy
     * server - will apply damage in weapon Fire() functions
     * simulated proxy - will play cosmetics
     */
    LocalFire(TraceHitTarget);
}

void UWTRCombatComponent::Server_FireShotgun_Implementation(const TArray<FVector_NetQuantize>& TraceHitTargets)
{
    Multicast_FireShotgun(TraceHitTargets);
}

void UWTRCombatComponent::Multicast_FireShotgun_Implementation(const TArray<FVector_NetQuantize>& TraceHitTargets)
{
    // The same logic like Multicast_Fire
    if (Character && Character->IsLocallyControlled()) return;

    LocalFireShotgun(TraceHitTargets);
}

void UWTRCombatComponent::LocalStopFire()
{
    if (EquippedWeapon)
    {
        EquippedWeapon->StopFire();
    }
}

void UWTRCombatComponent::Server_StopFire_Implementation()
{
    Multicast_StopFire();
}

void UWTRCombatComponent::Multicast_StopFire_Implementation()
{
    // The same logic like Multicast_Fire
    if (Character && Character->IsLocallyControlled()) return;

    LocalStopFire();
}

void UWTRCombatComponent::Reload()
{
    if (CarriedAmmo > 0 && EquippedWeapon && !EquippedWeapon->IsFull() && CombatState == ECombatState::ECS_Unoccupied && !bLocallyReloading)
    {
        Server_Reload();
        ReloadHandle();

        bLocallyReloading = true;
    }
}

void UWTRCombatComponent::Server_Reload_Implementation()
{
    if (!Character) return;

    CombatState = ECombatState::ECS_Reloading;
    LastEquippedWeapon = EquippedWeapon;

    if (!Character->IsLocallyControlled())
    {
        ReloadHandle();
    }
}

void UWTRCombatComponent::StopReloadWhileEquip()
{
    if (CombatState == ECombatState::ECS_Reloading && Character)
    {
        CombatState = ECombatState::ECS_Unoccupied;
        Character->StopReloadMontage();
        bLocallyReloading = false;

        const bool bShowScope = Character->IsLocallyControlled() && bIsAiming && EquippedWeapon &&
                                EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle;
        if (bShowScope)
        {
            Character->SetShowScopeAnimation(bShowScope);
        }
    }
}

void UWTRCombatComponent::ThrowGrenade()
{
    if (Grenades <= 0 || CombatState != ECombatState::ECS_Unoccupied || !EquippedWeapon) return;

    CombatState = ECombatState::ECS_ThrowingGrenade;

    if (Character)
    {
        Character->PlayThrowGrenadeMontage();
        AttachActorToLeftHand(EquippedWeapon);
        SetShowGrenadeMesh(true);
    }

    if (Character && !Character->HasAuthority())
    {
        Server_ThrowGrenade();
    }
}

void UWTRCombatComponent::Server_ThrowGrenade_Implementation()
{
    if (Grenades <= 0) return;

    if (Character && EquippedWeapon)
    {
        CombatState = ECombatState::ECS_ThrowingGrenade;
        Character->PlayThrowGrenadeMontage();
        AttachActorToLeftHand(EquippedWeapon);
        SetShowGrenadeMesh(true);
    }
}

void UWTRCombatComponent::ThrowGrenadeFinished()
{
    CombatState = ECombatState::ECS_Unoccupied;

    if (EquippedWeapon)
    {
        AttachWeaponByTypeToRightHand(EquippedWeapon);
    }
}

void UWTRCombatComponent::LaunchGrenade()
{
    SetShowGrenadeMesh(false);

    if (Character && Character->IsLocallyControlled())
    {
        Server_LaunchGrenade(HitTarget);
    }
}

void UWTRCombatComponent::Server_LaunchGrenade_Implementation(const FVector_NetQuantize& Target)
{
    if (Character && Character->HasAuthority() && GrenadeClass && Character->GetGrenadeMesh())
    {
        const FVector StartLocation = Character->GetGrenadeMesh()->GetComponentLocation();
        const FVector ToTarget = Target - StartLocation;
        const FRotator Rotation = ToTarget.Rotation();

        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = Character;
        SpawnParams.Instigator = Character;

        if (GetWorld())
        {
            GetWorld()->SpawnActor<AWTRProjectile>(  //
                GrenadeClass,                        //
                StartLocation,                       //
                Rotation,                            //
                SpawnParams                          //
            );
        }

        Grenades = FMath::Clamp(Grenades - 1, 0, MaxGrenades);
        UpdateHUDGrenades();
    }
}

void UWTRCombatComponent::SetShowGrenadeMesh(bool bShow)
{
    if (Character && Character->GetGrenadeMesh())
    {
        Character->GetGrenadeMesh()->SetVisibility(bShow);
    }
}

void UWTRCombatComponent::OnRep_Grenades()
{
    UpdateHUDGrenades();
}

void UWTRCombatComponent::OnRep_CombatState()
{
    switch (CombatState)
    {
        case ECombatState::ECS_Unoccupied:  //
            if (bFireButtonPressed)
            {
                Fire();
            }

            if (Character && Character->IsLocallyControlled() && bIsAiming && EquippedWeapon &&
                EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle)
            {
                Character->SetShowScopeAnimation(true);
            }

            if (Character)
            {
                Character->StopReloadMontage();
            }

            SetShowGrenadeMesh(false);
            break;

        case ECombatState::ECS_Reloading:  //
            if (!Character->IsLocallyControlled())
            {
                ReloadHandle();
            }
            break;

        case ECombatState::ECS_ThrowingGrenade:  //
            if (Character && !Character->IsLocallyControlled() && EquippedWeapon)
            {
                Character->PlayThrowGrenadeMontage();
                AttachActorToLeftHand(EquippedWeapon);
                SetShowGrenadeMesh(true);
            }
            break;
    }
}

void UWTRCombatComponent::ReloadHandle()
{
    Character->PlayReloadMontage();
}

int32 UWTRCombatComponent::AmmoToReload()
{
    if (!EquippedWeapon) return 0;

    const int32 EmplyPlaceInMag = EquippedWeapon->GetMagazineCapacity() - EquippedWeapon->GetAmmo();
    if (CarriedAmmoByWeaponTypeMap.Contains(EquippedWeapon->GetWeaponType()))
    {
        const int32 AmountCarried = CarriedAmmoByWeaponTypeMap[EquippedWeapon->GetWeaponType()];
        const int32 Least = FMath::Min(EmplyPlaceInMag, AmountCarried);

        return FMath::Clamp(EmplyPlaceInMag, 0, Least);
    }

    return 0;
}

void UWTRCombatComponent::ReloadWeaponAndSubCarriedAmmo()
{
    if (!EquippedWeapon || LastEquippedWeapon != EquippedWeapon) return;

    // Reloading weapon and subtract carried ammo for this weapon type
    const int32 ReloadAmmo = AmmoToReload();
    if (CarriedAmmoByWeaponTypeMap.Contains(EquippedWeapon->GetWeaponType()))
    {
        CarriedAmmoByWeaponTypeMap[EquippedWeapon->GetWeaponType()] -= ReloadAmmo;
        CarriedAmmo = CarriedAmmoByWeaponTypeMap[EquippedWeapon->GetWeaponType()];
    }
    EquippedWeapon->AddAmmo(ReloadAmmo);

    // Need to update for the server
    SetHUDCarriedAmmo();
}

void UWTRCombatComponent::ReloadShotgunAndSubCarriedAmmo()
{
    if (!EquippedWeapon || LastEquippedWeapon != EquippedWeapon || EquippedWeapon->GetWeaponType() != EWeaponType::EWT_Shotgun) return;

    // Reloading shotgun and subtract 1 ammo
    if (CarriedAmmoByWeaponTypeMap.Contains(EWeaponType::EWT_Shotgun))
    {
        CarriedAmmoByWeaponTypeMap[EWeaponType::EWT_Shotgun] -= 1;
        CarriedAmmo = CarriedAmmoByWeaponTypeMap[EWeaponType::EWT_Shotgun];
    }
    EquippedWeapon->AddAmmo(1);

    // Need to update for the server
    SetHUDCarriedAmmo();

    // TODO minus value of carried weapon on clients
    bCanFire = true;

    if (EquippedWeapon->IsFull() || CarriedAmmo == 0)
    {
        JumpToShotgunEnd();
    }
}

void UWTRCombatComponent::ShotgunShellReload()
{
    if (Character && Character->HasAuthority())
    {
        ReloadShotgunAndSubCarriedAmmo();
    }
}

void UWTRCombatComponent::FinishReloading()
{
    if (!Character) return;

    bLocallyReloading = false;

    if (Character->HasAuthority() && CombatState == ECombatState::ECS_Reloading)
    {
        CombatState = ECombatState::ECS_Unoccupied;

        const bool bShowScope = Character->IsLocallyControlled() && bIsAiming && EquippedWeapon &&
                                EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle;
        if (bShowScope)
        {
            Character->SetShowScopeAnimation(bShowScope);
        }

        ReloadWeaponAndSubCarriedAmmo();
    }

    if (bFireButtonPressed)
    {
        Fire();
    }
}

void UWTRCombatComponent::JumpToShotgunEnd()
{
    if (Character && Character->GetMesh() && Character->GetReloadMontage())
    {
        UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance();
        if (AnimInstance)
        {
            AnimInstance->Montage_JumpToSection(FName("ShotgunEnd"), Character->GetReloadMontage());
        }
    }
}

void UWTRCombatComponent::AddPickupAmmo(EWeaponType Type, int32 Ammo)
{
    if (CarriedAmmoByWeaponTypeMap.Contains(Type))
    {
        int32 MaxCarriedAmmoForType = 0;
        switch (Type)
        {
            case EWeaponType::EWT_AssaultRifle: MaxCarriedAmmoForType = Max_AssaultRifleCarrAmmo; break;
            case EWeaponType::EWT_RocketLauncher: MaxCarriedAmmoForType = Max_RocketLauncherCarrAmmo; break;
            case EWeaponType::EWT_Pistol: MaxCarriedAmmoForType = Max_PistolCarrAmmo; break;
            case EWeaponType::EWT_SubmachineGun: MaxCarriedAmmoForType = Max_SubmachineGunCarrAmmo; break;
            case EWeaponType::EWT_Shotgun: MaxCarriedAmmoForType = Max_ShotgunCarrAmmo; break;
            case EWeaponType::EWT_SniperRifle: MaxCarriedAmmoForType = Max_SniperRifleCarrAmmo; break;
            case EWeaponType::EWT_GrenadeLauncher: MaxCarriedAmmoForType = Max_GrenadeLauncherCarrAmmo; break;
            case EWeaponType::EWT_Flamethrower: MaxCarriedAmmoForType = Max_FlamethrowerCarrAmmo; break;
        }

        CarriedAmmoByWeaponTypeMap[Type] = FMath::Clamp(CarriedAmmoByWeaponTypeMap[Type] + Ammo, 0, MaxCarriedAmmoForType);

        UpdateCarriedAmmoAndHUD();
    }

    if (EquippedWeapon && EquippedWeapon->IsEmpty() && EquippedWeapon->GetWeaponType() == Type)
    {
        Reload();
    }
}

void UWTRCombatComponent::TraceFromScreen(FHitResult& TraceFromScreenHitResult)
{
    FVector2D ViewportSize;
    if (GEngine && GEngine->GameViewport)
    {
        GEngine->GameViewport->GetViewportSize(ViewportSize);
    }

    const FVector2D CrosshairLocation = FVector2D(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);
    FVector CrosshairWorldPosition;
    FVector CrosshairWorldDirection;

    bool bDeprojectScreen = UGameplayStatics::DeprojectScreenToWorld(  //
        UGameplayStatics::GetPlayerController(this, 0),                //
        CrosshairLocation,                                             //
        CrosshairWorldPosition,                                        //
        CrosshairWorldDirection                                        //
    );

    if (bDeprojectScreen && GetWorld())
    {
        FVector Start = CrosshairWorldPosition;
        if (Character)
        {
            const float DistanceToCharacter = (Character->GetActorLocation() - Start).Size();
            Start += CrosshairWorldDirection * (DistanceToCharacter + DistanceFromCamera);
        }

        const FVector End = CrosshairWorldPosition + CrosshairWorldDirection * TRACE_RANGE;

        GetWorld()->LineTraceSingleByChannel(  //
            TraceFromScreenHitResult,          //
            Start,                             //
            End,                               //
            ECC_Visibility                     //
        );

        if (EquippedWeapon && EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Flamethrower)
        {
            TraceFromScreenHitResult.ImpactPoint = End;
            HUDPackage.CrosshairColor = CrosshairColorWithoutTarget;
            return;
        }

        if (!TraceFromScreenHitResult.bBlockingHit)
        {
            TraceFromScreenHitResult.ImpactPoint = End;
        }

        if (TraceFromScreenHitResult.GetActor() && TraceFromScreenHitResult.GetActor()->Implements<UInteractWithCrosshairInterface>())
        {
            HUDPackage.CrosshairColor = CrosshairColorWithTarget;
        }
        else
        {
            HUDPackage.CrosshairColor = CrosshairColorWithoutTarget;
        }
    }
}

void UWTRCombatComponent::SetAiming(bool bAiming)
{
    if (bIsAiming == bAiming || !Character) return;

    bIsAiming = bAiming;

    Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;

    if (Character->IsLocallyControlled())
    {
        bAimButtonPressed = bAiming;
    }

    if (Character->IsLocallyControlled() && EquippedWeapon && EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle)
    {
        // We don`t want to scope if we are not Unoccupied at this time
        if (bAiming && CombatState == ECombatState::ECS_Unoccupied)
        {
            Character->SetShowScopeAnimation(bAiming);
        }
        else if (!bAiming && CombatState == ECombatState::ECS_Unoccupied)
        {
            Character->SetShowScopeAnimation(bAiming);
        }
    }

    Server_SetAiming(bAiming);
}

void UWTRCombatComponent::Server_SetAiming_Implementation(bool bAiming)
{
    bIsAiming = bAiming;

    if (Character)
    {
        Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
    }
}

void UWTRCombatComponent::OnRep_IsAiming()
{
    if (Character && Character->IsLocallyControlled())
    {
        bIsAiming = bAimButtonPressed;
    }
}

bool UWTRCombatComponent::CanFire() const
{
    const bool bIsShotgunCanFire = EquippedWeapon && !EquippedWeapon->IsEmpty() &&
                                   EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Shotgun && bCanFire &&
                                   CombatState == ECombatState::ECS_Reloading;
    if (bIsShotgunCanFire)
    {
        return true;
    }
    return EquippedWeapon && !EquippedWeapon->IsEmpty() && bCanFire && CombatState == ECombatState::ECS_Unoccupied;
}

bool UWTRCombatComponent::CanSwapWeapon() const
{
    return EquippedWeapon && SecondWeapon;
}

EWeaponType UWTRCombatComponent::GetEquippedWeaponType() const
{
    if (EquippedWeapon)
    {
        return EquippedWeapon->GetWeaponType();
    }
    return EWeaponType::EWT_MAX;
}

int32 UWTRCombatComponent::GetEquippedWeaponAmmo() const
{
    if (EquippedWeapon)
    {
        return EquippedWeapon->GetAmmo();
    }
    return 0;
}
