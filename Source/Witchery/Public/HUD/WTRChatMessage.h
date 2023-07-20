// Witchery. Copyright Liemander. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WTRChatMessage.generated.h"

class UHorizontalBox;
class UMultiLineEditableTextBox;

UCLASS()
class WITCHERY_API UWTRChatMessage : public UUserWidget
{
    GENERATED_BODY()

public:
    void SetMessageText(APlayerState* Sender, const FString& Message);

    UPROPERTY(meta = (BindWidget))
    UHorizontalBox* ChatMessageBox;

    UPROPERTY(meta = (BindWidget))
    UMultiLineEditableTextBox* ChatMessage;
};
