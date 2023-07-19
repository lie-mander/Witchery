// Witchery. Copyright Liemander. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WTRElimAnnouncementWidget.generated.h"

class UHorizontalBox;
class UTextBlock;

UCLASS()
class WITCHERY_API UWTRElimAnnouncementWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    void SetElimAnnouncementText(const FString& AttackerName, const FString& VictimName);

    UPROPERTY(meta = (BindWidget))
    UHorizontalBox* ElimHorizontalBox;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* ElimText;
};
