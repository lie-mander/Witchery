// Witchery. Copyright Liemander. All Rights Reserved.

#include "Character/WTRCharacter.h"
#include "Character/WTRPlayerController.h"
#include "Character/WTRAnimInstance.h"
#include "Net/UnrealNetwork.h"
#include "Weapons/WTRWeapon.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerState.h"
#include "Camera/CameraComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/WTRCombatComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/TextRenderComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "HUD/OverheadWidget.h"
#include "HUD/WTR_HUD.h"
#include "GameModes/WTRGameMode.h"

AWTRCharacter::AWTRCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
    SpringArmComponent->SetupAttachment(GetMesh());
    SpringArmComponent->TargetArmLength = 600.f;
    SpringArmComponent->SetRelativeTransform(FTransform(FQuat4d(FRotator::ZeroRotator), FVector3d(0.f, 0.f, 180.f)));
    SpringArmComponent->bUsePawnControlRotation = true;

    CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
    CameraComponent->SetupAttachment(SpringArmComponent, USpringArmComponent::SocketName);
    CameraComponent->bUsePawnControlRotation = true;

    bUseControllerRotationYaw = false;
    GetCharacterMovement()->bOrientRotationToMovement = true;

    OverheadWidget = CreateDefaultSubobject<UWidgetComponent>("OverheadWidget");
    OverheadWidget->SetupAttachment(RootComponent);

    OverheadText = CreateDefaultSubobject<UTextRenderComponent>("OverheadText");
    OverheadText->SetupAttachment(RootComponent);
    OverheadText->bOwnerNoSee = true;

    Combat = CreateDefaultSubobject<UWTRCombatComponent>("Combat");
    Combat->SetIsReplicated(true);

    GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
    GetCharacterMovement()->RotationRate = FRotator(0.f, 0.f, 800.f);
    GetCharacterMovement()->MaxWalkSpeedCrouched = 250.f;
    GetCharacterMovement()->JumpZVelocity = 800.f;
    GetCharacterMovement()->AirControl = 0.3f;
    GetCharacterMovement()->bApplyGravityWhileJumping = false;

    GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECollisionResponse::ECR_Ignore);
    GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);
    GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECollisionResponse::ECR_Ignore);
    GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECollisionResponse::ECR_Block);

    TurningInPlace = ETurningInPlace::ETIP_NotTurning;

    NetUpdateFrequency = 66.f;
    MinNetUpdateFrequency = 33.f;
}

void AWTRCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME_CONDITION(AWTRCharacter, OverlappingWeapon, COND_OwnerOnly);
    DOREPLIFETIME(AWTRCharacter, Username);
    DOREPLIFETIME(AWTRCharacter, Health);
}

void AWTRCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (GetLocalRole() > ENetRole::ROLE_SimulatedProxy && IsLocallyControlled())
    {
        UpdateAimOffset(DeltaTime);
    }
    else
    {
        CalculateAO_Pitch();

        TimeSinceLastMovementReplication += DeltaTime;
        if (TimeSinceLastMovementReplication > 0.25f)
        {
            OnRep_ReplicateMovement();
        }
    }

    HideCharacterWithWeaponIfCameraClose();
}

void AWTRCharacter::BeginPlay()
{
    Super::BeginPlay();

    check(FireWeaponMontage);
    check(HitReactMontage);

    // Send controller and HUD to Combat component
    WTRPlayerController = Cast<AWTRPlayerController>(Controller);
    if (Combat && WTRPlayerController)
    {
        Combat->Controller = WTRPlayerController;
        if (WTRPlayerController)
        {
            AWTR_HUD* WTR_HUD = Cast<AWTR_HUD>(WTRPlayerController->GetHUD());
            if (WTR_HUD)
            {
                Combat->HUD = WTR_HUD;
            }
        }
    }

    // Set current health at start
    if (WTRPlayerController)
    {
        WTRPlayerController->SetHUDHealth(Health, MaxHealth);
    }

    // Set callbacks for authority character
    if (HasAuthority())
    {
        OnTakeAnyDamage.AddDynamic(this, &ThisClass::OnTakeAnyDamageCallback);
    }

    // Set user name
    if (IsLocallyControlled() && GetPlayerState() && HasAuthority())
    {
        Username = GetPlayerState()->GetPlayerName();
    }
    else if (IsLocallyControlled() && !HasAuthority())
    {
        Server_SetUsername();
    }
}

