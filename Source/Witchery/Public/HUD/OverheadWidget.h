// Witchery. Copyright Liemander. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OverheadWidget.generated.h"

UCLASS()
class WITCHERY_API UOverheadWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable)
    void ShowPlayerNetRole(APawn* InPawn);

    void SetDisplayText(FString Text);

protected:
    virtual void OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld);

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* OverheadText;
};
