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

    SetRenderOpacity(1.f);
    ScrollDown();
}

void UWTRChat::OpenChat()
{
    SetRenderOpacity(1.f);
    SetIsFocusable(true);

    ScrollDown();
}

void UWTRChat::CloseChat()
{
    SetRenderOpacity(0.f);
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

