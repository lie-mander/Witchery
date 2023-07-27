// Witchery. Copyright Liemander. All Rights Reserved.

#include "Character/WTRPlayerController.h"
#include "Character/WTRCharacter.h"
#include "HUD/WTR_HUD.h"
#include "HUD/WTRCharacterOverlayWidget.h"
#include "HUD/WTRAnnouncementWidget.h"
#include "HUD/WTRReturnToMainMenu.h"
#include "HUD/WTRChat.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/WTRCombatComponent.h"
#include "Components/Image.h"
#include "Kismet/KismetStringLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "GameModes/WTRGameMode.h"
#include "GameStates/WTRGameState.h"
#include "TimerManager.h"
#include "WTRPlayerState.h"
#include "Sound/SoundClass.h"

DECLARE_LOG_CATEGORY_CLASS(WTR_PlayerController, All, All);

void AWTRPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AWTRPlayerController, MatchState);
    DOREPLIFETIME(AWTRPlayerController, TimeOfMapCreation);
}

void AWTRPlayerController::BeginPlay()
{
    Super::BeginPlay();

    WTR_HUD = Cast<AWTR_HUD>(GetHUD());
}

void AWTRPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    if (!InputComponent) return;

    InputComponent->BindAction("AudioUp", EInputEvent::IE_Pressed, this, &ThisClass::VolumeUp);
    InputComponent->BindAction("AudioDown", EInputEvent::IE_Pressed, this, &ThisClass::TurnDownTheVolume);
    InputComponent->BindAction("Quit", EInputEvent::IE_Pressed, this, &ThisClass::OnQuitButtonPressed);
    InputComponent->BindAction("Chat", EInputEvent::IE_Pressed, this, &ThisClass::OnChatButtonPressed);
}

void AWTRPlayerController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    UpdateSyncTime(DeltaTime);
    ShowFPS(DeltaTime);
    PingTick(DeltaTime);

    DelayInit();
    SetHUDTime();

    Debug_ShowHUDTime();
}

void AWTRPlayerController::DelayInit()
{
    if (bDelayInit_AnnouncementWidget)
    {
        WTR_HUD = GetWTR_HUD();
        if (WTR_HUD && IsLocalController())
        {
            WTR_HUD->AddAnnouncement();
            AnnouncementWidget = Cast<UWTRAnnouncementWidget>(WTR_HUD->AnnouncementWidget);

            if (bShowDelayInit && GEngine)
            {
                GEngine->AddOnScreenDebugMessage(
                    -1, 5.f, FColor::Green, FString::Printf(TEXT("AnnouncementWidget created [DelayInit]")), false);
            }

            bDelayInit_AnnouncementWidget = false;
        }
    }

    if (bDelayInit_CharacterOverlayDelayInit)
    {
        WTR_HUD = GetWTR_HUD();
        if (WTR_HUD && IsLocalController())
        {
            WTR_HUD->AddCharacterOverlay();
            CharacterOverlay = Cast<UWTRCharacterOverlayWidget>(WTR_HUD->CharacterOverlayWidget);

            if (bShowDelayInit && GEngine)
            {
                GEngine->AddOnScreenDebugMessage(
                    -1, 5.f, FColor::Green, FString::Printf(TEXT("CharacterOverlayWidget created [DelayInit]")), false);
            }

            if (GameModeType == EGameModeType::EGMT_TeamsMatch)
                ShowTeamsScore();
            else
                HideTeamsScore();

            bDelayInit_CharacterOverlayDelayInit = false;
        }
    }

    if (!WTRGameMode)
    {
        WTRGameMode = (WTRGameMode == nullptr) ? Cast<AWTRGameMode>(UGameplayStatics::GetGameMode(this)) : WTRGameMode;
        if (WTRGameMode)
        {
            if (bShowDelayInit && GEngine)
            {
                GEngine->AddOnScreenDebugMessage(
                    0, 5.f, FColor::Cyan, FString::Printf(TEXT("GameMode find [DelayInit, WTRGameMode]")), false);
            }
            Server_CheckMatchState();
        }
    }
}

void AWTRPlayerController::Server_CheckMatchState_Implementation()
{
    WTRGameMode = (WTRGameMode == nullptr) ? Cast<AWTRGameMode>(UGameplayStatics::GetGameMode(this)) : WTRGameMode;
    if (WTRGameMode)
    {
        WarmupTime = WTRGameMode->GetWarmupTime();
        MatchTime = WTRGameMode->GetMatchTime();
        CooldownTime = WTRGameMode->GetCooldownTime();
        MatchState = WTRGameMode->GetMatchState();
        GameModeType = WTRGameMode->GetGameModeType();

        if (IsLocalController())
        {
            if (GameModeType == EGameModeType::EGMT_TeamsMatch)
                ShowTeamsScore();
            else
                HideTeamsScore();
        }

        Client_ApplyMatchState(WarmupTime, MatchTime, CooldownTime, MatchState, GameModeType);
    }
}

