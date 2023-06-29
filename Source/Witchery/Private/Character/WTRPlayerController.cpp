// Witchery. Copyright Liemander. All Rights Reserved.

#include "Character/WTRPlayerController.h"
#include "Character/WTRCharacter.h"
#include "HUD/WTR_HUD.h"
#include "HUD/WTRCharacterOverlayWidget.h"
#include "HUD/WTRAnnouncementWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/WTRCombatComponent.h"
#include "Kismet/KismetStringLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "GameModes/WTRGameMode.h"
#include "GameStates/WTRGameState.h"
#include "TimerManager.h"
#include "WTRPlayerState.h"
#include "Sound/SoundClass.h"

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

void AWTRPlayerController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    UpdateSyncTime(DeltaTime);
    DelayInit();

    SetHUDTime();

    ShowFPS(DeltaTime);
    Debug_ShowHUDTime();
}

void AWTRPlayerController::DelayInit()
{
    if (!CharacterOverlay)
    {
        WTR_HUD = GetWTR_HUD();
        if (WTR_HUD)
        {
            CharacterOverlay = Cast<UWTRCharacterOverlayWidget>(WTR_HUD->CharacterOverlayWidget);

            if (CharacterOverlay)
            {
                SetHUDHealth(DelayInit_CurrentHealth, DelayInit_MaxHealth);
                SetHUDShield(DelayInit_CurrentShield, DelayInit_MaxShield);
                SetHUDScore(DelayInit_ScoreAmount);
                SetHUDDefeats(DelayInit_DefeatsAmount);
                SetHUDWeaponAmmo(DelayInit_WeaponAmmo);
                SetHUDCarriedAmmo(DelayInit_CarriedAmmo);
                SetHUDWeaponType(DelayInit_WeaponType);

                WTRCharacter = Cast<AWTRCharacter>(GetPawn());
                if (WTRCharacter && WTRCharacter->GetCombatComponent())
                {
                    SetHUDGrenades(WTRCharacter->GetCombatComponent()->GetCurrentGrenades());
                }

                if (!bShowFPS)
                {
                    CharacterOverlay->FPS_String->SetVisibility(ESlateVisibility::Hidden);
                }
                else
                {
                    TimeToFPSUpdate = TimeFPSUpdateFrequency;
                    CharacterOverlay->FPS_String->SetVisibility(ESlateVisibility::Visible);
                }
            }
        }
    }

    if (!AnnouncementWidget)
    {
        WTR_HUD = GetWTR_HUD();
        if (IsLocalController() && WTR_HUD && (MatchState == MatchState::WaitingToStart || MatchState == MatchState::Cooldown) &&
            !WTR_HUD->AnnouncementWidget)
        {
            WTR_HUD->AddAnnouncement();
            AnnouncementWidget = Cast<UWTRAnnouncementWidget>(WTR_HUD->AnnouncementWidget);
        }
    }

    if (!WTRGameMode)
    {
        WTRGameMode = (WTRGameMode == nullptr) ? Cast<AWTRGameMode>(UGameplayStatics::GetGameMode(this)) : WTRGameMode;
        if (WTRGameMode)
        {
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

        Client_ApplyMatchState(WarmupTime, MatchTime, CooldownTime, MatchState);
    }
}

void AWTRPlayerController::Client_ApplyMatchState_Implementation(
    float TimeofWarmup, float TimeOfMatch, float TimeOfCooldown, const FName& State)
{
    WarmupTime = TimeofWarmup;
    MatchTime = TimeOfMatch;
    CooldownTime = TimeOfCooldown;
    MatchState = State;

    if (IsLocalController() && WTR_HUD && (MatchState == MatchState::WaitingToStart || MatchState == MatchState::Cooldown) &&
        !WTR_HUD->AnnouncementWidget)
    {
        WTR_HUD->AddAnnouncement();
    }
    else if (IsLocalController() && WTR_HUD && MatchState == MatchState::InProgress && !WTR_HUD->CharacterOverlayWidget)
    {
        WTR_HUD->AddCharacterOverlay();
    }
}

void AWTRPlayerController::SetMatchState(const FName& State)
{
    MatchState = State;
    if (MatchState == MatchState::WaitingToStart)
    {
        HandleMatchStateWaitingToStart();
    }
    if (MatchState == MatchState::InProgress)
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
    if (MatchState == MatchState::InProgress)
    {
        HandleMatchStateInProgress();
    }
    else if (MatchState == MatchState::Cooldown)
    {
        HandleMatchCooldown();
    }
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
        float CurrentServerTime = GetWorld()->GetTimeSeconds();
        Client_SendServerTime(ClientTimeOfSending, CurrentServerTime);
    }
}

