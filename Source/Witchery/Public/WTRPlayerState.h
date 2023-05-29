// Witchery. Copyright Liemander. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "WTRPlayerState.generated.h"

class AWTRCharacter;
class AWTRPlayerController;

UCLASS()
class WITCHERY_API AWTRPlayerState : public APlayerState
{
    GENERATED_BODY()

public:
    virtual void OnRep_Score() override;
    void AddToScore(float ScoreToAdd);

private:
    AWTRCharacter* WTRCharacter;
    AWTRPlayerController* WTRPlayerController;

    void UpdateHUDScore(float ScoreAmount);
};