void AWTRPlayerController::Client_ApplyMatchState_Implementation(
    float TimeofWarmup, float TimeOfMatch, float TimeOfCooldown, const FName& State, EGameModeType GameType)
{
    WarmupTime = TimeofWarmup;
    MatchTime = TimeOfMatch;
    CooldownTime = TimeOfCooldown;
    GameModeType = GameType;
}

void AWTRPlayerController::SetMatchState(const FName& State)
{
    MatchState = State;
    if (MatchState == MatchState::WaitingToStart)
    {
        HandleMatchStateWaitingToStart();
    }
    else if (MatchState == MatchState::InProgress)
    {
        HandleMatchStateInProgress();
    }
    else if (MatchState == MatchState::Cooldown)
    {
        HandleMatchCooldown();
    }
}

void AWTRPlayerController::OnRep_MatchState()
{
    if (MatchState == MatchState::WaitingToStart)
    {
        HandleMatchStateWaitingToStart();
    }
    else if (MatchState == MatchState::InProgress)
    {
        HandleMatchStateInProgress();
    }
    else if (MatchState == MatchState::Cooldown)
    {
        HandleMatchCooldown();
    }
}

void AWTRPlayerController::BroadcastElim(const APlayerState* AttackerState, const APlayerState* VictimState)
{
    if (!AttackerState || !VictimState) return;

    Client_ElimAnnouncement(AttackerState, VictimState);
}

void AWTRPlayerController::Client_ElimAnnouncement_Implementation(const APlayerState* AttackerState, const APlayerState* VictimState)
{
    WTR_HUD = GetWTR_HUD();
    const APlayerState* Self = GetPlayerState<APlayerState>();
    if (!AttackerState || !VictimState || !WTR_HUD || !Self) return;

    if (AttackerState == Self && VictimState != Self)
    {
        WTR_HUD->AddElimAnnouncement(FString("You"), VictimState->GetPlayerName());
        return;
    }

    if (AttackerState != Self && VictimState == Self)
    {
        WTR_HUD->AddElimAnnouncement(AttackerState->GetPlayerName(), FString("you"));
        return;
    }

    if (AttackerState == Self && VictimState == Self)
    {
        WTR_HUD->AddElimAnnouncement(FString("You"), FString("yourself"));
        return;
    }

    if (AttackerState != Self && VictimState != Self && AttackerState == VictimState)
    {
        WTR_HUD->AddElimAnnouncement(AttackerState->GetPlayerName(), FString("themselves"));
        return;
    }

    WTR_HUD->AddElimAnnouncement(AttackerState->GetPlayerName(), VictimState->GetPlayerName());
}

void AWTRPlayerController::UpdateSyncTime(float DeltaTime)
{
    TimeToSyncUpdate += DeltaTime;
    if (IsLocalController() && TimeToSyncUpdate >= TimeSyncUpdateFrequency)
    {
        Server_SendClientTime(GetWorld()->GetTimeSeconds());
        TimeToSyncUpdate = 0.f;
    }
}

void AWTRPlayerController::Server_SendClientTime_Implementation(float ClientTimeOfSending)
{
    if (GetWorld())
    {
        const float CurrentServerTime = GetWorld()->GetTimeSeconds();
        Client_SendServerTime(ClientTimeOfSending, CurrentServerTime);
    }
}

void AWTRPlayerController::Client_SendServerTime_Implementation(float ClientTimeOfSending, float ServerTimeResponse)
{
    if (GetWorld())
    {
        const float RoundTrip = GetWorld()->GetTimeSeconds() - ClientTimeOfSending;
        SingleTripTime = RoundTrip * 0.5f;
        const float CurrentServerTime = ServerTimeResponse + SingleTripTime;
        ClientServerTimeDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();
    }
}

float AWTRPlayerController::GetServerTime()
{
    if (GetWorld())
    {
        return GetWorld()->GetTimeSeconds() + ClientServerTimeDelta;
    }

    return 0.0f;
}

void AWTRPlayerController::ReceivedPlayer()
{
    Super::ReceivedPlayer();

    if (IsLocalController() && GetWorld())
    {
        Server_SendClientTime(GetWorld()->GetTimeSeconds());
    }
}

void AWTRPlayerController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    AWTRGameMode* TempGameMode = Cast<AWTRGameMode>(UGameplayStatics::GetGameMode(this));
    if (TempGameMode && TempGameMode->GetGameModeType() == EGameModeType::EGMT_TeamsMatch && IsLocalController())
    {
        TempGameMode->PlayerStartByTeam(this);
    }

    // Hidden DeathMessage (OnPossess called only on the server, need to call rpc to client)
    if (IsLocalController())
    {
        SetHUDDeathMessage(false);
    }
    else
    {
        Client_OnPossess();
    }

    WTRCharacter = Cast<AWTRCharacter>(InPawn);
    WTR_HUD = GetWTR_HUD();

    if (WTRCharacter && WTR_HUD)
    {
        WTRCharacter->OnPossessHandle(this, WTR_HUD);
        SetHUDHealth(WTRCharacter->GetHealth(), WTRCharacter->GetMaxHealth());
        SetHUDShield(WTRCharacter->GetShield(), WTRCharacter->GetMaxShield());

        if (WTRCharacter && WTRCharacter->GetCombatComponent())
        {
            SetHUDGrenades(WTRCharacter->GetCombatComponent()->GetCurrentGrenades());
            SetHUDWeaponAmmo(WTRCharacter->GetCombatComponent()->GetEquippedWeaponAmmo());
            SetHUDCarriedAmmo(WTRCharacter->GetCombatComponent()->GetCarriedAmmo());
            SetHUDWeaponType(WTRCharacter->GetCombatComponent()->GetEquippedWeaponType());
        }
    }
}

