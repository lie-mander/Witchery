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
#include "Components/WTRBuffComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/TextRenderComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "HUD/OverheadWidget.h"
#include "HUD/WTR_HUD.h"
#include "GameModes/WTRGameMode.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystemComponent.h"
#include "WTRPlayerState.h"
#include "WTRTypes.h"
#include "WTRTools.h"
#include "Animation/Notifies/WTRReloadFinishedAnimNotify.h"

AWTRCharacter::AWTRCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>("SpringArmComponent");
    SpringArmComponent->SetupAttachment(GetMesh());
    SpringArmComponent->TargetArmLength = 600.f;
    SpringArmComponent->SetRelativeTransform(FTransform(FQuat4d(FRotator::ZeroRotator), FVector3d(0.f, 0.f, 180.f)));
    SpringArmComponent->bUsePawnControlRotation = true;

    CameraComponent = CreateDefaultSubobject<UCameraComponent>("CameraComponent");
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

    Buff = CreateDefaultSubobject<UWTRBuffComponent>("Buff");
    Buff->SetIsReplicated(true);

    DissolveTimelineComponent = CreateDefaultSubobject<UTimelineComponent>("DissolveTimelineComponent");

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

    GrenadeMesh = CreateDefaultSubobject<UStaticMeshComponent>("GrenadeMesh");
    GrenadeMesh->SetupAttachment(GetMesh(), FName("GrenadeSocket"));
    GrenadeMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    TurningInPlace = ETurningInPlace::ETIP_NotTurning;

    NetUpdateFrequency = 66.f;
    MinNetUpdateFrequency = 33.f;

    SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
}

void AWTRCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME_CONDITION(AWTRCharacter, OverlappingWeapon, COND_OwnerOnly);
    DOREPLIFETIME(AWTRCharacter, Username);
    DOREPLIFETIME(AWTRCharacter, Health);
    DOREPLIFETIME(AWTRCharacter, bDisableGameplay);
}

void AWTRCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    RotateInPlace(DeltaTime);
    HideCharacterWithWeaponIfCameraClose();
    PullInit();
}

