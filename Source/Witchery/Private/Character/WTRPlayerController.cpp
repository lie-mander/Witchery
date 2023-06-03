// Witchery. Copyright Liemander. All Rights Reserved.

#include "Character/WTRPlayerController.h"
#include "Character/WTRCharacter.h"
#include "HUD/WTR_HUD.h"
#include "HUD/WTRCharacterOverlayWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Kismet/KismetStringLibrary.h"
#include "Net/UnrealNetwork.h"
#include "GameModes/WTRGameMode.h"

void AWTRPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const 
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AWTRPlayerController, MatchState);
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

    WorldTime += DeltaTime;
    SetHUDMatchCountdownTime(WorldTime);
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

void AWTRPlayerController::SetMatchState(const FName& State) 
{
    MatchState = State;

    AddCharacterOverlay();
}

void AWTRPlayerController::OnRep_MatchState() 
{
    AddCharacterOverlay();
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
        FString TimeString = UKismetStringLibrary::TimeSecondsToString(Time + ClientServerTimeDelta);
        TimeString = UKismetStringLibrary::GetSubstring(TimeString, 0, 5);

        WTR_HUD->CharacterOverlayWidget->MatchCountdownText->SetText(FText::FromString(TimeString));
    }
}

AWTR_HUD* AWTRPlayerController::GetWTR_HUD() 
{
    return (WTR_HUD == nullptr) ? Cast<AWTR_HUD>(GetHUD()) : WTR_HUD;
}

void AWTRPlayerController::AddCharacterOverlay() 
{
    WTR_HUD = GetWTR_HUD();
    if (WTR_HUD && MatchState == MatchState::InProgress)
    {
        WTR_HUD->AddCharacterOverlay();
    }
}
