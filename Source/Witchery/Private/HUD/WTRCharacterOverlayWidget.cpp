// Witchery. Copyright Liemander. All Rights Reserved.

#include "HUD/WTRCharacterOverlayWidget.h"
#include "HUD/WTRChat.h"
#include "Components/EditableTextBox.h"
#include "GameFramework/PlayerState.h"
#include "Character/WTRPlayerController.h"
#include "TimerManager.h"

void UWTRCharacterOverlayWidget::AddChatMessage(APlayerState* Sender, const FString& Message)
{
    if (!Chat || !Sender) return;

    Chat->AddChatMessage(Sender, Message);
}

void UWTRCharacterOverlayWidget::ApplyChatMessage(APlayerState* Sender, const FString& Message) 
{
    if (!Chat || !Sender) return;

    Chat->ApplyChatMessage(Sender, Message);

    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(ChatCloseHandle, this, &UWTRCharacterOverlayWidget::ChatCloseTimerFinished, 3.f, false);
    }
}

void UWTRCharacterOverlayWidget::OpenChat()
{
    PlayerController =
        (PlayerController == nullptr) ? Cast<AWTRPlayerController>(GetWorld()->GetFirstPlayerController()) : PlayerController;
    if (PlayerController && ChatWriteBox)
    {
        FInputModeGameAndUI InputMode;
        InputMode.SetWidgetToFocus(ChatWriteBox->TakeWidget());
        PlayerController->SetInputMode(InputMode);
        PlayerController->SetShowMouseCursor(false);

        if (ChatWriteBox)
        {
            ChatWriteBox->SetVisibility(ESlateVisibility::Visible);
        }
    }

    if (Chat)
    {
        Chat->OpenChat();
    }

    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(ChatCloseHandle);
    }
}

void UWTRCharacterOverlayWidget::CloseChat()
{
    PlayerController =
        (PlayerController == nullptr) ? Cast<AWTRPlayerController>(GetWorld()->GetFirstPlayerController()) : PlayerController;
    if (PlayerController)
    {
        const FInputModeGameOnly InputMode;
        PlayerController->SetInputMode(InputMode);
        PlayerController->SetShowMouseCursor(false);

        if (ChatWriteBox)
        {
            ChatWriteBox->SetVisibility(ESlateVisibility::Hidden);
        }
    }

    if (Chat)
    {
        Chat->CloseChat();
    }
}

bool UWTRCharacterOverlayWidget::Initialize()
{
    if (!Super::Initialize()) return false;

    if (ChatWriteBox)
    {
        ChatWriteBox->OnTextCommitted.AddDynamic(this, &UWTRCharacterOverlayWidget::OnTextCommit);
    }

    return true;
}

void UWTRCharacterOverlayWidget::OnTextCommit(const FText& Text, ETextCommit::Type CommitMethod)
{
    PlayerController =
        (PlayerController == nullptr) ? Cast<AWTRPlayerController>(GetWorld()->GetFirstPlayerController()) : PlayerController;

    if (PlayerController && CommitMethod == ETextCommit::OnEnter && !Text.IsEmpty())
    {
        APlayerState* State = PlayerController->GetPlayerState<APlayerState>();
        PlayerController->Server_SendChatMessage(State, Text.ToString());
    }

    if (PlayerController)
    {
        const FInputModeGameOnly InputMode;
        PlayerController->SetInputMode(InputMode);
        PlayerController->SetShowMouseCursor(false);
        PlayerController->bChatOpen = false;
    }

    if (ChatWriteBox)
    {
        ChatWriteBox->SetText(FText());
        ChatWriteBox->SetVisibility(ESlateVisibility::Hidden);
    }

    if (Chat)
    {
        Chat->ScrollDown();
    }

    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(ChatCloseHandle, this, &UWTRCharacterOverlayWidget::ChatCloseTimerFinished, 3.f, false);
    }
}

void UWTRCharacterOverlayWidget::ChatCloseTimerFinished() 
{
    if (Chat)
    {
        Chat->CloseChat();
    }
}