void AWTRPlayerController::Client_OnPossess_Implementation()
{
    SetHUDDeathMessage(false);
}

void AWTRPlayerController::SetHUDHealth(float CurrentHealth, float MaxHealth)
{
    WTR_HUD = GetWTR_HUD();

    const bool bHUDValid = WTR_HUD &&                      //
                           CharacterOverlay &&             //
                           CharacterOverlay->HealthBar &&  // HealthBar
                           CharacterOverlay->HealthText;   // HealthText

    if (bHUDValid)
    {
        const float HealthPercent = CurrentHealth / MaxHealth;
        CharacterOverlay->HealthBar->SetPercent(HealthPercent);

        const FString HealthText = FString::Printf(TEXT("%d / %d"), FMath::CeilToInt(CurrentHealth), FMath::CeilToInt(MaxHealth));
        CharacterOverlay->HealthText->SetText(FText::FromString(HealthText));
    }
    else
    {
        DelayInit_CurrentHealth = CurrentHealth;
        DelayInit_MaxHealth = MaxHealth;
    }
}

void AWTRPlayerController::SetHUDShield(float CurrentShield, float MaxShield)
{
    WTR_HUD = GetWTR_HUD();

    const bool bHUDValid = WTR_HUD &&                      //
                           CharacterOverlay &&             //
                           CharacterOverlay->ShieldBar &&  // ShieldBar
                           CharacterOverlay->ShieldText;   // ShieldText

    if (bHUDValid)
    {
        const float ShieldPercent = CurrentShield / MaxShield;
        CharacterOverlay->ShieldBar->SetPercent(ShieldPercent);

        const FString ShieldText = FString::Printf(TEXT("%d / %d"), FMath::CeilToInt(CurrentShield), FMath::CeilToInt(MaxShield));
        CharacterOverlay->ShieldText->SetText(FText::FromString(ShieldText));
    }
    else
    {
        DelayInit_CurrentShield = CurrentShield;
        DelayInit_MaxShield = MaxShield;
    }
}

void AWTRPlayerController::SetHUDScore(float ScoreAmount)
{
    WTR_HUD = GetWTR_HUD();

    const bool bHUDValid = WTR_HUD &&                    //
                           CharacterOverlay &&           //
                           CharacterOverlay->ScoreText;  // ScoreText

    if (bHUDValid)
    {
        const FString ScoreText = FString::Printf(TEXT("%d"), FMath::FloorToInt(ScoreAmount));
        CharacterOverlay->ScoreText->SetText(FText::FromString(ScoreText));
    }
    else
    {
        DelayInit_ScoreAmount = ScoreAmount;
    }
}

void AWTRPlayerController::SetHUDDefeats(int32 DefeatsAmount)
{
    WTR_HUD = GetWTR_HUD();

    const bool bHUDValid = WTR_HUD &&                      //
                           CharacterOverlay &&             //
                           CharacterOverlay->DefeatsText;  // DefeatsText

    if (bHUDValid)
    {
        const FString DefeatsText = FString::Printf(TEXT("%d"), DefeatsAmount);
        CharacterOverlay->DefeatsText->SetText(FText::FromString(DefeatsText));
    }
    else
    {
        DelayInit_DefeatsAmount = DefeatsAmount;
    }
}

void AWTRPlayerController::SetHUDDeathMessage(bool bVisible)
{
    WTR_HUD = GetWTR_HUD();

    const bool bHUDValid = WTR_HUD &&                           //
                           CharacterOverlay &&                  //
                           CharacterOverlay->DeathMessageText;  // DeathMessageText

    if (bHUDValid)
    {
        const ESlateVisibility SlateVisibility = bVisible ? ESlateVisibility::Visible : ESlateVisibility::Hidden;
        CharacterOverlay->DeathMessageText->SetVisibility(SlateVisibility);
    }
}

void AWTRPlayerController::SetHUDWeaponAmmo(int32 AmmoAmount)
{
    WTR_HUD = GetWTR_HUD();

    const bool bHUDValid = WTR_HUD &&                         //
                           CharacterOverlay &&                //
                           CharacterOverlay->WeaponAmmoText;  // WeaponAmmoText

    if (bHUDValid)
    {
        const FString AmmoText = FString::Printf(TEXT("%d"), AmmoAmount);
        CharacterOverlay->WeaponAmmoText->SetText(FText::FromString(AmmoText));
    }
    else
    {
        DelayInit_WeaponAmmo = AmmoAmount;
    }
}

