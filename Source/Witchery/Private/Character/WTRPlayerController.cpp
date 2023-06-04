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
#include "TimerManager.h"

void AWTRPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AWTRPlayerController, MatchState);
}

void AWTRPlayerController::BeginPlay()
{
    Super::BeginPlay();

    WTR_HUD = Cast<AWTR_HUD>(GetHUD());

    Server_CheckMatchState();
}

void AWTRPlayerController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    UpdateSyncTime(DeltaTime);
    DelayInit();

    SetHUDTime();
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
                SetHUDScore(DelayInit_ScoreAmount);
                SetHUDDefeats(DelayInit_DefeatsAmount);
            }
        }
    }
}

void AWTRPlayerController::Server_CheckMatchState_Implementation()
{
    AWTRGameMode* WTRGameMode = Cast<AWTRGameMode>(UGameplayStatics::GetGameMode(this));
    if (WTRGameMode)
    {
        WarmupTime = WTRGameMode->GetWarmupTime();
        MatchTime = WTRGameMode->GetMatchTime();
        CooldownTime = WTRGameMode->GetCooldownTime();
        TimeOfMapCreation = WTRGameMode->GetTimeOfMapCreation();
        MatchState = WTRGameMode->GetMatchState();

        Client_ApplyMatchState(WarmupTime, MatchTime, CooldownTime, TimeOfMapCreation, MatchState);
    }
}

void AWTRPlayerController::Client_ApplyMatchState_Implementation(
    float TimeofWarmup, float TimeOfMatch, float TimeOfCooldown, float MapCreationTime, const FName& State)
{
    WarmupTime = TimeofWarmup;
    MatchTime = TimeOfMatch;
    CooldownTime = TimeOfCooldown;
    TimeOfMapCreation = MapCreationTime;
    MatchState = State;

    if (IsLocalController() && WTR_HUD && MatchState == MatchState::WaitingToStart)
    {
        WTR_HUD->AddAnnouncement();
    }
}

void AWTRPlayerController::SetMatchState(const FName& State)
{
    MatchState = State;

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

            case EWeaponType::EWT_MAX:         //
                WeaponTypeText = FString("");  //
                break;
        }

        WTR_HUD->CharacterOverlayWidget->WeaponTypeText->SetText(FText::FromString(WeaponTypeText));
    }
}

void AWTRPlayerController::SetHUDMatchCountdownTime(float Time)
{
    WTR_HUD = GetWTR_HUD();

    bool bHUDValid = WTR_HUD &&                                            //
                     WTR_HUD->CharacterOverlayWidget &&                    //
                     WTR_HUD->CharacterOverlayWidget->MatchCountdownText;  // MatchCountdownText

    if (bHUDValid)
    {
        if (Time < 0.f)
        {
            WTR_HUD->AnnouncementWidget->WarmupText->SetText(FText::FromString("00:00"));
            return;
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
            WTR_HUD->AnnouncementWidget->WarmupText->SetText(FText::FromString("00:00"));
            return;
        }

        FString TimeString = UKismetStringLibrary::TimeSecondsToString(Time);
        TimeString = UKismetStringLibrary::GetSubstring(TimeString, 0, 5);

        WTR_HUD->AnnouncementWidget->WarmupText->SetText(FText::FromString(TimeString));
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
            WTR_HUD->AnnouncementWidget->InfoText;

        if (bWTR_HUD)
        {
            WTR_HUD->AnnouncementWidget->AnnouncementText->SetText(FText::FromString(AnnounCooldownText));
            WTR_HUD->AnnouncementWidget->InfoText->SetText(FText::FromString(AnnounInfoText));

            WTR_HUD->AnnouncementWidget->InfoText->SetVisibility(ESlateVisibility::Hidden);

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
