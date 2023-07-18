// Witchery. Copyright Liemander. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WTRReturnToMainMenu.generated.h"

class UButton;
class APlayerController;
class UMultiplayerSessionsSubsystem;

UCLASS()
class WITCHERY_API UWTRReturnToMainMenu : public UUserWidget
{
    GENERATED_BODY()

public:
    void MenuSetup();
    void MenuTearDown();

private:
    UPROPERTY(meta = (BindWidget))
    UButton* ReturnButton;

    UPROPERTY()
    APlayerController* PlayerController;

    UPROPERTY()
    UMultiplayerSessionsSubsystem* MultiplayerSessionsSubsystem;

    UFUNCTION()
    void OnReturnButtonClicked();

    UFUNCTION()
    void OnDestroySessionComplete(bool bWasSuccessful);

    UFUNCTION()
    void OnPlayerLeftGame();
};