void AWTRCharacter::BeginPlay()
{
    Super::BeginPlay();

    check(EliminationMontage);
    check(FireWeaponMontage);
    check(HitReactMontage);
    check(DissolveCurve);
    check(DissolveMaterialInst);
    check(ElimBotParticleSys);

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

    // Set callbacks for anim notifies
    if (ReloadMontage)
    {
        const auto NotifyEvents = ReloadMontage->Notifies;
        for (const auto NotifyStructs : NotifyEvents)
        {
            const auto WTRReloadFinishedAnimNotify = Cast<UWTRReloadFinishedAnimNotify>(NotifyStructs.Notify);
            if (WTRReloadFinishedAnimNotify)
            {
                WTRReloadFinishedAnimNotify->OnNotifyPlayed.AddUObject(this, &ThisClass::OnReloadFinishedNotifyPlayed);
            }
        }
    }

    // Need to hide grenade mesh when player spawned
    if (GrenadeMesh)
    {
        GrenadeMesh->SetVisibility(false);
    }

    // Need to disable input if match state == cooldown
    if (GetWTRGameMode() && GetWTRGameMode()->GetMatchState() == MatchState::Cooldown)
    {
        bDisableGameplay = true;
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
    PlayerInputComponent->BindAction("Reload", EInputEvent::IE_Pressed, this, &ThisClass::OnReloadButtonPressed);
    PlayerInputComponent->BindAction("AudioUp", EInputEvent::IE_Pressed, this, &ThisClass::OnAudioUpButtonPressed);
    PlayerInputComponent->BindAction("AudioDown", EInputEvent::IE_Pressed, this, &ThisClass::OnAudioDownButtonPressed);
    PlayerInputComponent->BindAction("Grenade", EInputEvent::IE_Pressed, this, &ThisClass::OnGrenadeButtonPressed);

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

    if (Buff)
    {
        Buff->Character = this;
        if (GetCharacterMovement())
        {
            Buff->InitialBaseSpeed = GetCharacterMovement()->MaxWalkSpeed;
            Buff->InitialCrouchSpeed = GetCharacterMovement()->MaxWalkSpeedCrouched;
        }
    }
}

void AWTRCharacter::Destroyed()
{
    Super::Destroyed();

    const AWTRGameMode* WTRGameMode = GetWTRGameMode();
    bool bMatchIsNotInProgress = WTRGameMode && WTRGameMode->GetMatchState() != MatchState::InProgress;

    if (Combat && Combat->EquippedWeapon && bMatchIsNotInProgress)
    {
        Combat->EquippedWeapon->Destroy();
    }
}

void AWTRCharacter::PullInit()
{
    if (!WTRPlayerState)
    {
        WTRPlayerState = GetPlayerState<AWTRPlayerState>();
        if (WTRPlayerState)
        {
            WTRPlayerState->AddToScore(0.f);
            WTRPlayerState->AddToDefeats(0);

            // Set username
            if (HasAuthority() && OverheadText)
            {
                Username = WTRPlayerState->GetPlayerName();
                OverheadText->SetText(FText::FromString(Username));
                OverheadText->SetTextRenderColor(FColor::MakeRandomColor());
            }
        }
    }
}

void AWTRCharacter::RotateInPlace(float DeltaTime)
{
    if (bDisableGameplay)
    {
        bUseControllerRotationYaw = false;
        TurningInPlace = ETurningInPlace::ETIP_NotTurning;
        return;
    }

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
}

void AWTRCharacter::OnPossessHandle(AWTRPlayerController* NewController, AWTR_HUD* NewHUD)
{
    if (Combat && NewController && NewHUD)
    {
        Combat->Controller = NewController;
        Combat->HUD = NewHUD;
    }
}

void AWTRCharacter::MoveForward(float Amount)
{
    if (bDisableGameplay) return;

    if (Controller && Amount != 0.f)
    {
        const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
        const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
        AddMovementInput(Direction, Amount);
    }
}

void AWTRCharacter::MoveRight(float Amount)
{
    if (bDisableGameplay) return;

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
    if (!Combat || !Combat->EquippedWeapon) return;

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
    if (bDisableGameplay) return;

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

void AWTRCharacter::PlayReloadMontage()
{
    if (!Combat || !Combat->EquippedWeapon || !GetMesh() || !ReloadMontage) return;

    UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
    if (!AnimInstance) return;

    AnimInstance->Montage_Play(ReloadMontage);

    FName SectionName;

    switch (Combat->EquippedWeapon->GetWeaponType())
    {
        case EWeaponType::EWT_AssaultRifle: SectionName = FName("Rifle"); break;
        case EWeaponType::EWT_RocketLauncher: SectionName = FName("RocketLauncher"); break;
        case EWeaponType::EWT_Pistol: SectionName = FName("Pistol"); break;
        case EWeaponType::EWT_SubmachineGun: SectionName = FName("Pistol"); break;
        case EWeaponType::EWT_Shotgun: SectionName = FName("Shotgun"); break;
        case EWeaponType::EWT_SniperRifle: SectionName = FName("SniperRifle"); break;
        case EWeaponType::EWT_GrenadeLauncher: SectionName = FName("RocketLauncher"); break;
    }

    UE_LOG(LogTemp, Display, TEXT("%s"), *SectionName.ToString());
    AnimInstance->Montage_JumpToSection(SectionName);
}

void AWTRCharacter::StopReloadMontage()
{
    if (!Combat || !Combat->EquippedWeapon || !GetMesh() || !ReloadMontage) return;

    UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
    if (!AnimInstance) return;

    AnimInstance->Montage_Stop(0.3f, ReloadMontage);
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

void AWTRCharacter::PlayEliminationMontage()
{
    if (!GetMesh() || !EliminationMontage) return;

    UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
    if (!AnimInstance) return;

    AnimInstance->Montage_Play(EliminationMontage);
}

void AWTRCharacter::PlayThrowGrenadeMontage()
{
    if (!GetMesh() || !ThrowGrenadeMontage) return;

    UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
    if (!AnimInstance) return;

    AnimInstance->Montage_Play(ThrowGrenadeMontage);
}

void AWTRCharacter::StopThrowGrenadeMontage()
{
    if (!Combat || !Combat->EquippedWeapon || !GetMesh() || !ThrowGrenadeMontage) return;

    UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
    if (!AnimInstance) return;

    AnimInstance->Montage_Stop(0.3f, ThrowGrenadeMontage);
}

void AWTRCharacter::OnEquipButtonPressed()
{
    if (bDisableGameplay) return;

    if (Combat)
    {
        Combat->SetAiming(false);

        if (HasAuthority())
        {
            Combat->EquipWeapon(OverlappingWeapon);
        }
        else
        {
            // Need to call on client for stopping reload montage, for server this logic calls in Combat->EquipWeapon()
            Combat->StopReloadWhileEquip();

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
    if (bDisableGameplay) return;

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
    if (bDisableGameplay) return;

    if (Combat && Combat->EquippedWeapon)
    {
        Combat->SetAiming(true);
    }
}

void AWTRCharacter::OnAimButtonReleased()
{
    if (bDisableGameplay) return;

    if (Combat && Combat->EquippedWeapon)
    {
        Combat->SetAiming(false);
    }
}

void AWTRCharacter::OnFireButtonPressed()
{
    if (bDisableGameplay) return;

    if (Combat)
    {
        Combat->OnFireButtonPressed(true);
    }
}

void AWTRCharacter::OnFireButtonReleased()
{
    if (bDisableGameplay) return;

    if (Combat)
    {
        Combat->OnFireButtonPressed(false);
    }
}

void AWTRCharacter::OnReloadButtonPressed()
{
    if (bDisableGameplay) return;

    if (Combat)
    {
        Combat->Reload();
    }
}

void AWTRCharacter::OnAudioUpButtonPressed()
{
    WTRPlayerController = (WTRPlayerController == nullptr) ? Cast<AWTRPlayerController>(Controller) : WTRPlayerController;
    if (WTRPlayerController)
    {
        WTRPlayerController->VolumeUp();
    }
}

void AWTRCharacter::OnAudioDownButtonPressed()
{
    WTRPlayerController = (WTRPlayerController == nullptr) ? Cast<AWTRPlayerController>(Controller) : WTRPlayerController;
    if (WTRPlayerController)
    {
        WTRPlayerController->TurnDownTheVolume();
    }
}

void AWTRCharacter::OnGrenadeButtonPressed()
{
    if (bDisableGameplay) return;

    if (Combat)
    {
        Combat->ThrowGrenade();
    }
}

void AWTRCharacter::OnPauseButtonPressed()
{
    const auto PlayerController = Cast<APlayerController>(Controller);
    UKismetSystemLibrary::QuitGame(GetWorld(), PlayerController, EQuitPreference::Quit, true);
}

void AWTRCharacter::OnReloadFinishedNotifyPlayed(USkeletalMeshComponent* MeshComp)
{
    if (Combat)
    {
        Combat->FinishReloading();
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

void AWTRCharacter::Elim()
{
    if (Combat && Combat->EquippedWeapon)
    {
        Combat->EquippedWeapon->Dropped();
    }
    if (Combat && WTRPlayerController)
    {
        Combat->CarriedAmmo = 0;
        Combat->SetHUDCarriedAmmo();

        WTRPlayerController->SetHUDWeaponType(EWeaponType::EWT_MAX);
    }

    GetWorldTimerManager().SetTimer(                //
        EliminatedTimerHandle,                      //
        this,                                       //
        &AWTRCharacter::OnEliminatedTimerFinished,  //
        EliminatedTimerDelay);

    Multicast_Elim();
}

void AWTRCharacter::Multicast_Elim_Implementation()
{
    PlayEliminationMontage();
    bElimmed = true;

    // Play dissolve animation with dissolve material
    if (DissolveMaterialInst)
    {
        DissolveMaterialInstDynamic = UMaterialInstanceDynamic::Create(DissolveMaterialInst, this);
        GetMesh()->SetMaterial(0, DissolveMaterialInstDynamic);

        DissolveMaterialInstDynamic->SetScalarParameterValue(FName("Dissolve"), -0.55f);
        DissolveMaterialInstDynamic->SetScalarParameterValue(FName("Glow"), DissolveMaterialGlow);
    }
    StartDissolve();

    // Disable character movement and firing
    bDisableGameplay = true;
    GetCharacterMovement()->StopMovementImmediately();
    if (Combat)
    {
        Combat->OnFireButtonPressed(false);
    }

    if (WTRPlayerController)
    {
        DisableInput(WTRPlayerController);
    }

    // Disable collision
    if (GetCapsuleComponent() && GetMesh())
    {
        GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    // Fix drop through the ground
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->GravityScale = 0.f;
    }

    // Spawn ElimBot
    if (GetWorld() && ElimBotParticleSys)
    {
        FVector ElimBotLocation(GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z + ElimBotHeightAbovePlayer);
        ElimBotParticleSysComponent = UGameplayStatics::SpawnEmitterAtLocation(  //
            GetWorld(),                                                          //
            ElimBotParticleSys,                                                  //
            ElimBotLocation,                                                     //
            GetActorRotation()                                                   //
        );
    }

    if (ElimBotSound)
    {
        UGameplayStatics::PlaySoundAtLocation(  //
            this,                               //
            ElimBotSound,                       //
            GetActorLocation()                  //
        );
    }

    bool bHideScoup = IsLocallyControlled() && Combat && Combat->bIsAiming && Combat->EquippedWeapon &&
                      Combat->EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle;
    if (bHideScoup)
    {
        Combat->SetAiming(false);
    }

    // Show DeathMessage (will hidden in WTRPlayerController.cpp in OnPossess() function)
    // Set weapon ammo to 0
    // Set weapon type to NONE
    if (WTRPlayerController)
    {
        WTRPlayerController->SetHUDDeathMessage(true);
        WTRPlayerController->SetHUDWeaponAmmo(0);
        WTRPlayerController->SetHUDWeaponType(EWeaponType::EWT_MAX);
    }

    if (GrenadeMesh)
    {
        GrenadeMesh->SetVisibility(false);
    }

    if (OverheadText)
    {
        OverheadText->SetVisibility(false);
    }
}

void AWTRCharacter::OnEliminatedTimerFinished()
{
    if (GetWTRGameMode())
    {
        GetWTRGameMode()->RequestRespawn(this, Controller);
    }

    Multicast_OnDestroyed();
}

void AWTRCharacter::StartDissolve()
{
    OnDissolveTimelineFloat.BindDynamic(this, &AWTRCharacter::OnDissolveTrackFloatChange);
    if (DissolveTimelineComponent && DissolveCurve)
    {
        DissolveTimelineComponent->AddInterpFloat(DissolveCurve, OnDissolveTimelineFloat);
        DissolveTimelineComponent->Play();
    }
}

void AWTRCharacter::OnDissolveTrackFloatChange(float DissolveValue)
{
    DissolveMaterialInstDynamic->SetScalarParameterValue(FName("Dissolve"), DissolveValue);
}

void AWTRCharacter::Multicast_OnDestroyed_Implementation()
{
    ElimBotParticleSysComponent->DestroyComponent();
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
    if (bElimmed) return;

    const AWTRGameMode* WTRGameMode = GetWTRGameMode();
    if (WTRGameMode && WTRGameMode->GetMatchState() == MatchState::Cooldown) return;

    Health = FMath::Clamp(Health - Damage, 0.f, MaxHealth);

    UpdateHUDHealth();
    PlayHitReactMontage();

    if (Health <= 0.f && GetWorld())
    {
        if (GetWTRGameMode())
        {
            WTRPlayerController = (WTRPlayerController == nullptr) ? Cast<AWTRPlayerController>(Controller) : WTRPlayerController;
            AWTRPlayerController* AttackerController = Cast<AWTRPlayerController>(InstigatedBy);

            GetWTRGameMode()->PlayerEliminated(this, WTRPlayerController, AttackerController);
        }
    }

    if (Combat && Combat->CombatState == ECombatState::ECS_Reloading)
    {
        StopReloadMontage();
        Combat->CombatState = ECombatState::ECS_Unoccupied;
    }

    if (Combat && Combat->CombatState == ECombatState::ECS_ThrowingGrenade && GrenadeMesh)
    {
        StopThrowGrenadeMontage();
        Combat->ThrowGrenadeFinished();
        GrenadeMesh->SetVisibility(false);
    }
}

void AWTRCharacter::OnRep_Health(float LastHealth)
{
    UpdateHUDHealth();

    if (Health > 0.f && Health < LastHealth)
    {
        PlayHitReactMontage();
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

int32 AWTRCharacter::GetCarriedAmmo() const
{
    if (!Combat) return 0;
    return Combat->CarriedAmmo;
}

ECombatState AWTRCharacter::GetCombatState() const
{
    if (!Combat) return ECombatState::ECS_MAX;
    return Combat->CombatState;
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
    if (!IsLocallyControlled()) return;

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

AWTRGameMode* AWTRCharacter::GetWTRGameMode() const
{
    AWTRGameMode* WTRGameMode = Cast<AWTRGameMode>(UGameplayStatics::GetGameMode(this));
    return WTRGameMode;
}
