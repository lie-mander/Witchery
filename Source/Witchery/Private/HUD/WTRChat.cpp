// Witchery. Copyright Liemander. All Rights Reserved.

#include "HUD/WTRChat.h"
#include "HUD/WTRChatMessage.h"
#include "Components/ScrollBox.h"

void UWTRChat::AddChatMessage(APlayerState* Sender, const FString& Message)
{
    if (!Sender) return;

    PlayerController = (PlayerController == nullptr) ? GetWorld()->GetFirstPlayerController() : PlayerController;
    if (PlayerController && ChatMessageClass)
    {
        UWTRChatMessage* NewMessage = CreateWidget<UWTRChatMessage>(PlayerController, ChatMessageClass);
        if (NewMessage)
        {
            NewMessage->SetMessageText(Sender, Message);
            if (ChatScrollBox)
            {
                ChatScrollBox->AddChild(NewMessage);
            }
        }
    }
}

void UWTRChat::ApplyChatMessage(APlayerState* Sender, const FString& Message)
{
    AddChatMessage(Sender, Message);

    SetVisibility(ESlateVisibility::Visible);
    ScrollDown();
}

void UWTRChat::OpenChat()
{
    SetVisibility(ESlateVisibility::Visible);
    SetIsFocusable(true);

    ScrollDown();
}

void UWTRChat::CloseChat()
{
    SetVisibility(ESlateVisibility::Hidden);
    SetIsFocusable(false);

    ScrollDown();
}

void UWTRChat::ScrollDown() 
{
    if (ChatScrollBox)
    {
        ChatScrollBox->ScrollToEnd();
    }
}

