// Witchery. Copyright Liemander. All Rights Reserved.

#include "Character/WTRCharacter.h"
#include "Character/WTRPlayerController.h"
#include "Character/WTRAnimInstance.h"
#include "Net/UnrealNetwork.h"
#include "Weapons/WTRWeapon.h"
#include "Weapons/WTRFlamethrower.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerState.h"
#include "Camera/CameraComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/WTRCombatComponent.h"
#include "Components/WTRBuffComponent.h"
#include "Components/WTRLagCompensationComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/TextRenderComponent.h"
#include "Components/BoxComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "HUD/OverheadWidget.h"
#include "HUD/WTR_HUD.h"
#include "GameModes/WTRGameMode.h"
#include "GameStates/WTRGameState.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystemComponent.h"
#include "WTRPlayerState.h"
#include "WTRTypes.h"
#include "WTRTools.h"
#include "Animation/Notifies/WTRReloadFinishedAnimNotify.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"

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

    LagCompensation = CreateDefaultSubobject<UWTRLagCompensationComponent>("LagCompensation");

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

    /*
     * Hit boxes for server-side rewind
     * They named by skeleton bones to which they are attached
     */
    head = CreateDefaultSubobject<UBoxComponent>("head");
    head->SetupAttachment(GetMesh(), FName("head"));
    HitBoxesMap.Add(FName("head"), head);

    pelvis = CreateDefaultSubobject<UBoxComponent>("pelvis");
    pelvis->SetupAttachment(GetMesh(), FName("pelvis"));
    HitBoxesMap.Add(FName("pelvis"), pelvis);

    spine_01 = CreateDefaultSubobject<UBoxComponent>("spine_01");
    spine_01->SetupAttachment(GetMesh(), FName("spine_01"));
    HitBoxesMap.Add(FName("spine_01"), spine_01);

    spine_02 = CreateDefaultSubobject<UBoxComponent>("spine_02");
    spine_02->SetupAttachment(GetMesh(), FName("spine_02"));
    HitBoxesMap.Add(FName("spine_02"), spine_02);

    spine_03 = CreateDefaultSubobject<UBoxComponent>("spine_03");
    spine_03->SetupAttachment(GetMesh(), FName("spine_03"));
    HitBoxesMap.Add(FName("spine_03"), spine_03);

    upperarm_l = CreateDefaultSubobject<UBoxComponent>("upperarm_l");
    upperarm_l->SetupAttachment(GetMesh(), FName("upperarm_l"));
    HitBoxesMap.Add(FName("upperarm_l"), upperarm_l);

    lowerarm_l = CreateDefaultSubobject<UBoxComponent>("lowerarm_l");
    lowerarm_l->SetupAttachment(GetMesh(), FName("lowerarm_l"));
    HitBoxesMap.Add(FName("lowerarm_l"), lowerarm_l);

    hand_l = CreateDefaultSubobject<UBoxComponent>("hand_l");
    hand_l->SetupAttachment(GetMesh(), FName("hand_l"));
    HitBoxesMap.Add(FName("hand_l"), hand_l);

    upperarm_r = CreateDefaultSubobject<UBoxComponent>("upperarm_r");
    upperarm_r->SetupAttachment(GetMesh(), FName("upperarm_r"));
    HitBoxesMap.Add(FName("upperarm_r"), upperarm_r);

    lowerarm_r = CreateDefaultSubobject<UBoxComponent>("lowerarm_r");
    lowerarm_r->SetupAttachment(GetMesh(), FName("lowerarm_r"));
    HitBoxesMap.Add(FName("lowerarm_r"), lowerarm_r);

    hand_r = CreateDefaultSubobject<UBoxComponent>("hand_r");
    hand_r->SetupAttachment(GetMesh(), FName("hand_r"));
    HitBoxesMap.Add(FName("hand_r"), hand_r);

    thigh_l = CreateDefaultSubobject<UBoxComponent>("thigh_l");
    thigh_l->SetupAttachment(GetMesh(), FName("thigh_l"));
    HitBoxesMap.Add(FName("thigh_l"), thigh_l);

    calf_l = CreateDefaultSubobject<UBoxComponent>("calf_l");
    calf_l->SetupAttachment(GetMesh(), FName("calf_l"));
    HitBoxesMap.Add(FName("calf_l"), calf_l);

    foot_l = CreateDefaultSubobject<UBoxComponent>("foot_l");
    foot_l->SetupAttachment(GetMesh(), FName("foot_l"));
    HitBoxesMap.Add(FName("foot_l"), foot_l);

    thigh_r = CreateDefaultSubobject<UBoxComponent>("thigh_r");
    thigh_r->SetupAttachment(GetMesh(), FName("thigh_r"));
    HitBoxesMap.Add(FName("thigh_r"), thigh_r);

    calf_r = CreateDefaultSubobject<UBoxComponent>("calf_r");
    calf_r->SetupAttachment(GetMesh(), FName("calf_r"));
    HitBoxesMap.Add(FName("calf_r"), calf_r);

    foot_r = CreateDefaultSubobject<UBoxComponent>("foot_r");
    foot_r->SetupAttachment(GetMesh(), FName("foot_r"));
    HitBoxesMap.Add(FName("foot_r"), foot_r);

    backpack = CreateDefaultSubobject<UBoxComponent>("backpack");
    backpack->SetupAttachment(GetMesh(), FName("backpack"));
    HitBoxesMap.Add(FName("backpack"), backpack);

    blanket = CreateDefaultSubobject<UBoxComponent>("blanket");
    blanket->SetupAttachment(GetMesh(), FName("backpack"));
    HitBoxesMap.Add(FName("blanket"), blanket);

    for (auto HitBoxPair : HitBoxesMap)
    {
        if (HitBoxPair.Value)
        {
            HitBoxPair.Value->SetCollisionObjectType(ECC_HitBox);
            HitBoxPair.Value->SetCollisionResponseToAllChannels(ECR_Ignore);
            HitBoxPair.Value->SetCollisionResponseToChannel(ECC_HitBox, ECR_Block);
            HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        }
    }
}

void AWTRCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME_CONDITION(AWTRCharacter, OverlappingWeapon, COND_OwnerOnly);
    DOREPLIFETIME(AWTRCharacter, Username);
    DOREPLIFETIME(AWTRCharacter, Health);
    DOREPLIFETIME(AWTRCharacter, Shield);
    DOREPLIFETIME(AWTRCharacter, bDisableGameplay);
    DOREPLIFETIME(AWTRCharacter, bDamageFromFlamethrower);
}

void AWTRCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    RotateInPlace(DeltaTime);
    HideCharacterWithWeaponIfCameraClose();
    PullInit();
    DelayInitHUD();
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

    // Set current health and shield at start
    if (WTRPlayerController)
    {
        WTRPlayerController->SetHUDHealth(Health, MaxHealth);
        WTRPlayerController->SetHUDShield(Shield, MaxShield);
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

    if (WTRPlayerController)
    {
        if (WTRPlayerController && WTRPlayerController->IsLocalController())
        {
            WTRPlayerController->OnMatchStateChanged.AddUObject(this, &AWTRCharacter::OnMatchStateChanged);
            OnMatchStateChanged(WTRPlayerController->GetMatchState());
        }
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
    PlayerInputComponent->BindAction("Reload", EInputEvent::IE_Pressed, this, &ThisClass::OnReloadButtonPressed);
    PlayerInputComponent->BindAction("Grenade", EInputEvent::IE_Pressed, this, &ThisClass::OnGrenadeButtonPressed);
    PlayerInputComponent->BindAction("WeaponSwap", EInputEvent::IE_Pressed, this, &ThisClass::OnWeaponSwapButtonPressed);

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
            Buff->InitialJumpZVelocity = GetCharacterMovement()->JumpZVelocity;
        }
    }

    if (LagCompensation)
    {
        LagCompensation->Character = this;
        if (Controller)
        {
            WTRPlayerController = Cast<AWTRPlayerController>(Controller);
            LagCompensation->Controller = WTRPlayerController;
        }
    }
}