void AWTRPlayerController::SetHUDCarriedAmmo(int32 AmmoAmount)
{
    WTR_HUD = GetWTR_HUD();

    const bool bHUDValid = WTR_HUD &&                          //
                           CharacterOverlay &&                 //
                           CharacterOverlay->CarriedAmmoText;  // CarriedAmmoText

    if (bHUDValid)
    {
        const FString AmmoText = FString::Printf(TEXT("%d"), AmmoAmount);
        CharacterOverlay->CarriedAmmoText->SetText(FText::FromString(AmmoText));
    }
    else
    {
        DelayInit_CarriedAmmo = AmmoAmount;
    }
}

void AWTRPlayerController::SetHUDWeaponType(EWeaponType Type)
{
    WTR_HUD = GetWTR_HUD();

    const bool bHUDValid = WTR_HUD &&                         //
                           CharacterOverlay &&                //
                           CharacterOverlay->WeaponTypeText;  // WeaponTypeText

    if (bHUDValid)
    {
        FString WeaponTypeText;
        switch (Type)
        {
            case EWeaponType::EWT_AssaultRifle:     //
                WeaponTypeText = FString("Rifle");  //
                break;

            case EWeaponType::EWT_GrenadeLauncher:             //
                WeaponTypeText = FString("Grenade launcher");  //
                break;

            case EWeaponType::EWT_Pistol:            //
                WeaponTypeText = FString("Pistol");  //
                break;

            case EWeaponType::EWT_RocketLauncher:             //
                WeaponTypeText = FString("Rocket launcher");  //
                break;

            case EWeaponType::EWT_Shotgun:            //
                WeaponTypeText = FString("Shotgun");  //
                break;

            case EWeaponType::EWT_SniperRifle:             //
                WeaponTypeText = FString("Sniper rifle");  //
                break;

            case EWeaponType::EWT_SubmachineGun:             //
                WeaponTypeText = FString("Submachine gun");  //
                break;

            case EWeaponType::EWT_Flamethrower:            //
                WeaponTypeText = FString("Flamethrower");  //
                break;

            case EWeaponType::EWT_MAX:         //
                WeaponTypeText = FString("");  //
                break;
        }

        CharacterOverlay->WeaponTypeText->SetText(FText::FromString(WeaponTypeText));
    }
    else
    {
        DelayInit_WeaponType = Type;
    }
}

void AWTRPlayerController::SetHUDMatchCountdownTime(float Time)
{
    WTR_HUD = GetWTR_HUD();

    const bool bHUDValid = WTR_HUD &&                             //
                           CharacterOverlay &&                    //
                           CharacterOverlay->Blinking &&          //
                           CharacterOverlay->MatchCountdownText;  // MatchCountdownText

    if (bHUDValid)
    {
        if (Time < 0.f)
        {
            GEngine->AddOnScreenDebugMessage(14, 1.f, FColor::Red, "Time < 0.f");
            CharacterOverlay->MatchCountdownText->SetText(FText::FromString("00:00"));
            return;
        }

        if (Time <= BlinkStartTime && Time >= 0.f)
        {
            CharacterOverlay->PlayAnimation(CharacterOverlay->Blinking);
        }

        FString TimeString = UKismetStringLibrary::TimeSecondsToString(Time);
        TimeString = UKismetStringLibrary::GetSubstring(TimeString, 0, 5);

        CharacterOverlay->MatchCountdownText->SetText(FText::FromString(TimeString));
    }
}

void AWTRPlayerController::SetHUDWarmupTime(float Time)
{
    WTR_HUD = GetWTR_HUD();

    const bool bHUDValid = WTR_HUD &&                       //
                           AnnouncementWidget &&            //
                           AnnouncementWidget->WarmupText;  // WarmupText

    if (bHUDValid)
    {
        if (Time < 0.f)
        {
            GEngine->AddOnScreenDebugMessage(14, 1.f, FColor::Red, "Time < 0.f");
            AnnouncementWidget->WarmupText->SetText(FText::FromString("00:00"));
            return;
        }

        FString TimeString = UKismetStringLibrary::TimeSecondsToString(Time);
        TimeString = UKismetStringLibrary::GetSubstring(TimeString, 0, 5);

        AnnouncementWidget->WarmupText->SetText(FText::FromString(TimeString));
    }
}

void AWTRPlayerController::SetHUD_FPS()
{
    WTR_HUD = GetWTR_HUD();

    const bool bHUDValid = WTR_HUD &&                   //
                           CharacterOverlay &&          //
                           CharacterOverlay->FPS_Text;  // FPS_Text

    if (bHUDValid)
    {
        const FString FPS_Text = FString::Printf(TEXT("%d"), FMath::FloorToInt(FPS));
        CharacterOverlay->FPS_Text->SetText(FText::FromString(FPS_Text));
    }
}

