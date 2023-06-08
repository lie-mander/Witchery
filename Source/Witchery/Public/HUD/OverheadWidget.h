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

    UFUNCTION(BlueprintCallable)
    void ShowPlayerName(APawn* InPawn);

    void SetDisplayText(FString Text);

protected:
    virtual void NativeDestruct() override;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* OverheadText;
};
