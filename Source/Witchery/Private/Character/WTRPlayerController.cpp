// Witchery. Copyright Liemander. All Rights Reserved.

#include "Character/WTRPlayerController.h"
#include "HUD/WTR_HUD.h"
#include "HUD/WTRCharacterOverlayWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void AWTRPlayerController::BeginPlay()
{
    Super::BeginPlay();

    WTR_HUD = Cast<AWTR_HUD>(GetHUD());
}

void AWTRPlayerController::SetHUDHealth(float CurrentHealth, float MaxHealth)
{
    WTR_HUD = (WTR_HUD == nullptr) ? Cast<AWTR_HUD>(GetHUD()) : WTR_HUD;

    bool bHUDValid = WTR_HUD &&                                     //
                     WTR_HUD->CharacterOverlayWidget &&             //
                     WTR_HUD->CharacterOverlayWidget->HealthBar &&  //
                     WTR_HUD->CharacterOverlayWidget->HealthText;

    if (bHUDValid)
    {
        const float HealthPercent = CurrentHealth / MaxHealth;
        WTR_HUD->CharacterOverlayWidget->HealthBar->SetPercent(HealthPercent);

        const FString HealthText = FString::Printf(TEXT("%d / %d"), FMath::CeilToInt(CurrentHealth), FMath::CeilToInt(MaxHealth));
        WTR_HUD->CharacterOverlayWidget->HealthText->SetText(FText::FromString(HealthText));
    }
}
