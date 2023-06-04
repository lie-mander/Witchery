// Witchery. Copyright Liemander. All Rights Reserved.

#include "Components/WTRCombatComponent.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Weapons/WTRWeapon.h"
#include "Character/WTRCharacter.h"
#include "Character/WTRPlayerController.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "HUD/WTR_HUD.h"
#include "Camera/CameraComponent.h"
#include "TimerManager.h"
#include "Sound/SoundCue.h"

UWTRCombatComponent::UWTRCombatComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UWTRCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(UWTRCombatComponent, EquippedWeapon);
    DOREPLIFETIME(UWTRCombatComponent, bIsAiming);
    DOREPLIFETIME(UWTRCombatComponent, CombatState);
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
        FVector2D CharacterVelocityRange(0.f, Character->GetCharacterMovement()->MaxWalkSpeed);
        FVector2D OutputVelocityRange(0.f, 1.f);
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
    if (!EquippedWeapon || !Character->GetCameraComponent())
    {
        return;
    }

    if (bIsAiming)
    {
        CurrentZoomFOV = FMath::FInterpTo(CurrentZoomFOV, EquippedWeapon->GetZoomedFOV(), DeltaTime, EquippedWeapon->GetZoomInterpSpeed());
    }
    else
    {
        CurrentZoomFOV = FMath::FInterpTo(CurrentZoomFOV, DefaultZoomFOV, DeltaTime, ZoomInterpSpeed);
    }

    Character->GetCameraComponent()->SetFieldOfView(CurrentZoomFOV);
}

void UWTRCombatComponent::EquipWeapon(AWTRWeapon* WeaponToEquip)
{
    if (!Character || !WeaponToEquip) return;

    if (EquippedWeapon)
    {
        EquippedWeapon->Dropped();
        EquippedWeapon = nullptr;
    }

    EquippedWeapon = WeaponToEquip;
    EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
    EquippedWeapon->SetOwner(Character);

    // Need to know weapon owner, must be set after SetOwner() function
    EquippedWeapon->SetHUDAmmo();

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

    // Play pickup sound (for server, for client will play in OnRep_EquippedWeapon)
    UGameplayStatics::PlaySoundAtLocation(  //
        this,                               //
        EquippedWeapon->PickupSound,        //
        Character->GetActorLocation());

    // Want to reload if equipped weapon is empty
    if (EquippedWeapon->IsEmpty())
    {
        Reload();
    }

    Character->GetCharacterMovement()->bOrientRotationToMovement = false;
    Character->bUseControllerRotationYaw = true;
    Character->GetSpringArm()->SocketOffset = SpringArmOffsetWhileEquipped;
    // Character->GetSpringArm()->SetRelativeTransform(FTransform(FQuat4d(FRotator::ZeroRotator), SpringArmOffsetWhileEquipped));

    const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));
    if (HandSocket)
    {
        HandSocket->AttachActor(EquippedWeapon, Character->GetMesh());
    }
}

void UWTRCombatComponent::OnRep_EquippedWeapon()
{
    if (Character && EquippedWeapon)
    {
        EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
        const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));
        if (HandSocket)
        {
            HandSocket->AttachActor(EquippedWeapon, Character->GetMesh());
        }
        Character->GetCharacterMovement()->bOrientRotationToMovement = false;
        Character->bUseControllerRotationYaw = true;
        Character->GetSpringArm()->SocketOffset = SpringArmOffsetWhileEquipped;
        // Character->GetSpringArm()->SetRelativeTransform(FTransform(FQuat4d(FRotator::ZeroRotator), SpringArmOffsetWhileEquipped));

        // Play pickup sound
        UGameplayStatics::PlaySoundAtLocation(  //
            this,                               //
            EquippedWeapon->PickupSound,        //
            Character->GetActorLocation());

        Controller = (Controller == nullptr) ? Cast<AWTRPlayerController>(Character->Controller) : Controller;
        if (Controller)
        {
            Controller->SetHUDWeaponType(EquippedWeapon->GetWeaponType());
        }
    }
}

void UWTRCombatComponent::OnRep_CarriedAmmo()
{
    SetHUDCarriedAmmo();
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
    }
}

