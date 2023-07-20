// Witchery. Copyright Liemander. All Rights Reserved.

#include "HUD/WTRChatMessage.h"
#include "Components/MultiLineEditableTextBox.h"
#include "GameFramework/PlayerState.h"

void UWTRChatMessage::SetMessageText(APlayerState* Sender, const FString& Message)
{
    if (!ChatMessage || !Sender) return;

    const FString NewMessage = FString::Printf(TEXT("%s: %s"), *Sender->GetPlayerName(), *Message);
    ChatMessage->SetText(FText::FromString(NewMessage));
}
