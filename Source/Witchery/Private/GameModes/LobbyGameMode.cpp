// Witchery. Copyright Liemander. All Rights Reserved.

#include "GameModes/LobbyGameMode.h"
#include "GameFramework/GameStateBase.h"

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer) 
{
    Super::PostLogin(NewPlayer);

    int32 NumOfPlayers = GameState.Get()->PlayerArray.Num();
    if (NumOfPlayers == PlayersToStart)
    {
        const auto World = GetWorld();
        if (World)
        {
            bUseSeamlessTravel = true;
            World->ServerTravel("/Game/Maps/BattleMap_0?listen"); // TODO break magic string
        }
    }
}