void AWTRCharacter::Destroyed()
{
    Super::Destroyed();

    const AWTRGameMode* WTRGameMode = GetWTRGameMode();
    const bool bMatchIsNotInProgress = WTRGameMode && WTRGameMode->GetMatchState() != MatchState::InProgress;

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

            // Spawn crown if character is lead
            AWTRGameState* WTRGameState = Cast<AWTRGameState>(UGameplayStatics::GetGameState(this));
            if (WTRGameState)
            {
                if (WTRGameState->GetTopPlayers().Contains(WTRPlayerState))
                {
                    Multicast_GetLead();
                }
            }
        }
    }

    if (!WTRPlayerController)
    {
        WTRPlayerController = Cast<AWTRPlayerController>(Controller);
        if (WTRPlayerController && WTRPlayerController->IsLocalController())
        {
            WTRPlayerController->OnMatchStateChanged.AddUObject(this, &AWTRCharacter::OnMatchStateChanged);
            OnMatchStateChanged(WTRPlayerController->GetMatchState());
        }
    }
}

void AWTRCharacter::OnMatchStateChanged(const FName& State)
{
    if (State == MatchState::InProgress)
    {
        WTRPlayerController = (WTRPlayerController == nullptr) ? Cast<AWTRPlayerController>(Controller) : WTRPlayerController;

        if (WTRPlayerController)
        {
            WTRPlayerController->SetHUDHealth(Health, MaxHealth);
            WTRPlayerController->SetHUDShield(Shield, MaxShield);

            AWTRPlayerState* TempPlayerState = Cast<AWTRPlayerState>(WTRPlayerController->PlayerState);
            if (TempPlayerState)
            {
                WTRPlayerController->SetHUDScore(TempPlayerState->GetScore());
                WTRPlayerController->SetHUDDefeats(TempPlayerState->GetDefeats());
            }
            else
            {
                bDelayInitHUD_PlayerState = true;
            }

            if (Combat)
            {
                WTRPlayerController->SetHUDCarriedAmmo(Combat->CarriedAmmo);
                WTRPlayerController->SetHUDGrenades(Combat->GetCurrentGrenades());

                if (Combat->EquippedWeapon)
                {
                    WTRPlayerController->SetHUDWeaponAmmo(Combat->EquippedWeapon->GetAmmo());
                    WTRPlayerController->SetHUDWeaponType(Combat->EquippedWeapon->GetWeaponType());
                }
            }
        }
        else
        {
            bDelayInitHUD_Controller = true;
        }
    }
}