void AWTRCharacter::Server_SetUsername_Implementation()
{
    if (GetPlayerState() && OverheadText)
    {
        Username = GetPlayerState()->GetPlayerName();

        OverheadText->SetText(FText::FromString(Username));
        OverheadText->SetTextRenderColor(FColor::MakeRandomColor());
    }
}

void AWTRCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    PlayerInputComponent->BindAction("Jump", EInputEvent::IE_Pressed, this, &ThisClass::Jump);
    PlayerInputComponent->BindAction("Equip", EInputEvent::IE_Pressed, this, &ThisClass::OnEquipButtonPressed);
    PlayerInputComponent->BindAction("Crouch", EInputEvent::IE_Pressed, this, &ThisClass::OnCrouchButtonPressed);
    PlayerInputComponent->BindAction("Aim", EInputEvent::IE_Pressed, this, &ThisClass::OnAimButtonPressed);
    PlayerInputComponent->BindAction("Aim", EInputEvent::IE_Released, this, &ThisClass::OnAimButtonReleased);
    PlayerInputComponent->BindAction("Fire", EInputEvent::IE_Pressed, this, &ThisClass::OnFireButtonPressed);
    PlayerInputComponent->BindAction("Fire", EInputEvent::IE_Released, this, &ThisClass::OnFireButtonReleased);
    PlayerInputComponent->BindAction("Pause", EInputEvent::IE_Pressed, this, &ThisClass::OnPauseButtonPressed);

    PlayerInputComponent->BindAxis(FName("MoveForward"), this, &ThisClass::MoveForward);
    PlayerInputComponent->BindAxis(FName("MoveRight"), this, &ThisClass::MoveRight);
    PlayerInputComponent->BindAxis(FName("Turn"), this, &ThisClass::Turn);
    PlayerInputComponent->BindAxis(FName("LookUp"), this, &ThisClass::LookUp);
}

void AWTRCharacter::PostInitializeComponents()
{
    Super::PostInitializeComponents();

    if (Combat)
    {
        Combat->Character = this;
    }
}

void AWTRCharacter::MoveForward(float Amount)
{
    if (Controller && Amount != 0.f)
    {
        const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
        const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
        AddMovementInput(Direction, Amount);
    }
}

void AWTRCharacter::MoveRight(float Amount)
{
    if (Controller && Amount != 0.f)
    {
        const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
        const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
        AddMovementInput(Direction, Amount);
    }
}

void AWTRCharacter::Turn(float Amount)
{
    AddControllerYawInput(Amount);
}

void AWTRCharacter::LookUp(float Amount)
{
    AddControllerPitchInput(Amount);
}

void AWTRCharacter::UpdateAimOffset(float DeltaTime)
{
    CalculateAO_Pitch();

    // If character without weapon - he`s using unequipped walk animations and we must to update StartAimRotation
    // Because can be glitching situations
    if (!IsWeaponEquipped())
    {
        UpdateIfIsNotStanding();
        return;
    }

    const float Speed = CalculateSpeed();
    const bool bIsInAir = GetMovementComponent()->IsFalling();

    // Character is moving and we want to 'use control yaw rotation'
    // We need to save every frame StartAimRotation, and when we will stop, we can use it for calcucation delta rotation
    // And also we want to set yaw to 0.f for currect start using aim offset without glitching
    if (Speed > 0.f || bIsInAir)
    {
        bRotateRootBone = false;

        UpdateIfIsNotStanding();

        bUseControllerRotationYaw = true;
        return;
    }

    // Character stands and does not jump
    // So we can update yaw and pitch for aiming offsets
    // We must turn off 'use control yaw rotation', cause we want that character stay an same pose (legs) and move only hands
    if (FMath::IsNearlyZero(Speed) && !bIsInAir)
    {
        bRotateRootBone = true;

        const FRotator CurrentAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
        const FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation, StartAimRotation);
        AO_Yaw = DeltaAimRotation.Yaw;

        if (TurningInPlace == ETurningInPlace::ETIP_NotTurning)
        {
            InterpAO_Yaw = AO_Yaw;
        }

        SetTurningInPlace(DeltaTime);

        bUseControllerRotationYaw = true;
    }
}

void AWTRCharacter::OnRep_ReplicateMovement()
{
    Super::OnRep_ReplicateMovement();

    SimProxiesTurn();
    TimeSinceLastMovementReplication = 0.f;
}