void AWTRPlayerController::Client_SendServerTime_Implementation(float ClientTimeOfSending, float ServerTimeResponse)
{
    if (GetWorld())
    {
        float RoundTrip = GetWorld()->GetTimeSeconds() - ClientTimeOfSending;
        float CurrentServerTime = ServerTimeResponse - (RoundTrip * 0.5f);
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

    bool bHUDValid = WTR_HUD &&                                     //
                     WTR_HUD->CharacterOverlayWidget &&             //
                     WTR_HUD->CharacterOverlayWidget->HealthBar &&  // HealthBar
                     WTR_HUD->CharacterOverlayWidget->HealthText;   // HealthText

    if (bHUDValid)
    {
        const float HealthPercent = CurrentHealth / MaxHealth;
        WTR_HUD->CharacterOverlayWidget->HealthBar->SetPercent(HealthPercent);

        const FString HealthText = FString::Printf(TEXT("%d / %d"), FMath::CeilToInt(CurrentHealth), FMath::CeilToInt(MaxHealth));
        WTR_HUD->CharacterOverlayWidget->HealthText->SetText(FText::FromString(HealthText));
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

    bool bHUDValid = WTR_HUD &&                                     //
                     WTR_HUD->CharacterOverlayWidget &&             //
                     WTR_HUD->CharacterOverlayWidget->ShieldBar &&  // ShieldBar
                     WTR_HUD->CharacterOverlayWidget->ShieldText;   // ShieldText

    if (bHUDValid)
    {
        const float ShieldPercent = CurrentShield / MaxShield;
        WTR_HUD->CharacterOverlayWidget->ShieldBar->SetPercent(ShieldPercent);

        const FString ShieldText = FString::Printf(TEXT("%d / %d"), FMath::CeilToInt(CurrentShield), FMath::CeilToInt(MaxShield));
        WTR_HUD->CharacterOverlayWidget->ShieldText->SetText(FText::FromString(ShieldText));
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

    bool bHUDValid = WTR_HUD &&                                   //
                     WTR_HUD->CharacterOverlayWidget &&           //
                     WTR_HUD->CharacterOverlayWidget->ScoreText;  // ScoreText

    if (bHUDValid)
    {
        const FString ScoreText = FString::Printf(TEXT("%d"), FMath::FloorToInt(ScoreAmount));
        WTR_HUD->CharacterOverlayWidget->ScoreText->SetText(FText::FromString(ScoreText));
    }
    else
    {
        DelayInit_ScoreAmount = ScoreAmount;
    }
}

void AWTRPlayerController::SetHUDDefeats(int32 DefeatsAmount)
{
    WTR_HUD = GetWTR_HUD();

    bool bHUDValid = WTR_HUD &&                                     //
                     WTR_HUD->CharacterOverlayWidget &&             //
                     WTR_HUD->CharacterOverlayWidget->DefeatsText;  // DefeatsText

    if (bHUDValid)
    {
        const FString DefeatsText = FString::Printf(TEXT("%d"), DefeatsAmount);
        WTR_HUD->CharacterOverlayWidget->DefeatsText->SetText(FText::FromString(DefeatsText));
    }
    else
    {
        DelayInit_DefeatsAmount = DefeatsAmount;
    }
}

void AWTRPlayerController::SetHUDDeathMessage(bool bVisible)
{
    WTR_HUD = GetWTR_HUD();

    bool bHUDValid = WTR_HUD &&                                          //
                     WTR_HUD->CharacterOverlayWidget &&                  //
                     WTR_HUD->CharacterOverlayWidget->DeathMessageText;  // DeathMessageText

    if (bHUDValid)
    {
        ESlateVisibility SlateVisibility = bVisible ? ESlateVisibility::Visible : ESlateVisibility::Hidden;
        WTR_HUD->CharacterOverlayWidget->DeathMessageText->SetVisibility(SlateVisibility);
    }
}

void AWTRPlayerController::SetHUDWeaponAmmo(int32 AmmoAmount)
{
    WTR_HUD = GetWTR_HUD();

    bool bHUDValid = WTR_HUD &&                                        //
                     WTR_HUD->CharacterOverlayWidget &&                //
                     WTR_HUD->CharacterOverlayWidget->WeaponAmmoText;  // WeaponAmmoText

    if (bHUDValid)
    {
        const FString AmmoText = FString::Printf(TEXT("%d"), AmmoAmount);
        WTR_HUD->CharacterOverlayWidget->WeaponAmmoText->SetText(FText::FromString(AmmoText));
    }
    else
    {
        DelayInit_WeaponAmmo = AmmoAmount;
    }
}

void AWTRPlayerController::SetHUDCarriedAmmo(int32 AmmoAmount)
{
    WTR_HUD = GetWTR_HUD();

    bool bHUDValid = WTR_HUD &&                                         //
                     WTR_HUD->CharacterOverlayWidget &&                 //
                     WTR_HUD->CharacterOverlayWidget->CarriedAmmoText;  // CarriedAmmoText

    if (bHUDValid)
    {
        const FString AmmoText = FString::Printf(TEXT("%d"), AmmoAmount);
        WTR_HUD->CharacterOverlayWidget->CarriedAmmoText->SetText(FText::FromString(AmmoText));
    }
    else
    {
        DelayInit_CarriedAmmo = AmmoAmount;
    }
}

void AWTRPlayerController::SetHUDWeaponType(EWeaponType Type)
{
    WTR_HUD = GetWTR_HUD();

    bool bHUDValid = WTR_HUD &&                                        //
                     WTR_HUD->CharacterOverlayWidget &&                //
                     WTR_HUD->CharacterOverlayWidget->WeaponTypeText;  // WeaponTypeText

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

            case EWeaponType::EWT_MAX:         //
                WeaponTypeText = FString("");  //
                break;
        }

        WTR_HUD->CharacterOverlayWidget->WeaponTypeText->SetText(FText::FromString(WeaponTypeText));
    }
    else
    {
        DelayInit_WeaponType = Type;
    }
}

void AWTRPlayerController::SetHUDMatchCountdownTime(float Time)
{
    WTR_HUD = GetWTR_HUD();

    bool bHUDValid = WTR_HUD &&                                            //
                     WTR_HUD->CharacterOverlayWidget &&                    //
                     WTR_HUD->CharacterOverlayWidget->Blinking &&          //
                     WTR_HUD->CharacterOverlayWidget->MatchCountdownText;  // MatchCountdownText

    if (bHUDValid)
    {
        if (Time < 0.f)
        {
            GEngine->AddOnScreenDebugMessage(14, 1.f, FColor::Red, "Time < 0.f");
            WTR_HUD->CharacterOverlayWidget->MatchCountdownText->SetText(FText::FromString("00:00"));
            return;
        }

        if (Time <= BlinkStartTime && Time >= 0.f)
        {
            WTR_HUD->CharacterOverlayWidget->PlayAnimation(WTR_HUD->CharacterOverlayWidget->Blinking);
        }

        FString TimeString = UKismetStringLibrary::TimeSecondsToString(Time);
        TimeString = UKismetStringLibrary::GetSubstring(TimeString, 0, 5);

        WTR_HUD->CharacterOverlayWidget->MatchCountdownText->SetText(FText::FromString(TimeString));
    }
}

void AWTRPlayerController::SetHUDWarmupTime(float Time)
{
    WTR_HUD = GetWTR_HUD();

    bool bHUDValid = WTR_HUD &&                                //
                     WTR_HUD->AnnouncementWidget &&            //
                     WTR_HUD->AnnouncementWidget->WarmupText;  // WarmupText

    if (bHUDValid)
    {
        if (Time < 0.f)
        {
            GEngine->AddOnScreenDebugMessage(14, 1.f, FColor::Red, "Time < 0.f");
            WTR_HUD->AnnouncementWidget->WarmupText->SetText(FText::FromString("00:00"));
            return;
        }

        FString TimeString = UKismetStringLibrary::TimeSecondsToString(Time);
        TimeString = UKismetStringLibrary::GetSubstring(TimeString, 0, 5);

        WTR_HUD->AnnouncementWidget->WarmupText->SetText(FText::FromString(TimeString));
    }
}

void AWTRPlayerController::SetHUD_FPS()
{
    WTR_HUD = GetWTR_HUD();

    bool bHUDValid = WTR_HUD &&                                  //
                     WTR_HUD->CharacterOverlayWidget &&          //
                     WTR_HUD->CharacterOverlayWidget->FPS_Text;  // FPS_Text

    if (bHUDValid)
    {
        const FString FPS_Text = FString::Printf(TEXT("%d"), FMath::FloorToInt(FPS));
        WTR_HUD->CharacterOverlayWidget->FPS_Text->SetText(FText::FromString(FPS_Text));
    }
}

void AWTRPlayerController::SetHUDGrenades(int32 Grenades)
{
    WTR_HUD = GetWTR_HUD();

    bool bHUDValid = WTR_HUD &&                                     //
                     WTR_HUD->CharacterOverlayWidget &&             //
                     WTR_HUD->CharacterOverlayWidget->GrenadeText;  // GrenadeText

    if (bHUDValid)
    {
        const FString GrenadeText = FString::Printf(TEXT("%d"), Grenades);
        WTR_HUD->CharacterOverlayWidget->GrenadeText->SetText(FText::FromString(GrenadeText));
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

void AWTRPlayerController::HandleMatchStateWaitingToStart()
{
    if (HasAuthority())
    {
        TimeOfMapCreation = GetWorld()->GetTimeSeconds();
    }
}

void AWTRPlayerController::HandleMatchStateInProgress()
{
    WTR_HUD = GetWTR_HUD();
    if (WTR_HUD)
    {
        if (WTR_HUD->AnnouncementWidget)
        {
            WTR_HUD->AnnouncementWidget->SetVisibility(ESlateVisibility::Hidden);
        }
        WTR_HUD->AddCharacterOverlay();
    }
}

void AWTRPlayerController::HandleMatchCooldown()
{
    WTR_HUD = GetWTR_HUD();
    if (WTR_HUD && WTR_HUD->CharacterOverlayWidget)
    {
        WTR_HUD->CharacterOverlayWidget->RemoveFromParent();

        bool bWTR_HUD =                                       //
            WTR_HUD->AnnouncementWidget &&                    //
            WTR_HUD->AnnouncementWidget->AnnouncementText &&  //
            WTR_HUD->AnnouncementWidget->InfoText &&          //
            WTR_HUD->AnnouncementWidget->TopPlayersText;

        if (bWTR_HUD)
        {
            AWTRGameState* WTRGameState = Cast<AWTRGameState>(UGameplayStatics::GetGameState(this));
            AWTRPlayerState* WTRPlayerState = GetPlayerState<AWTRPlayerState>();

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

            WTR_HUD->AnnouncementWidget->AnnouncementText->SetText(FText::FromString(AnnounCooldownText));
            WTR_HUD->AnnouncementWidget->InfoText->SetText(FText::FromString(AnnounInfoText));
            WTR_HUD->AnnouncementWidget->TopPlayersText->SetText(FText::FromString(TopPlayersText));

            WTR_HUD->AnnouncementWidget->SetVisibility(ESlateVisibility::Visible);
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