void AWTRCharacter::DelayInitHUD()
{
    if (bDelayInitHUD_Controller)
    {
        WTRPlayerController = (WTRPlayerController == nullptr) ? Cast<AWTRPlayerController>(Controller) : WTRPlayerController;

        if (WTRPlayerController && WTRPlayerController->GetWTR_HUD())
        {
            WTRPlayerController->SetHUDHealth(Health, MaxHealth);
            WTRPlayerController->SetHUDShield(Shield, MaxShield);

            AWTRPlayerState* TempPlayerState = Cast<AWTRPlayerState>(WTRPlayerController->PlayerState);
            if (TempPlayerState && Combat)
            {
                WTRPlayerController->SetHUDScore(TempPlayerState->GetScore());
                WTRPlayerController->SetHUDDefeats(TempPlayerState->GetDefeats());

                WTRPlayerController->SetHUDCarriedAmmo(Combat->CarriedAmmo);
                WTRPlayerController->SetHUDGrenades(Combat->GetCurrentGrenades());

                if (Combat->EquippedWeapon)
                {
                    WTRPlayerController->SetHUDWeaponAmmo(Combat->EquippedWeapon->GetAmmo());
                    WTRPlayerController->SetHUDWeaponType(Combat->EquippedWeapon->GetWeaponType());
                }

                bDelayInitHUD_Controller = false;
            }
        }
    }

    if (bDelayInitHUD_PlayerState)
    {
        WTRPlayerController = (WTRPlayerController == nullptr) ? Cast<AWTRPlayerController>(Controller) : WTRPlayerController;

        if (WTRPlayerController && WTRPlayerController->GetWTR_HUD())
        {
            AWTRPlayerState* TempPlayerState = Cast<AWTRPlayerState>(WTRPlayerController->PlayerState);
            if (TempPlayerState)
            {
                WTRPlayerController->SetHUDScore(TempPlayerState->GetScore());
                WTRPlayerController->SetHUDDefeats(TempPlayerState->GetDefeats());

                bDelayInitHUD_PlayerState = false;
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
        const FVector2D InRange(270.f, 360.f);
        const FVector2D OutRange(-90.f, 0.f);
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
        case EWeaponType::EWT_Flamethrower: SectionName = FName("Flamethrower"); break;
    }

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

void AWTRCharacter::PlaySwapingWeaponsMontage()
{
    if (!GetMesh() || !SwapingWeaponsMontage) return;

    UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
    if (!AnimInstance) return;

    AnimInstance->Montage_Play(SwapingWeaponsMontage);
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

    if (Combat && Combat->CombatState != ECombatState::ECS_SwapingWeapons)
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

void AWTRCharacter::OnGrenadeButtonPressed()
{
    if (bDisableGameplay) return;

    if (Combat)
    {
        Combat->ThrowGrenade();
    }
}

void AWTRCharacter::OnWeaponSwapButtonPressed()
{
    if (bDisableGameplay || !bCanSwap || !Combat || !Combat->CanSwapWeapon() || Combat->CombatState != ECombatState::ECS_Unoccupied) return;

    bFinishedSwapping = false;

    GetWorldTimerManager().SetTimer(              //
        SwapButtonTimerHandle,                    //
        this,                                     //
        &AWTRCharacter::SwapButtonTimerFinished,  //
        DelaySwapButton                           //
    );

    Combat->SetAiming(false);

    if (HasAuthority())
    {
        Combat->SwapWeapon();
    }
    else
    {
        Server_OnWeaponSwapButtonPressed();
        PlaySwapingWeaponsMontage();
        Combat->CombatState = ECombatState::ECS_SwapingWeapons;
    }

    bCanSwap = false;
}

void AWTRCharacter::Server_OnWeaponSwapButtonPressed_Implementation()
{
    if (Combat)
    {
        Combat->SwapWeapon();
    }
}

void AWTRCharacter::SwapButtonTimerFinished()
{
    bCanSwap = true;
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
    if (Weapon &&                                                   //
        (Weapon->GetWeaponState() == EWeaponState::EWS_Equipped ||  //
            Weapon->GetWeaponState() == EWeaponState::EWS_EquippedSecond))
        return;

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

void AWTRCharacter::Server_LeaveGame_Implementation()
{
    WTRPlayerState = (WTRPlayerState == nullptr) ? GetPlayerState<AWTRPlayerState>() : WTRPlayerState;
    if (GetWTRGameMode() && WTRPlayerState)
    {
        GetWTRGameMode()->LeaveGame(WTRPlayerState);
    }
}

void AWTRCharacter::Elim(bool bIsLeave)
{
    DropOrDestroyWeapons();

    if (Combat && WTRPlayerController)
    {
        Combat->CarriedAmmo = 0;
        Combat->SetHUDCarriedAmmo();

        WTRPlayerController->SetHUDWeaponType(EWeaponType::EWT_MAX);
    }

    Multicast_Elim(bIsLeave);
}

void AWTRCharacter::Multicast_Elim_Implementation(bool bIsLeave)
{
    bIsLeaving = bIsLeave;

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

    if (Combat)
    {
        Combat->SetAiming(false);
    }

    // Show DeathMessage (will hidden in WTRPlayerController.cpp in OnPossess() function)
    if (WTRPlayerController && !bIsLeaving)
    {
        WTRPlayerController->SetHUDDeathMessage(true);
    }

    // Set weapon ammo to 0
    // Set weapon type to NONE
    if (WTRPlayerController)
    {
        WTRPlayerController->SetHUDWeaponAmmo(0);
        WTRPlayerController->SetHUDWeaponType(EWeaponType::EWT_MAX);
    }

    if (CrownComponent)
    {
        CrownComponent->DestroyComponent();
    }

    if (GrenadeMesh)
    {
        GrenadeMesh->SetVisibility(false);
    }

    if (OverheadText)
    {
        OverheadText->SetVisibility(false);
    }

    GetWorldTimerManager().SetTimer(                //
        EliminatedTimerHandle,                      //
        this,                                       //
        &AWTRCharacter::OnEliminatedTimerFinished,  //
        EliminatedTimerDelay);
}

void AWTRCharacter::OnEliminatedTimerFinished()
{
    if (GetWTRGameMode() && !bIsLeaving)
    {
        GetWTRGameMode()->RequestRespawn(this, Controller);
    }
    else if (bIsLeaving && IsLocallyControlled())
    {
        OnLeaveGame.Broadcast();
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

void AWTRCharacter::UpdateHUDShield()
{
    WTRPlayerController = (WTRPlayerController == nullptr) ? Cast<AWTRPlayerController>(Controller) : WTRPlayerController;
    if (WTRPlayerController)
    {
        WTRPlayerController->SetHUDShield(Shield, MaxShield);
    }
}

void AWTRCharacter::OnTakeAnyDamageCallback(
    AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
    if (bElimmed) return;

    const AWTRGameMode* WTRGameMode = GetWTRGameMode();
    if (WTRGameMode && WTRGameMode->GetMatchState() == MatchState::Cooldown) return;

    AWTRFlamethrower* WTRFlamethrower = Cast<AWTRFlamethrower>(DamageCauser);
    if (WTRFlamethrower)
    {
        bDamageFromFlamethrower = true;
    }
    else
    {
        bDamageFromFlamethrower = false;
    }

    float DamageThroughShield = Damage;
    if (Shield > 0.f)
    {
        if (Shield >= Damage)
        {
            Shield = FMath::Clamp(Shield - Damage, 0.f, MaxShield);
            DamageThroughShield = 0.f;
        }
        else
        {
            DamageThroughShield = FMath::Clamp(Damage - Shield, 0.f, Damage);
            Shield = 0.f;
        }
    }

    Health = FMath::Clamp(Health - DamageThroughShield, 0.f, MaxHealth);

    UpdateHUDHealth();
    UpdateHUDShield();

    if (!bDamageFromFlamethrower)
    {
        PlayHitReactMontage();
    }

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

    if (Health > 0.f && Health < LastHealth && !bDamageFromFlamethrower)
    {
        PlayHitReactMontage();
    }
}

void AWTRCharacter::OnRep_Shield(float LastShield)
{
    UpdateHUDShield();

    if (Shield > 0.f && Shield < LastShield && !bDamageFromFlamethrower)
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

void AWTRCharacter::Multicast_GetLead_Implementation()
{
    if (!CrownSystem) return;

    if (!CrownComponent && GetCapsuleComponent())
    {
        CrownComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(  //
            CrownSystem,                                                //
            GetCapsuleComponent(),                                      //
            FName(),                                                    //
            GetActorLocation() + FVector(0.f, 0.f, 110.f),              //
            GetActorRotation(),                                         //
            EAttachLocation::KeepWorldPosition,                         //
            false                                                       //
        );
    }
    if (CrownComponent)
    {
        CrownComponent->Activate();
    }
}

void AWTRCharacter::Multicast_LostLead_Implementation()
{
    if (CrownComponent)
    {
        CrownComponent->DestroyComponent();
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

bool AWTRCharacter::IsLocallyReloading() const
{
    return Combat && Combat->bLocallyReloading;
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
        if (Combat && Combat->SecondWeapon && Combat->SecondWeapon->GetWeaponMesh())
        {
            Combat->SecondWeapon->GetWeaponMesh()->bOwnerNoSee = true;
        }
    }
    else
    {
        GetMesh()->SetVisibility(true);
        if (Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh())
        {
            Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = false;
        }
        if (Combat && Combat->SecondWeapon && Combat->SecondWeapon->GetWeaponMesh())
        {
            Combat->SecondWeapon->GetWeaponMesh()->bOwnerNoSee = false;
        }
    }
}

void AWTRCharacter::DropOrDestroyWeapons()
{
    if (Combat)
    {
        if (Combat->EquippedWeapon)
        {
            Combat->DropOrDestroyWeapon(Combat->EquippedWeapon);
        }
        if (Combat->SecondWeapon)
        {
            Combat->DropOrDestroyWeapon(Combat->SecondWeapon);
        }
    }
}

AWTRGameMode* AWTRCharacter::GetWTRGameMode() const
{
    AWTRGameMode* WTRGameMode = Cast<AWTRGameMode>(UGameplayStatics::GetGameMode(this));
    return WTRGameMode;
}
