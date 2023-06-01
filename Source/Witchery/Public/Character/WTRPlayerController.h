// Witchery. Copyright Liemander. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "WTRPlayerController.generated.h"

class AWTR_HUD;
class AWTRCharacter;

UCLASS()
class WITCHERY_API AWTRPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    virtual void OnPossess(APawn* InPawn) override;

    void SetHUDHealth(float CurrentHealth, float MaxHealth);
    void SetHUDScore(float ScoreAmount);
    void SetHUDDefeats(int32 DefeatsAmount);
    void SetHUDDeathMessage(bool bVisible);
    void SetHUDWeaponAmmo(int32 AmmoAmount);
    void SetHUDCarriedAmmo(int32 AmmoAmount);

protected:
    virtual void BeginPlay() override;

private:
    AWTR_HUD* WTR_HUD;

    UFUNCTION(Client, Reliable)
    void Client_OnPossess();
};