void AWTRPlayerController::SetHUDGrenades(int32 Grenades)
{
    WTR_HUD = GetWTR_HUD();

    const bool bHUDValid = WTR_HUD &&                      //
                           CharacterOverlay &&             //
                           CharacterOverlay->GrenadeText;  // GrenadeText

    if (bHUDValid)
    {
        const FString GrenadeText = FString::Printf(TEXT("%d"), Grenades);
        CharacterOverlay->GrenadeText->SetText(FText::FromString(GrenadeText));
    }
}

void AWTRPlayerController::SetHUDRedScore(int32 Score)
{
    WTR_HUD = GetWTR_HUD();

    const bool bHUDValid = WTR_HUD &&                       //
                           CharacterOverlay &&              //
                           CharacterOverlay->RedTeamScore;  // RedTeamScore

    if (bHUDValid)
    {
        const FString ScoreString = FString::Printf(TEXT("%d"), Score);
        CharacterOverlay->RedTeamScore->SetText(FText::FromString(ScoreString));
    }
}

void AWTRPlayerController::SetHUDBlueScore(int32 Score)
{
    WTR_HUD = GetWTR_HUD();

    const bool bHUDValid = WTR_HUD &&                        //
                           CharacterOverlay &&               //
                           CharacterOverlay->BlueTeamScore;  // BlueTeamScore

    if (bHUDValid)
    {
        const FString ScoreString = FString::Printf(TEXT("%d"), Score);
        CharacterOverlay->BlueTeamScore->SetText(FText::FromString(ScoreString));
    }
}

void AWTRPlayerController::ShowTeamsScore()
{
    WTR_HUD = GetWTR_HUD();

    const bool bHUDValid = WTR_HUD &&                          //
                           CharacterOverlay &&                 //
                           CharacterOverlay->RedTeamScore &&   // RedTeamScore
                           CharacterOverlay->BlueTeamScore &&  // BlueTeamScore
                           CharacterOverlay->SpacerTeamScore;  // SpacerTeamScore

    if (bHUDValid)
    {
        CharacterOverlay->RedTeamScore->SetText(FText::FromString("0"));
        CharacterOverlay->BlueTeamScore->SetText(FText::FromString("0"));
        CharacterOverlay->SpacerTeamScore->SetText(FText::FromString("|"));
    }
}

void AWTRPlayerController::HideTeamsScore()
{
    WTR_HUD = GetWTR_HUD();

    const bool bHUDValid = WTR_HUD &&                          //
                           CharacterOverlay &&                 //
                           CharacterOverlay->RedTeamScore &&   // RedTeamScore
                           CharacterOverlay->BlueTeamScore &&  // BlueTeamScore
                           CharacterOverlay->SpacerTeamScore;  // SpacerTeamScore

    if (bHUDValid)
    {
        CharacterOverlay->RedTeamScore->SetText(FText());
        CharacterOverlay->BlueTeamScore->SetText(FText());
        CharacterOverlay->SpacerTeamScore->SetText(FText());
    }
}

AWTR_HUD* AWTRPlayerController::GetWTR_HUD()
{
    return (WTR_HUD == nullptr) ? Cast<AWTR_HUD>(GetHUD()) : WTR_HUD;
}

void AWTRPlayerController::SetHUDTime()
{
    float TimeLeft = 0.f;

    if (MatchState == MatchState::WaitingToStart)
        TimeLeft = WarmupTime - GetServerTime() + TimeOfMapCreation;
    else if (MatchState == MatchState::InProgress)
        TimeLeft = WarmupTime + MatchTime - GetServerTime() + TimeOfMapCreation;
    else if (MatchState == MatchState::Cooldown)
        TimeLeft = WarmupTime + MatchTime + CooldownTime - GetServerTime() + TimeOfMapCreation;

    SecondsInteger = FMath::CeilToInt(TimeLeft);

    if (SecondsInteger != Previous)
    {
        if (MatchState == MatchState::WaitingToStart)
            SetHUDWarmupTime(TimeLeft);
        else if (MatchState == MatchState::InProgress)
            SetHUDMatchCountdownTime(TimeLeft);
        else if (MatchState == MatchState::Cooldown)
            SetHUDWarmupTime(TimeLeft);
    }

    Previous = SecondsInteger;
}

