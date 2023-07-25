// Witchery. Copyright Liemander. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WTRCharacterOverlayWidget.generated.h"

class UProgressBar;
class UTextBlock;
class UWidgetAnimation;
class UImage;
class UWTRChat;
class APlayerState;
class UEditableTextBox;
class AWTRPlayerController;

UCLASS()
class WITCHERY_API UWTRCharacterOverlayWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    void AddChatMessage(APlayerState* Sender, const FString& Message);
    void ApplyChatMessage(APlayerState* Sender, const FString& Message);
    void OpenChat();
    void CloseChat();

    UPROPERTY(meta = (BindWidget))
    UWTRChat* Chat;

    UPROPERTY(meta = (BindWidget))
    UEditableTextBox* ChatWriteBox;

    UPROPERTY(Transient, meta = (BindWidgetAnim))
    UWidgetAnimation* ChatHide;

    UPROPERTY(meta = (BindWidget))
    UProgressBar* HealthBar;

    UPROPERTY(meta = (BindWidget))
    UProgressBar* ShieldBar;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* HealthText;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* ShieldText;

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

    UPROPERTY(meta = (BindWidget))
    UImage* PingImage;

    UPROPERTY(Transient, meta = (BindWidgetAnim))
    UWidgetAnimation* Ping;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* RedTeamScore;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* BlueTeamScore;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* SpacerTeamScore;

    /*
    * Base variables
    */
    UPROPERTY()
    AWTRPlayerController* PlayerController;

protected:
    virtual bool Initialize() override;

private:
    FTimerHandle ChatCloseHandle;

    void ChatCloseTimerFinished();

    UFUNCTION()
    void OnTextCommit(const FText& Text, ETextCommit::Type CommitMethod);
};