void AWTRCharacter::SimProxiesTurn()
{
    if (!Combat || !Combat->EquippedWeapon)
    {
        return;
    }

    bRotateRootBone = false;

    const float Speed = CalculateSpeed();
    if (Speed > 0.f)
    {
        TurningInPlace = ETurningInPlace::ETIP_NotTurning;
        return;
    }

    SimProxyLastFrameRotation = SimProxyRotation;
    SimProxyRotation = GetActorRotation();
    SimProxyDeltaYaw = UKismetMathLibrary::NormalizedDeltaRotator(SimProxyRotation, SimProxyLastFrameRotation).Yaw;

    if (FMath::Abs(SimProxyDeltaYaw) > SimProxyTurnThreshold)
    {
        if (SimProxyDeltaYaw > SimProxyTurnThreshold)
        {
            TurningInPlace = ETurningInPlace::ETIP_Right;
        }
        else if (SimProxyDeltaYaw < -SimProxyTurnThreshold)
        {
            TurningInPlace = ETurningInPlace::ETIP_Left;
        }
        else
        {
            TurningInPlace = ETurningInPlace::ETIP_NotTurning;
        }
        return;
    }
    TurningInPlace = ETurningInPlace::ETIP_NotTurning;
}

void AWTRCharacter::CalculateAO_Pitch()
{
    // We can update pitch every frame
    AO_Pitch = GetBaseAimRotation().Pitch;

    // Fix for sending pitch to the simulated proxy if it greater then 90 degrees
    // Because when we sent it = negative float convert to signed number in [270; 360) area
    // We must convert it back to [-90; 0)
    if (AO_Pitch > 90.f && !IsLocallyControlled())
    {
        FVector2D InRange(270.f, 360.f);
        FVector2D OutRange(-90.f, 0.f);
        AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);
    }
}

void AWTRCharacter::SetTurningInPlace(float DeltaTime)
{
    if (AO_Yaw > AngleToTurn)
    {
        TurningInPlace = ETurningInPlace::ETIP_Right;
    }
    else if (AO_Yaw < -AngleToTurn)
    {
        TurningInPlace = ETurningInPlace::ETIP_Left;
    }

    if (TurningInPlace != ETurningInPlace::ETIP_NotTurning)
    {
        InterpAO_Yaw = FMath::FInterpTo(InterpAO_Yaw, 0.f, DeltaTime, 4.f);
        AO_Yaw = InterpAO_Yaw;
        if (FMath::Abs(AO_Yaw) < 15.f)
        {
            StartAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
            TurningInPlace = ETurningInPlace::ETIP_NotTurning;
        }
    }
}

void AWTRCharacter::UpdateIfIsNotStanding()
{
    StartAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
    AO_Yaw = 0.f;
    TurningInPlace = ETurningInPlace::ETIP_NotTurning;
}

float AWTRCharacter::CalculateSpeed() const
{
    FVector Velocity = GetVelocity();
    Velocity.Z = 0.f;
    return Velocity.Size();
}

void AWTRCharacter::Jump()
{
    if (bIsCrouched)
    {
        UnCrouch();
    }
    else if (!GetMovementComponent()->IsFalling())
    {
        Super::Jump();
    }
}

void AWTRCharacter::PlayFireMontage(bool bAiming)
{
    if (!Combat || !Combat->EquippedWeapon || !GetMesh() || !FireWeaponMontage) return;

    UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
    if (!AnimInstance) return;

    AnimInstance->Montage_Play(FireWeaponMontage);
    const FName SectionName = bAiming ? FName("RifleHip") : FName("RifleAim");
    AnimInstance->Montage_JumpToSection(SectionName);
}

void AWTRCharacter::PlayHitReactMontage()
{
    if (!Combat || !Combat->EquippedWeapon || !GetMesh() || !HitReactMontage) return;

    UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
    if (!AnimInstance) return;

    AnimInstance->Montage_Play(HitReactMontage);
    const FName SectionName("FromFront");
    AnimInstance->Montage_JumpToSection(SectionName);
}

void AWTRCharacter::OnEquipButtonPressed()
{
    if (Combat)
    {
        if (HasAuthority())
        {
            Combat->EquipWeapon(OverlappingWeapon);
        }
        else
        {
            Server_OnEquippedButtonPressed();
        }
    }
}

void AWTRCharacter::Server_OnEquippedButtonPressed_Implementation()
{
    if (Combat)
    {
        Combat->EquipWeapon(OverlappingWeapon);
    }
}