void AWTRPlayerController::Debug_ShowHUDTime()
{
    if (GEngine && GetWorld() && bShowTime)
    {
        GEngine->AddOnScreenDebugMessage(0, 1.f, FColor::Orange, FString::Printf(TEXT("MatchState: %s"), *MatchState.ToString()));
        GEngine->AddOnScreenDebugMessage(2, 1.f, FColor::Orange, FString::Printf(TEXT("WarmupTime: %.3f"), WarmupTime));
        GEngine->AddOnScreenDebugMessage(3, 1.f, FColor::Orange, FString::Printf(TEXT("MatchTime: %.3f"), MatchTime));
        GEngine->AddOnScreenDebugMessage(4, 1.f, FColor::Orange, FString::Printf(TEXT("CooldownTime: %.3f"), CooldownTime));
        GEngine->AddOnScreenDebugMessage(5, 1.f, FColor::Orange, FString::Printf(TEXT("TimeOfMapCreation: %.3f\n"), TimeOfMapCreation));
        GEngine->AddOnScreenDebugMessage(7, 1.f, FColor::Orange, FString::Printf(TEXT("Client time: %.3f"), GetWorld()->GetTimeSeconds()));
        GEngine->AddOnScreenDebugMessage(8, 1.f, FColor::Orange, FString::Printf(TEXT("Server time: %.3f\n"), GetServerTime()));

        float TimeLeft = 0.f;

        if (MatchState == MatchState::WaitingToStart)
        {
            TimeLeft = WarmupTime - GetServerTime() + TimeOfMapCreation;
            GEngine->AddOnScreenDebugMessage(11, 1.f, FColor::Orange,
                FString::Printf(TEXT("MatchState::WaitingToStart time:\nWarmupTime - GetServerTime() + TimeOfMapCreation\n"
                                     "%.3f - %.3f + %.3f = %.3f"),
                    WarmupTime, GetServerTime(), TimeOfMapCreation, TimeLeft));
        }
        else if (MatchState == MatchState::InProgress)
        {
            TimeLeft = WarmupTime + MatchTime - GetServerTime() + TimeOfMapCreation;
            GEngine->AddOnScreenDebugMessage(12, 1.f, FColor::Orange,
                FString::Printf(TEXT("MatchState::InProgress time:\nWarmupTime + MatchTime - GetServerTime() + TimeOfMapCreation\n"
                                     "%.3f + %.3f - %.3f + %.3f = %.3f"),
                    WarmupTime, MatchTime, GetServerTime(), TimeOfMapCreation, TimeLeft));
        }
        else if (MatchState == MatchState::Cooldown)
        {
            TimeLeft = WarmupTime + MatchTime + CooldownTime - GetServerTime() + TimeOfMapCreation;
            GEngine->AddOnScreenDebugMessage(13, 1.f, FColor::Orange,
                FString::Printf(
                    TEXT("MatchState::Cooldown time:\nWarmupTime + MatchTime + CooldownTime - GetServerTime() + TimeOfMapCreation\n"
                         "%.3f + %.3f + %.3f - %.3f + %.3f = %.3f\n\n"),
                    WarmupTime, MatchTime, CooldownTime, GetServerTime(), TimeOfMapCreation, TimeLeft));
        }
    }
}

void AWTRPlayerController::ShowFPS(float DeltaTime)
{
    FPS = 1.f / DeltaTime;

    if (bShowFPS)
    {
        TimeToFPSUpdate += DeltaTime;
        if (TimeToFPSUpdate >= TimeFPSUpdateFrequency)
        {
            SetHUD_FPS();
            TimeToFPSUpdate = 0.f;
        }
    }
}

void AWTRPlayerController::PingTick(float DeltaTime)
{
    const bool bPingAnimPlaying = WTR_HUD &&                                                     //
                                  CharacterOverlay &&                                            //
                                  CharacterOverlay->Ping &&                                      //
                                  CharacterOverlay->IsAnimationPlaying(CharacterOverlay->Ping);  //

    ShowPingFrequencyRuntime += DeltaTime;
    if (ShowPingFrequencyRuntime >= ShowPingFrequency && !bPingAnimPlaying)
    {
        PlayerState = (PlayerState == nullptr) ? GetPlayerState<APlayerState>() : PlayerState;

        if (PlayerState && PlayerState->GetPingInMilliseconds() >= PingThreshold)
        {
            ShowPing();
            Server_ReportHighPingStatus(true);
        }
        else
        {
            Server_ReportHighPingStatus(false);
        }
    }
    else if (bPingAnimPlaying)
    {
        ShowPingDurationRuntime += DeltaTime;
        if (ShowPingDurationRuntime >= ShowPingDuration)
        {
            HidePing();

            ShowPingDurationRuntime = 0.f;
            ShowPingFrequencyRuntime = 0.f;
        }
    }
}

void AWTRPlayerController::ShowPing()
{
    WTR_HUD = GetWTR_HUD();

    const bool bHUDValid = WTR_HUD &&                      //
                           CharacterOverlay &&             //
                           CharacterOverlay->PingImage &&  // PingImage
                           CharacterOverlay->Ping;         // Ping

    if (bHUDValid)
    {
        CharacterOverlay->PingImage->SetOpacity(1.f);
        CharacterOverlay->PlayAnimation(CharacterOverlay->Ping, 0.f, static_cast<int32>(ShowPingDuration));
    }
}

void AWTRPlayerController::HidePing()
{
    WTR_HUD = GetWTR_HUD();

    const bool bHUDValid = WTR_HUD &&                      //
                           CharacterOverlay &&             //
                           CharacterOverlay->PingImage &&  // PingImage
                           CharacterOverlay->Ping;         // Ping

    if (bHUDValid)
    {
        CharacterOverlay->PingImage->SetOpacity(0.f);
        if (CharacterOverlay->IsAnimationPlaying(CharacterOverlay->Ping))
        {
            CharacterOverlay->StopAnimation(CharacterOverlay->Ping);
        }
    }
}

