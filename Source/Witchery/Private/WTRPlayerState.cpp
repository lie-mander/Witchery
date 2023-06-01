// Witchery. Copyright Liemander. All Rights Reserved.

#include "WTRPlayerState.h"
#include "WTRTools.h"
#include "Character/WTRCharacter.h"
#include "Character/WTRPlayerController.h"
#include "Net/UnrealNetwork.h"

void AWTRPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AWTRPlayerState, Defeats);
}

void AWTRPlayerState::AddToScore(float ScoreToAdd)
{
    SetScore(GetScore() + ScoreToAdd);

    UpdateHUDScore(GetScore());
}

void AWTRPlayerState::OnRep_Score()
{
    Super::OnRep_Score();

    UpdateHUDScore(GetScore());
}

void AWTRPlayerState::UpdateHUDScore(float ScoreAmount)
{
    WTRPlayerController = (WTRPlayerController == nullptr) ? UWTRTools::GetPlayerControllerByActor(GetPawn()) : WTRPlayerController;
    if (WTRPlayerController)
    {
        WTRPlayerController->SetHUDScore(ScoreAmount);
    }
}

void AWTRPlayerState::AddToDefeats(int32 DefeatsToAdd)
{
    Defeats += DefeatsToAdd;

    UpdateHUDDefeats(Defeats);
}

void AWTRPlayerState::OnRep_Defeats()
{
    UpdateHUDDefeats(Defeats);
}

void AWTRPlayerState::UpdateHUDDefeats(int32 DefeatsAmount)
{
    WTRPlayerController = (WTRPlayerController == nullptr) ? UWTRTools::GetPlayerControllerByActor(GetPawn()) : WTRPlayerController;
    if (WTRPlayerController)
    {
        WTRPlayerController->SetHUDDefeats(DefeatsAmount);
    }
}