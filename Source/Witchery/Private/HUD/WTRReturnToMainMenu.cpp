// Witchery. Copyright Liemander. All Rights Reserved.

#include "HUD/WTRReturnToMainMenu.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/GameModeBase.h"
#include "Components/Button.h"
#include "MultiplayerSessionsSubsystem.h"

void UWTRReturnToMainMenu::MenuSetup()
{
    if (!GetWorld()) return;

    AddToViewport();
    SetVisibility(ESlateVisibility::Visible);
    SetIsFocusable(true);

    PlayerController = (PlayerController == nullptr) ? GetWorld()->GetFirstPlayerController() : PlayerController;
    if (PlayerController)
    {
        FInputModeGameAndUI InputMode;
        InputMode.SetWidgetToFocus(TakeWidget());
        PlayerController->SetInputMode(InputMode);
        PlayerController->SetShowMouseCursor(true);
    }

    UGameInstance* GameInstance = GetGameInstance();
    if (GameInstance)
    {
        MultiplayerSessionsSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
        if (MultiplayerSessionsSubsystem && !MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.IsBound())
        {
            MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.AddDynamic(
                this, &UWTRReturnToMainMenu::OnDestroySessionComplete);
        }
    }

    if (ReturnButton && !ReturnButton->OnClicked.IsBound())
    {
        ReturnButton->OnClicked.AddDynamic(this, &UWTRReturnToMainMenu::OnReturnButtonClicked);
    }
}

void UWTRReturnToMainMenu::MenuTearDown()
{
    RemoveFromParent();

    PlayerController = (PlayerController == nullptr) ? GetWorld()->GetFirstPlayerController() : PlayerController;
    if (PlayerController)
    {
        FInputModeGameOnly InputMode;
        PlayerController->SetInputMode(InputMode);
        PlayerController->SetShowMouseCursor(false);
    }

    if (MultiplayerSessionsSubsystem && MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.IsBound())
    {
        MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.RemoveDynamic(this, &UWTRReturnToMainMenu::OnDestroySessionComplete);
    }

    if (ReturnButton && ReturnButton->OnClicked.IsBound())
    {
        ReturnButton->OnClicked.RemoveDynamic(this, &UWTRReturnToMainMenu::OnReturnButtonClicked);
    }
}

void UWTRReturnToMainMenu::OnReturnButtonClicked()
{
    if (ReturnButton)
    {
        ReturnButton->SetIsEnabled(false);
    }

    if (MultiplayerSessionsSubsystem)
    {
        MultiplayerSessionsSubsystem->DestroySession();
    }
}

void UWTRReturnToMainMenu::OnDestroySessionComplete(bool bWasSuccessful)
{
    if (!bWasSuccessful && ReturnButton)
    {
        ReturnButton->SetIsEnabled(true);
        return;
    }

    if (GetWorld())
    {
        AGameModeBase* GameMode = GetWorld()->GetAuthGameMode<AGameModeBase>();
        if (GameMode)
        {
            GetWorld()->GetAuthGameMode<AGameModeBase>()->ReturnToMainMenuHost();
        }
        else
        {
            PlayerController = (PlayerController == nullptr) ? GetWorld()->GetFirstPlayerController() : PlayerController;
            if (PlayerController)
            {
                PlayerController->ClientReturnToMainMenuWithTextReason(FText());
            }
        }
    }
}