void AWTRPlayerController::Server_ReportHighPingStatus_Implementation(bool bHighPing)
{
    IsPingHighDelegate.Broadcast(bHighPing);
}

void AWTRPlayerController::HandleMatchStateWaitingToStart()
{
    if (HasAuthority())
    {
        TimeOfMapCreation = GetWorld()->GetTimeSeconds();
    }

    WTR_HUD = GetWTR_HUD();
    if (WTR_HUD && IsLocalController())
    {
        WTR_HUD->AddAnnouncement();
        AnnouncementWidget = Cast<UWTRAnnouncementWidget>(WTR_HUD->AnnouncementWidget);

        if (bShowDelayInit && GEngine)
        {
            GEngine->AddOnScreenDebugMessage(
                -1, 5.f, FColor::Green, FString::Printf(TEXT("AnnouncementWidget created [HandleMatchStateWaitingToStart]")), false);
        }
    }
    else if (!WTR_HUD && IsLocalController())
    {
        bDelayInit_AnnouncementWidget = true;

        if (bShowDelayInit && GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red,
                FString::Printf(TEXT("AnnouncementWidget can`t created! WTR_HUD is NULL [HandleMatchStateWaitingToStart]")), false);
        }
    }

    if (IsLocalController())
    {
        OnMatchStateChanged.Broadcast(MatchState::WaitingToStart);
    }
}

void AWTRPlayerController::HandleMatchStateInProgress()
{
    WTR_HUD = GetWTR_HUD();
    if (WTR_HUD && IsLocalController())
    {
        if (AnnouncementWidget)
        {
            AnnouncementWidget->SetVisibility(ESlateVisibility::Hidden);
        }

        if (!CharacterOverlay)
        {
            WTR_HUD->AddCharacterOverlay();
            CharacterOverlay = Cast<UWTRCharacterOverlayWidget>(WTR_HUD->CharacterOverlayWidget);

            if (CharacterOverlay)
            {
                if (GameModeType == EGameModeType::EGMT_TeamsMatch)
                    ShowTeamsScore();
                else
                    HideTeamsScore();

                if (bShowDelayInit && GEngine)
                {
                    GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green,
                        FString::Printf(TEXT("CharacterOverlayWidget created [HandleMatchStateInProgress]")), false);
                }
            }
        }
    }
    else if (!WTR_HUD && IsLocalController())
    {
        bDelayInit_CharacterOverlayDelayInit = true;

        if (bShowDelayInit && GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red,
                FString::Printf(TEXT("CharacterOverlayWidget can`t created! WTR_HUD is NULL [HandleMatchStateInProgress]")), false);
        }
    }

    if (IsLocalController())
    {
        OnMatchStateChanged.Broadcast(MatchState::InProgress);
    }
}

void AWTRPlayerController::HandleMatchCooldown()
{
    WTR_HUD = GetWTR_HUD();
    if (WTR_HUD && CharacterOverlay && IsLocalController())
    {
        CharacterOverlay->RemoveFromParent();

        if (!AnnouncementWidget)
        {
            WTR_HUD->AddAnnouncement();
            AnnouncementWidget = Cast<UWTRAnnouncementWidget>(WTR_HUD->AnnouncementWidget);
        }

        const bool bWTR_HUD =                        //
            AnnouncementWidget &&                    //
            AnnouncementWidget->AnnouncementText &&  //
            AnnouncementWidget->InfoText &&          //
            AnnouncementWidget->TopPlayersText;

        if (bWTR_HUD)
        {
            FString BottomText = "";
            if (GameModeType == EGameModeType::EGMT_DeathMatch)
            {
                BottomText = GetCooldownDeathMatchText();
            }
            else if (GameModeType == EGameModeType::EGMT_TeamsMatch)
            {
                BottomText = GetCooldownTeamsMatchText();
            }

            AnnouncementWidget->AnnouncementText->SetText(FText::FromString(AnnounCooldownText));
            AnnouncementWidget->InfoText->SetText(FText::FromString(AnnounInfoText));
            AnnouncementWidget->TopPlayersText->SetText(FText::FromString(BottomText));

            AnnouncementWidget->SetVisibility(ESlateVisibility::Visible);
        }
    }

    AWTRCharacter* OwnerCharacter = Cast<AWTRCharacter>(GetPawn());
    if (OwnerCharacter && OwnerCharacter->GetCombatComponent())
    {
        OwnerCharacter->SetDisableGameplay(true);
        OwnerCharacter->GetCombatComponent()->OnFireButtonPressed(false);
        OwnerCharacter->GetCombatComponent()->SetAiming(false);
        if (OwnerCharacter->bIsCrouched)
        {
            OwnerCharacter->UnCrouch();
        }
    }

    if (IsLocalController())
    {
        OnMatchStateChanged.Broadcast(MatchState::Cooldown);
    }
}