void AWTRCharacter::OnCrouchButtonPressed()
{
    if (bIsCrouched)
    {
        UnCrouch();
    }
    else
    {
        Crouch();
    }
}

void AWTRCharacter::OnAimButtonPressed()
{
    if (Combat && Combat->EquippedWeapon)
    {
        Combat->SetAiming(true);
    }
}

void AWTRCharacter::OnAimButtonReleased()
{
    if (Combat && Combat->EquippedWeapon)
    {
        Combat->SetAiming(false);
    }
}

void AWTRCharacter::OnFireButtonPressed()
{
    if (Combat)
    {
        Combat->OnFireButtonPressed(true);
    }
}

void AWTRCharacter::OnFireButtonReleased()
{
    if (Combat)
    {
        Combat->OnFireButtonPressed(false);
    }
}

void AWTRCharacter::OnPauseButtonPressed()
{
    const auto PlayerController = Cast<APlayerController>(Controller);
    UKismetSystemLibrary::QuitGame(GetWorld(), PlayerController, EQuitPreference::Quit, true);
}

void AWTRCharacter::SetOverlappingWeapon(AWTRWeapon* Weapon)
{
    if (OverlappingWeapon)
    {
        OverlappingWeapon->SetShowWidget(false);
    }
    OverlappingWeapon = Weapon;
    if (IsLocallyControlled())
    {
        if (OverlappingWeapon)
        {
            OverlappingWeapon->SetShowWidget(true);
        }
    }
}

void AWTRCharacter::Elim() 
{

}

void AWTRCharacter::UpdateHUDHealth()
{
    WTRPlayerController = (WTRPlayerController == nullptr) ? Cast<AWTRPlayerController>(Controller) : WTRPlayerController;
    if (WTRPlayerController)
    {
        WTRPlayerController->SetHUDHealth(Health, MaxHealth);
    }
}

void AWTRCharacter::OnTakeAnyDamageCallback(
    AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
    Health = FMath::Clamp(Health - Damage, 0.f, MaxHealth);

    UpdateHUDHealth();
    PlayHitReactMontage();

    if (Health <= 0.f && GetWorld())
    {
        AWTRGameMode* WTRGameMode = Cast<AWTRGameMode>(GetWorld()->GetAuthGameMode());
        if (WTRGameMode)
        {
            WTRPlayerController = (WTRPlayerController == nullptr) ? Cast<AWTRPlayerController>(Controller) : WTRPlayerController;
            AWTRPlayerController* AttackerController = Cast<AWTRPlayerController>(InstigatedBy);

            WTRGameMode->PlayerEliminated(this, WTRPlayerController, AttackerController);
        }
    }
}

void AWTRCharacter::OnRep_Health()
{
    UpdateHUDHealth();
    PlayHitReactMontage();
}

void AWTRCharacter::OnRep_Username()
{
    if (OverheadText)
    {
        OverheadText->SetText(FText::FromString(Username));
        OverheadText->SetTextRenderColor(FColor::MakeRandomColor());
    }
}

void AWTRCharacter::OnRep_OverlappingWeapon(AWTRWeapon* LastWeapon)
{
    if (OverlappingWeapon)
    {
        OverlappingWeapon->SetShowWidget(true);
    }
    if (LastWeapon)
    {
        LastWeapon->SetShowWidget(false);
    }
}

AWTRWeapon* AWTRCharacter::GetEquippedWeapon() const
{
    if (!Combat) return nullptr;
    return Combat->EquippedWeapon;
}

FVector AWTRCharacter::GetHitTarget() const
{
    if (!Combat) return FVector();
    return Combat->HitTarget;
}

bool AWTRCharacter::IsWeaponEquipped() const
{
    return Combat && Combat->EquippedWeapon;
}

bool AWTRCharacter::IsAiming() const
{
    return Combat && Combat->bIsAiming;
}

void AWTRCharacter::HideCharacterWithWeaponIfCameraClose()
{
    if (!IsLocallyControlled())
    {
        return;
    }

    if (CameraComponent && (CameraComponent->GetComponentLocation() - GetActorLocation()).Size() < DistanceForHidingCamera)
    {
        GetMesh()->SetVisibility(false);
        if (Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh())
        {
            Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = true;
        }
    }
    else
    {
        GetMesh()->SetVisibility(true);
        if (Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh())
        {
            Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = false;
        }
    }
}
