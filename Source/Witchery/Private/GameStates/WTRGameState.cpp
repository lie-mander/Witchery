// Witchery. Copyright Liemander. All Rights Reserved.

#include "GameStates/WTRGameState.h"
#include "Net/UnrealNetwork.h"
#include "WTRPlayerState.h"
#include "Character/WTRPlayerController.h"

void AWTRGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const 
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AWTRGameState, TopPlayers);
    DOREPLIFETIME(AWTRGameState, RedTeamScore);
    DOREPLIFETIME(AWTRGameState, BlueTeamScore);
}

void AWTRGameState::UpdateTopPlayers(AWTRPlayerState* PlayerState) 
{
    if (!PlayerState) return;

    if (TopPlayers.IsEmpty())
    {
        TopPlayers.Add(PlayerState);
        TopScore = PlayerState->GetScore();
    }
    else if (PlayerState->GetScore() == TopScore)
    {
        TopPlayers.AddUnique(PlayerState);
    }
    else if (PlayerState->GetScore() > TopScore)
    {
        TopPlayers.Empty();
        TopPlayers.AddUnique(PlayerState);
        TopScore = PlayerState->GetScore();
    }
}

void AWTRGameState::RedTeamScores() 
{
    ++RedTeamScore;

    if (GetWorld() && GetWorld()->GetFirstPlayerController())
    {
        AWTRPlayerController* PlayerController = Cast<AWTRPlayerController>(GetWorld()->GetFirstPlayerController());
        if (PlayerController)
        {
            PlayerController->SetHUDRedScore(RedTeamScore);
        }
    }
}

void AWTRGameState::BlueTeamScores() 
{
    ++BlueTeamScore;

    if (GetWorld() && GetWorld()->GetFirstPlayerController())
    {
        AWTRPlayerController* PlayerController = Cast<AWTRPlayerController>(GetWorld()->GetFirstPlayerController());
        if (PlayerController)
        {
            PlayerController->SetHUDBlueScore(BlueTeamScore);
        }
    }
}

void AWTRGameState::OnRep_RedTeamScore() 
{
    if (GetWorld() && GetWorld()->GetFirstPlayerController())
    {
        AWTRPlayerController* PlayerController = Cast<AWTRPlayerController>(GetWorld()->GetFirstPlayerController());
        if (PlayerController)
        {
            PlayerController->SetHUDRedScore(RedTeamScore);
        }
    }
}

void AWTRGameState::OnRep_BlueTeamScore() 
{
    if (GetWorld() && GetWorld()->GetFirstPlayerController())
    {
        AWTRPlayerController* PlayerController = Cast<AWTRPlayerController>(GetWorld()->GetFirstPlayerController());
        if (PlayerController)
        {
            PlayerController->SetHUDBlueScore(BlueTeamScore);
        }
    }
}
