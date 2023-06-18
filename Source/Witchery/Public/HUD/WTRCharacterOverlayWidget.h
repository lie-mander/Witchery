// Witchery. Copyright Liemander. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WTRCharacterOverlayWidget.generated.h"

class UProgressBar;
class UTextBlock;
class UWidgetAnimation;

UCLASS()
class WITCHERY_API UWTRCharacterOverlayWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UPROPERTY(meta = (BindWidget))
    UProgressBar* HealthBar;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* HealthText;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* ScoreText;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* DefeatsText;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* DeathMessageText;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* WeaponAmmoText;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* CarriedAmmoText;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* WeaponTypeText;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* MatchCountdownText;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* GrenadeText;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* FPS_Text;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* FPS_String;

    UPROPERTY(Transient, meta = (BindWidgetAnim))
    UWidgetAnimation* Blinking;
};
