// Witchery. Copyright Liemander. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WTRAnnouncementWidget.generated.h"

class UTextBlock;

UCLASS()
class WITCHERY_API UWTRAnnouncementWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UPROPERTY(meta = (BindWidget))
    UTextBlock* AnnouncementText;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* WarmupText;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* InfoText;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* TopPlayersText;
};
