// Witchery. Copyright Liemander. All Rights Reserved.

#include "WTRPlayerState.h"
#include "Character/WTRCharacter.h"
#include "Character/WTRPlayerController.h"

void AWTRPlayerState::AddToScore(float ScoreToAdd)
{
    Score += ScoreToAdd;

    UpdateHUDScore(Score);
}

void AWTRPlayerState::OnRep_Score() 
{
    Super::OnRep_Score();

    UpdateHUDScore(Score);
}

void AWTRPlayerState::UpdateHUDScore(float ScoreAmount)
{
    WTRCharacter = (WTRCharacter == nullptr) ? Cast<AWTRCharacter>(GetPawn()) : WTRCharacter;
    if (WTRCharacter)
    {
        WTRPlayerController = (WTRPlayerController == nullptr) ? Cast<AWTRPlayerController>(WTRCharacter->Controller) : WTRPlayerController;
        if (WTRPlayerController)
        {
            WTRPlayerController->SetHUDScore(ScoreAmount);
        }
    }
}
