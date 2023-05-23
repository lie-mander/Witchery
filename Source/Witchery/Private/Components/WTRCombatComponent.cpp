// Witchery. Copyright Liemander. All Rights Reserved.

#include "Components/WTRCombatComponent.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Weapons/WTRWeapon.h"
#include "Character/WTRCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Character/WTRPlayerController.h"
#include "HUD/WTR_HUD.h"
#include "Camera/CameraComponent.h"
#include "TimerManager.h"

UWTRCombatComponent::UWTRCombatComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UWTRCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(UWTRCombatComponent, EquippedWeapon);
    DOREPLIFETIME(UWTRCombatComponent, bIsAiming);
}

void UWTRCombatComponent::BeginPlay()
{
    Super::BeginPlay();

    if (Character && Character->GetCameraComponent())
    {
        DefaultZoomFOV = Character->GetCameraComponent()->FieldOfView;
        CurrentZoomFOV = DefaultZoomFOV;
    }
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

void UWTRCombatComponent::DrawCrosshair(float DeltaTime)
{
    if (!Character || !Controller || !HUD) return;

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

    EquippedWeapon = WeaponToEquip;
    EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
    EquippedWeapon->SetOwner(Character);

    Character->GetCharacterMovement()->bOrientRotationToMovement = false;
    Character->bUseControllerRotationYaw = true;
    Character->GetSpringArm()->SetRelativeTransform(FTransform(FQuat4d(FRotator::ZeroRotator), SpringArmOffsetWhileEquipped));

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
        Character->GetCharacterMovement()->bOrientRotationToMovement = false;
        Character->bUseControllerRotationYaw = true;
        Character->GetSpringArm()->SetRelativeTransform(FTransform(FQuat4d(FRotator::ZeroRotator), SpringArmOffsetWhileEquipped));
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
    if (bCanFire)
    {
        bCanFire = false;
        ServerFire(HitTarget);

        if (EquippedWeapon)
        {
            CrosshairShootingFactor = ShootingFactorSpread;
        }

        FireTimerStart();
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

void UWTRCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
    MulticastFire(TraceHitTarget);
}

void UWTRCombatComponent::MulticastFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
    if (Character && EquippedWeapon)
    {
        Character->PlayFireMontage(bIsAiming);
        EquippedWeapon->Fire(TraceHitTarget);
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

    ServerSetAiming(bAiming);
}

void UWTRCombatComponent::ServerSetAiming_Implementation(bool bAiming)
{
    bIsAiming = bAiming;

    if (Character)
    {
        Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
    }
}
