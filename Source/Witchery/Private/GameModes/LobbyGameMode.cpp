// Witchery. Copyright Liemander. All Rights Reserved.

#include "GameModes/LobbyGameMode.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "WTRGameInstance.h"

DEFINE_LOG_CATEGORY_STATIC(WTRLobbyGameModeLog, All, All);

void ALobbyGameMode::StartPlay()
{
    Super::StartPlay();

    const auto WTRGameInstance = Cast<UWTRGameInstance>(GetGameInstance());
    if (WTRGameInstance)
    {
        PlayersToStart = WTRGameInstance->LobbyPlayersToStart;
        UE_LOG(WTRLobbyGameModeLog, Display, TEXT("Players to start: %i"), PlayersToStart);
    }
    else
    {
        UE_LOG(WTRLobbyGameModeLog, Error, TEXT("Incorrect Game Instance"));
    }
}

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);

    const int32 NumOfPlayers = GameState.Get()->PlayerArray.Num();
    if (NumOfPlayers == PlayersToStart)
    {
        if (GetWorld())
        {
            bUseSeamlessTravel = true;
            GetWorld()->ServerTravel("/Game/Maps/BattleMap_0?listen");  // TODO break magic string
        }
    }
}
