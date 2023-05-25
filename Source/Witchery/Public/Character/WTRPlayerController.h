// Witchery. Copyright Liemander. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "WTRPlayerController.generated.h"

class AWTR_HUD;

UCLASS()
class WITCHERY_API AWTRPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    void SetHUDHealth(float CurrentHealth, float MaxHealth);

protected:
    virtual void BeginPlay() override;

private:
    AWTR_HUD* WTR_HUD;
};
