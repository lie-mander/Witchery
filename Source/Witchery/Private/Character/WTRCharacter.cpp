// Witchery. Copyright Liemander. All Rights Reserved.

#include "Character/WTRCharacter.h"
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
#include "HUD/OverheadWidget.h"

AWTRCharacter::AWTRCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
    SpringArmComponent->SetupAttachment(GetMesh());
    SpringArmComponent->TargetArmLength = 600.f;
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

    GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECollisionResponse::ECR_Ignore);
    GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECollisionResponse::ECR_Ignore);

    TurningInPlace = ETurningInPlace::ETIP_NotTurning;

    NetUpdateFrequency = 66.f;
    MinNetUpdateFrequency = 33.f;
}

void AWTRCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME_CONDITION(AWTRCharacter, OverlappingWeapon, COND_OwnerOnly);
    DOREPLIFETIME(AWTRCharacter, Username);
}

void AWTRCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    UpdateAimOffset(DeltaTime);
}

void AWTRCharacter::BeginPlay()
{
    Super::BeginPlay();

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

    PlayerInputComponent->BindAction("Jump", EInputEvent::IE_Pressed, this, &ACharacter::Jump);
    PlayerInputComponent->BindAction("Equip", EInputEvent::IE_Pressed, this, &ThisClass::OnEquipButtonPressed);
    PlayerInputComponent->BindAction("Crouch", EInputEvent::IE_Pressed, this, &ThisClass::OnCrouchButtonPressed);
    PlayerInputComponent->BindAction("Aim", EInputEvent::IE_Pressed, this, &ThisClass::OnAimButtonPressed);
    PlayerInputComponent->BindAction("Aim", EInputEvent::IE_Released, this, &ThisClass::OnAimButtonReleased);

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

    // If character without weapon - he`s using unequipped walk animations and we must to update StartAimRotation
    // Because can be glitching situations
    if (!IsWeaponEquipped())
    {
        UpdateIfIsNotStanding();
        return;
    }

    FVector Velocity = GetVelocity();
    Velocity.Z = 0.f;
    const float Speed = Velocity.Size();
    const bool bIsInAir = GetMovementComponent()->IsFalling();

    // Character is moving and we want to 'use control yaw rotation'
    // We need to save every frame StartAimRotation, and when we will stop, we can use it for calcucation delta rotation
    // And also we want to set yaw to 0.f for currect start using aim offset without glitching
    if (Speed > 0.f || bIsInAir)
    {
        UpdateIfIsNotStanding();

        bUseControllerRotationYaw = true;
        return;
    }

    // Character stands and does not jump
    // So we can update yaw and pitch for aiming offsets
    // We must turn off 'use control yaw rotation', cause we want that character stay an same pose (legs) and move only hands
    if (FMath::IsNearlyZero(Speed) && !bIsInAir)
    {
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

void AWTRCharacter::SetTurningInPlace(float DeltaTime)
{
    if (AO_Yaw > 90.f)
    {
        TurningInPlace = ETurningInPlace::ETIP_Right;
    }
    else if (AO_Yaw < -90.f)
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
    if (Combat)
    {
        Combat->SetAiming(true);
    }
}

void AWTRCharacter::OnAimButtonReleased()
{
    if (Combat)
    {
        Combat->SetAiming(false);
    }
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

bool AWTRCharacter::IsWeaponEquipped() const
{
    return Combat && Combat->EquippedWeapon;
}

bool AWTRCharacter::IsAiming() const
{
    return Combat && Combat->bIsAiming;
}
