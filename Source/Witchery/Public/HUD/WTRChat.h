// Witchery. Copyright Liemander. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WTRChat.generated.h"

class APlayerState;
class UScrollBox;

UCLASS()
class WITCHERY_API UWTRChat : public UUserWidget
{
    GENERATED_BODY()

public:
    void AddChatMessage(APlayerState* Sender, const FString& Message);
    void ApplyChatMessage(APlayerState* Sender, const FString& Message);
    void OpenChat();
    void CloseChat();
    void ScrollDown();

    UPROPERTY(EditDefaultsOnly, Category = "WTR | Chat")
    TSubclassOf<UUserWidget> ChatMessageClass;

    UPROPERTY(meta = (BindWidget))
    UScrollBox* ChatScrollBox;

    UPROPERTY()
    APlayerController* PlayerController;
};