FString AWTRPlayerController::GetCooldownTeamsMatchText()
{
    AWTRGameState* WTRGameState = Cast<AWTRGameState>(UGameplayStatics::GetGameState(this));

    if (WTRGameState)
    {
        if (WTRGameState->RedTeamScore == 0 && WTRGameState->BlueTeamScore == 0)
        {
            AnnounInfoText = AnnounInfoText;
        }
        else if (WTRGameState->RedTeamScore == WTRGameState->BlueTeamScore)
        {
            AnnounInfoText = TextTeamFoughtForWin;
        }
        else if (WTRGameState->RedTeamScore > WTRGameState->BlueTeamScore)
        {
            AnnounInfoText = FString(TEXT("THE RED TEAM WON!"));
        }
        else if (WTRGameState->RedTeamScore < WTRGameState->BlueTeamScore)
        {
            AnnounInfoText = FString(TEXT("THE BLUE TEAM WON!"));
        }
    }

    return FString();
}

FString AWTRPlayerController::GetCooldownDeathMatchText()
{
    AWTRGameState* WTRGameState = Cast<AWTRGameState>(UGameplayStatics::GetGameState(this));
    const AWTRPlayerState* WTRPlayerState = GetPlayerState<AWTRPlayerState>();

    FString TopPlayersText = "";

    if (WTRGameState && WTRPlayerState)
    {
        if (WTRGameState->GetTopPlayers().IsEmpty())
        {
            AnnounInfoText = AnnounInfoText;
        }
        else if (WTRGameState->GetTopPlayers().Num() == 1 && WTRGameState->GetTopPlayers()[0] == WTRPlayerState)
        {
            AnnounInfoText = TextYouWinner;
        }
        else if (WTRGameState->GetTopPlayers().Num() == 1)
        {
            AnnounInfoText = FString(TEXT("WINNER:\n"));
            TopPlayersText = FString::Printf(TEXT("%s"), *WTRGameState->GetTopPlayers()[0]->GetPlayerName());
        }
        else if (WTRGameState->GetTopPlayers().Num() > 1)
        {
            AnnounInfoText = FString(TEXT("THEY FOUGHT FOR WIN:\n"));
            for (auto TopPlayer : WTRGameState->GetTopPlayers())
            {
                TopPlayersText.Append(FString::Printf(TEXT("%s\n"), *TopPlayer->GetPlayerName()));
            }
        }
    }

    return TopPlayersText;
}

void AWTRPlayerController::OnQuitButtonPressed()
{
    if (!ReturnToMainMenuWidgetClass) return;

    if (!ReturnToMainMenuWidget)
    {
        ReturnToMainMenuWidget = CreateWidget<UWTRReturnToMainMenu>(this, ReturnToMainMenuWidgetClass);
    }

    if (ReturnToMainMenuWidget)
    {
        bReturnToMainMenuOpen = !bReturnToMainMenuOpen;
        if (bReturnToMainMenuOpen)
        {
            ReturnToMainMenuWidget->MenuSetup();
        }
        else
        {
            ReturnToMainMenuWidget->MenuTearDown();
        }
    }
}

void AWTRPlayerController::OnChatButtonPressed()
{
    if (CharacterOverlay)
    {
        bChatOpen = !bChatOpen;
        if (bChatOpen)
        {
            CharacterOverlay->OpenChat();
        }
        else
        {
            CharacterOverlay->CloseChat();
        }
    }
}

void AWTRPlayerController::Server_SendChatMessage_Implementation(APlayerState* Sender, const FString& Message)
{
    WTRGameMode = (WTRGameMode == nullptr) ? Cast<AWTRGameMode>(UGameplayStatics::GetGameMode(this)) : WTRGameMode;

    if (WTRGameMode)
    {
        WTRGameMode->SendChatMessagesToAllClients(Sender, Message);
    }
}

void AWTRPlayerController::ApplyChatMessage(APlayerState* Sender, const FString& Message)
{
    if (HasAuthority() && !IsLocalController())
    {
        Client_ApplyChatMessage(Sender, Message);
        return;
    }

    if (CharacterOverlay)
    {
        CharacterOverlay->ApplyChatMessage(Sender, Message);
    }
}

void AWTRPlayerController::Client_ApplyChatMessage_Implementation(APlayerState* Sender, const FString& Message)
{
    if (CharacterOverlay)
    {
        CharacterOverlay->ApplyChatMessage(Sender, Message);
    }
}

void AWTRPlayerController::VolumeUp()
{
    if (MasterSoundClass)
    {
        MasterSoundClass->Properties.Volume = FMath::Clamp(MasterSoundClass->Properties.Volume + 0.1f, 0, 1.f);
    }
}

void AWTRPlayerController::TurnDownTheVolume()
{
    if (MasterSoundClass)
    {
        MasterSoundClass->Properties.Volume = FMath::Clamp(MasterSoundClass->Properties.Volume - 0.1f, 0, 1.f);
    }
}
