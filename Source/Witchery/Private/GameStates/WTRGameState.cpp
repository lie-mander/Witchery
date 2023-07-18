// Witchery. Copyright Liemander. All Rights Reserved.

#include "GameStates/WTRGameState.h"
#include "Net/UnrealNetwork.h"
#include "WTRPlayerState.h"

void AWTRGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const 
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AWTRGameState, TopPlayers);
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
