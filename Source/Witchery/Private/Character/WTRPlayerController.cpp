// Witchery. Copyright Liemander. All Rights Reserved.

#include "Character/WTRPlayerController.h"
#include "Character/WTRCharacter.h"
#include "HUD/WTR_HUD.h"
#include "HUD/WTRCharacterOverlayWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void AWTRPlayerController::BeginPlay()
{
    Super::BeginPlay();

    WTR_HUD = Cast<AWTR_HUD>(GetHUD());
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
        Client_HideDeathMessage();
    }

    AWTRCharacter* WTRCharacter = Cast<AWTRCharacter>(InPawn);
    if (WTRCharacter)
    {
        SetHUDHealth(WTRCharacter->GetHealth(), WTRCharacter->GetMaxHealth());
    }
}

void AWTRPlayerController::Client_HideDeathMessage_Implementation()
{
    SetHUDDeathMessage(false);
}

void AWTRPlayerController::SetHUDHealth(float CurrentHealth, float MaxHealth)
{
    WTR_HUD = (WTR_HUD == nullptr) ? Cast<AWTR_HUD>(GetHUD()) : WTR_HUD;

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
}

void AWTRPlayerController::SetHUDScore(float ScoreAmount)
{
    WTR_HUD = (WTR_HUD == nullptr) ? Cast<AWTR_HUD>(GetHUD()) : WTR_HUD;

    bool bHUDValid = WTR_HUD &&                                   //
                     WTR_HUD->CharacterOverlayWidget &&           //
                     WTR_HUD->CharacterOverlayWidget->ScoreText;  // ScoreText

    if (bHUDValid)
    {
        const FString ScoreText = FString::Printf(TEXT("%d"), FMath::FloorToInt(ScoreAmount));
        WTR_HUD->CharacterOverlayWidget->ScoreText->SetText(FText::FromString(ScoreText));
    }
}

void AWTRPlayerController::SetHUDDefeats(int32 DefeatsAmount)
{
    WTR_HUD = (WTR_HUD == nullptr) ? Cast<AWTR_HUD>(GetHUD()) : WTR_HUD;

    bool bHUDValid = WTR_HUD &&                                     //
                     WTR_HUD->CharacterOverlayWidget &&             //
                     WTR_HUD->CharacterOverlayWidget->DefeatsText;  // DefeatsText

    if (bHUDValid)
    {
        const FString DefeatsText = FString::Printf(TEXT("%d"), DefeatsAmount);
        WTR_HUD->CharacterOverlayWidget->DefeatsText->SetText(FText::FromString(DefeatsText));
    }
}

void AWTRPlayerController::SetHUDDeathMessage(bool bVisible)
{
    WTR_HUD = (WTR_HUD == nullptr) ? Cast<AWTR_HUD>(GetHUD()) : WTR_HUD;

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
    WTR_HUD = (WTR_HUD == nullptr) ? Cast<AWTR_HUD>(GetHUD()) : WTR_HUD;

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
    WTR_HUD = (WTR_HUD == nullptr) ? Cast<AWTR_HUD>(GetHUD()) : WTR_HUD;

    bool bHUDValid = WTR_HUD &&                                        //
                     WTR_HUD->CharacterOverlayWidget &&                //
                     WTR_HUD->CharacterOverlayWidget->CarriedAmmoText;  // CarriedAmmoText

    if (bHUDValid)
    {
        const FString AmmoText = FString::Printf(TEXT("%d"), AmmoAmount);
        WTR_HUD->CharacterOverlayWidget->CarriedAmmoText->SetText(FText::FromString(AmmoText));
    }
}