void UWTRCombatComponent::Fire()
{
    if (CanFire())
    {
        bCanFire = false;

        Server_Fire(HitTarget);

        if (EquippedWeapon)
        {
            CrosshairShootingFactor = ShootingFactorSpread;
        }

        FireTimerStart();
    }
    else if (EquippedWeapon && EquippedWeapon->IsEmpty())
    {
        Reload();
    }
}

void UWTRCombatComponent::FireTimerStart()
{
    if (!Character || !EquippedWeapon)
    {
        return;
    }

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

void UWTRCombatComponent::Server_Fire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
    Multicast_Fire(TraceHitTarget);
}

void UWTRCombatComponent::Multicast_Fire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
    if (Character && EquippedWeapon && CombatState == ECombatState::ECS_Unoccupied)
    {
        Character->PlayFireMontage(bIsAiming);
        EquippedWeapon->Fire(TraceHitTarget);
    }
}

void UWTRCombatComponent::Reload()
{
    if (CarriedAmmo > 0 && EquippedWeapon && !EquippedWeapon->IsFull() && CombatState != ECombatState::ECS_Reloading)
    {
        Server_Reload();
    }
}

void UWTRCombatComponent::Server_Reload_Implementation()
{
    if (!Character)
    {
        return;
    }

    CombatState = ECombatState::ECS_Reloading;
    ReloadHandle();
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
            break;

        case ECombatState::ECS_Reloading:  //
            ReloadHandle();
            break;
    }
}

void UWTRCombatComponent::ReloadHandle()
{
    Character->PlayReloadMontage();
}

int32 UWTRCombatComponent::AmmoToReload()
{
    if (!EquippedWeapon)
    {
        return 0;
    }

    int32 EmplyPlaceInMag = EquippedWeapon->GetMagazineCapacity() - EquippedWeapon->GetAmmo();
    if (CarriedAmmoByWeaponTypeMap.Contains(EquippedWeapon->GetWeaponType()))
    {
        int32 AmountCarried = CarriedAmmoByWeaponTypeMap[EquippedWeapon->GetWeaponType()];
        int32 Least = FMath::Min(EmplyPlaceInMag, AmountCarried);

        return FMath::Clamp(EmplyPlaceInMag, 0, Least);
    }

    return 0;
}

void UWTRCombatComponent::ReloadWeaponAndSubCarriedAmmo()
{
    if (!EquippedWeapon)
    {
        return;
    }

    // Reloading weapon and subtract carried ammo for this weapon type
    int32 ReloadAmmo = AmmoToReload();
    if (CarriedAmmoByWeaponTypeMap.Contains(EquippedWeapon->GetWeaponType()))
    {
        CarriedAmmoByWeaponTypeMap[EquippedWeapon->GetWeaponType()] -= ReloadAmmo;
        CarriedAmmo = CarriedAmmoByWeaponTypeMap[EquippedWeapon->GetWeaponType()];
    }
    EquippedWeapon->AddAmmo(ReloadAmmo);

    // Need to update for the server
    SetHUDCarriedAmmo();
}

void UWTRCombatComponent::FinishReloading()
{
    if (!Character)
    {
        return;
    }

    if (Character->HasAuthority() && CombatState == ECombatState::ECS_Reloading)
    {
        CombatState = ECombatState::ECS_Unoccupied;

        ReloadWeaponAndSubCarriedAmmo();
    }

    if (bFireButtonPressed)
    {
        Fire();
    }
}

void UWTRCombatComponent::TraceFromScreen(FHitResult& TraceFromScreenHitResult)
{
    FVector2D ViewportSize;
    if (GEngine && GEngine->GameViewport)
    {
        GEngine->GameViewport->GetViewportSize(ViewportSize);
    }

    FVector2D CrosshairLocation = FVector2D(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);
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

        FVector End = CrosshairWorldPosition + CrosshairWorldDirection * TraceRange;

        GetWorld()->LineTraceSingleByChannel(  //
            TraceFromScreenHitResult,          //
            Start,                             //
            End,                               //
            ECC_Visibility                     //
        );

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
    bIsAiming = bAiming;

    if (Character)
    {
        Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
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

bool UWTRCombatComponent::CanFire() const
{
    return EquippedWeapon && !EquippedWeapon->IsEmpty() && bCanFire && CombatState == ECombatState::ECS_Unoccupied;
}
